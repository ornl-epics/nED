#!../../bin/linux-x86_64/ned

< envPaths

epicsEnvSet("IOCNAME", "bl99-Det-nED")
epicsEnvSet("PREFIX", "BL99:Det:")
epicsEnvSet("STREAM_PROTOCOL_PATH", "../../protocol/")

## Register all support components
dbLoadDatabase("../../dbd/ned.dbd",0,0)
ned_registerRecordDeviceDriver(pdbbase)

# Autosave
epicsEnvSet SAVE_DIR /home/controls/var/$(IOCNAME)
system("install -m 777 -d $(SAVE_DIR)")
save_restoreSet_Debug(0)
save_restoreSet_status_prefix("$(PREFIX)")
set_requestfile_path("$(SAVE_DIR)")
set_savefile_path("$(SAVE_DIR)")
save_restoreSet_NumSeqFiles(3)
save_restoreSet_SeqPeriodInSeconds(600)
###set_pass0_restoreFile("$(IOCNAME).sav")
###set_pass0_restoreFile("$(IOCNAME)_pass0.sav")
set_pass0_restoreFile("$(IOCNAME).sav")
set_pass1_restoreFile("$(IOCNAME).sav")

# asyn reporting - per port basis
#asynSetTraceIOMask("<port>", 0, 0xFF)
#asynSetTraceMask("<port>", 0, 0xFF)

# Access Security
# NOTE: All nED configuration and output records are writable only by BEAMLINE
#       Uncomment and update next 2 lines to enable access security
#asSetFilename("$(TOP)/../bl99-Det-nED/Db/bl7.acf")
#asSetSubstitutions("P=BL99:CS")

dbLoadRecords("../../db/nEDCommon.db","P=$(PREFIX)")

## Load record instances
OccConfigure("occ", "/dev/snsocb1", 40000000)
dbLoadRecords("../../db/OccPortDriver.db","P=$(PREFIX)occ:,PORT=occ")

CmdDispatcherConfigure("cmd", "occ")
dbLoadRecords("../../db/CmdDispatcherPlugin.db","P=$(PREFIX)cmd:,PORT=cmd")

PixelMapPluginConfigure("PixMap", "occ", 1, "/tmp/test.pixelmap", 4194304)
dbLoadRecords("../../db/PixelMapPlugin.db","P=$(PREFIX)pm1:,PORT=PixMap")

AdaraPluginConfigure("Adara1", "occ", 1, 1)
dbLoadRecords("../../db/AdaraPlugin.db","P=$(PREFIX)adara1:,PORT=Adara1")

ProxyPluginConfigure("proxy1", "PixMap")
dbLoadRecords("../../db/ProxyPlugin.db","P=$(PREFIX)proxy1:,PORT=proxy1")

RocPvaPluginConfigure("rocPva1", "occ", "$(PREFIX)rocpva1:Neutrons")
dbLoadRecords("../../db/RocPvaPlugin.db","P=$(PREFIX)rocpva1:,PORT=rocPva1")

#DspPluginConfigure("Dsp1", "occ", "0x15FA76DF")
DspPluginConfigure("Dsp1", "cmd", "21.250.118.223", "v63", 0)
dbLoadRecords("../../db/DspPlugin_v63.db","P=$(PREFIX)dsp1:,PORT=Dsp1")

DiscoverPluginConfigure("Disc", "occ")
dbLoadRecords("../../db/DiscoverPlugin.db","P=$(PREFIX)disc:,PORT=Disc")

RocPluginConfigure("roc1", "cmd", "20.39.216.73", "v52", 0)
#dbLoadRecords("../../db/ROCHV.db","P=$(PREFIX)HV1,G=$(PREFIX)HVG,PORT=roc1")
dbLoadRecords("../../db/RocPlugin_v52.db","P=$(PREFIX)roc1:,PORT=roc1")

DumpPluginConfigure("dump", "occ", 0)
dbLoadRecords("../../db/DumpPlugin.db","P=$(PREFIX)dump:,PORT=dump")

StatPluginConfigure("stat", "occ", 0)
dbLoadRecords("../../db/StatPlugin.db","P=$(PREFIX)stat:,PORT=stat")

RtdlPluginConfigure("rtdl", "occ", 0)
dbLoadRecords("../../db/RtdlPlugin.db","P=$(PREFIX)rtdl:,PORT=rtdl")

FemPluginConfigure("fem1", "cmd", "0x603B0817", "v22", 1)
dbLoadRecords("../../db/FemPlugin_v32.db","P=$(PREFIX)fem1:,PORT=fem1")

AcpcFemPluginConfigure("fem2", "cmd", "20.128.234.122", "v22", 0)
dbLoadRecords("../../db/AcpcFemPlugin_v22.db","P=$(PREFIX)fem2:,PORT=fem2")

DebugPluginConfigure("gm", "cmd", 0)
dbLoadRecords("../../db/DebugPlugin.db","P=$(PREFIX)gm:,PORT=gm")

FlatFieldPluginConfigure("ff", "occ", 0)
dbLoadRecords("../../db/FlatFieldPlugin.db","P=$(PREFIX)ff:,PORT=ff")

AcpcPvaPluginConfigure("AcpcPva", "occ", "$(PREFIX)pva:Neutrons")
dbLoadRecords("../../db/AcpcPvaPlugin.db","P=$(PREFIX)pva:,PORT=AcpcPva")

iocInit()

# Create request file and start periodic 'save'
makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME).req", "autosaveFields")
###makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME)_pass0.req", "autosaveFields_pass0")
create_monitor_set("$(IOCNAME).req", 30)
###create_monitor_set("$(IOCNAME)_pass0.req", 30)

# Display status
save_restoreShow(10)

# Fanout record for init in HVROC.db instead of PINI mechanism
#epicsThreadSleep 1 
#dbpf $(PREFIX)HV1:InitProc.PROC 1

startPVAServer
