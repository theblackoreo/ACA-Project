#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#define N 20

int A[N][N];
int dilation_size = 1;

using namespace cv;
using namespace std;

int main(){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            A[i][j] = rand()%2;
            if (i == j)
                A[i][j] = 1;
        }
    }
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
           // printf("%d ", A[i][j]);
        }
      //  printf("\n");
    }

    cv::Mat B(20, 20, CV_64F, A);
  // std::memcpy(B.data, A, 20*20*sizeof(double));

    cv::Mat C(20, 20, CV_64F, A);



        cv::Mat element = getStructuringElement( MORPH_RECT,
                                             Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                             Point( dilation_size, dilation_size ) );
        dilate(B, C, MORPH_RECT);
        imshow( "Dilation Demo", C );






}