cmake_minimum_required(VERSION 3.28.3)

project(XYYTools)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.h")

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog fmt::fmt Boost::filesystem Boost::xpressive)