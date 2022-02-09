#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>

using namespace std;
using namespace cv;

const int MAXBYTES=8*1024*1024;
// direction vectors
const int dx[] = {+1, 0, -1, 0};
const int dy[] = {0, +1, 0, -1};

uchar buffer[MAXBYTES];
uchar buffer_to_recv[MAXBYTES];
int my_rank, size, rows, cols, bytes, dims[4];
MPI_Status status;

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

void second_matscatter(Mat& m, int my_rank, int &element_size, int &modality){
  if(my_rank == 0){
    bytes=rows*cols;

    if(!m.isContinuous())
      m = m.clone();

    //piecetobring is the number of further rows to be sent to avoid having a wrong result of the dilation operation in slaves
    int piecetobring = element_size*modality;
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
    dims[3] = modality;

    for(int i = 1; i < size; i++){
      MPI_Send(&dims,4*sizeof(int),MPI_INT,i,555, MPI_COMM_WORLD);
    }
  }
  else {
    MPI_Recv(&dims,4*sizeof(int),MPI_INT,0,555, MPI_COMM_WORLD, &status);
  }

  //Updating gloabal variables
  bytes = dims[0]*dims[1];
  rows = dims[0];
  cols = dims[1];
  element_size = dims[2];
  modality = dims[3];

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
  Mat src, label_dst, element, dilation_dst, erosion_dst, solution;
  int i, j, quattro, modality, element_size;
  Scalar sum_var;
  char filename[50];

  MPI_Init(&argc, &argv);

  // Get my rank
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Get number of processes and check that the buffer size can be splitted among all processes
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(my_rank == 0)
  {
    modality = atoi(argv[2]) + 1 ;

    if(modality<2 || modality>3) {
      fprintf(stderr, "Wrong modality: 1 or 2, this is based on the size of the maze path\n");
      exit(-2);
    }

    //Reading image to obtain a Mat object
    src = imread(argv[1], IMREAD_REDUCED_GRAYSCALE_2);
    int height = src.rows - (src.rows%size);
    int width = src.cols;
    //The resize process depends on the number of cores and aims at obtaining a number of rows divisible for size (cores' number)
    resize(src, src, Size(width, height));
    rows = src.rows;
    cols = src.cols;
    std::cout << "Image read correctly, its size is: " << src.rows << "x" << src.cols << '\n' << "Choosen modality: " << modality-1 << "\n";
  }

  //Dividing the matrix in submatrices to apply all the necessary operations in parallel
  matscatter(src, my_rank);

  //Following instructions are executed both in master and slave processes
  Mat received = Mat(rows,cols,0,buffer_to_recv);

  //Transformation of the submatrix to have only 0s and 1s.
  threshold(received, received, 128,1,THRESH_BINARY);

  // Complement of the submatrix
  for(int i = 0; i < received.rows; i++){
    for(int j = 0; j < received.cols; j++){
      received.at<unsigned char>(i,j) = (received.at<unsigned char>(i,j) - 1) * (-1);
    }
  }

  //Giving control back to the master process
  matgather(received);

  if(my_rank == 0) {
    cout << "The control has been given back to the master after threshold and complement operations" << '\n';
    imwrite("results/BinarizedImage.png", received*255);

    //Evaluation of the element size based on the maze to be solved
    //Three sides must be checked to be sure to find at least one way out
    sum_var = sum(received.row(0));
    element_size = received.cols - sum_var.val[0];

    if(!element_size) {
      sum_var = sum(received.col(0));
      element_size = received.rows - sum_var.val[0];
    }

    if(!element_size) {
      sum_var = sum(received.row(received.rows-1));
      element_size = received.cols - sum_var.val[0];
    }

    //Creating a matrix which will collect the result of labelization
    label_dst = Mat::zeros(received.rows, received.cols, CV_8UC1);
    label_dst = find_components(received, label_dst);
    imwrite("results/LabelledImage.png", label_dst*50);
    std::cout << "Labelization terminated correctly, matrix ready to be redistributed among all cores" << '\n' << "The element size is: " << element_size << "\n";
  } //operations performed only by the master concluded

  cols = label_dst.cols;
  rows = label_dst.rows;
  //Giving back submatrices to slave processes
  second_matscatter(label_dst, my_rank, element_size, modality);

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

  element = getStructuringElement( MORPH_RECT, Size(element_size*modality, element_size*modality));
  dilate(received, dilation_dst, element);
  sprintf(filename, "results/Dilation-Process%d.png", my_rank);
  imwrite(filename, dilation_dst*255);

  erode(dilation_dst, erosion_dst, element);
  sprintf(filename, "results/Erosion-Process%d.png", my_rank);
  imwrite(filename, erosion_dst*255);


  absdiff(dilation_dst, erosion_dst, solution);
  // sprintf(filename, "results/PartialSolution-Process%d.png", my_rank);
  // imwrite(filename, solution*255);

  //Waiting for all slaves to conclude their operations
  MPI_Barrier(MPI_COMM_WORLD);

  int fragment = modality*element_size;
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

  /* Terminate MPI */
  MPI_Finalize();

  return 0;
}
