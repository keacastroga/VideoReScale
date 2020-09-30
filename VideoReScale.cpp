#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: VideoReScale.out <Video_Path>\n");
        return -1;
    }
    VideoCapture cap(argv[1]);
    if(!cap.isOpened()){
	    cout << "Error opening video stream or file" << endl;
	    return -1;
	} 
    while(1){
        Mat frame;
        cap >> frame;
        if(frame.empty())
            break;
        imshow("Frame", frame);
        char c=(char)waitKey(25);
        if(c==27)
            break;
    }
    cap.release();
    destroyAllWindows();
    return 0;
}