#include <stdio.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
void videoSave(vector<Mat> frames, double fps, int fourcc, Size S);

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
    double fps = cap.get(CAP_PROP_FPS);
    int fourcc = static_cast<int>(cap.get(CAP_PROP_FOURCC)); 



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
/*     int i = 0;
    while(1){
        imshow("Frame", frames[i]);
        i++;
        frames.pop_back();
        char c=(char)waitKey(25);
        if(c==27)
            break;
    } */

    //TODO: funcion para cambiar el tamaño del video

    // Resolucion del video origianl, debe ser cambiado para obtener nueve resolucion
    Size S = Size((int) cap.get(CAP_PROP_FRAME_WIDTH),    
                  (int) cap.get(CAP_PROP_FRAME_HEIGHT));
    videoSave(frames, fps, fourcc, S);
    cap.release();
    destroyAllWindows();
    return 0;
}

void videoSave(vector<Mat> frames, double fps, int fourcc, Size S){
    cout << "Guardando video..." << endl;
    VideoWriter out;
    out.open("out.mp4", fourcc, fps, S);
    if (!out.isOpened())
    {
        cout  << "Could not open the output video for write: " << endl;
        return;
    }
    for (Mat frame : frames)
    {
        out << frame;
    }
    out.release();
}