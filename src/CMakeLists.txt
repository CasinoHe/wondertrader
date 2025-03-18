CMAKE_MINIMUM_REQUIRED(VERSION 3.28.3)

# disable FindBoost module removed warning
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# project name
PROJECT(XYYTrader)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TEST "Build test projects" OFF)

# Fix for ambiguous 'byte' symbol when compiling with MSVC
if(MSVC)
  add_definitions(-D_HAS_STD_BYTE=0)
  # Add Windows API version target for Windows 10/11
  add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# Set the project root directory
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
get_filename_component(PROJECT_ROOT_DIR ${PROJECT_ROOT_DIR} ABSOLUTE)
set(SRC_ROOT_DIR ${PROJECT_ROOT_DIR}/src)

# include directories
include_directories(${SRC_ROOT_DIR}/Includes)

# find all dependencies and add them to the project
# the dependencies is built by vcpkg or custom build
set(THIRDPARTY_DIR ${PROJECT_ROOT_DIR}/thirdparties)
find_package(Boost 1.86.0 REQUIRED COMPONENTS program_options algorithm pool locale filesystem xpressive asio interprocess circular_buffer property_tree)
find_package(RapidJSON CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(nanomsg CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

#basic libraries
add_subdirectory(WTSUtils)
add_subdirectory(WTSTools)

# uft engine
add_subdirectory(WtUftCore)
add_subdirectory(WtUftRunner)

# tools
add_subdirectory(CTPLoader)
add_subdirectory(CTPOptLoader)
add_subdirectory(Traders/TraderDumper)
add_subdirectory(LoaderRunner)
add_subdirectory(WtMsgQue)
add_subdirectory(WtShareHelper)

# product
add_subdirectory(WtCore)
add_subdirectory(WtExecMon)
add_subdirectory(WtPorter)
add_subdirectory(WtRunner)

# plugins
add_subdirectory(WtCtaStraFact)
add_subdirectory(WtUftStraFact)
add_subdirectory(WtSelStraFact)
add_subdirectory(WtHftStraFact)
add_subdirectory(WtExeFact)
add_subdirectory(WtRiskMonFact)

# Parsers
add_subdirectory(Parsers/ParserCTP)
add_subdirectory(Parsers/ParserCTPMini)
add_subdirectory(Parsers/ParserCTPOpt)
add_subdirectory(Parsers/ParserFemas)
add_subdirectory(Parsers/ParserHuaX)
# add_subdirectory(Parsers/ParserOES)
add_subdirectory(Parsers/ParserShm)
add_subdirectory(Parsers/ParserUDP)
add_subdirectory(Parsers/ParserXeleSkt)
add_subdirectory(Parsers/ParserXTP)
add_subdirectory(Parsers/ParserYD)

# DataKit
add_subdirectory(QuoteFactory)
add_subdirectory(WtDataStorage)
add_subdirectory(WtDataStorageAD)
add_subdirectory(WtDtCore)
add_subdirectory(WtDtHelper)
add_subdirectory(WtDtPorter)
add_subdirectory(WtDtServo)

# backtest
add_subdirectory(WtBtCore)
add_subdirectory(WtBtPorter)
add_subdirectory(WtBtRunner)

# traders
# add_subdirectory(Traders/TraderATP)
add_subdirectory(Traders/TraderCTP)
add_subdirectory(Traders/TraderCTPMini)
add_subdirectory(Traders/TraderCTPOpt)
# add_subdirectory(Traders/TraderDD)
add_subdirectory(Traders/TraderFemas)
add_subdirectory(Traders/TraderHuaX)
add_subdirectory(Traders/TraderMocker)
# add_subdirectory(Traders/TraderOES)
add_subdirectory(Traders/TraderXTP)
add_subdirectory(Traders/TraderXTPXAlgo)
add_subdirectory(Traders/TraderYD)

add_subdirectory(WtLatencyHFT)
add_subdirectory(WtLatencyUFT)

# test projects
if (BUILD_TEST)
  add_subdirectory(test/TestBtPorter)
  add_subdirectory(test/TestDtPorter)
  add_subdirectory(test/TestExecPorter)
  add_subdirectory(test/TestPorter)
  add_subdirectory(test/TestTrader)
  add_subdirectory(test/TestParser)
  # add_subdirectory(test/TestUnits)
endif (BUILD_TEST)