cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYPorter)

file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)

if(MSVC)
	list (APPEND SRCS ../Common/mdump.cpp)
endif()

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only XYYCore XYYTools XYYUtils)

if(MSVC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  ws2_32)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(GNUCC)
	target_link_libraries(${PROJECT_NAME} PRIVATE  dl pthread Boost::filesystem Boost::thread)
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1 LINK_FLAGS_RELEASE -s)
	IF(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE  ws2_32 iconv)
	ENDIF()
endif()

# file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/parsers/)
# file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/traders/)
# file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH}/executer/)

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserUDP${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/
	
# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}ParserShm${SUFFIX} ${LIBRARY_OUTPUT_PATH}/parsers/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderCTP${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderXTP${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderMocker${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderCTPMini${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderCTPOpt${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}TraderFemas${SUFFIX} ${LIBRARY_OUTPUT_PATH}/traders/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtExeFact${SUFFIX} ${LIBRARY_OUTPUT_PATH}/executer/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtDataStorage${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtDataStorageAD${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtRiskMonFact${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 	COMMAND ${CMAKE_COMMAND} -E
# 	copy ${CMAKE_BINARY_DIR}/build_${PLATFORM}/${CMAKE_BUILD_TYPE}/bin/${PREFIX}WtMsgQue${SUFFIX} ${LIBRARY_OUTPUT_PATH}/

# 	COMMENT "depends modules copied")
