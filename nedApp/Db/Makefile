TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
DB += OccPortDriver.db
DB += DiscoverPlugin.db
DB += DspPlugin.db
DB += FemPlugin.db
DB += AcpcFemPlugin.db
DB += RocPlugin_v51.db
DB += RocPlugin_v52.db
DB += RocPlugin_v54.db
DB += RocPlugin_v56.db
DB += RB_DspPlugin.db
DB += RB_FemPlugin.db
DB += RB_RocPlugin_v51.db
DB += RB_RocPlugin_v52.db
DB += RB_RocPlugin_v54.db
DB += RB_RocPlugin_v56.db
DB += RocHv.template
DB += ROCHV.db
DB += ROCHVSumBL16b.db
DB += DumpPlugin.db
DB += StatPlugin.db
DB += RtdlPlugin.db
DB += AdaraPlugin.db
DB += CmdDispatcherPlugin.db
DB += GenericModulePlugin.db
DB += FlatFieldPlugin.db
DB += ProxyPlugin.db
DB += PixelMapPlugin.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>
#ROCHV_template += ../ROCHVCal.template

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
#
test: $(TOP)/db/ROCHV.db
	dbReadTest -I$(TOP)/db -I$(TOP)/dbd -SP=test:area:L -SPORT=any -SADDR=0 -SG=test:area:G ned.dbd ROCHV.db

