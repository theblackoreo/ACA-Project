# ACA-Project

 The aim of the algorithm is solving a maze which has some specific properties:
 - The maze must be extracted from a binary or greyscale image;
 - The maze must have only one entrance and one exit.
 - The maze must be a “perfect maze”, this means that only one path solves the labyrinth;

To reach the aim mentioned above the OpenCV library is used given that it provides optimised operations to deal with images and matrices.

## Serial Maze Solver

 PRE-REQUISITES:

 Library:

 OpenCV installed in own computer

 HOW TO RUN

  - step 1: Clone the repository with "git clone" or download the folder from this page
  - step 2: Compile the program with a c++ compiler (Ubuntu: g++ mazeSolver.cpp -o mazeSolver -std=c++11 $(pkg-config --cflags --libs opencv))
  - step 3: Run the program specifing the image source and the modality. The modality indicates how large must be the element size depending on the original image. So  the suggestion is to test the program using both 1 and 2 modality and choose the best result. ./mazeSolver ImgSRC modality(1 or 2). (Ubuntu: ./mazeSolver test6.png  1)
