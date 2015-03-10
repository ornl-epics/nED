TOP = ..
include $(TOP)/configure/CONFIG
# Don't generate documentation by default since it depends on external tools
#DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *doc*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Src*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *db*))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Db*))
include $(TOP)/configure/RULES_DIRS

