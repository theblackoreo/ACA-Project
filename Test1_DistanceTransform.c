// distance trasnform between 2 points in a matrix of ones and zeros (simulate binary image)

#include <stdio.h> 
#include <stdlib.h>

#define N 20

int A[N][N];

int main(){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            A[i][j] = rand()%2;
			if (i == j)
				A[i][j] = 1;
        }
    }
	 for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", A[i][j]);
        }
		printf("\n");
    }

    //test

}