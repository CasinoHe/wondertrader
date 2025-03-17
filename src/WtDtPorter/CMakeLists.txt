cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYDtPorter)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

if(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
endif()

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYUtils XYYTools XYYDtCore)

if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  ws2_32)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GCC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  dl pthread)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1 LINK_FLAGS_RELEASE -s)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE iconv)
	endif(WIN32)
endif(MSVC)

# file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/parsers/)
# add_custom_command(TARGET WtDtPorter POST_BUILD
# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserCTP${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/

# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserXTP${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/

# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserFemas${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/

# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserCTPMini${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/

# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtDataStorage${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 		COMMAND ${CMAKE_COMMAND} -E
# 		copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtDataStorageAD${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 		COMMENT "depends modules copied")
		