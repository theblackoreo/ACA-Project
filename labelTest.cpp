
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

  // check near bytes from a specific position
  bool checkNearByte(int label, int r, int c, Mat src){

    if(C[r][c] == true) return false;
    else C[r][c] = true;
    int count = 0;
    bool cond;

        if( src.at<unsigned char>(r+1,c)==1){
          src.at<unsigned char>(r+1,c) = label;
          checkNearByte(label, r+1, c, src);
          cond = true;
        }

        if( src.at<unsigned char>(r-1,c)==1){
          src.at<unsigned char>(r-1,c) = label;
          cond = checkNearByte(label, r-1, c, src);
          cond = true;
        }

        if(src.at<unsigned char>(r,c+1)==1){
          src.at<unsigned char>(r,c+1) = label;
          cond = checkNearByte(label, r, c+1, src);
          cond = true;
        }

        if(src.at<unsigned char>(r,c-1)==1){
          src.at<unsigned char>(r,c-1) = label;
          cond = checkNearByte(label, r, c-1, src);
          cond = true;
        }

      if(count == 0) return true;
      return cond;

  }

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
    moveWindow( "Dilation Demo", N, 0 );
    moveWindow( "Source", N, 0 );

    //show original image
    imshow( "Source", src*255);

    //print original image
  //  cout << "src = " << endl << " "  << src << endl << endl;

    int rows = src.rows;
    int cols = src.cols;
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
    for(int i = 1; i < rows-1; i++){
        for(int j = 1; j < cols-1; j++){
            if (src.at<unsigned char>(i,j) == 1){
               if(C[i][j]== false) src.at<unsigned char>(i,j) = label;
              if(checkNearByte(label, i, j, src) == true) label++;
            }
        }
    }

    cout << "src = " << endl << " "  << src << endl << endl;
    imshow( "Dilation Demo", src*50);
    //int LABELS[][] = {};
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

    namedWindow( "DilationOK", WINDOW_AUTOSIZE );


    for(int i = 0; i<rows; i++)
        for(int j = 0 ; j<cols; j++){
        if(src.at<unsigned char>(i,j) != trackToFollow)
            src.at<unsigned char>(i,j)= 0;
            else {
              src.at<unsigned char>(i,j)= 1;
            }
        }


    Dilation(0, 0, src, dilation_dst);


    //imshow( "DilationOK", dilation_dst );

    cout << "dilation_dst = " << endl << " "  << dilation_dst << endl << endl;

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
