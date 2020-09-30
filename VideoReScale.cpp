#include <stdio.h>
#include <iostream>
#include <vector>
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
    //Cargando video
    VideoCapture cap(argv[1]);
    if(!cap.isOpened()){
	    cout << "Error opening video stream or file" << endl;
	    return -1;
	} 

    //Pasar frames del video a vector de Mat
    vector<Mat> frames;
    while(1){
        Mat frame;
        cap >> frame;
        if(frame.empty())
            break;
        frames.push_back(frame);
    }

    //mostrando video por medio del vector de frames
    int i = 0;
    while(1){
        imshow("Frame", frames[i]);
        i++;
        frames.pop_back();
        char c=(char)waitKey(25);
        if(c==27)
            break;
    }
    cap.release();
    destroyAllWindows();
    return 0;
}