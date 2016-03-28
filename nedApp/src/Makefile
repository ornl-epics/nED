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
#USR_CXXFLAGS += -DMAX_PACKET_LEN=36000 # Uncomment when dealing with legacy DSP
USR_CXXFLAGS += -std=c++0x

#USR_CXXFLAGS += -fopenmp
#USR_LDFLAGS  += -fopenmp
CXXFLAGS = -Wall $(USR_CXXFLAGS)
CXXFLAGS += -g -ggdb -O0
#CXXFLAGS += -O2 # Improve std::vector iteration times by factor 30 or so

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
$(PROD_NAME)_SRCS  += ValueConvert.cpp
$(PROD_NAME)_SRCS  += Timer.cpp
$(PROD_NAME)_SRCS  += Thread.cpp
$(PROD_NAME)_SRCS  += OccPortDriver.cpp
$(PROD_NAME)_SRCS  += BaseCircularBuffer.cpp
$(PROD_NAME)_SRCS  += DmaCircularBuffer.cpp
$(PROD_NAME)_SRCS  += CircularBuffer.cpp
$(PROD_NAME)_SRCS  += DmaCopier.cpp
$(PROD_NAME)_SRCS  += DasPacket.cpp
$(PROD_NAME)_SRCS  += DasPacketList.cpp
$(PROD_NAME)_SRCS  += BasePlugin.cpp
$(PROD_NAME)_SRCS  += BaseSocketPlugin.cpp
$(PROD_NAME)_SRCS  += ProxyPlugin.cpp
$(PROD_NAME)_SRCS  += AdaraPlugin.cpp
$(PROD_NAME)_SRCS  += BaseDispatcherPlugin.cpp
$(PROD_NAME)_SRCS  += CmdDispatcher.cpp
$(PROD_NAME)_SRCS  += BaseModulePlugin.cpp
$(PROD_NAME)_SRCS  += DspPlugin.cpp
$(PROD_NAME)_SRCS  += DspPlugin_v51.cpp
$(PROD_NAME)_SRCS  += DspPlugin_v63.cpp
$(PROD_NAME)_SRCS  += DspPlugin_v64.cpp
$(PROD_NAME)_SRCS  += DspPlugin_v65.cpp
$(PROD_NAME)_SRCS  += RocPlugin.cpp
$(PROD_NAME)_SRCS  += AdcRocPlugin.cpp
$(PROD_NAME)_SRCS  += AdcRocPlugin_v02.cpp
$(PROD_NAME)_SRCS  += AdcRocPlugin_v03.cpp
$(PROD_NAME)_SRCS  += BnlRocPlugin.cpp
$(PROD_NAME)_SRCS  += BnlRocPlugin_v00.cpp
$(PROD_NAME)_SRCS  += BnlRocPlugin_v20.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v14.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v45.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v47.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v51.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v52.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v54.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v56.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v57.cpp
$(PROD_NAME)_SRCS  += RocPlugin_v58.cpp
$(PROD_NAME)_SRCS  += FemPlugin.cpp
$(PROD_NAME)_SRCS  += FemPlugin_v32.cpp
$(PROD_NAME)_SRCS  += FemPlugin_v35.cpp
$(PROD_NAME)_SRCS  += FemPlugin_v36.cpp
$(PROD_NAME)_SRCS  += FemPlugin_v37.cpp
$(PROD_NAME)_SRCS  += FemPlugin_v38.cpp
$(PROD_NAME)_SRCS  += AcpcFemPlugin.cpp
$(PROD_NAME)_SRCS  += AcpcFemPlugin_v22.cpp
$(PROD_NAME)_SRCS  += DiscoverPlugin.cpp
$(PROD_NAME)_SRCS  += DumpPlugin.cpp
$(PROD_NAME)_SRCS  += StatPlugin.cpp
$(PROD_NAME)_SRCS  += RtdlPlugin.cpp
$(PROD_NAME)_SRCS  += DebugPlugin.cpp
$(PROD_NAME)_SRCS  += FlatFieldPlugin.cpp
$(PROD_NAME)_SRCS  += FlatFieldTable.cpp
$(PROD_NAME)_SRCS  += PvaNeutronData.cpp
$(PROD_NAME)_SRCS  += BasePvaPlugin.cpp
$(PROD_NAME)_SRCS  += AcpcPvaPlugin.cpp
$(PROD_NAME)_SRCS  += RocPvaPlugin.cpp
$(PROD_NAME)_SRCS  += BnlRocPvaPlugin.cpp
$(PROD_NAME)_SRCS  += PvaMetaData.cpp
$(PROD_NAME)_SRCS  += PvaCalcVerifyData.cpp
$(PROD_NAME)_SRCS  += PixelMapPlugin.cpp
$(PROD_NAME)_SRCS  += AcpcPlugin.cpp
$(PROD_NAME)_SRCS  += AcpcPlugin_v144.cpp
$(PROD_NAME)_SRCS  += ArocPlugin.cpp
$(PROD_NAME)_SRCS  += ArocPlugin_v23.cpp
$(PROD_NAME)_SRCS  += TimingPlugin.cpp
$(PROD_NAME)_SRCS  += BnlFlatFieldPlugin.cpp
$(PROD_NAME)_SRCS  += BnlPosCalcPlugin.cpp

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
