
// PARALLEL APPLICATION

#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
int rows, cols, size;
Mat src;

// parallel implementation for thresholding image
int Bthreshold(int *array){
  cv::Mat A(1, cols, CV_8UC1, array);
  threshold(A, A, 127,1,THRESH_BINARY);
  cout << "SOURCE = " << endl << " "  << A << endl << endl;
  return 0;
}

int main (int argc, char *argv[]) {
  int myrank, **buffer_to_send, **buffer_to_recv;

  /* 1. Initialize MPI */
  MPI_Init(&argc, &argv);

  /* 2. Get my rank */
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  /* 3. Get the total number of processes */
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  //OUR BUFFER TO SEND IS THE MATRIX


  if(myrank == 0) {
    if(argc != 3) {
      fprintf(stderr, "Wrong number of parameters: ./program <file> <size of path: 1 or 2> \n");
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
    rows = src.rows;
    cols = src.cols;

    cv::resize(src, src, cv::Size(rows-(rows%size), cols));
    rows = rows - (rows%size);
    // cout << "Resized = " << endl << " "  << src << endl << endl;
    // imshow("Immagine ridotta",src);
    // waitKey(0);

    if (rows % size != 0){
      if(myrank == 0) printf("The number of elements in the matrix connot be splitted among all");
      MPI_Finalize();
      printf("Resize failed\n");
      return 0;
    }

    // mat = (int **) malloc(rows*sizeof(int*));
    // for (i = 0; i<rows; i++) {
    //   mat[i] = (int *) malloc(cols*sizeof(int));
    //   for (j = 0; j<cols; j++)
    //     mat[i][j] = src.at<unsigned int>(i,j);
    // }

    int mat[rows][cols];
    for (i = 0; i<rows; i++) {
      for (j = 0; j<cols; j++)
        mat[i][j] = src.at<unsigned char>(i,j);
      }

      int (*ptr)[rows][cols];
      ptr = &mat[0][0];
      //*buffer_to_send = &mat;
      // for (i = 0; i<rows; i++) {
      //   for (j = 0; j<cols; j++)
      //   printf("%d ", mat[i][j]);
      //   printf("\n");
      // }

      cout << (*ptr)[0][0];

  }

  for (int i = 0; i<rows; i++) {
    for (int j = 0; j<cols; j++)
      printf("%d ", buffer_to_send[i][j]);
    printf("\n");
  }

  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
