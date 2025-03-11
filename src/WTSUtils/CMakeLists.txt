cmake_minimum_required(VERSION 3.28.3)

project(XYYUtils)

LIST(APPEND INCS "./yamlcpp")

#7. Add source code
file(GLOB SRCS "*.cpp" "./zstdlib/*.c" "./yamlcpp/*.cpp" "./lmdb/*.c")
file(GLOB HDRS "*.h" "./zstdlib/*.h")

IF(MSVC)
LIST(APPEND SRCS "./StackTracer/StackTracer.cpp" "./StackTracer/StackWalker.cpp")
ELSE (GNUCC)
LIST(APPEND SRCS "./StackTracer/StackTracer.cpp")
ENDIF()

INCLUDE_DIRECTORIES(${INCS})

#8. Compile into a static library and generate the library file
add_library(WTSUtils STATIC ${SRCS} ${HDRS})