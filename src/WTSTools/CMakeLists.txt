cmake_minimum_required(VERSION 3.28.3)

project(XYYTools)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.h")

add_library(XYYTools STATIC ${SRCS} ${HDRS})