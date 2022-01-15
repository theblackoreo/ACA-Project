
// serial code

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
Mat src, label_dst, erosion_dst, dilation_dst;
int erosion_elem = 0;
int erosion_size = 0;
int dilation_elem = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;
int maxit = 1;
void Erosion( int, void* );
void Dilation( int, void* );
int rows, cols;


// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

void checkNearByte(int current_label, int r, int c, Mat src) {
  if (r < 0 || r == rows) return; // out of bounds
  if (c < 0 || c == cols) return; // out of bounds
  if (label_dst.at<unsigned char>(r,c) || !src.at<unsigned char>(r,c)) return; // already labeled or not marked with 1 in src

  // mark the current cell
  label_dst.at<unsigned char>(r,c) = current_label;

  // recursively mark the neighbors
  for (int direction = 0; direction < 4; ++direction)
  checkNearByte(current_label, r + dx[direction], c + dy[direction], src);
}

void find_components() {
  int component = 0;
  for (int i = 0; i < rows; i++)
  for (int j = 0; j < cols; j++)
  if (!label_dst.at<unsigned char>(i,j) && src.at<unsigned char>(i,j)) {
    checkNearByte(++component, i, j, src);
    printf("Sono entrato con i=%d e j=%d mentre label=%d\n", i, j, component);
  }
}

void Erosion( int, void*, Mat src, Mat erosion_dst)
{

  Mat element = getStructuringElement( MORPH_RECT,
    Size(3,3));
    erode( src, erosion_dst, element );

  }

  void Dilation( int, void*,  Mat src, Mat dilation_dst)
  {
    Mat element = getStructuringElement( MORPH_RECT,
      Size(3,3));
      dilate(src, dilation_dst, element);

    }


    int main(){

      //CommandLineParser parser( argc, argv, "{@input | test.png | input image}" );
      src = imread("test6.png", IMREAD_REDUCED_GRAYSCALE_2);

      threshold(src, src, 127,1,THRESH_BINARY);

      // namedWindow( "Erosion Demo", WINDOW_AUTOSIZE );
      // namedWindow( "Dilation Demo", WINDOW_AUTOSIZE );
      // namedWindow( "Complement", WINDOW_AUTOSIZE );
      // namedWindow( "Source", WINDOW_AUTOSIZE );
      // namedWindow( "Solution Demo", WINDOW_AUTOSIZE );

      //show original image
      imshow( "Source", src*255);

      //print original image

      cout << "SOURCE = " << endl << " "  << src << endl << endl;

      rows = src.rows;
      cols = src.cols;


      //Complememt
      for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
          src.at<unsigned char>(i,j) =  (src.at<unsigned char>(i,j) - 1)* (-1);
        }
      }

      imshow( "Complement", src*255);

      // labelization
      label_dst = Mat::zeros(rows, cols, CV_8UC1);
      find_components();

      cout << "LABEL = " << endl << " "  << label_dst << endl << endl;

      imshow( "LABEL", label_dst*50);

      int quattro = 4;
      if(rows > cols){
        maxit = rows;
      }
      else {
        maxit = cols;
      }

      int LABELS[quattro][maxit];

      for(int i = 0; i < 4; i++){
        for(int j = 0; j < maxit; j++){
          LABELS[i][j] = 0;
        }
      }
      // check if a track exists
      for(int j = 0; j < cols; j++){
        LABELS[0][j] = src.at<unsigned char>(0,j);
        LABELS[1][j] = src.at<unsigned char>(rows-1,j);
      }

      for(int j = 1; j < rows-1; j++){
        LABELS[2][j] = src.at<unsigned char>(j,0);
        LABELS[3][j] = src.at<unsigned char>(j,cols-1);
      }

      printf("\n");
      for(int i = 0; i < 4; i++){
        for(int j = 0; j < maxit; j++){
          printf("%d ", LABELS[i][j]);
        }
        printf("\n");
      }


      int correctLabel[maxit];
      for(int i = 0; i < maxit; i++){
        correctLabel[i] = 0;
      }

      int count = 0;

      for(int i = 0; i < 4; i++){
        for(int j = 0; j < maxit; j++){
          if(LABELS[i][j] != 0) {
            correctLabel[LABELS[i][j]+1]++;
          }
        }
      }

      int trackToFollow = 0;

      for(int i = 0; i < maxit; i++){
        if(correctLabel[i] > 1) trackToFollow = i-1;
      }

      printf("Track to follow: %d\n", trackToFollow);

      //  namedWindow( "DilationOK", WINDOW_AUTOSIZE );

      for(int i = 0; i<rows; i++)
      for(int j = 0 ; j<cols; j++){
        if(label_dst.at<unsigned char>(i,j) != trackToFollow)
        label_dst.at<unsigned char>(i,j)= 0;
        else {
          label_dst.at<unsigned char>(i,j)= 1;
        }
      }

      namedWindow( "correctTrack", WINDOW_AUTOSIZE );
      imshow( "correctTrack", src *255);

      cout << "after change bits = " << endl << " "  << label_dst << endl << endl;

      /*
      //Complememt
      // We need the complement because otherwise the dilation is done on 1s, which are our path
      // all zeros would disappear without complement
      for(int i = 0; i < rows; i++){
      for(int j = 0; j < cols; j++){
      label_dst.at<unsigned char>(i,j) =  (label_dst.at<unsigned char>(i,j) - 1)* (-1);
    }
  }
  */

  Mat element = getStructuringElement( MORPH_RECT, Size(3,3));
  dilate(label_dst, dilation_dst, element);

  imshow( "Dilation Demo", dilation_dst *255);

  cout << "dilation_dst = " << endl << " "  << dilation_dst << endl << endl;


  erode(dilation_dst, erosion_dst, element);
  cout << "erosion_dst = " << endl << " "  << erosion_dst << endl << endl;
  imshow( "Erosion Demo", dilation_dst *255);

  //NOW THE CORRECT PATH IS GIVEN BY 0s BECAUSE THE DILATION DILATES 1s
  //SO KEEPING ALL WITHOUT COMPLEMENT WOULD HAVE RESULTED IN A MATRIX FULL OF 1s
  //IN WHICH SOME WALLS DISAPPEARED.

  Mat solution = Mat::zeros(rows, cols, CV_8UC1);
  absdiff(dilation_dst, erosion_dst, solution);
  cout << "solution = " << endl << " "  << solution << endl << endl;
  imshow( "Solution Demo", solution *255);

  waitKey(0);
  return 0;

}
