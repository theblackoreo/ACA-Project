# ACA-Project

 The aim of the algorithm is solving a maze which has some specific properties:
 - The maze must be extracted from a binary or greyscale image;
 - The maze must have only one entrance and one exit.
 - The maze must be a “perfect maze”, this means that only one path solves the labyrinth;

To reach the aim mentioned above the OpenCV library is used given that it provides optimised operations to deal with images and matrices.

## Serial Mze Solver Algorithm

 PRE-REQUISITES:

 Library:

 OpenCV installed in own computer

 HOW TO RUN

  - step 1: Clone the repository with "git clone" or download the folder from this page
  - step 2: execute the command 'ulimit -s unlimited', it is necessary to increase stack for very big matrices (until 72 billion of pixels)
  - step 3: Compile the program with a c++ compiler (Ubuntu: g++ mazeSolver.cpp -o mazeSolver -std=c++11 $(pkg-config --cflags --libs opencv))
  - step 4: Run the program specifing the image source. (i.e Ubuntu: ./mazeSolver test5.png)

## Parallel implementation
 
 PRE-REQUISITES:

 Library:

 * OpenCV installed
 * OpenMPI installed

HOW TO RUN

  - step 1: Clone the repository with "git clone" or download the folder from this page
  - step 3: execute the command 'ulimit -s unlimited', it is necessary to increase stack for very big matrices (until 72 billion of pixels)
  - step 4: Compile the program with a mpic++ compiler (Ubuntu: mpic++ --std=c++11 $(pkg-config --cflags --libs opencv)) googleFinal.cpp -o googlefinal
  - step 5: Run the program specifing the image source. (i.e Ubuntu: mpirun -n "ncores" ./googlefinal test5.png)

In this file is discussed a performace analysis in terms of speedup and timing focusing on running parallel implementation using Google Cloud Cluster of VMs. 




 
 

 
