#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"

#include <opencv2/opencv.hpp>


/*
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
            printf("%d ", A[i][j]);
        }
        printf("\n");
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
*/


using namespace cv;
using namespace std;
Mat src, erosion_dst, dilation_dst;
int erosion_elem = 0;
int erosion_size = 0;
int dilation_elem = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;
void Erosion( int, void* );
void Dilation( int, void* );
int main( int argc, char** argv )
{
  //CommandLineParser parser( argc, argv, "{@input | test.png | input image}" );
  src = imread("test.png", IMREAD_REDUCED_GRAYSCALE_2);

  threshold(src, src, 127,255,THRESH_BINARY);


  //cout << "src = " << endl << " "  << src << endl << endl;

  cv::Mat M(N, N, CV_64F);
  std::memcpy(M.data, A, N*N*sizeof(int));
  src = M;

  if( src.empty() )
  {
    cout << "Could not open or find the image!\n" << endl;
    cout << "Usage: " << argv[0] << " <Input image>" << endl;
    return -1;
  }
  namedWindow( "Erosion Demo", WINDOW_AUTOSIZE );
  namedWindow( "Dilation Demo", WINDOW_AUTOSIZE );
  moveWindow( "Dilation Demo", src.cols, 0 );

  //Erosion( 0, 0 );
  Dilation( 0, 0 );
  waitKey(0);
  return 0;
}

void Erosion( int, void* )
{

  Mat element = getStructuringElement( MORPH_RECT,
                       Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                       Point( erosion_size, erosion_size ) );
  erode( src, M, element );
  imshow( "Erosion Demo", erosion_dst );
}

void Dilation( int, void* )
{
  Mat element = getStructuringElement( MORPH_RECT,
                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                       Point( dilation_size, dilation_size ) );
  dilate( src, M, element );
  imshow( "Dilation Demo", dilation_dst );
}
