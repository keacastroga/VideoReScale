#include <stdio.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#include <helper_cuda.h>

using namespace std;
using namespace cv;



__global__ void CUDAScale(uchar *pixels, uchar *newPixels, int newRows, int newCols, int rowLen, int newRowLen);

int main(int argc, char **argv)
{
    int rows, cols, channels, rowLen, newRows, newCols, newRowLen;
    cudaError_t err = cudaSuccess;
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
    //int fourcc = static_cast<int>(cap.get(CAP_PROP_FOURCC));
    int fourcc = VideoWriter::fourcc('H', '2', '6', '4'); 
    rows = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    cols = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    channels = 3;
    rowLen = channels * cols;
    newRows = rows * factor;
    newCols = cols * factor;
    newRowLen = channels * newCols;

    Size S = Size(newCols, newRows);

    size_t sizeOrig = sizeof(uchar) * cols * channels * rows;
    size_t sizeNew = sizeof(uchar) * newCols * channels * newRows;
    unsigned char *h_newPixels;
    h_newPixels = (uchar *)malloc(sizeNew);
    
    
    float *d_newPixels = NULL;
    err = cudaMalloc((void **)&d_newPixels, sizeNew);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device new pixels array (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    float *d_pixels = NULL;
    err = cudaMalloc((void **)&d_pixels, sizeOrig);

    if (err != cudaSuccess)
    {
        fprintf(stderr, "Failed to allocate device original pixels array (error code %s)!\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    
    cudaSetDevice(0);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, 0);
    int threadsPerBlock = _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * 2;
    int blocksPerGrid =(newCols + threadsPerBlock - 1) / threadsPerBlock;
    printf("CUDA kernel launch with %d blocks of %d threads\n", blocksPerGrid, threadsPerBlock);

    out.open(argv[3], fourcc, fps, S);
    if (!out.isOpened())
    {
        cout << "Could not open the output video for write: " << endl;
        return -1;
    }

    Mat frame;
    Mat outFrame(newRows, newCols, CV_8UC3, h_newPixels);
    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);
    while (1)
    {
        cap >> frame;
        if (frame.empty())
            break;
        cudaMemcpy(d_pixels, frame.data, sizeOrig, cudaMemcpyHostToDevice);
        CUDAScale<<<blocksPerGrid, threadsPerBlock>>>((uchar *)d_pixels, (uchar *)d_newPixels, newRows, newCols, rowLen, newRowLen);
        cudaMemcpy(h_newPixels, d_newPixels, sizeNew, cudaMemcpyDeviceToHost);
        out << outFrame;
    }
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    cudaFree(d_pixels);
    cudaFree(d_newPixels);
    free(h_newPixels);
    out.release();
    cap.release();
    return 0;
}


__global__ void CUDAScale(uchar *pixels, uchar *newPixels, int newRows, int newCols, int rowLen, int newRowLen){
    int scaledCol = blockDim.x * blockIdx.x + threadIdx.x;
    int origCol;
    uchar *pixel1, *pixel2, *pixel3, *pixel4;
    uchar *newPixel;
    if (scaledCol < newCols)
    {
        origCol = scaledCol * 6;
        for (int scaledRow = 0; scaledRow < newRows; scaledRow++)
        {
            pixel1 = pixels + origCol + scaledRow * 2 * rowLen;
            pixel2 = pixel1 + 3;
            pixel3 = pixel1 + rowLen;
            pixel4 = pixel3 + 3;
            newPixel = newPixels + scaledCol * 3 + scaledRow * newRowLen;
            *(newPixel) = (*pixel1 + *pixel2 + *pixel3 + *pixel4) / 4;
            *(newPixel + 1) = (*(pixel1 + 1) + *(pixel2 + 1) + *(pixel3 + 1) + *(pixel4 + 1)) / 4;
            *(newPixel + 2) = (*(pixel1 + 2) + *(pixel2 + 2) + *(pixel3 + 2) + *(pixel4 + 2)) / 4;
        }
    }
}
