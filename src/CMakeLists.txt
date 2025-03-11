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

# Set the project root directory
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
get_filename_component(PROJECT_ROOT_DIR ${PROJECT_ROOT_DIR} ABSOLUTE)
set(SRC_ROOT_DIR ${PROJECT_ROOT_DIR}/src)

include_directories(${SRC_ROOT_DIR}/Includes)

#basic libraries
add_subdirectory(WTSUtils)
# ADD_SUBDIRECTORY(WTSTools)

#contract Loade through ctp channel
# ADD_SUBDIRECTORY(CTPLoader)
# ADD_SUBDIRECTORY(CTPOptLoader)
# ADD_SUBDIRECTORY(LoaderRunner)

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

# #back test core library
# ADD_SUBDIRECTORY(WtBtCore)
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

# ADD_SUBDIRECTORY(WtMsgQue)
# ADD_SUBDIRECTORY(WtCore)
# ADD_SUBDIRECTORY(WtPorter)
# ADD_SUBDIRECTORY(WtExecMon)
# ADD_SUBDIRECTORY(WtRunner)

# ADD_SUBDIRECTORY(TraderDumper)

# ADD_SUBDIRECTORY(WtShareHelper)

# #uft
# ADD_SUBDIRECTORY(WtUftCore)
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
