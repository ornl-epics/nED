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
DB += ROCHV.db
DB += ROCHVSumBL16b.db
DB += DumpPlugin.db
DB += StatPlugin.db
DB += RtdlPlugin.db
DB += AdaraPlugin.db
DB += CmdDispatcherPlugin.db
DB += DebugPlugin.db
DB += DataDiagPlugin.db
DB += FlatFieldPlugin.db
DB += FlatFieldPosition.db
DB += ProxyPlugin.db
DB += PixelMapPlugin.db
DB += RocPvaPlugin.db
DB += AcpcPvaPlugin.db
DB += BnlRocPvaPlugin.db
DB += GlobalConG.db
DB += ApplyConfig.db
DB += PulsedMagnet.db
DB += TimingPlugin.db
DB += TofCorrectPlugin.db
DB += TofCorrectPixels.template
DB += BnlFlatFieldPlugin.db
DB += BnlPosCalcPlugin.db
DB += CRocPosCalcPlugin.db
DB += CRocPvaPlugin.db

DB += DspPlugin_v51.db
DB += DspPlugin_v52.db
DB += DspPlugin_v63.db
DB += DspPlugin_v64.db
DB += DspPlugin_v65.db
DB += DspPlugin_v66.db
DB += DspWPlugin_v10.db
DB += FemPlugin_v32.db
DB += FemPlugin_v35.db
DB += FemPlugin_v36.db
DB += FemPlugin_v37.db
DB += FemPlugin_v38.db
DB += FemPlugin_v39.db
DB += AcpcFemPlugin_v14.db
DB += AcpcFemPlugin_v22.db
DB += RocPlugin_v14.db
DB += RocPlugin_v43.db
DB += RocPlugin_v44.db
DB += RocPlugin_v45.db
DB += RocPlugin_v47.db
DB += RocPlugin_v50.db
DB += RocPlugin_v51.db
DB += RocPlugin_v52.db
DB += RocPlugin_v54.db
DB += RocPlugin_v56.db
DB += RocPlugin_v57.db
DB += RocPlugin_v58.db
DB += AdcRocPlugin_v02.db
DB += AdcRocPlugin_v03.db
DB += AdcRocPlugin_v05.db
DB += BnlRocPlugin_v00.db
DB += BnlRocPlugin_v20.db
DB += AcpcPlugin_v144.db
DB += AcpcPlugin_v171.db
DB += AcpcPlugin_v172.db
DB += ArocPlugin_v23.db
DB += ArocPlugin_v24.db
DB += ArocPlugin_v255255.db
DB += CRocPmt.db
DB += CRocPlugin_v93.db
DB += CRocPlugin_v94.db


#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
#

DspPlugin_v51.template: DspPlugin_v51.generated
DspPlugin_v52.template: DspPlugin_v52.generated
DspPlugin_v63.template: DspPlugin_v63.generated
DspPlugin_v64.template: DspPlugin_v64.generated
DspPlugin_v65.template: DspPlugin_v65.generated
DspPlugin_v66.template: DspPlugin_v66.generated
DspWPlugin_v10.template: DspWPlugin_v10.generated
FemPlugin_v32.template: FemPlugin_v32.generated
FemPlugin_v35.template: FemPlugin_v35.generated
FemPlugin_v36.template: FemPlugin_v36.generated
FemPlugin_v37.template: FemPlugin_v37.generated
FemPlugin_v38.template: FemPlugin_v38.generated
FemPlugin_v39.template: FemPlugin_v39.generated
AcpcFemPlugin_v14.template: AcpcFemPlugin_v14.generated
AcpcFemPlugin_v22.template: AcpcFemPlugin_v22.generated
RocPlugin_v14.template: RocPlugin_v14.generated
RocPlugin_v43.template: RocPlugin_v43.generated
RocPlugin_v44.template: RocPlugin_v45.generated
RocPlugin_v45.template: RocPlugin_v45.generated
RocPlugin_v47.template: RocPlugin_v47.generated
RocPlugin_v50.template: RocPlugin_v50.generated
RocPlugin_v51.template: RocPlugin_v51.generated
RocPlugin_v52.template: RocPlugin_v52.generated
RocPlugin_v54.template: RocPlugin_v54.generated
RocPlugin_v56.template: RocPlugin_v56.generated
RocPlugin_v57.template: RocPlugin_v57.generated
RocPlugin_v58.template: RocPlugin_v58.generated
AdcRocPlugin_v02.template: AdcRocPlugin_v02.generated
AdcRocPlugin_v03.template: AdcRocPlugin_v03.generated
AdcRocPlugin_v05.template: AdcRocPlugin_v05.generated
BnlRocPlugin_v00.template: BnlRocPlugin_v00.generated
BnlRocPlugin_v20.template: BnlRocPlugin_v20.generated
AcpcPlugin_v144.template: AcpcPlugin_v144.generated
AcpcPlugin_v171.template: AcpcPlugin_v171.generated
AcpcPlugin_v172.template: AcpcPlugin_v172.generated
ArocPlugin_v23.template: ArocPlugin_v23.generated
ArocPlugin_v24.template: ArocPlugin_v24.generated
ArocPlugin_v255255.template: ArocPlugin_v255255.generated
CRocPlugin_v93.template: CRocPlugin_v93.generated
CRocPlugin_v94.template: CRocPlugin_v94.generated

%.generated: ../src/%.cpp
	../export2epics.py -i $^ -o $@
