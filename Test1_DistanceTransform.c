// distance trasnform between 2 points in a matrix of ones and zeros (simulate binary image)

#include <stdio.h> 
#include <stdlib.h>

int A[100][100];

int main(){
    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 100; j++){
            A[i][j] = rand()%2;
        }
    }
	 for(int i = 0; i < 100; i++){
        for(int j = 0; j < 100; j++){
            printf("%d ", A[i][j]);
        }
		prinftf("\n");
    }
}