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

# --------------------- build lua start ---------------------------
set(LUA_ROOT "${THIRDPARTY_DIR}/lua")
file(GLOB LUA_SOURCES LIST_DIRECTORIES false ${LUA_ROOT}/*.c)
list(REMOVE_ITEM LUA_SOURCES ${LUA_ROOT}/lua.c ${LUA_ROOT}/onelua.c)
include_directories(${LUA_ROOT})
# --------------------- build lua end ---------------------------

# Compile into a static library and generate the library file
add_library(${PROJECT_NAME} STATIC ${SRCS} ${LUA_SOURCES} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE rapidjson Boost::locale Boost::pool utf8::cpp utf8cpp::utf8cpp)