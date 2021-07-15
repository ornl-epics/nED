TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC support library

#=============================
# Build the IOC application

OCCLIB_DIR=$(OCCLIB)
R_INCLUDES = -I$(OCCLIB_DIR)
occ_DIR = $(OCCLIB_DIR)

USR_CXXFLAGS += -DBITFIELD_LSB_FIRST
USR_CXXFLAGS += -std=c++0x -g -ggdb -O0
#USR_LDFLAGS  += -fopenmp

PROD_IOC = example
# example.dbd will be created and installed
DBD += example.dbd

# example.dbd will be made up from these files:
example_DBD += base.dbd
example_DBD += nedSupport.dbd
example_DBD += asSupport.dbd
example_DBD += busySupport.dbd
example_DBD += pvMediatorSupport.dbd

example_LIBS += ned
example_LIBS += asyn
example_LIBS += occ
example_LIBS += autosave
example_LIBS += stream
example_LIBS += pvDatabase
#example_LIBS += pvaSrv
example_LIBS += pvAccess
example_LIBS += pvData
example_LIBS += seq
example_LIBS += pvMediator
example_LIBS += busy
example_LIBS += calc

# example_registerRecordDeviceDriver.cpp derives from example.dbd
example_SRCS += example_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
example_SRCS_DEFAULT += exampleMain.cpp
example_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#example_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Link in the code from the support library
#example_LIBS += exampleSupport

# NOTE: To build SNL programs, SNCSEQ must be defined
# in the <top>/configure/RELEASE file

# Finally link to the EPICS Base libraries
example_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

