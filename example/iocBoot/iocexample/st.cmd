#!../../bin/linux-x86_64/example

< envPaths

epicsEnvSet("IOCNAME", "bl100-Det5-nED")
epicsEnvSet("PREFIX", "BL100:Det5:")
epicsEnvSet("STREAM_PROTOCOL_PATH", "$(NED)/protocol/")

## Register all support components
dbLoadDatabase("$(TOP)/dbd/example.dbd",0,0)
example_registerRecordDeviceDriver(pdbbase) 

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

dbLoadRecords("$(NED)/db/nEDCommon.db","P=$(PREFIX)")

### Core Support Modules

OccConfigure("occ", "/dev/snsocb0", 41943040)
#OccConfigure("occ", "/tmp/occ.rx,/tmp/occ.tx") # occ_sim
dbLoadRecords("$(NED)/db/OccPortDriver.db","P=$(PREFIX)occ1:,PORT=occ")

CmdDispatcherConfigure("cmd", "occ")
dbLoadRecords("$(NED)/db/CmdDispatcherPlugin.db","P=$(PREFIX)cmd1:,PORT=cmd")

# For legacy DSP without timing setups
#TimingPluginConfigure("time", "occ")
#dbLoadRecords("$(NED)/db/TimingPluginConfigure.db","P=$(PREFIX)time:,PORT=time")

# Beammonitor tof correction plugin
#TofCorrectPluginConfigure("tof", "occ", 0)
#dbLoadRecords("$(NED)/db/TofCorrectPlugin.db","P=$(PREFIX)tof:,PORT=tof")

### Global RX Plug-ins

AdaraPluginConfigure("adara", "occ", 1, 1)
dbLoadRecords("$(NED)/db/AdaraPlugin.db","P=$(PREFIX)adara1:,PORT=adara")

#ProxyPluginConfigure("proxy", "occ")
#dbLoadRecords("$(NED)/db/ProxyPlugin.db","P=$(PREFIX)proxy1:,PORT=proxy")

DiscoverPluginConfigure("disc", "occ")
dbLoadRecords("$(NED)/db/DiscoverPlugin.db","P=$(PREFIX)disc1:,PORT=disc")

DumpPluginConfigure("dump", "occ", 0)
dbLoadRecords("$(NED)/db/DumpPlugin.db","P=$(PREFIX)dump1:,PORT=dump")

StatPluginConfigure("stat", "occ", 0)
dbLoadRecords("$(NED)/db/StatPlugin.db","P=$(PREFIX)stat1:,PORT=stat")

RtdlPluginConfigure("rtdl", "occ", 0)
dbLoadRecords("$(NED)/db/RtdlPlugin.db","P=$(PREFIX)rtdl1:,PORT=rtdl")

DebugPluginConfigure("dbg", "cmd", 0)
dbLoadRecords("$(NED)/db/DebugPlugin.db","P=$(PREFIX)dbg1:,PORT=dbg")


### Data pre-processing Plugins

#FlatFieldPluginConfigure("ff", "occ", "$(NED)/FlatField", 4194304)
#dbLoadRecords("$(NED)/db/FlatFieldPlugin.db","P=$(PREFIX)ff:,PORT=ff")

#PixelMapPluginConfigure("pixmap", "occ", 1, "/tmp/test.pixelmap", 4194304)
#dbLoadRecords("$(NED)/db/PixelMapPlugin.db","P=$(PREFIX)pm1:,PORT=pixmap")

#BnlFlatFieldPluginConfigure("ff", "occ", "$(NED)/FlatField/9_152_37_177.prmcalc", "$(NED)/FlatField/9_152_37_177.val", 41836544)
#dbLoadRecords("$(NED)/db/BnlFlatFieldPlugin.db","P=$(PREFIX)ff:,PORT=ff")


### PV Access Plugins

#AcpcPvaPluginConfigure("pva", "ff", "$(PREFIX)pva1:")
#dbLoadRecords("$(NED)/db/AcpcPvaPlugin.db","P=$(PREFIX)pva1:,PORT=pva")

RocPvaPluginConfigure("pva", "occ", "$(PREFIX)pva1:")
dbLoadRecords("$(NED)/db/RocPvaPlugin.db","P=$(PREFIX)pva1:,PORT=pva")

### Detectors configuration in external file
< detectors

### Load detectors database
#dbLoadRecords("$(TOP)/db/example.db")

### Mediators and Multiplexers

dbLoadRecords("$(NED)/db/GlobalConG.db","P=$(PREFIX)GblCon:")
dbLoadRecords("$(NED)/db/ApplyConfig.db","P=$(PREFIX)CfgApply:")

var mediatorVerbosity 0
mediateSeverity calc $(PREFIX)HVG:AlarmSum .*hv.*:AlarmSum

mediateSetpoint mbbo $(PREFIX)rocG:OutputMode .*:roc.*:OutputMode
mediateSetpoint mbbo $(PREFIX)rocG:CmdReq .*:roc.*:CmdReq
mediateEquality mbbi $(PREFIX)rocG:CmdRsp .*:roc.*:CmdRsp 1

# DataMode for all base plugins must match that which is set for the ROC
mediateSetpoint mbbo $(PREFIX)BPG:DataMode .*DataMode
# DataMode readback includes global setpoint above
mediateEquality mbbo $(PREFIX)BPG:DataModeRdbk .*DataMode 

mediateSetpoint mbbo $(PREFIX)BMPG:CmdReq .*CmdReq
mediateEquality mbbi $(PREFIX)BMPG:CmdRsp .*CmdRsp 1
mediateSetpoint mbbo $(PREFIX)BMPG:CfgSection .*CfgSection

### Postamble

#Increase size of cbLow thread ring buffer
#due to the large amount of record callbacks at IOC init.
callbackSetQueueSize(10000)

iocInit()

# Create request file and start periodic 'save'
#makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME).req", "autosaveFields")
###makeAutosaveFileFromDbInfo("$(SAVE_DIR)/$(IOCNAME)_pass0.req", "autosaveFields_pass0")
#create_monitor_set("$(IOCNAME).req", 30)
###create_monitor_set("$(IOCNAME)_pass0.req", 30)

# Display status
#save_restoreShow(10)

# Fanout record for init in HVROC.db instead of PINI mechanism
#epicsThreadSleep 1
#dbpf $(PREFIX)HV1:InitProc.PROC 1

startPVAServer

# Start sequencers
#seq GlobalCon "GPREFIX=$(PREFIX)GblCon:"
#seq ApplyConfig "PREFIX=$(PREFIX),SEQ_ID=CfgApply:"
