
cmake_minimum_required(VERSION 3.3)
project(ACA-Project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(Test1_DistanceTransform ../../CLionProjects/ACA-Project/Test1_DistanceTransform.cpp)

# linking
target_link_libraries(Test1_DistanceTransform ${OpenCV_LIBS})