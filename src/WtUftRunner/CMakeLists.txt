cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYUftRunner)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

IF(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
ENDIF()

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYUftCore XYYTools XYYUtils)

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

# file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/parsers/)
# file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/traders/)

# add_custom_command(TARGET WtUftRunner POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderCTP${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/traders/
    
#     COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserCTP${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/parsers/

# 	COMMENT "depends modules copied")
