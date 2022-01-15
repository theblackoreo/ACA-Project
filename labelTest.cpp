
// serial code

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
Mat src, erosion_dst, dilation_dst;
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

#define N 7 //dimension test matrix

int A[N][N]; //main matrix
bool C[N][N] = {}; // check matrix


//show matrix
void printMatrix(int matrix[N][N]) {
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
  printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");
}

/*

// check near bytes from a specific position
bool checkNearByte(int label, int r, int c){

if(C[r][c] == true) return false;
else C[r][c] = true;
int count = 0;
bool cond;

if( A[r+1][c]==1){
A[r+1][c] = label;
checkNearByte(label, r+1, c);
cond = true;
}

if( A[r-1][c]==1){
A[r-1][c] = label;
cond = checkNearByte(label, r-1, c);
cond = true;
}

if(A[r][c+1]==1){
A[r][c+1] = label;
cond = checkNearByte(label, r, c+1);
cond = true;
}

if(A[r][c-1]==1){
A[r][c-1] = label;
cond = checkNearByte(label, r, c-1);
cond = true;
}

if(count == 0) return true;
return cond;

}

*/

bool checkDown(int label, int r, int c, Mat src) {
  if( src.at<unsigned char>(r+1,c)==1 && r != rows ){
    src.at<unsigned char>(r+1,c) = label;
      C[r+1][c] = true;
  } else {
    return true;
  }
  return checkDown(label, r+1, c, src);
}

bool checkUp(int label, int r, int c, Mat src) {
  if( src.at<unsigned char>(r-1,c)==1 && r != 0 ){
    src.at<unsigned char>(r-1,c) = label;
      C[r-1][c] = true;
  } else {
    return true;
  }
  return checkUp(label, r-1, c, src);
}

bool checkRight(int label, int r, int c, Mat src) {
  if( src.at<unsigned char>(r,c+1)==1 && c != cols ){
    src.at<unsigned char>(r,c+1) = label;
      C[r][c+1] = true;
  } else {
    return true;
  }
  return checkRight(label, r, c+1, src);
}

bool checkLeft(int label, int r, int c, Mat src) {
  if( src.at<unsigned char>(r,c-1)==1 && c != 0 ){
    src.at<unsigned char>(r,c-1) = label;
      C[r][c-1] = true;
  } else {
    return true;
  }
  return checkLeft(label, r, c-1, src);
}

// Second version but very similar to the previous
bool checkNearByte(int label, int r, int c, Mat src) {
  if(C[r][c]) {
    //This pixel has been checked already
    return true;
  } else {
    C[r][c] = true;
  }

  bool isLabelFinished = false;
  bool up, down, right, left = false;
  down = checkDown(label, r, c, src);
  up = checkUp(label, r, c, src);
  right = checkRight(label, r, c, src);
  left = checkLeft(label, r, c, src);

  if(up && down && right && left)
  isLabelFinished = true;

  return isLabelFinished;

}

/*
// check near bytes from a specific position
bool checkNearByte(int label, int r, int c, Mat src){

if(C[r][c] == true) return false;
else C[r][c] = true;

bool cond = false;
bool c1, c2, c3, c4 = false;

if( src.at<unsigned char>(r+1,c)==1 && r != rows ){
src.at<unsigned char>(r+1,c) = label;
c1 = checkNearByte(label, r+1, c, src);
} else if (r == rows) {
c1 = true;
}

if( src.at<unsigned char>(r-1,c)==1 && r != 0){
src.at<unsigned char>(r-1,c) = label;
c2 =checkNearByte(label, r-1, c, src);
}else if (r == 0) {
c2 = true;
}

if(src.at<unsigned char>(r,c+1)==1 && c!= cols){
src.at<unsigned char>(r,c+1) = label;
c3 = checkNearByte(label, r, c+1, src);
} else if (c == cols) {
c3 = true;
}

if(src.at<unsigned char>(r,c-1)==1 && c!=0){
src.at<unsigned char>(r,c-1) = label;
c4 = checkNearByte(label, r, c-1, src);
} else if (c == 0) {
c4 = true;
}

if(C[r+1][c] == true && C[r-1][c] == true && C[r][c-1] == true && C[r][c+1] == true){
cond = true;
} else {
cond = false;
}

if(c1 && c2 && c3 &&c4 && cond){
cond = true;
} else {
cond = false;
}

return cond;

}

*/

/*

// find correct maze track to follow
int checkSolutionLabel(int matrix[4][maxit]){
int correctLabel[maxit];
int count = 0;

for(int i = 0; i < 4; i++){
for(int j = 0; j < maxit; j++){
if(matrix[i][j] != 0) {
correctLabel[matrix[i][j]+1]++;
}
}
}

for(int i = 0; i < maxit; i++){
if(correctLabel[i] > 1) return i-1;
}
return 0;
}

*/


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
      /*
      //construct matrix A random  (simulate maze)
      for(int i = 0; i < N; i++){
      for(int j = 0; j < N; j++){

      A[rand()%7][rand()%7] = 1;
      C[i][j] = false;
    }

  }

  //simulate maze track
  for(int j = 0; j < N; j++){

  A[0][j] = 0;
  A[6][j] = 0;
  A[j][0] = 0;
  A[j][6] = 0;
}
//A[2][5] = 0;
A[0][3] = 1;
A[1][6] = 1;

//Complememt
for(int i = 0; i < N; i++){
for(int j = 0; j < N; j++){

//   A[i][j] =  (A[i][j] - 1)* (-1);

}
}


for(int i = 0; i < N; i++){
for(int j = 0; j < N; j++){
printf("%d ", A[i][j]);
}
printf("\n");
}

int label = 1;

for(int i = 1; i < N-1; i++){
for(int j = 1; j < N-1; j++){

if (A[i][j] == 1){
if(C[i][j]== false) A[i][j] = label;
if(checkNearByte(label, i, j) == true) label++;
}

}

}

for(int i = 1; i < 4; i++){
for(int j = 1; j < N; j++){
LABELS [i][j]  = 0;

}

}


// check if a track exists
for(int j = 0; j < N; j++){
LABELS[0][j] = A[0][j];
LABELS[1][j] = A[N-1][j];
}

for(int j = 1; j < N-1; j++){
LABELS[2][j] = A[j][0];
LABELS[3][j] = A[j][N-1];
}

printf("\n");
for(int i = 0; i < 4; i++){
for(int j = 0; j < N; j++){
printf("%d ", LABELS[i][j]);
}
printf("\n");
}


printMatrix(A);


int path = checkSolutionLabel(LABELS);
printf("Label to follow for solution: %d\n", path);

for(int i = 0; i<N; i++)
for(int j = 0 ; j<N; j++){
if(A[i][j] != path)
A[i][j]= 0;
else {
A[i][j]= 1;
}
}

printMatrix(A);


cv::Mat M(N, N, CV_8UC1);

for(int i = 0; i<N; i++)
for(int j = 0 ; j<N; j++){
M.at<unsigned char>(i,j) = A[i][j];
}

src = M;

//std::memcpy(M.data, A, N*N*sizeof(int));
//src = M;
*/

//CommandLineParser parser( argc, argv, "{@input | test.png | input image}" );
src = imread("test.png", IMREAD_REDUCED_GRAYSCALE_2);

threshold(src, src, 127,1,THRESH_BINARY);

namedWindow( "Erosion Demo", WINDOW_AUTOSIZE );
namedWindow( "Dilation Demo", WINDOW_AUTOSIZE );
namedWindow( "Complement", WINDOW_AUTOSIZE );

namedWindow( "Source", WINDOW_AUTOSIZE );
moveWindow( "LABEL", N, 0 );
moveWindow( "Source", N, 0 );

//show original image
imshow( "Source", src*255);

//print original image

cout << "SOURCE = " << endl << " "  << src << endl << endl;

rows = src.rows;
cols = src.cols;
int label = 1;

/*
//Complememt
for(int i = 0; i < rows; i++){
for(int j = 0; j < cols; j++){
src.at<unsigned char>(i,j) =  (src.at<unsigned char>(i,j) - 1)* (-1);
}
}

imshow( "Complement", src*255);
*/
// labelization
for(int i = 0; i < rows; i++){
  for(int j = 0; j < cols; j++){
    if (src.at<unsigned char>(i,j) == 1){
      if(C[i][j]== false) src.at<unsigned char>(i,j) = label;
      if(checkNearByte(label, i, j, src) == true) label++;
    }
    else {
      C[i][j] = true;
    }
  }
}

cout << "LABEL = " << endl << " "  << src << endl << endl;

imshow( "LABEL", src*50);

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
  if(src.at<unsigned char>(i,j) != trackToFollow)
  src.at<unsigned char>(i,j)= 0;
  else {
    src.at<unsigned char>(i,j)= 1;
  }
}

namedWindow( "correctTrack", WINDOW_AUTOSIZE );
imshow( "correctTrack", src *255);

cout << "after change bits = " << endl << " "  << src << endl << endl;

//Complememt
// We need the complement because otherwise the dilation is done on 1s, which are our path
// all zeros would disappear without complement
for(int i = 0; i < rows; i++){
  for(int j = 0; j < cols; j++){
    src.at<unsigned char>(i,j) =  (src.at<unsigned char>(i,j) - 1)* (-1);
  }
}

Mat element = getStructuringElement( MORPH_RECT, Size(3,3));
dilate(src, dilation_dst, element);

imshow( "DilationOK", dilation_dst *255);

cout << "dilation_dst = " << endl << " "  << dilation_dst << endl << endl;

/*
erode(dilation_dst, erosion_dst, element);
cout << "erosion_dst = " << endl << " "  << erosion_dst << endl << endl;
*/

//Printing C just to know if all pixels are checked
/*
for(int i = 0; i < rows; i++){
for(int j = 0; j < cols; j++){
printf("%d ", C[i][j]);
}
printf("\n");
}
*/

waitKey(0);
return 0;

/*

cout << "dilation_dst = " << endl << " "  << dilation_dst << endl << endl;
Mat element = getStructuringElement( MORPH_RECT,
Size(3,3));
//cv::morphologyEx(dilation_dst, dilation_dst, MORPH_OPEN, element);

//  cout << "dilation_dst opening= " << endl << " "  << dilation_dst << endl << endl;


Erosion( 0, 0 );


cv::Mat track(N, N, CV_8UC1);

for(int i = 0; i<N; i++)
for(int j = 0 ; j<N; j++){
track.at<unsigned char>(i,j) = A[i][j];
}


dilation_dst = dilation_dst - erosion_dst;


cout << "maze track= " << endl << " "  << erosion_dst << endl << endl;

waitKey(0);

*/

}
