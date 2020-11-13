#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

using namespace std;
using namespace cv;

int rows, cols, channels, rowLen, newRows, newCols, newRowLen, origRow;

unsigned char *pixel1, *pixel2, *pixel3, *pixel4, *newPixel;

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

    rows = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    cols = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    channels = 3;
    rowLen = channels * cols;
    newRows = rows * factor;
    newCols = cols * factor;
    newRowLen = channels * newCols;

    Size S = Size(newCols, newRows);

    unsigned char *newPixels = (uchar *)malloc(sizeof(uchar) * newCols * channels * newRows);

    out.open(argv[3], fourcc, fps, S);
    if (!out.isOpened())
    {
        cout << "Could not open the output video for write: " << endl;
        return -1;
    }

    Mat frame;
    Mat outFrame(newRows, newCols, CV_8UC3, newPixels);
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    while (1)
    {
        cap >> frame;
        if (frame.empty())
            break;
        sequentialScale(frame.data, newPixels);
        out << outFrame;
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    free(newPixels);
    frame.release();
    outFrame.release();
    out.release();
    cap.release();
    return 0;
}

void sequentialScale(uchar *pixels, uchar *newPixels)
{
    for (int scaledRow = 0; scaledRow < newRows; scaledRow++)
    {
        origRow = scaledRow * 2 * rowLen;
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