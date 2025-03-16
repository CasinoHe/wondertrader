cmake_minimum_required(VERSION 3.28.3)

project(XYYCore)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only 
                                              Boost::asio 
                                              Boost::interprocess
                                              Boost::circular_buffer
                                              )
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
endif(MSVC)