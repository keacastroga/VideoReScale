#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <mpi.h>

using namespace std;
using namespace cv;

int rows, cols, channels, rowLen, newRows, newCols, newRowLen, origRow;

void sequentialScale(uchar *pixels, uchar *newPixels, int ID, int numprocs);

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int j, tag = 1;
    MPI_Status status;
    if (argc != 4)
    {
        printf("usage: VideoReScale.out <Video_Path> <scale factor> <Output_path>\n");
        return -1;
    }

    VideoCapture cap;
    double fps;
    int frames;
    if (rank == 0)
    {
        cap = VideoCapture(argv[1]);
        if (!cap.isOpened())
        {
            cout << "Error opening video stream or file" << endl;
            return -1;
        }
        fps = cap.get(CAP_PROP_FPS);
        frames = cap.get(CAP_PROP_FRAME_COUNT);
        rows = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
        cols = (int)cap.get(CAP_PROP_FRAME_WIDTH);
        for (j = 1; j < size; j++)
        {
            MPI_Send(&frames, 1, MPI_INTEGER, j, tag, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INTEGER, j, tag, MPI_COMM_WORLD);
            MPI_Send(&cols, 1, MPI_INTEGER, j, tag, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&frames, 1, MPI_INTEGER, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INTEGER, 0, tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&cols, 1, MPI_INTEGER, 0, tag, MPI_COMM_WORLD, &status);
    }

    Mat frame;
    Mat outFrame;
    double factor = stod(argv[2]);

    int fourcc = VideoWriter::fourcc('H', '2', '6', '4');
    channels = 3;
    rowLen = channels * cols;
    newRows = rows * factor;
    newCols = cols * factor;
    newRowLen = channels * newCols;

    Size S = Size(newCols, newRows);

    size_t sizeNew = sizeof(uchar) * newCols * channels * newRows;
    size_t sizeOrig = sizeof(uchar) * cols * channels * rows;
    unsigned char *newPixels = (uchar *)malloc(sizeNew);
    unsigned char *newPixelsPart = (uchar *)malloc(sizeNew);
    unsigned char *pixels = (uchar *)malloc(sizeOrig);

    VideoWriter out;
    if (rank == 0)
    {
        out.open(argv[3], fourcc, fps, S);
        if (!out.isOpened())
        {
            cout << "Could not open the output video for write: " << endl;
            return -1;
        }
        outFrame = Mat(newRows, newCols, CV_8UC3, newPixels);
    }
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    int i = 0;
    int sectionSize = rows/size*channels*cols;
    while (i < frames)
    {
        if (rank == 0)
        {
            cap >> frame;
            if (frame.empty())
            {
                break;
            }
            pixels = frame.data;
            for (j = 1; j < size; j++)
                MPI_Send(pixels+(j*sectionSize), (sectionSize), MPI_UNSIGNED_CHAR, j, tag, MPI_COMM_WORLD);
            cout << "frame: " << i << "/" << frames << "\r";
        }
        else
        {
            MPI_Recv(pixels+(rank*sectionSize), (sectionSize), MPI_UNSIGNED_CHAR, 0, tag, MPI_COMM_WORLD, &status);
        }
        sequentialScale(pixels, newPixelsPart, rank, size);
        MPI_Reduce(newPixelsPart, newPixels, sizeNew, MPI_UNSIGNED_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0)
            out << outFrame;
        i++;
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    if (rank == 0)
    {
        printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
        frame.release();
        outFrame.release();
        out.release();
    }
    else
    {
        free(pixels);
    }
    cap.release();
    MPI_Finalize();
    return 0;
}

void sequentialScale(uchar *pixels, uchar *newPixels, int ID, int numprocs)
{
    int start, end;
    start = (newRows / numprocs) * ID;
    end = start + ((newRows / numprocs));
    for (int scaledRow = start; scaledRow < end; scaledRow++)
    {
        origRow = scaledRow * 2 * rowLen;
        uchar *pixel1, *pixel2, *pixel3, *pixel4, *newPixel;
        for (int scaledCol = 0; scaledCol < newCols; scaledCol++)
        {
            pixel1 = pixels + scaledCol * 2 * channels + origRow;
            pixel2 = pixel1 + channels;
            pixel3 = pixel1 + rowLen;
            pixel4 = pixel3 + channels;
            newPixel = newPixels + scaledCol * channels + scaledRow * newRowLen;
            *(newPixel) = (*pixel1 + *pixel2 + *pixel3 + *pixel4) / 4;
            *(newPixel + 1) = (*(pixel1 + 1) + *(pixel2 + 1) + *(pixel3 + 1) + *(pixel4 + 1)) / 4;
            *(newPixel + 2) = (*(pixel1 + 2) + *(pixel2 + 2) + *(pixel3 + 2) + *(pixel4 + 2)) / 4;
        }
    }
}