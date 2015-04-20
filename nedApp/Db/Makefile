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
DB += nEDCommon.db
DB += OccPortDriver.db
DB += DiscoverPlugin.db
DB += RocHv.template
DB += ROCHV.db
DB += ROCHVSumBL16b.db
DB += DumpPlugin.db
DB += StatPlugin.db
DB += RtdlPlugin.db
DB += AdaraPlugin.db
DB += CmdDispatcherPlugin.db
DB += DebugPlugin.db
DB += FlatFieldPlugin.db
DB += ProxyPlugin.db
DB += PixelMapPlugin.db
DB += RocPvaPlugin.db
DB += AcpcPvaPlugin.db
DB += GlobalConG.db
DB += ApplyConfig.db

DB += DspPlugin_v63.db
DB += DspPlugin_v64.db
DB += FemPlugin_v32.db
DB += FemPlugin_v35.db
DB += FemPlugin_v36.db
DB += FemPlugin_v37.db
DB += AcpcFemPlugin_v22.db
DB += RocPlugin_v44.db
DB += RocPlugin_v45.db
DB += RocPlugin_v47.db
DB += RocPlugin_v51.db
DB += RocPlugin_v52.db
DB += RocPlugin_v54.db
DB += RocPlugin_v56.db
DB += RocPlugin_v57.db
DB += AdcRocPlugin_v02.db
DB += AdcRocPlugin_v03.db
DB += AcpcPlugin_v144.db
DB += ArocPlugin_v23.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
#

DspPlugin_v63.template: DspPlugin_v63.generated
DspPlugin_v64.template: DspPlugin_v64.generated
FemPlugin_v32.template: FemPlugin_v32.generated
FemPlugin_v35.template: FemPlugin_v35.generated
FemPlugin_v36.template: FemPlugin_v36.generated
FemPlugin_v37.template: FemPlugin_v37.generated
AcpcFemPlugin_v22.template: AcpcFemPlugin_v22.generated
RocPlugin_v44.template: RocPlugin_v45.generated
RocPlugin_v45.template: RocPlugin_v45.generated
RocPlugin_v47.template: RocPlugin_v47.generated
RocPlugin_v51.template: RocPlugin_v51.generated
RocPlugin_v52.template: RocPlugin_v52.generated
RocPlugin_v54.template: RocPlugin_v54.generated
RocPlugin_v56.template: RocPlugin_v56.generated
RocPlugin_v57.template: RocPlugin_v57.generated
AdcRocPlugin_v02.template: AdcRocPlugin_v02.generated
AdcRocPlugin_v03.template: AdcRocPlugin_v03.generated
AcpcPlugin_v144.template: AcpcPlugin_v144.generated
ArocPlugin_v23.template: ArocPlugin_v23.generated

%.generated: ../src/%.cpp
	../export2epics.py -i $^ -o $@
