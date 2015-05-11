TOP = ../..
include $(TOP)/configure/CONFIG
ARCH = linux-x86_64
TARGETS = envPaths
include $(TOP)/configure/RULES.ioc

detectors: detectors.substitution detectors.template
	msi -S detectors.substitution -o $@ detectors.template
