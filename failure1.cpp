#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int MAXBYTES=8*1024*1024;
uchar buffer[MAXBYTES];
uchar buffer_to_recv[MAXBYTES];
int my_rank, size, rows, cols, bytes;
Mat src;
int dims[2];

void matscatter(Mat& m, int my_rank){

if(my_rank == 0){

  cols = m.cols;

  int bytespersample=1; // change if using shorts or floats
  bytes=m.rows*m.cols*bytespersample;

  cout << "matsnd: bytes=" << bytes << endl;
  cout << endl << m << endl << endl;
  if(!m.isContinuous())
  {
    m = m.clone();
  }

  memcpy(&buffer[0*sizeof(int)],m.data,bytes);

  printf("to send: ");
  for (int i = 0; i < bytes; i++){
    printf("%hhu ", buffer[i]);
  }

  dims[0] = rows/size;
  dims[1] = cols;
  //bytes = bytes/size;

  for(int i = 1; i < size; i++){
    MPI_Send(&dims,2*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
  }

}
  else {
  MPI_Status status;
  MPI_Recv(&dims,2*sizeof(int),MPI_INT,0,555, MPI_COMM_WORLD, &status);

  printf("RICEVUTO %d\n", dims[0]*dims[1]);

  }

  bytes = dims[0]*dims[1];
  rows = dims[0];
  cols = dims[1];


  MPI_Scatter(&buffer, bytes, MPI_UNSIGNED_CHAR, buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);


  Mat a = Mat(rows,cols,0,buffer_to_recv);

  cout << endl << a << endl << endl;
  /*
  for (int i = 0; i < bytes; i++){
    printf("process: %d, element n. %d ---> %hhu\n", my_rank, i, buffer_to_recv[i]);
  }
  */

}

Mat reconstruct(){
  // Make the mat
  Mat received= Mat(1,0,0,(uchar*)&buffer[0*sizeof(int)]);

  return received;
}

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  // Get my rank
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Get number of processes and check that the buffer size can be splitted among all processes
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(my_rank == 0)
  {
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

    resize(src, src, Size(rows-(rows%size), cols));
    rows = rows - (rows%size);



  }

  matscatter(src, my_rank);

//  Mat m = reconstruct();


  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
