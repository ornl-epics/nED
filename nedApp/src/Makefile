TOP=../..

OCCLIB_DIR=$(OCCLIB)

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Split build into static library and app for unit tests
# to pick-up functionlity from library.

# The C/C++ bitfield ordering in structs is platform AND compiler dependant
#
USR_CXXFLAGS += -DBITFIELD_LSB_FIRST
USR_CXXFLAGS += -std=c++11

#USR_CXXFLAGS += -fopenmp
#USR_LDFLAGS  += -fopenmp
CXXFLAGS = -Wall $(USR_CXXFLAGS)
#CXXFLAGS += -g -ggdb -O0
CXXFLAGS += -O2 # Improve std::vector iteration times by factor 30 or so

# Product
PROD_NAME = ned
PROD_IOC = $(PROD_NAME)

USR_INCLUDES = -I$(OCCLIB_DIR)
occ_DIR = $(OCCLIB_DIR)

DBD += $(PROD_NAME).dbd
$(PROD_NAME)_DBD += nedSupport.dbd
$(PROD_NAME)_DBD += asSupport.dbd
DBD += nedSupport.dbd

PROD_LIBS += ned
PROD_LIBS += asyn
PROD_LIBS += occ
PROD_LIBS += autosave
PROD_LIBS += stream
PROD_LIBS += pvMediator
PROD_LIBS += pvDatabase
PROD_LIBS += pvaSrv
PROD_LIBS += pvAccess
PROD_LIBS += pvData
PROD_LIBS += seq
PROD_LIBS += $(EPICS_BASE_IOC_LIBS)
PROD_LIBS += calc

PROD_SRCS  += ned_registerRecordDeviceDriver.cpp
PROD_SRCS  += main.cpp
PROD_SRCS  += GlobalCon.st
PROD_SRCS  += HVScan.st
PROD_SRCS  += ApplyConfig.st

# Static library
LIBRARY = ned
SHARED_LIBRARIES = NO

# Headers used by unit-tests
INC += CircularBuffer.h

LIB_SRCS  += GlobalCon.st
LIB_SRCS  += HVScan.st
LIB_SRCS  += ApplyConfig.st
$(PROD_NAME)_SRCS  += Common.cpp
$(PROD_NAME)_SRCS  += McsFile.cpp
$(PROD_NAME)_SRCS  += ValueConvert.cpp
$(PROD_NAME)_SRCS  += Timer.cpp
$(PROD_NAME)_SRCS  += Thread.cpp
$(PROD_NAME)_SRCS  += BasePortPlugin.cpp
$(PROD_NAME)_SRCS  += OccPlugin.cpp
$(PROD_NAME)_SRCS  += TcpClientPlugin.cpp
$(PROD_NAME)_SRCS  += FileReplayPlugin.cpp
$(PROD_NAME)_SRCS  += BaseCircularBuffer.cpp
$(PROD_NAME)_SRCS  += DmaCircularBuffer.cpp
$(PROD_NAME)_SRCS  += CircularBuffer.cpp
$(PROD_NAME)_SRCS  += FileCircularBuffer.cpp
$(PROD_NAME)_SRCS  += DmaCopier.cpp
$(PROD_NAME)_SRCS  += DasPacket.cpp
$(PROD_NAME)_SRCS  += Packet.cpp
$(PROD_NAME)_SRCS  += Event.cpp
$(PROD_NAME)_SRCS  += PluginMessage.cpp
$(PROD_NAME)_SRCS  += BasePlugin.cpp
$(PROD_NAME)_SRCS  += BaseSocketPlugin.cpp
#$(PROD_NAME)_SRCS  += ProxyPlugin.cpp
$(PROD_NAME)_SRCS  += AdaraPlugin.cpp
$(PROD_NAME)_SRCS  += BaseModulePlugin.cpp
$(PROD_NAME)_SRCS  += ModulesPlugin.cpp
$(PROD_NAME)_SRCS  += DumpPlugin.cpp
$(PROD_NAME)_SRCS  += StatPlugin.cpp
$(PROD_NAME)_SRCS  += RtdlPlugin.cpp
$(PROD_NAME)_SRCS  += Das1CommDebugPlugin.cpp
$(PROD_NAME)_SRCS  += CommDebugPlugin.cpp
$(PROD_NAME)_SRCS  += FlatFieldPlugin.cpp
$(PROD_NAME)_SRCS  += FlatFieldTable.cpp
$(PROD_NAME)_SRCS  += PvaNeutronsPlugin.cpp
$(PROD_NAME)_SRCS  += PixelMapPlugin.cpp
#$(PROD_NAME)_SRCS  += TofCorrectPlugin.cpp
#$(PROD_NAME)_SRCS  += BnlFlatFieldPlugin.cpp
$(PROD_NAME)_SRCS  += BnlPosCalcPlugin.cpp
#$(PROD_NAME)_SRCS  += CRocPosCalcPlugin.cpp
$(PROD_NAME)_SRCS  += StateAnalyzerPlugin.cpp
$(PROD_NAME)_SRCS  += TimeSync.cpp

# Module handler plugins - in alphabetical order
$(PROD_NAME)_SRCS  += AcpcFemPlugin.cpp
$(PROD_NAME)_SRCS  += AcpcPlugin.cpp
$(PROD_NAME)_SRCS  += AdcRocPlugin.cpp
$(PROD_NAME)_SRCS  += ArocPlugin.cpp
$(PROD_NAME)_SRCS  += BnlRocPlugin.cpp
$(PROD_NAME)_SRCS  += DspPlugin.cpp
$(PROD_NAME)_SRCS  += DspWPlugin.cpp
$(PROD_NAME)_SRCS  += CRocPlugin.cpp
$(PROD_NAME)_SRCS  += FemPlugin.cpp
$(PROD_NAME)_SRCS  += RocPlugin.cpp
$(PROD_NAME)_SRCS  += $(notdir $(wildcard ../*Plugin_v*.cpp))

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
