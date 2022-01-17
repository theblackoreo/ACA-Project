
// PARALLEL APPLICATION

#include <stdio.h>
#include <mpi.h>
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

// parallel implementation for thresholding image
void Bthreshold(int **matrix, int istart, int iend, int jstart, int jend){

}

int main (int argc, char *argv[]) {


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

  int myrank, size;
  int N;

  src = imread(argv[1], IMREAD_REDUCED_GRAYSCALE_2);
  rows = src.rows;
  cols = src.cols;

  /* 1. Initialize MPI */
  MPI_Init(&argc, &argv);

  /* 2. Get my rank */
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  /* 3. Get the total number of processes */
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (N % size != 0){
    if(my_rank == 0) printf("The number %d of elements in the matrix connot be splitted among all");
    MPI_Finalize();
    return 0;
  }

  int buffer_to_send[rows][cols];
  int buffer_to_recv[rows/size][cols];




  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
