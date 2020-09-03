TOP=../../..

include $(TOP)/configure/CONFIG

PROD_LIBS += Com
PROD_LIBS += asyn
PROD_LIBS += ned

CXXFLAGS = -std=c++0x -g -ggdb

USR_INCLUDES += -I../..

# Tests for the parameters
TESTPROD_HOST += testCircularBuffer
TESTPROD_HOST += testValueConvert
TESTPROD_HOST += testObjectPool
testCircularBuffer_SRCS += testCircularBuffer.cpp
testValueConvert_SRCS += testValueConvert.cpp
testObjectPool_SRCS += testObjectPool.cpp
TESTS += testCircularBuffer
TESTS += testValueConvert
TESTS += testObjectPool

TESTSCRIPTS_HOST += $(TESTS:%=%.t)

include $(TOP)/configure/RULES

test:
	sh $(EPICS_HOST_ARCH)/
