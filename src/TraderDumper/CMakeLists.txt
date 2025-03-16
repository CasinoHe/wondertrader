cmake_minimum_required(VERSION 3.28.3)

PROJECT(TraderDumper)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

IF(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
ENDIF()

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYTools XYYUtils)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GCC)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1 LINK_FLAGS_RELEASE -s)
	target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread Boost::filesystem Boost::thread)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 iconv)
	endif(WIN32)
ENDIF(MSVC)
