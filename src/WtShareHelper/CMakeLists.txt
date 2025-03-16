cmake_minimum_required(VERSION 3.28.3)

PROJECT(XYYShareHelper)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.h")

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only Boost::filesystem)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE "/MT")
elseif(UNIX)
	target_link_libraries(${PROJECT_NAME} PRIVATE dl)
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1 LINK_FLAGS_RELEASE -s)
endif(MSVC)
