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

const int MAXBYTES=4*8*1024*1024;
// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

uchar buffer[MAXBYTES];
uchar buffer_to_recv[MAXBYTES];
int my_rank, size, rows, cols, bytes, dims[3];
MPI_Status status;
Mat label_dst, rcv;


Mat binarization(Mat m){

  for(int i = 0; i < rows; i++){
    for(int j = 0; j < cols; j++){
      if(m.at<unsigned char>(i,j) < 147)
      m.at<unsigned char>(i,j) = 0;
      else
      m.at<unsigned char>(i,j) = 1;
    }
  }
  return m;
}

Mat difference(Mat dil, Mat ero){

  for(int i = 0; i < dil.rows; i++){
    for(int j = 0; j < dil.cols; j++){
      dil.at<unsigned char>(i,j) = dil.at<unsigned char>(i,j) - ero.at<unsigned char>(i,j);
    }
  }
  return dil;
}

Mat hopeErode(Mat m, int element_size){

  int n = 0;
  Mat eroded = m.clone();

  while(n < element_size/2){
    for(int i = 1; i < m.rows -1 ; i++){
      for(int j = 1; j < m.cols -1; j++){

        if(m.at<unsigned char>(i,j) && m.at<unsigned char>(i+1,j) && m.at<unsigned char>(i+1,j+1) &&
        m.at<unsigned char>(i,j+1) && m.at<unsigned char>(i-1,j+1) && m.at<unsigned char>(i-1,j) &&
        m.at<unsigned char>(i-1,j-1) && m.at<unsigned char>(i,j-1) && m.at<unsigned char>(i+1,j-1));

        else{
          eroded.at<unsigned char>(i,j) = 0;
          eroded.at<unsigned char>(i+1,j)= 0;
          eroded.at<unsigned char>(i+1,j+1)= 0;
          eroded.at<unsigned char>(i,j+1)= 0;
          eroded.at<unsigned char>(i-1,j+1)= 0;
          eroded.at<unsigned char>(i-1,j)= 0;
          eroded.at<unsigned char>(i-1,j-1)= 0;
          eroded.at<unsigned char>(i,j-1)= 0;
          eroded.at<unsigned char>(i+1,j-1)= 0;
        }
      }
    }
    n++;
    m = eroded.clone();
  }
  return eroded;
}

Mat hopeDilate(Mat m, int element_size) {
  int i,j,n = 0;
  Mat dilated = m.clone();

  while (n < element_size/2) {
    for(i = 1; i < m.rows-1; i++) {
      for (j = 1; j < m.cols-1; j++) {
        if(!m.at<unsigned char>(i,j) && !m.at<unsigned char>(i+1,j) && !m.at<unsigned char>(i+1,j+1) &&
        !m.at<unsigned char>(i,j+1) && !m.at<unsigned char>(i-1,j+1) && !m.at<unsigned char>(i-1,j) &&
        !m.at<unsigned char>(i-1,j-1) && !m.at<unsigned char>(i,j-1) && !m.at<unsigned char>(i+1,j-1));
        else {
          dilated.at<unsigned char>(i-1,j-1) = 1;
          dilated.at<unsigned char>(i-1,j) = 1;
          dilated.at<unsigned char>(i-1,j+1) = 1;
          dilated.at<unsigned char>(i,j-1) = 1;
          dilated.at<unsigned char>(i,j) = 1;
          dilated.at<unsigned char>(i,j+1) = 1;
          dilated.at<unsigned char>(i+1,j-1) = 1;
          dilated.at<unsigned char>(i+1,j) = 1;
          dilated.at<unsigned char>(i+1,j+1) = 1;
        }
      }
    }
    m = dilated.clone();
    n++;
  }
  return dilated;
}


void checkNearByte(int current_label, int r, int c) {

  if (r < 0 || r == rows) return; // out of bounds
  if (c < 0 || c == cols) return; // out of bounds
  if (label_dst.at<unsigned char>(r,c) || !rcv.at<unsigned char>(r,c)) return; // already labeled or not marked with 1 in src
  // mark the current cell
  label_dst.at<unsigned char>(r,c) = current_label;

  for (int direction = 0; direction < 4; ++direction)
    checkNearByte(current_label, r + dx[direction], c + dy[direction]);

}

//function to identify different regions of the labirinth
Mat find_components(Mat& mmi, Mat& label_dst) {
  int component = 0;
  rcv = mmi.clone();
  rows = rcv.rows;
  cols = rcv.cols;
  for (int i = 0; i < rows; i++)
  for (int j = 0; j < cols; j++)
  if (!label_dst.at<unsigned char>(i,j) && rcv.at<unsigned char>(i,j)) {
    checkNearByte(++component, i, j);
  }
  return label_dst;
}


void matscatter(Mat& m, int my_rank){
  if(my_rank == 0){
    bytes=rows*cols; //bytes to send
    if(!m.isContinuous())
    m = m.clone();
    //Obtaining an array out of a matrix
    memcpy(&buffer[0],m.data,bytes);
    //Defining variables to be used in the slave processes to reconstruct the submutrix
    dims[0] = (rows/size);
    dims[1] = cols;
    for(int i = 1; i < size; i++){
      MPI_Send(&dims,2*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
    }
  }
  else {
    MPI_Recv(&dims,2*sizeof(int),MPI_INT,0,555, MPI_COMM_WORLD, &status);
  }
  //Updating gloabal variables
  bytes = dims[0]*dims[1];
  rows = dims[0];
  cols = dims[1];
  MPI_Scatter(&buffer, bytes, MPI_UNSIGNED_CHAR, buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
}

void second_matscatter(Mat& m, int my_rank, int& element_size){
  if(my_rank == 0){
    bytes=rows*cols;
    if(!m.isContinuous())
    m = m.clone();
    //piecetobring is the number of further rows to be sent to avoid having a wrong result of the dilation operation in slaves
    int piecetobring = element_size;
    //to_copy is the number of values which will be given to each slave process
    int to_copy = (bytes/size)+cols*piecetobring;
    //Copying the matrix in the array so that the last rows destinated to a slave are also present in the following segment
    //which will be given to the following slave
    for (int i = 0; i < size-1; i++) {
      memcpy(&buffer[(i*bytes/size)+i*cols*piecetobring],&m.data[(i*bytes/size)], to_copy);
    }
    memcpy(&buffer[((size-1)*bytes/size)+(size-1)*cols*piecetobring],&m.data[((size-1)*bytes/size)], to_copy-piecetobring*cols);
    //Defining variables to be used in slave processes to reconstruct the submutrix
    dims[0] = (rows/size)+piecetobring;
    dims[1] = cols;
    dims[2] = element_size;

    for(int i = 1; i < size; i++){
      MPI_Send(&dims,3*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
    }
  }
  else {
    MPI_Recv(&dims,3*sizeof(int),MPI_INT,0,555, MPI_COMM_WORLD, &status);
  }

  //Updating gloabal variables
  bytes = dims[0]*dims[1];
  rows = dims[0];
  cols = dims[1];
  element_size = dims[2];

  MPI_Scatter(&buffer, bytes, MPI_UNSIGNED_CHAR, buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

}

void matgather(Mat& m) {
  //Getting an array out of a matrix
  memcpy(&buffer_to_recv[0],m.data,bytes);
  //Sending back the submatrices
  MPI_Gather(&buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, buffer, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  if(my_rank == 0) {
    //Recontruction of the whole matrix
    m = Mat(rows*size,cols,0,buffer);
  }
}

void second_matgather(Mat& m, int fragment) {
  bytes = m.rows*m.cols;
  memcpy(&buffer_to_recv[0],m.data, bytes);
  MPI_Gather(&buffer_to_recv, bytes, MPI_UNSIGNED_CHAR, buffer, bytes, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  if(my_rank == 0) {
    //Recontruction of the matrix --> final solution
    m = Mat(rows*size,cols,0,buffer);
    imwrite("results/FinalSolution.png", m*255);
    std::cout << "All operations have been performed, see the results in results folder" << '\n';
  }
}


int main(int argc, char* argv[])
{
  Mat src, element, dilation_dst, erosion_dst, solution;
  int i, j, quattro, element_size;
  int sum_var;
  char filename[50];
  int64 tin, t1, t2, tfi, clock;
  double timing;

  MPI_Init(&argc, &argv);

  // Get my rank
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Get number of processes and check that the buffer size can be splitted among all processes
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(my_rank == 0)
  {
    t1 = getTickCount();
    //Reading image to obtain a Mat object
    src = imread(argv[1], IMREAD_REDUCED_GRAYSCALE_2);
    int height = src.rows - (src.rows%size);
    int width = src.cols;
    //The resize process depends on the number of cores and aims at obtaining a number of rows divisible for size (cores' number)
    resize(src, src, Size(width, height));
    rows = src.rows;
    cols = src.cols;

    printf("Total element: %d\n", rows*cols);
  }

  //Dividing the matrix in submatrices to apply all the necessary operations in parallel
  matscatter(src, my_rank);

  //Following instructions are executed both in master and slave processes
  Mat received = Mat(rows,cols,0,buffer_to_recv);

  //Transformation of the submatrix to have only 0s and 1s.
  received = binarization(received);

  // Complement of the submatrix
  for(int i = 0; i < received.rows; i++){
    for(int j = 0; j < received.cols; j++){
      received.at<unsigned char>(i,j) = (received.at<unsigned char>(i,j) - 1) * (-1);
    }
  }

  //Giving control back to the master process
  matgather(received);

  if(my_rank == 0) {
    printf("THRESHOLD & COMPLEMENT TERMINATED\n");
    cout << "The control has been given back to the master after threshold and complement operations" << '\n';
    imwrite("results/BinarizedImage.png", received*255);

    cols = received.cols;
    rows = received.rows;
    //Evaluation of the element size based on the maze to be solved
    //Three sides must be checked to be sure to find at least one way out
    sum_var = 0;
    for(j = 0; j < cols; j++) {
      sum_var += received.at<unsigned char>(0,j);
    }
    element_size = cols - sum_var;

    if(!element_size) {
      sum_var = 0;
      for(i = 0; i < rows; i++) {
        sum_var += received.at<unsigned char>(i,0);
      }
      element_size = rows - sum_var;
    }

    if(!element_size) {
      sum_var = 0;
      for(j = 0; j < cols; j++) {
        sum_var += received.at<unsigned char>(rows-1,j);
      }
      element_size = cols - sum_var;
    }

    //Creating a matrix which will collect the result of labelization
    label_dst = Mat::zeros(rows, cols, CV_8UC1);

    //labelization --> it must be executed in serial
    label_dst = find_components(received, label_dst);

    imwrite("results/LabelledImage.png", label_dst*50);
    std::cout << "Labelization terminated correctly, matrix ready to be redistributed among all cores" << '\n' << "The element size is: " << element_size << "\n";
  } //operations performed only by the master concluded

  //Giving back submatrices to slave processes
  second_matscatter(label_dst, my_rank, element_size);

  //Submatrix received after scatter
  received = Mat(rows,cols,0,buffer_to_recv);

  //Setting to 0 the track to follow and to 1 all the other values
  for(i = 0; i<rows; i++)
  for(j = 0 ; j<cols; j++){
    if(received.at<unsigned char>(i,j) != 1)
    received.at<unsigned char>(i,j) = 0;
    else {
      received.at<unsigned char>(i,j) = 1;
    }
  }

  // dilation
  dilation_dst = hopeDilate(received, element_size);
  sprintf(filename, "results/Dilation-Process%d.png", my_rank);
  imwrite(filename, dilation_dst*255);

  // erosion
  erosion_dst = hopeErode(dilation_dst, element_size);
  sprintf(filename, "results/Erosion-Process%d.png", my_rank);
  imwrite(filename, erosion_dst*255);

  solution = dilation_dst.clone();

  // morphological gradient
  solution = difference(dilation_dst, erosion_dst);
  sprintf(filename, "results/PartialSolution-Process%d.png", my_rank);
  imwrite(filename, solution*255);

  //Waiting for all slaves to conclude their operations
  MPI_Barrier(MPI_COMM_WORLD);

  int fragment = element_size;
  rows = rows - fragment;
  Mat fragmentMat = solution.rowRange(rows, rows + fragment);
  bytes = fragment * cols;

  int to = (my_rank + 1) % size;
  int from = ((my_rank + size) - 1)%size;

  memcpy(&buffer[0],fragmentMat.data,bytes);

  MPI_Sendrecv(&buffer,bytes, MPI_INT, to, 111, &buffer_to_recv, bytes, MPI_INT, from, 111, MPI_COMM_WORLD, &status);

  Mat fragmentReceived;
  if(my_rank == 0){
    //Master process must ignore what it receives from last slave process
    fragmentReceived = Mat::zeros(fragment, cols, 0);
  } else{
    fragmentReceived = Mat(fragment, cols, 0, buffer_to_recv);
  }

  //Preparation of final submatrix solution to be gathered
  //This is obtained thanks to the logical OR between each couple of values
  //of the rows which are shared by a slave and the subsequent one
  Mat first = solution.rowRange(0, fragment);
  bitwise_or(fragmentReceived, first, fragmentReceived);
  vconcat(fragmentReceived, solution.rowRange(fragment, rows), solution);
  solution.rowRange(0, fragment) = fragmentReceived;

  //Before gathering, wait until every porcess has concluded its operations
  MPI_Barrier(MPI_COMM_WORLD);

  //Giving control back to the master process to obtain the final solution
  second_matgather(solution, fragment);
  if(my_rank == 0) {
    t2 = getTickCount();
    timing = (t2-t1)/getTickFrequency();
    clock = t2 - t1;
    std::cout << "\nTotal time (s): " << timing << '\n';
    std::cout << "\nTotal n clock cycles: " << clock << '\n';
  }

  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
