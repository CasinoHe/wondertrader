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
add_subdirectory(WtMsgQue)
add_subdirectory(WtCore)

# executable file
add_subdirectory(LoaderRunner)

# uft engine
add_subdirectory(WtUftCore)

#contract Loade through ctp channel
add_subdirectory(CTPLoader)
add_subdirectory(CTPOptLoader)
add_subdirectory(TraderDumper)
add_subdirectory(WtShareHelper)

# backtesting and live trading
add_subdirectory(WtBtCore)
add_subdirectory(WtBtPorter)
add_subdirectory(WtBtRunner)

# #parser for quote date from ctp channel
add_subdirectory(ParserCTP)
add_subdirectory(ParserCTPMini)
add_subdirectory(ParserCTPOpt)
add_subdirectory(ParserFemas)
add_subdirectory(ParserHuaX)
# add_subdirectory(ParserOES)
add_subdirectory(ParserShm)
add_subdirectory(ParserUDP)
add_subdirectory(ParserXeleSkt)
add_subdirectory(ParserXTP)
add_subdirectory(ParserYD)
add_subdirectory(WtDataStorage)
add_subdirectory(WtDataStorageAD)
add_subdirectory(WtDtCore)
add_subdirectory(WtDtHelper)
add_subdirectory(WtDtPorter)
add_subdirectory(WtDtServo)
# #quote date processor
add_subdirectory(QuoteFactory)

add_subdirectory(TraderATP)
# add_subdirectory(TraderCTP)
# add_subdirectory(TraderCTPMini)
# add_subdirectory(TraderCTPOpt)
# add_subdirectory(TraderDD)
# add_subdirectory(TraderFemas)
# add_subdirectory(TraderHuaX)
# add_subdirectory(TraderMocker)
# add_subdirectory(TraderOES)
# add_subdirectory(TraderXTP)
# add_subdirectory(TraderXTPXAlgo)
# add_subdirectory(TraderYD)
add_subdirectory(WtRiskMonFact)

add_subdirectory(WtPorter)
add_subdirectory(WtRunner)

add_subdirectory(WtExeFact)
add_subdirectory(WtExecMon)

# #uft
add_subdirectory(WtUftStraFact)
add_subdirectory(WtUftRunner)

add_subdirectory(WtLatencyHFT)
add_subdirectory(WtLatencyUFT)

# #test projects
# ADD_SUBDIRECTORY(TestBtPorter)
# ADD_SUBDIRECTORY(TestDtPorter)
# ADD_SUBDIRECTORY(TestExecPorter)
# ADD_SUBDIRECTORY(TestPorter)
# ADD_SUBDIRECTORY(TestTrader)
# ADD_SUBDIRECTORY(TestParser)
# ADD_SUBDIRECTORY(TestUnits)
