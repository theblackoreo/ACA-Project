#!/bin/bash

c++ Test1_DistanceTransform.cpp -o executable -std=c++11 -lopencv_highgui -lopencv_imgproc -lopencv_dnn -lopencv_core -I /usr/local/include/opencv4

./executable
