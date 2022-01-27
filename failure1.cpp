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
uchar buffer_to_send[MAXBYTES];
int my_rank, size, rows, cols, bytes;
Mat src;
int dims[2];

// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

void checkNearByte(int current_label, int r, int c, Mat& src, Mat& label_dst) {
  rows = src.rows;
  cols = src.cols;
  if (r < 0 || r == rows) return; // out of bounds
  if (c < 0 || c == cols) return; // out of bounds
  if (label_dst.at<unsigned char>(r,c) || !src.at<unsigned char>(r,c)) return; // already labeled or not marked with 1 in src

  // mark the current cell
  label_dst.at<unsigned char>(r,c) = current_label;

  // recursively mark the neighbors
  for (int direction = 0; direction < 4; ++direction)
  checkNearByte(current_label, r + dx[direction], c + dy[direction], src, label_dst);
}

//function to identify different regions of the labirinth
Mat find_components(Mat& rcv, Mat& label_dst) {
  int component = 0;
  for (int i = 0; i < rcv.rows; i++)
  for (int j = 0; j < rcv.cols; j++)
  if (!label_dst.at<unsigned char>(i,j) && rcv.at<unsigned char>(i,j)) {
    checkNearByte(++component, i, j, rcv, label_dst);
    printf("Sono entrato con i=%d e j=%d mentre label=%d\n", i, j, component);
  }
  imshow("LABELLED MAT TO RETURN", label_dst*50);
  return label_dst;
}


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

    dims[0] = (rows/size);
    dims[1] = cols;
    //bytes = bytes/size;

    for(int i = 1; i < size; i++){
      MPI_Send(&dims,2*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
      /*
      memcpy(&buffer_to_send[0*sizeof(int)],m.data+160*161/size, 160*161/size);
      Mat mat_to_snd = Mat(80, 161,0,buffer_to_send);
      imshow("mat to send", mat_to_snd);
      waitKey(0);
      */
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


  // for (int i = 0; i < bytes; i++){
  // printf("process: %d, element n. %d ---> %hhu\n", my_rank, i, buffer_to_recv[i]);
  // }

}

void second_matscatter(Mat& m, int my_rank){
  if(my_rank == 0){
    cols = m.cols;
    rows = m.rows;
    bytes=m.rows*m.cols;

    // cout << "matsnd: bytes=" << bytes << endl;
    // cout << endl << m << endl << endl;
    std::cout << "SECOND MATSCATTER REACHED" << '\n';
    if(!m.isContinuous())
    {
      m = m.clone();
    }
    for (size_t i = 0; i < size; i++) {
      int to_copy = (bytes/size)+cols;
      memcpy(&buffer[(i*bytes/size)+i*cols],&m.data[(i*bytes/size)], to_copy);
    }

    dims[0] = ((rows+size)/size);
    dims[1] = cols;


    for(int i = 1; i < size; i++){
      MPI_Send(&dims,2*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
      /*
      memcpy(&buffer_to_send[0*sizeof(int)],m.data+160*161/size, 160*161/size);
      Mat mat_to_snd = Mat(rows+size, cols,0,buffer);
      imshow("mat to send", mat_to_snd*50);
      waitKey(0);
      */
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
  //
  MPI_Scatter(&buffer, bytes, MPI_UNSIGNED_CHAR, buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);


  // for (int i = 0; i < bytes; i++){
  // printf("process: %d, element n. %d ---> %hhu\n", my_rank, i, buffer_to_recv[i]);
  // }

}

void matgather(Mat& m) {
  memcpy(&buffer_to_recv[0*sizeof(int)],m.data,bytes);
  MPI_Gather(&buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, buffer, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  if(my_rank == 0) {
    m = Mat(rows*size,cols,0,buffer);
    imshow("Returned", m*255);
    waitKey(0);
  }
}

int main(int argc, char* argv[])
{
  Mat label_dst;

  MPI_Init(&argc, &argv);

  // Get my rank
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Get number of processes and check that the buffer size can be splitted among all processes
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(my_rank == 0)
  {
    int i, j, element_size, quattro, mode;
    mode = atoi(argv[2]) + 1 ;

    if(mode<2 || mode>3) {
      fprintf(stderr, "Wrong mode: 1 or 2\n");
      exit(-2);
    }

    src = imread(argv[1], IMREAD_REDUCED_GRAYSCALE_2);
    int height = src.rows - (src.rows%size);
    int width = src.cols;
    resize(src, src, Size(width, height));
    rows = src.rows;
    cols = src.cols;
    // printf("%d x %d\n", src.rows, src.cols);
    // getchar();
    //cout << endl << src << endl << endl;
    imshow("Resized", src);
    waitKey(0);
  }

  matscatter(src, my_rank);
  Mat received = Mat(rows,cols,0,buffer_to_recv);
  //cout << endl << received << endl << endl;

  //We tranform the matrix so that we have 0s and 1s.
  threshold(received, received, 128,1,THRESH_BINARY);
  //printf("Process %d: \n", my_rank);


  // Complement of the received matrix
  for(int i = 0; i < received.rows; i++){
    for(int j = 0; j < received.cols; j++){
      received.at<unsigned char>(i,j) = (received.at<unsigned char>(i,j) - 1) * (-1);
    }

    //Printing the thresholded and complemented matrix
    // cout << endl << received << endl << endl;
    // imshow("Thresholded and complemented", received*255);
    // waitKey(0);
  }

  matgather(received);
  // imshow("Received in main", received*50);
  // waitKey(0);

  if(my_rank == 0) {
    label_dst = Mat::zeros(received.rows, received.cols, CV_8UC1);
    label_dst = find_components(received, label_dst);
    imshow("Components", label_dst*50);
    waitKey(0);


    // std::vector<int> up;
    // label_dst.row(0).copyTo(up);
    // std::sort(up.begin(), up.end());
    // // for (int el: up)
    // // std::cout << "(" << el << ") ";
    // // printf("\n\n");
    //
    // std::vector<int> down;
    // label_dst.row(label_dst.rows-1).copyTo(down);
    // std::sort(down.begin(), down.end());
    // // for (int el: down)
    // // std::cout << "(" << el << ") ";
    // // printf("\n\n");
    //
    // std::vector<int> left;
    // label_dst.col(0).copyTo(left);
    // std::sort(left.begin(), left.end());
    // // for (int el: left)
    // // std::cout << "(" << el << ") ";
    // // printf("\n\n");
    //
    // std::vector<int> right;
    // label_dst.col(label_dst.cols-1).copyTo(right);
    // std::sort(right.begin(), right.end());
    // // for (int el: right)
    // // std::cout << "(" << el << ") ";
    // // printf("\n\n");
    //
    // for (size_t i = 0; i < 2; i++) {
    //   if(up.at(i) == 0)
    // }

  } //close of the if for father process

  second_matscatter(label_dst, my_rank);
  received = Mat(rows,cols,0,buffer_to_recv);
  imshow("Ricevuta dal padre", received*50);
  waitKey(0);

  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
