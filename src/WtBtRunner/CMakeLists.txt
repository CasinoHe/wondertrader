cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYBtRunner)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

IF(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
ENDIF()

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYBtCore XYYTools XYYUtils)

if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  ws2_32)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GCC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  dl pthread Boost::filesystem Boost::thread)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE iconv)
	endif(WIN32)
endif(MSVC)
