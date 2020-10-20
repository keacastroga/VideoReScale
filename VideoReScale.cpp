#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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
    while (1)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;
        unsigned char *pixels = frame.data;
        int rows = frame.rows;
        int cols = frame.cols;
        int channels = frame.step[1];
        int rowLen = channels * cols;

        int newRows = rows*factor;
        int newCols = cols*factor;
        int newRowLen = channels * newCols;
        unsigned char *newPixels = (uchar *)malloc(sizeof(uchar) * newCols * channels * newRows);
        unsigned char pixel1, pixel2, pixel3, pixel4;
        int origRow, origCol;
        for (int k = 0; k < channels; k++)
        {
            for (int scaledRow = 0; scaledRow < newRows; scaledRow++)
            {
                origRow = scaledRow*2;
                for (int scaledCol = 0; scaledCol < newCols; scaledCol++)
                {
                    origCol = scaledCol*2;
                    pixel1 = *(pixels + origCol * channels + origRow * rowLen + k);
                    pixel2 = *(pixels + origCol * channels + (origRow+1) * rowLen + k);
                    pixel3 = *(pixels + (origCol+1) * channels + origRow * rowLen + k);
                    pixel4 = *(pixels + (origCol+1) * channels + (origRow+1) * rowLen + k);
                    *(newPixels + scaledCol * channels + scaledRow * newRowLen + k) = (pixel1 + pixel2 + pixel3 + pixel4) / 4;
                }
            }
        }

        Mat outFrame(newRows, newCols, CV_8UC3, newPixels);
        out << outFrame;
        frame.release();
        outFrame.release();
        free(newPixels);
        
    }
    out.release();
    cap.release();
    return 0;
}