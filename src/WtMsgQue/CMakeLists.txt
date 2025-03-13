cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYMsgQueue)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE nanomsg ws2_32 mswsock)
    # target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GNUCC)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1 LINK_FLAGS_RELEASE -s)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread nanomsg ws2_32 mswsock)
	elseif(UNIX)
		target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread nanomsg)
	endIF(WIN32)
endif(MSVC)