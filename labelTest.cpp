
  // serial code

  #include <iostream>
  #include <stdio.h>
  #include <stdlib.h>

  #define N 7 //dimension test matrix

  int A[N][N]; //main matrix
  bool C[N][N]; // check matrix
  int LABELS[4][N]; // border matrix

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

  // find correct maze track to follow 
  int checkSolutionLabel(int matrix[4][N]){
    int correctLabel[N] = {};
    int count = 0;

    for(int i = 0; i < 4; i++){
      for(int j = 0; j < N; j++){
        if(matrix[i][j] != 0) {
          correctLabel[matrix[i][j]+1]++;
        }
      }
    }

    for(int i = 0; i < N; i++){
      if(correctLabel[i] > 1) return i-1;
    }
    return 0;
  }

  int main(){

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
        A[2][5] = 0;
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
        }
    printMatrix(A);


  }

