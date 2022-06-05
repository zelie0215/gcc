cmake_minimum_required(VERSION 3.21)
project(C)

set(CMAKE_CXX_STANDARD 14)

set(LNC_DIR D:/C++/curl-7.81.0/include)
include_directories(${LNC_DIR})
set(LINK_DIR D:/C++/curl-7.81.0/lib)
link_directories(${LINK_DIR})


add_executable(C main.cpp 排序/1.1.冒泡排序.cpp 排序/1.2.选择排序.cpp)
