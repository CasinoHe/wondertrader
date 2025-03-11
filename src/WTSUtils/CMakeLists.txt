cmake_minimum_required(VERSION 3.28.3)

project(XYYUtils)

# Add source code
# TODO: 2025.03.11, link all third-party libraries using submodule or vcpkg for better maintenance
file(GLOB SRCS "*.cpp" "./zstdlib/*.c" "./yamlcpp/*.cpp" "./lmdb/*.c")
file(GLOB HDRS "*.h" "./zstdlib/*.h")

if(MSVC)
    list(APPEND SRCS "./StackTracer/StackTracer.cpp" "./StackTracer/StackWalker.cpp")
elseif(GCC)
    list(APPEND SRCS "./StackTracer/StackTracer.cpp")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/yamlcpp)

# Compile into a static library and generate the library file
add_library(XYYUtils STATIC ${SRCS} ${HDRS})