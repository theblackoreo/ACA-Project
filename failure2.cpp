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
int my_rank, size, rows, cols;
Mat src, toSend, label_dst, rcv;
int *byte;

// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

void checkNearByte(int current_label, int r, int c, Mat src) {
  rows = rcv.rows;
  cols = rcv.cols;
  if (r < 0 || r == rows) return; // out of bounds
  if (c < 0 || c == cols) return; // out of bounds
  if (label_dst.at<unsigned char>(r,c) || !rcv.at<unsigned char>(r,c)) return; // already labeled or not marked with 1 in src

  // mark the current cell
  label_dst.at<unsigned char>(r,c) = current_label;

  // recursively mark the neighbors
  for (int direction = 0; direction < 4; ++direction)
  checkNearByte(current_label, r + dx[direction], c + dy[direction], rcv);
}

//function to identify different regions of the labirinth
void find_components() {
  int component = 0;
  for (int i = 0; i < rcv.rows; i++)
  for (int j = 0; j < rcv.cols; j++)
  if (!label_dst.at<unsigned char>(i,j) && rcv.at<unsigned char>(i,j)) {
    checkNearByte(++component, i, j, rcv);
    printf("Sono entrato con i=%d e j=%d mentre label=%d\n", i, j, component);
  }
}

void matscatter(Mat& m, int dest){
  MPI_Request req;
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
  cout << "nProc: size=" << size << endl;
  cout << "sender: sender=" << my_rank << endl;
  if(!m.isContinuous())
  {
    m = m.clone();
  }
  memcpy(&buffer[3*sizeof(int)],m.data,bytes);

  MPI_Isend(&buffer,bytes+3*sizeof(int),MPI_UNSIGNED_CHAR,dest,my_rank,MPI_COMM_WORLD, &req);

}

Mat reconstruct(int source){
  MPI_Status status;
  int count,rows,cols,type,channels;
  MPI_Recv(&buffer,sizeof(buffer),MPI_UNSIGNED_CHAR,source,0,MPI_COMM_WORLD,&status);

  memcpy((uchar*)&rows,&buffer[0 * sizeof(int)], sizeof(int));
  memcpy((uchar*)&cols,&buffer[1 * sizeof(int)], sizeof(int));
  memcpy((uchar*)&type,&buffer[2 * sizeof(int)], sizeof(int));

  cout << "matrcv: Process=" << my_rank << endl;
  cout << "matrcv: rows=" << rows << endl;
  cout << "matrcv: cols=" << cols << endl;
  cout << "matrcv: type=" << type << endl;

  // Make the mat
  Mat received= Mat(rows,cols,type,(uchar*)&buffer[3*sizeof(int)]);
  return received;
}

int main(int argc, char* argv[])
{
  Mat my_submatrix;
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

    printf("Original matrix -> rows:%d, cols:%d\n", src.rows, src.cols);
    resize(src, src, Size(rows-(rows%size), cols));
    printf("Resized matrix -> rows:%d, cols:%d\n", src.rows, src.cols);
    rows = rows - (rows%size);

    for(int i = 1; i < size; i++){
      my_submatrix = src.rowRange(i*(rows/size), i*(rows/size)+(rows/size));
      matscatter(my_submatrix, i);
    }
    printf("Helloooooo");
    my_submatrix = src.rowRange(0, (rows/size));
    src = my_submatrix;
    for(int i = 1; i < size; i++){

      Mat returned = reconstruct(i);
      vconcat(src, returned, src);
    }
    cout << "reassembled = " << endl << " "  << src << endl << endl;
  }
  Mat rcv = (my_rank) ? reconstruct(0) : my_submatrix;
  printf("Received matrix -> rows:%d, cols:%d\n", rcv.rows, rcv.cols);
  threshold(rcv, rcv, 127,1,THRESH_BINARY);
  imshow("Lab", rcv*255);
  //cout << "rcv = " << endl << " "  << rcv << endl << endl;

  for(int i = 0; i < rcv.rows; i++){
    for(int j = 0; j < rcv.cols; j++){
      rcv.at<unsigned char>(i,j) = (rcv.at<unsigned char>(i,j) - 1) * (-1);
    }
  }
  imshow("Complement", rcv*255);

  // labelization
  label_dst = Mat::zeros(rcv.rows, rcv.cols, CV_8UC1);
  find_components();
  //  cout << "LABEL = " << endl << " "  << label_dst << endl << endl;
  imshow( "LABEL", rcv*50);
  //  waitKey(0);

  printf("Before\n");
  matscatter(rcv, 0);
  printf("After\n");

  /* Terminate MPI */
  MPI_Finalize();

  return EXIT_SUCCESS;
}
