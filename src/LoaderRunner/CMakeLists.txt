cmake_minimum_required(VERSION 3.28.3)

PROJECT(LoaderRunner)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} XYYUtils XYYTools XYYBtCore fmt::fmt-header-only)

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif (GCC)
	target_link_libraries(${PROJECT_NAME} dl)
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} iconv)
	endif(WIN32)
endif(MSVC)
