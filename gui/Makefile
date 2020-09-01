TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

SRC_DIR := ../nedApp/src
GEN_DIR := generated
SRC_FILES := $(wildcard $(SRC_DIR)/*Plugin_v*.cpp)
GEN_FILES := $(addprefix $(GEN_DIR)/, $(notdir $(SRC_FILES:.cpp=.bob)))

install: $(GEN_FILES)

$(GEN_DIR)/%.bob: $(SRC_DIR)/%.cpp
	./export2bob.py $< generated/

realclean:	clean

clean:
	rm -fR generated/
