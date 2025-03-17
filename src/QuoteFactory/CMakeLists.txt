cmake_minimum_required(VERSION 3.28.3)

PROJECT(QuoteFactory)

file(GLOB SRCS *.cpp)

IF(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
ENDIF()

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYBtCore XYYTools XYYUtils XYYDtCore Boost::filesystem)

if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  ws2_32)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GCC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  dl pthread Boost::thread)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 iconv)
	endif(WIN32)
endif(MSVC)


# file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/parsers/)
# add_custom_command(TARGET QuoteFactory POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserCTP${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/parsers/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserXTP${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/parsers/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserFemas${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/parsers/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserCTPMini${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/parsers/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtDataStorage${SUFFIX} ${EXECUTABLE_OUTPUT_PATH}/

# 	COMMENT "depends modules copied")
