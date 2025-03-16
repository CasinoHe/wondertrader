cmake_minimum_required(VERSION 3.28.3)

project(XYYTools)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.h")

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only fmt::fmt-header-only Boost::filesystem Boost::xpressive)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
endif(MSVC)