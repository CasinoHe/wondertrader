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
find_package(Boost 1.86.0 REQUIRED COMPONENTS program_options algorithm pool locale filesystem xpressive asio interprocess circular_buffer)
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
add_subdirectory(WtBtCore)

# executable file
add_subdirectory(LoaderRunner)

# uft engine
# add_subdirectory(WtUftCore)

#contract Loade through ctp channel
# ADD_SUBDIRECTORY(CTPLoader)
# ADD_SUBDIRECTORY(CTPOptLoader)
add_subdirectory(TraderDumper)
add_subdirectory(WtShareHelper)

# #parser for quote date from ctp channel
# ADD_SUBDIRECTORY(ParserCTP)
# ADD_SUBDIRECTORY(ParserCTPMini)
# ADD_SUBDIRECTORY(ParserCTPOpt)
# ADD_SUBDIRECTORY(ParserFemas)
# ADD_SUBDIRECTORY(ParserXTP)
# ADD_SUBDIRECTORY(ParserShm)
# ADD_SUBDIRECTORY(ParserXeleSkt)
# ADD_SUBDIRECTORY(WtDataStorage)
# ADD_SUBDIRECTORY(WtDataStorageAD)
# ADD_SUBDIRECTORY(WtDtCore)
# ADD_SUBDIRECTORY(WtDtHelper)
# ADD_SUBDIRECTORY(WtDtPorter)
# ADD_SUBDIRECTORY(WtDtServo)
# #quote date processor
# ADD_SUBDIRECTORY(QuoteFactory)

# #back test c porter for outter libraries
# ADD_SUBDIRECTORY(WtBtPorter)
# ADD_SUBDIRECTORY(WtBtRunner)

# ADD_SUBDIRECTORY(ParserUDP)
# ADD_SUBDIRECTORY(TraderCTP)
# ADD_SUBDIRECTORY(TraderCTPMini)
# ADD_SUBDIRECTORY(TraderCTPOpt)
# ADD_SUBDIRECTORY(TraderFemas)
# ADD_SUBDIRECTORY(TraderMocker)
# ADD_SUBDIRECTORY(TraderXTP)
# ADD_SUBDIRECTORY(TraderYD)
# ADD_SUBDIRECTORY(WtExeFact)
# ADD_SUBDIRECTORY(WtRiskMonFact)

# ADD_SUBDIRECTORY(WtPorter)
# ADD_SUBDIRECTORY(WtExecMon)
# ADD_SUBDIRECTORY(WtRunner)

# #uft
# ADD_SUBDIRECTORY(WtUftStraFact)
# ADD_SUBDIRECTORY(WtUftRunner)

# ADD_SUBDIRECTORY(WtLatencyHFT)
# ADD_SUBDIRECTORY(WtLatencyUFT)

# #test projects
# ADD_SUBDIRECTORY(TestBtPorter)
# ADD_SUBDIRECTORY(TestDtPorter)
# ADD_SUBDIRECTORY(TestExecPorter)
# ADD_SUBDIRECTORY(TestPorter)
# ADD_SUBDIRECTORY(TestTrader)
# ADD_SUBDIRECTORY(TestParser)
# ADD_SUBDIRECTORY(TestUnits)
