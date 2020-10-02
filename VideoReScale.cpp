#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv )
{
    if ( argc != 3 )
    {
        printf("usage: VideoReScale.out <Video_Path> <scale factor>\n");
        return -1;
    }

    VideoCapture cap(argv[1]);
    if(!cap.isOpened()){
	    cout << "Error opening video stream or file" << endl;
	    return -1;
	}

    VideoWriter out;
    double factor = stod(argv[2]);
    double fps = cap.get(CAP_PROP_FPS);
    int fourcc = static_cast<int>(cap.get(CAP_PROP_FOURCC)); 
    Size S = Size(((int) cap.get(CAP_PROP_FRAME_WIDTH)) * factor,    
                  ((int) cap.get(CAP_PROP_FRAME_HEIGHT)) * factor);
    out.open("out.mp4", fourcc, fps, S);
    if (!out.isOpened())
    {
        cout  << "Could not open the output video for write: " << endl;
        return -1;
    }
    
    Size s0 = Size();
    while(1){
        Mat frame;
        Mat outFrame;
        cap >> frame;
        if(frame.empty())
            break;
        resize(frame,outFrame,s0,factor,factor);        
        out << outFrame;   
        frame.release();
        outFrame.release();    
    }
    out.release();
    cap.release();
    return 0;
}