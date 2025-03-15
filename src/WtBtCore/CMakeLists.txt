cmake_minimum_required(VERSION 3.28.3)

project(XYYBtCore)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})
