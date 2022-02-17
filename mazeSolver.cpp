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
int rows, cols;

// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

//function to check in all directions (4-conn)
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

//function to identify different regions of the labirinth
void find_components() {
  int component = 0;
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) {
      if (!label_dst.at<unsigned char>(i,j) && src.at<unsigned char>(i,j)) {
        checkNearByte(++component, i, j, src);
      }
    }
}

int main(int argc, char*argv[]){
  if(argc != 3) {
    fprintf(stderr, "Wrong number of parameters: ./mazeSolver <file> <size of path: 1 or 2>\n");
    exit(-1);
  }

  int i, j, element_size, quattro, mode;
  mode = atoi(argv[2]) + 1 ;
  quattro = 4;

  if(mode<2 || mode>3) {
    fprintf(stderr, "Wrong mode: 1 or 2\n");
    exit(-2);
  }

  src = imread(argv[1], IMREAD_REDUCED_GRAYSCALE_2);
  threshold(src, src, 127,1,THRESH_BINARY);

  rows = src.rows;
  cols = src.cols;

  //Complememt
  for(i = 0; i < rows; i++){
    for(j = 0; j < cols; j++){
      src.at<unsigned char>(i,j) = (src.at<unsigned char>(i,j) - 1) * (-1);
    }
  }

  //Evaluation of the element size based on the maze to be solved
  //Three sides must be checked to be sure to find at least one way out
  Scalar sum_var = sum(src.row(0));
  element_size = cols - sum_var.val[0];

  if(!element_size) {
    sum_var = sum(src.col(0));
    element_size = rows - sum_var.val[0];
  }

  if(!element_size) {
    sum_var = sum(src.row(rows-1));
    element_size = cols - sum_var.val[0];
  }

  // labelization
  label_dst = Mat::zeros(rows, cols, CV_8UC1);
  find_components();

  int trackToFollow = 1;
  for(i = 0; i<rows; i++)
  for(j = 0 ; j<cols; j++){
    if(label_dst.at<unsigned char>(i,j) != trackToFollow)
    label_dst.at<unsigned char>(i,j) = 0;
    else {
      label_dst.at<unsigned char>(i,j) = 1;
    }
  }

  Mat element = getStructuringElement( MORPH_RECT, Size(element_size*mode, element_size*mode));
  dilate(label_dst, dilation_dst, element);

  erode(dilation_dst, erosion_dst, element);

  Mat solution = Mat::zeros(rows, cols, CV_8UC1);
  absdiff(dilation_dst, erosion_dst, solution);
  return 0;

}
