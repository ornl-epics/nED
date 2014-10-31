#!../../bin/linux-x86_64/ned

< envPaths

epicsEnvSet("IOCNAME", "bl99-iocned")

# StreamDevice
epicsEnvSet("STREAM_PROTOCOL_PATH", "../../protocol/")

## Register all support components
dbLoadDatabase("../../dbd/ned.dbd",0,0)
ned_registerRecordDeviceDriver(pdbbase) 

# Autosave
epicsEnvSet SAVE_DIR /home/controls/var/$(IOCNAME)
system("install -m 777 -d $(SAVE_DIR)")
save_restoreSet_Debug(0)
save_restoreSet_status_prefix("BL99:IOCNAME:")
set_requestfile_path("$(SAVE_DIR)")
set_savefile_path("$(SAVE_DIR)")
save_restoreSet_NumSeqFiles(3)
save_restoreSet_SeqPeriodInSeconds(600)
###set_pass0_restoreFile("$(IOCNAME).sav")
###set_pass0_restoreFile("$(IOCNAME)_pass0.sav")
set_pass0_restoreFile("$(IOCNAME).sav")
set_pass1_restoreFile("$(IOCNAME).sav")

## Load record instances
epicsEnvSet("PREFIX", "BL99:")
OccConfigure("occ1", "/dev/snsocb1", 40000000)
dbLoadRecords("../../db/OccPortDriver.db","P=$(PREFIX)Det:occ1:,PORT=occ1")

CmdDispatcherConfigure("cmd", "occ1")
dbLoadRecords("../../db/CmdDispatcherPlugin.db","P=$(PREFIX)Det:cmd:,PORT=cmd")

PixelMapPluginConfigure("PixMap", "occ1", 1, "/tmp/test.pixelmap", 4194304)
dbLoadRecords("../../db/PixelMapPlugin.template","P=$(PREFIX)Det:pm1:,PORT=PixMap")
dbLoadRecords("../../db/BasePlugin.template","P=$(PREFIX)Det:pm1:,PORT=PixMap")

AdaraPluginConfigure("Adara1", "occ1", 1, 2)
dbLoadRecords("../../db/AdaraPlugin.db","P=$(PREFIX)Det:adara1:,PORT=Adara1")

ProxyPluginConfigure("proxy1", "PixMap")
dbLoadRecords("../../db/ProxyPlugin.db","P=$(PREFIX)Det:proxy1:,PORT=proxy1")

#DspPluginConfigure("Dsp1", "occ1", "0x15FA76DF")
DspPluginConfigure("Dsp1", "cmd", "21.250.118.223", "v63", 0)
dbLoadRecords("../../db/DspPlugin.db","P=$(PREFIX)Det:dsp1:,PORT=Dsp1")

DiscoverPluginConfigure("Disc", "occ1")
dbLoadRecords("../../db/DiscoverPlugin.db","P=$(PREFIX)Det:disc:,PORT=Disc")

RocPluginConfigure("roc1", "cmd", "20.39.216.73", "v52", 0)
dbLoadRecords("../../db/ROCHV.db","P=$(PREFIX)Det:HV1,G=$(PREFIX)Det:HVG,PORT=roc1")
dbLoadRecords("../../db/RocPlugin_v52.db","P=$(PREFIX)Det:roc1:,PORT=roc1")

DumpPluginConfigure("dump", "occ1", 0)
dbLoadRecords("../../db/DumpPlugin.db","P=$(PREFIX)Det:dump:,PORT=dump")

StatPluginConfigure("stat", "occ1", 0)
dbLoadRecords("../../db/StatPlugin.db","P=$(PREFIX)Det:stat:,PORT=stat")

RtdlPluginConfigure("rtdl", "occ1", 0)
dbLoadRecords("../../db/RtdlPlugin.db","P=$(PREFIX)Det:rtdl:,PORT=rtdl")

FemPluginConfigure("fem1", "cmd", "0x603B0817", "10.0/5.0", 1)
dbLoadRecords("../../db/FemPlugin.db","P=$(PREFIX)Det:fem1:,PORT=fem1")

AcpcFemPluginConfigure("fem2", "cmd", "20.128.234.122", "10.0/5.0", 0)
dbLoadRecords("../../db/AcpcFemPlugin.db","P=$(PREFIX)Det:fem2:,PORT=fem2")

GenericModulePluginConfigure("gm", "cmd", 0)
dbLoadRecords("../../db/GenericModulePlugin.db","P=$(PREFIX)Det:gm:,PORT=gm")

FlatFieldPluginConfigure("ff", "occ1", 0)
dbLoadRecords("../../db/FlatFieldPlugin.db","P=$(PREFIX)Det:ff:,PORT=ff")

AcpcPvaPluginConfigure("AcpcPva", "occ1", "$(PREFIX)Det:pva1:Neutrons")

iocInit()

# Create request file and start periodic 'save'
makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME).req", "autosaveFields")
###makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME)_pass0.req", "autosaveFields_pass0")
create_monitor_set("$(IOCNAME).req", 30)
###create_monitor_set("$(IOCNAME)_pass0.req", 30)

# Display status
save_restoreShow(10)

# Fanout record for init in HVROC.db instead of PINI mechanism
epicsThreadSleep 1 
dbpf $(PREFIX)Det:HV1:InitProc.PROC 1

casr
startPVAServer
