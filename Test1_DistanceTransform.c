// distance trasnform between 2 points in a matrix of ones and zeros (simulate binary image)

#include <stdio.h> 

int A[100][100];



int main(){

    for(int j = 0; j < 100; j++){
        for(int i = 0; i < 100; i++){
            if(i%2 != 0){ 
                A[i][j] = 1;
            }
        }
    }
}