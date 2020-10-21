#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

using namespace std;
using namespace cv;

#define THREADS 8

int rows, cols, channels, rowLen, newRows, newCols, newRowLen, origRow, origCol;

void sequentialScale(uchar *pixels, uchar *newPixels);
void openMPScale(uchar *pixels, uchar *newPixels);

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("usage: VideoReScale.out <Video_Path> <scale factor> <Output_path>\n");
        return -1;
    }

    VideoCapture cap(argv[1]);
    if (!cap.isOpened())
    {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    VideoWriter out;
    double factor = stod(argv[2]);
    double fps = cap.get(CAP_PROP_FPS);
    int fourcc = static_cast<int>(cap.get(CAP_PROP_FOURCC));
    Size S = Size(((int)cap.get(CAP_PROP_FRAME_WIDTH) * factor),
                  ((int)cap.get(CAP_PROP_FRAME_HEIGHT) * factor));

    out.open(argv[3], fourcc, fps, S);
    if (!out.isOpened())
    {
        cout << "Could not open the output video for write: " << endl;
        return -1;
    }

    Size s0 = Size();

    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    while (1)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;
        unsigned char *pixels = frame.data;
        rows = frame.rows;
        cols = frame.cols;
        channels = frame.step[1];
        rowLen = channels * cols;
        newRows = rows * factor;
        newCols = cols * factor;
        newRowLen = channels * newCols;
        unsigned char *newPixels = (uchar *)malloc(sizeof(uchar) * newCols * channels * newRows);
        //openMPScale(pixels, newPixels);
        sequentialScale(pixels, newPixels);
        Mat outFrame(newRows, newCols, CV_8UC3, newPixels);
        out << outFrame;
        frame.release();
        outFrame.release();
        free(newPixels);
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    out.release();
    cap.release();
    return 0;
}

void sequentialScale(uchar *pixels, uchar *newPixels)
{
    unsigned char *pixel1, *pixel2, *pixel3, *pixel4;
    for (int k = 0; k < channels; k++)
    {
        for (int scaledRow = 0; scaledRow < newRows; scaledRow++)
        {
            origRow = scaledRow * 2;
            for (int scaledCol = 0; scaledCol < newCols; scaledCol++)
            {
                origCol = scaledCol * 2;
                uchar *pixel1 = pixels + origCol * channels + origRow * rowLen + k;
                uchar *pixel2 = pixel1 + channels + k;
                uchar *pixel3 = pixels + (origCol + 1) * channels + origRow * rowLen + k;
                uchar *pixel4 = pixel3 + channels + k;
                *(newPixels + scaledCol * channels + scaledRow * newRowLen + k) = (*pixel1 + *pixel2 + *pixel3 + *pixel4) / 4;
            }
        }
    }
}

void openMPScale(uchar *pixels, uchar *newPixels)
{
    for (int k = 0; k < channels; k++)
    {
#pragma omp parallel num_threads(THREADS) private(origRow, origCol)
        {
#pragma omp for
            for (int scaledRow = 0; scaledRow < newRows; scaledRow++)
            {
                origRow = scaledRow * 2;
                for (int scaledCol = 0; scaledCol < newCols; scaledCol++)
                {
                    origCol = scaledCol * 2;
                    uchar *pixel1 = pixels + origCol * channels + origRow * rowLen + k;
                    uchar *pixel2 = pixel1 + channels + k;
                    uchar *pixel3 = pixels + (origCol + 1) * channels + origRow * rowLen + k;
                    uchar *pixel4 = pixel3 + channels + k;
                    *(newPixels + scaledCol * channels + scaledRow * newRowLen + k) = (*pixel1 + *pixel2 + *pixel3 + *pixel4) / 4;
                }
            }
        }
    }
}
