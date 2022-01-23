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
int my_rank, size, rows, cols;
Mat src;

void matscatter(Mat& m, int my_rank){
  int rows  = m.rows;
  int cols  = m.cols;
  int type  = m.type();
  int channels = m.channels();
  memcpy(&buffer[0 * sizeof(int)],(uchar*)&rows,sizeof(int));
  memcpy(&buffer[1 * sizeof(int)],(uchar*)&cols,sizeof(int));
  memcpy(&buffer[2 * sizeof(int)],(uchar*)&type,sizeof(int));

  // See note at end of answer about "bytes" variable below!!!
  int bytespersample=1; // change if using shorts or floats
  int bytes=m.rows*m.cols*channels*bytespersample;
  cout << "matsnd: rows=" << rows << endl;
  cout << "matsnd: cols=" << cols << endl;
  cout << "matsnd: type=" << type << endl;
  cout << "matsnd: channels=" << channels << endl;
  cout << "matsnd: bytes=" << bytes << endl;

  if(!m.isContinuous())
  {
    m = m.clone();
  }
  memcpy(&buffer[3*sizeof(int)],m.data,bytes);
  MPI_Scatter(&buffer, bytes+3*sizeof(int)/size, MPI_UNSIGNED_CHAR, buffer_to_recv, bytes+3*sizeof(int)/size, MPI_UNSIGNED_CHAR, 0,MPI_COMM_WORLD);
}

Mat reconstruct(){
  int count,rows,cols,type,channels;
  memcpy((uchar*)&rows,&buffer[0 * sizeof(int)], sizeof(int));
  memcpy((uchar*)&cols,&buffer[1 * sizeof(int)], sizeof(int));
  memcpy((uchar*)&type,&buffer[2 * sizeof(int)], sizeof(int));

  cout << "matrcv: Count=" << count << endl;
  cout << "matrcv: rows=" << rows << endl;
  cout << "matrcv: cols=" << cols << endl;
  cout << "matrcv: type=" << type << endl;

  // Make the mat
  Mat received= Mat(rows,cols,type,(uchar*)&buffer[3*sizeof(int)]);
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
  Mat m = reconstruct();


  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}