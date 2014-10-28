/* AcpcFemPlugin.cpp
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "AcpcFemPlugin.h"
#include "Log.h"

#define NUM_ACPCFEMPLUGIN_PARAMS    0 //((int)(&LAST_ACPCFEMPLUGIN_PARAM - &FIRST_ACPCFEMPLUGIN_PARAM + 1))
#define HEX_BYTE_TO_DEC(a)      ((((a)&0xFF)/16)*10 + ((a)&0xFF)%16)

EPICS_REGISTER_PLUGIN(AcpcFemPlugin, 5, "Port name", string, "Dispatcher port name", string, "Hardware ID", string, "Hw & SW version", string, "Blocking", int);

const unsigned AcpcFemPlugin::NUM_ACPCFEMPLUGIN_DYNPARAMS       = 250;

struct RspReadVersion {
#ifdef BITFIELD_LSB_FIRST
    unsigned hw_revision:8;     // Board revision number
    unsigned hw_version:8;      // Board version number
    unsigned hw_year:16;        // Board year
    unsigned hw_day:8;          // Board day
    unsigned hw_month:8;        // Board month
    unsigned fw_revision:8;     // Firmware revision number
    unsigned fw_version:8;      // Firmware version number
    unsigned fw_year:16;        // Firmware year
    unsigned fw_day:8;          // Firmware day
    unsigned fw_month:8;        // Firmware month
#else
#error Missing RspReadVersion declaration
#endif // BITFIELD_LSB_FIRST
};


AcpcFemPlugin::AcpcFemPlugin(const char *portName, const char *dispatcherPortName, const char *hardwareId, const char *version, int blocking)
    : BaseModulePlugin(portName, dispatcherPortName, hardwareId, true,
                       blocking, NUM_ACPCFEMPLUGIN_PARAMS + NUM_ACPCFEMPLUGIN_DYNPARAMS)
    , m_version(version)
{
    createStatusParams();
    createConfigParams();

    setIntegerParam(HwType, DasPacket::MOD_TYPE_ACPCFEM);
    setIntegerParam(Supported, 1);

    LOG_DEBUG("Number of configured dynamic parameters: %zu", m_statusParams.size() + m_configParams.size());

    callParamCallbacks();
}

bool AcpcFemPlugin::rspDiscover(const DasPacket *packet)
{
    return (BaseModulePlugin::rspDiscover(packet) &&
            packet->cmdinfo.module_type == DasPacket::MOD_TYPE_ACPCFEM);
}

bool AcpcFemPlugin::rspReadVersion(const DasPacket *packet)
{
    char date[20];

    if (!BaseModulePlugin::rspReadVersion(packet))
        return false;

    BaseModulePlugin::Version version;
    if (!parseVersionRsp(packet, version)) {
        LOG_WARN("Bad READ_VERSION response");
        return false;
    }

    setIntegerParam(HwVer, version.hw_version);
    setIntegerParam(HwRev, version.hw_revision);
    setStringParam(HwDate, "");
    setIntegerParam(FwVer, version.fw_version);
    setIntegerParam(FwRev, version.fw_revision);
    snprintf(date, sizeof(date), "%04d/%02d/%02d", version.fw_year, version.fw_month, version.fw_day);
    setStringParam(FwDate, date);

    callParamCallbacks();

    if (version.hw_version == 2 && version.hw_revision == 5 && version.fw_version == 2 && version.fw_revision == 2)
        return true;

    return false;
}

bool AcpcFemPlugin::parseVersionRsp(const DasPacket *packet, BaseModulePlugin::Version &version)
{
    if (packet->getPayloadLength() != sizeof(RspReadVersion))
        return false;

    const RspReadVersion *response = reinterpret_cast<const RspReadVersion*>(packet->getPayload());
    version.hw_version  = response->hw_version;
    version.hw_revision = response->hw_revision;
    version.hw_year     = HEX_BYTE_TO_DEC(response->hw_year) + 2000;
    version.hw_month    = HEX_BYTE_TO_DEC(response->hw_month);
    version.hw_day      = HEX_BYTE_TO_DEC(response->hw_day);
    version.fw_version  = response->fw_version;
    version.fw_revision = response->fw_revision;
    version.fw_year     = HEX_BYTE_TO_DEC(response->fw_year) + 2000;
    version.fw_month    = HEX_BYTE_TO_DEC(response->fw_month);
    version.fw_day      = HEX_BYTE_TO_DEC(response->fw_day);

    return true;
}

void AcpcFemPlugin::createStatusParams()
{
//     BLXXX:Det:FemXX:| sig name |                      | EPICS record description | (bi and mbbi description)
    createStatusParam("Ch2:ErrFif",     0x0,  1, 15); // Chan2 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch2:ErrStr",     0x0,  1, 14); // Chan2 got START during packt (0=no error,1=error)
    createStatusParam("Ch2:ErrNoS",     0x0,  1, 13); // Chan2 got data without START (0=no error,1=error)
    createStatusParam("Ch2:ErrTim",     0x0,  1, 12); // Chan2 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch2:ErrLen",     0x0,  1, 11); // Chan2 long packet error      (0=no error,1=error)
    createStatusParam("Ch2:ErrSho",     0x0,  1, 10); // Chan2 short packet error     (0=no error,1=error)
    createStatusParam("Ch2:ErrCmd",     0x0,  1,  9); // Chan2 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch2:ErrPar",     0x0,  1,  8); // Chan2 parity error           (0=no error,1=error)
    createStatusParam("Ch1:ErrFif",     0x0,  1,  7); // Chan1 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch1:ErrStr",     0x0,  1,  6); // Chan1 got START during packt (0=no error,1=error)
    createStatusParam("Ch1:ErrNoS",     0x0,  1,  5); // Chan1 got data without START (0=no error,1=error)
    createStatusParam("Ch1:ErrTim",     0x0,  1,  4); // Chan1 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch1:ErrLen",     0x0,  1,  3); // Chan1 long packet error      (0=no error,1=error)
    createStatusParam("Ch1:ErrSho",     0x0,  1,  2); // Chan1 short packet error     (0=no error,1=error)
    createStatusParam("Ch1:ErrCmd",     0x0,  1,  1); // Chan1 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch1:ErrPar",     0x0,  1,  0); // Chan1 parity error           (0=no error,1=error)

    createStatusParam("Ch4:ErrFif",     0x1,  1, 15); // Chan4 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch4:ErrStr",     0x1,  1, 14); // Chan4 got START during packt (0=no error,1=error)
    createStatusParam("Ch4:ErrNoS",     0x1,  1, 13); // Chan4 got data without START (0=no error,1=error)
    createStatusParam("Ch4:ErrTim",     0x1,  1, 12); // Chan4 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch4:ErrLen",     0x1,  1, 11); // Chan4 long packet error      (0=no error,1=error)
    createStatusParam("Ch4:ErrSho",     0x1,  1, 10); // Chan4 short packet error     (0=no error,1=error)
    createStatusParam("Ch4:ErrCmd",     0x1,  1,  9); // Chan4 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch4:ErrPar",     0x1,  1,  8); // Chan4 parity error           (0=no error,1=error)
    createStatusParam("Ch3:ErrFif",     0x1,  1,  7); // Chan3 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch3:ErrStr",     0x1,  1,  6); // Chan3 got START during packt (0=no error,1=error)
    createStatusParam("Ch3:ErrNoS",     0x1,  1,  5); // Chan3 got data without START (0=no error,1=error)
    createStatusParam("Ch3:ErrTim",     0x1,  1,  4); // Chan3 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch3:ErrLen",     0x1,  1,  3); // Chan3 long packet error      (0=no error,1=error)
    createStatusParam("Ch3:ErrSho",     0x1,  1,  2); // Chan3 short packet error     (0=no error,1=error)
    createStatusParam("Ch3:ErrCmd",     0x1,  1,  1); // Chan3 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch3:ErrPar",     0x1,  1,  0); // Chan3 parity error           (0=no error,1=error)

    createStatusParam("Ch6:ErrFif",     0x2,  1, 15); // Chan6 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch6:ErrStr",     0x2,  1, 14); // Chan6 got START during packt (0=no error,1=error)
    createStatusParam("Ch6:ErrNoS",     0x2,  1, 13); // Chan6 got data without START (0=no error,1=error)
    createStatusParam("Ch6:ErrTim",     0x2,  1, 12); // Chan6 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch6:ErrLen",     0x2,  1, 11); // Chan6 long packet error      (0=no error,1=error)
    createStatusParam("Ch6:ErrSho",     0x2,  1, 10); // Chan6 short packet error     (0=no error,1=error)
    createStatusParam("Ch6:ErrCmd",     0x2,  1,  9); // Chan6 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch6:ErrPar",     0x2,  1,  8); // Chan6 parity error           (0=no error,1=error)
    createStatusParam("Ch5:ErrFif",     0x2,  1,  7); // Chan5 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch5:ErrStr",     0x2,  1,  6); // Chan5 got START during pack  (0=no error,1=error)
    createStatusParam("Ch5:ErrNoS",     0x2,  1,  5); // Chan5 got data without START (0=no error,1=error)
    createStatusParam("Ch5:ErrTim",     0x2,  1,  4); // Chan5 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch5:ErrLen",     0x2,  1,  3); // Chan5 long packet error      (0=no error,1=error)
    createStatusParam("Ch5:ErrSho",     0x2,  1,  2); // Chan5 short packet error     (0=no error,1=error)
    createStatusParam("Ch5:ErrCmd",     0x2,  1,  1); // Chan5 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch5:ErrPar",     0x2,  1,  0); // Chan5 parity error           (0=no error,1=error)

    createStatusParam("Ch8:ErrFif",     0x3,  1, 15); // Chan8 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch8:ErrStr",     0x3,  1, 14); // Chan8 got START during packt (0=no error,1=error)
    createStatusParam("Ch8:ErrNoS",     0x3,  1, 13); // Chan8 got data without START (0=no error,1=error)
    createStatusParam("Ch8:ErrTim",     0x3,  1, 12); // Chan8 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch8:ErrLen",     0x3,  1, 11); // Chan8 long packet error      (0=no error,1=error)
    createStatusParam("Ch8:ErrSho",     0x3,  1, 10); // Chan8 short packet error     (0=no error,1=error)
    createStatusParam("Ch8:ErrCmd",     0x3,  1,  9); // Chan8 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch8:ErrPar",     0x3,  1,  8); // Chan8 parity error           (0=no error,1=error)
    createStatusParam("Ch7:ErrFif",     0x3,  1,  7); // Chan7 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch7:ErrStr",     0x3,  1,  6); // Chan7 got START during pack  (0=no error,1=error)
    createStatusParam("Ch7:ErrNoS",     0x3,  1,  5); // Chan7 got data without START (0=no error,1=error)
    createStatusParam("Ch7:ErrTim",     0x3,  1,  4); // Chan7 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch7:ErrLen",     0x3,  1,  3); // Chan7 long packet error      (0=no error,1=error)
    createStatusParam("Ch7:ErrSho",     0x3,  1,  2); // Chan7 short packet error     (0=no error,1=error)
    createStatusParam("Ch7:ErrCmd",     0x3,  1,  1); // Chan7 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch7:ErrPar",     0x3,  1,  0); // Chan7 parity error           (0=no error,1=error)

    createStatusParam("Ctr:ErrFif",     0x4,  1, 15); // CTRL FIFO went full          (0=no,1=yes)
    createStatusParam("Ctr:ErrStr",     0x4,  1, 14); // CTRL got START during packe  (0=no error,1=error)
    createStatusParam("Ctr:ErrNoS",     0x4,  1, 13); // CTRL got data without START  (0=no error,1=error)
    createStatusParam("Ctr:ErrTim",     0x4,  1, 12); // CTRL got TIMEOUT             (0=no timeout,1=timeout)
    createStatusParam("Ctr:ErrLen",     0x4,  1, 11); // CTRL long packet error       (0=no error,1=error)
    createStatusParam("Ctr:ErrSho",     0x4,  1, 10); // CTRL short packet error      (0=no error,1=error)
    createStatusParam("Ctr:ErrCmd",     0x4,  1,  9); // CTRL DATA/COMMAND type err   (0=no error,1=error)
    createStatusParam("Ctr:ErrPar",     0x4,  1,  8); // CTRL parity error            (0=no error,1=error)
    createStatusParam("Ch9:ErrFif",     0x4,  1,  7); // Chan9 FIFO went full         (0=no,1=yes)
    createStatusParam("Ch9:ErrStr",     0x4,  1,  6); // Chan9 got START during pack  (0=no error,1=error)
    createStatusParam("Ch9:ErrNoS",     0x4,  1,  5); // Chan9 got data without START (0=no error,1=error)
    createStatusParam("Ch9:ErrTim",     0x4,  1,  4); // Chan9 got TIMEOUT            (0=no timeout,1=timeout)
    createStatusParam("Ch9:ErrLen",     0x4,  1,  3); // Chan9 long packet error      (0=no error,1=error)
    createStatusParam("Ch9:ErrSho",     0x4,  1,  2); // Chan9 short packet error     (0=no error,1=error)
    createStatusParam("Ch9:ErrCmd",     0x4,  1,  1); // Chan9 DATA/COMMAND type err  (0=no error,1=error)
    createStatusParam("Ch9:ErrPar",     0x4,  1,  0); // Chan9 parity error           (0=no error,1=error)

    createStatusParam("FltrNacks",      0x5,  8,  7); // Filtered NACKs
    createStatusParam("FltrNack",       0x5,  1,  6); // Filtered NACK                (0=no,1=yes)
    createStatusParam("ErrEeprom",      0x5,  1,  5); // EEPROM error                 (0=no error,1=error)
    createStatusParam("ErrNoHw",        0x5,  1,  4); // Missing hardware             (0=not missing,1=missing)
    createStatusParam("ErrUncfg",       0x5,  1,  3); // Unconfigured                 (0=configured,1=not confiured)
    createStatusParam("ErrProg",        0x5,  1,  2); // Programming error            (0=no error,1=error)
    createStatusParam("ErrCmdLen",      0x5,  1,  1); // Command length error         (0=no error,1=error)
    createStatusParam("ErrBadCmd",      0x5,  1,  0); // Unknown command              (0=no error,1=error)

    createStatusParam("Ch8:GotCmd",     0x6,  1, 15); // Chan8 got command packet     (0=no,1=yes)
    createStatusParam("Ch8:GotDat",     0x6,  1, 14); // Chan8 got data packet        (0=no,1=yes)
    createStatusParam("Ch7:GotCmd",     0x6,  1, 13); // Chan7 got command packet     (0=no,1=yes)
    createStatusParam("Ch7:GotDat",     0x6,  1, 12); // Chan7 got data packet        (0=no,1=yes)
    createStatusParam("Ch6:GotCmd",     0x6,  1, 11); // Chan6 got command packet     (0=no,1=yes)
    createStatusParam("Ch6:GotDat",     0x6,  1, 10); // Chan6 got data packet        (0=no,1=yes)
    createStatusParam("Ch5:GotCmd",     0x6,  1,  9); // Chan5 got command packet     (0=no,1=yes)
    createStatusParam("Ch5:GotDat",     0x6,  1,  8); // Chan5 got data packet        (0=no,1=yes)
    createStatusParam("Ch4:GotCmd",     0x6,  1,  7); // Chan4 got command packet     (0=no,1=yes)
    createStatusParam("Ch4:GotDat",     0x6,  1,  6); // Chan4 got data packet        (0=no,1=yes)
    createStatusParam("Ch3:GotCmd",     0x6,  1,  5); // Chan3 got command packet     (0=no,1=yes)
    createStatusParam("Ch3:GotDat",     0x6,  1,  4); // Chan3 got data packet        (0=no,1=yes)
    createStatusParam("Ch2:GotCmd",     0x6,  1,  3); // Chan2 got command packet     (0=no,1=yes)
    createStatusParam("Ch2:GotDat",     0x6,  1,  2); // Chan2 got data packet        (0=no,1=yes)
    createStatusParam("Ch1:GotCmd",     0x6,  1,  1); // Chan1 got command packet     (0=no,1=yes)
    createStatusParam("Ch1:GotDat",     0x6,  1,  0); // Chan1 got data packet        (0=no,1=yes)

    createStatusParam("FltrAcks",       0x7,  8,  8); // Filtered ACKS
    createStatusParam("OutputMode",     0x7,  2,  6); // Data mode TODO verify values (0=normal mode,1=off,2=raw mode,3=verbose mode)
    createStatusParam("Acquiring",      0x7,  1,  5); // Acquiring data               (0=not acquiring,1=acquiring)
    createStatusParam("FoundHw",        0x7,  1,  4); // Found new hardware           (0=no,1=yes)
    createStatusParam("Ctr:GotCmd",     0x7,  1,  3); // CTRL got command packet      (0=no,1=yes)
    createStatusParam("Ctr:GotDat",     0x7,  1,  2); // CTRL got data packet         (0=no,1=yes)
    createStatusParam("Ch9:GotCmd",     0x7,  1,  1); // Chan9 got command packet     (0=no,1=yes)
    createStatusParam("Ch9:GotDat",     0x7,  1,  0); // Chan9 got data packet        (0=no,1=yes)

    createStatusParam("Ch8:FifCmd",     0x8,  1, 15); // Chan8 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch8:FifoNE",     0x8,  1, 14); // Chan8 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch7:FifoAF",     0x8,  1, 13); // Chan7 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch7:FifoNE",     0x8,  1, 12); // Chan7 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch6:FifoAF",     0x8,  1, 11); // Chan6 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch6:FifoNE",     0x8,  1, 10); // Chan6 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch5:FifoAF",     0x8,  1,  9); // Chan5 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch5:FifoNE",     0x8,  1,  8); // Chan5 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch4:FifoAF",     0x8,  1,  7); // Chan4 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch4:FifoNE",     0x8,  1,  6); // Chan4 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch3:FifoAF",     0x8,  1,  5); // Chan3 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch3:FifoNE",     0x8,  1,  4); // Chan3 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch2:FifoAF",     0x8,  1,  3); // Chan2 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch2:FifoNE",     0x8,  1,  2); // Chan2 FIFO has data          (0=empty,1=has data)
    createStatusParam("Ch1:FifoAF",     0x8,  1,  1); // Chan1 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch1:FifoNE",     0x8,  1,  0); // Chan1 FIFO not empty         (0=empty,1=has data)

    createStatusParam("CmdFifoAF",      0x9,  1, 15); // CMD FIFO almost full         (0=no,1=yes)
    createStatusParam("CmdFifoNE",      0x9,  1, 14); // CMD FIFO has data            (0=empty,1=has data)
    createStatusParam("CmdInFifAF",     0x9,  1, 13); // CMD input FIFO almost full   (0=no,1=yes)
    createStatusParam("CmdInFifNE",     0x9,  1, 12); // CMD input FIFO has data      (0=empty,1=has data)
    createStatusParam("Ibc2Miss",       0xD,  1,  9); // Good: IBC2 missing           (0=no,1=yes)
    createStatusParam("Ibc1Miss",       0xD,  1,  8); // Good: IBC1 missing           (0=no,1=yes)
    createStatusParam("DffAF",          0x9,  1,  7); // DFF almost full              (0=no,1=yes)
    createStatusParam("DffNE",          0x9,  1,  6); // DFF has data                 (0=empty,1=has data)
    createStatusParam("CtrlFifoAF",     0x9,  1,  3); // CTRL FIFO almost full        (0=no,1=yes)
    createStatusParam("CtrlFifoNE",     0x9,  1,  2); // CTRL FIFO has data           (0=empty,1=has data)
    createStatusParam("Ch9:FifoAF",     0x9,  1,  1); // Chan9 FIFO almost full       (0=no,1=yes)
    createStatusParam("Ch9:FifoNE",     0x9,  1,  0); // Chan9 FIFO has data          (0=empty,1=has data)

    createStatusParam("FltrdRsps",      0xA, 16,  8); // Filtered responses
    createStatusParam("SysrstB",        0xA,  2,  6); // Sysrst_B low/high detected   (0=empty,1=low,2=high,3=low and high)
    createStatusParam("TxEnable",       0xA,  2,  4); // TXEN low/high detected       (0=empty,1=low,2=high,3=low and high)
    createStatusParam("Tsync",          0xA,  2,  2); // TSYNC                        (0=empty,1=low - not ok,2=high - not ok,3=low and high)
    createStatusParam("Tclk",           0xA,  2,  0); // TCLK                         (0=empty,1=low - not ok,2=high - not ok,3=low and high)

    createStatusParam("HwIdCnt",        0xC,  9, 10); // Hardware ID count

    createStatusParam("Mlos4",          0xD,  2, 14); // MLOS1                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos3",          0xD,  2, 12); // MLOS2                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos2",          0xD,  2, 10); // MLOS3                        (0=empty,1=always low,2=always high,3=low and high)
    createStatusParam("Mlos1",          0xD,  2,  8); // MLOS4                        (0=empty,1=always low,2=always high,3=low and high)

    createStatusParam("Ibc5:Time",      0xD,  1, 15); // Bad: IBC5 time overflow      (0=no,1=yes)
    createStatusParam("Ibc4:Time",      0xD,  1, 14); // Bad: IBC4 time overflow      (0=no,1=yes)
    createStatusParam("Ibc3:Time",      0xD,  1, 13); // Bad: IBC3 time overflow      (0=no,1=yes)
    createStatusParam("Ibc2:Time",      0xD,  1, 12); // Bad: IBC2 time overflow      (0=no,1=yes)
    createStatusParam("Ibc1:Time",      0xD,  1, 11); // Bad: IBC1 time overflow      (0=no,1=yes)
    createStatusParam("Ibc9:Miss",      0xD,  1, 10); // Good: IBC9 missing           (0=no,1=yes)
    createStatusParam("Ibc8:Miss",      0xD,  1,  9); // Good: IBC8 missing           (0=no,1=yes)
    createStatusParam("Ibc7:Miss",      0xD,  1,  8); // Good: IBC7 missing           (0=no,1=yes)
    createStatusParam("Ibc6:Miss",      0xD,  1,  7); // Good: IBC6 missing           (0=no,1=yes)
    createStatusParam("Ibc5:Miss",      0xD,  1,  6); // Good: IBC5 missing           (0=no,1=yes)
    createStatusParam("Ibc4:Miss",      0xD,  1,  5); // Good: IBC4 missing           (0=no,1=yes)
    createStatusParam("Ibc3:Miss",      0xD,  1,  4); // Good: IBC3 missing           (0=no,1=yes)
    createStatusParam("LvdsVerf",       0xD,  1,  3); // Detected LVDS verify         (0=no,1=yes)

    createStatusParam("EepromCode",     0xE, 16,  0); // EEPROM code

    createStatusParam("ErrDatHw",       0xF,  1, 15); // Data packet hardware error   (0=no,1=yes)
    createStatusParam("ErrMaskCh",      0xF,  1, 12); // Data packet changing mask    (0=no,1=yes)
    createStatusParam("ErrMaskCh1",     0xF,  1, 11); // Data packet mask change      (0=no,1=yes)
    createStatusParam("ErrNoIbc",       0xF,  1, 10); // Data packet missing IBCs     (0=no,1=yes)
    createStatusParam("ErrBadEv",       0xF,  1,  9); // Data packet bad event        (0=no,1=yes)
    createStatusParam("ErrBadIbc",      0xF,  1,  8); // Data packet bad IBC          (0=no,1=yes)
    createStatusParam("ErrTime",        0xF,  1,  7); // Data packet bad timestamp    (0=no,1=yes)
    createStatusParam("ErrTimeout",     0xF,  1,  6); // Data packet timeout          (0=no,1=yes)
    createStatusParam("ErrDatBad",      0xF,  6,  0); // Bad data packet              (0=no,1=yes)
/*
    // These are found in dcomserver. But the module respond with the length which ends here.
    // At least eeprom code parameter is at the correct position, it's value was verified
    // using expected BAD2.

    createStatusParam/disabled/("Ibc9Err",       0x10,  1, 12); // IBC9 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc8Err",       0x10,  1, 11); // IBC8 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc7Err",       0x10,  1, 10); // IBC7 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc6Err",       0x10,  1,  9); // IBC6 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc5Err",       0x10,  1,  8); // IBC5 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc4Err",       0x10,  1,  7); // IBC4 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc3Err",       0x10,  1,  6); // IBC3 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc2Err",       0x10,  1,  5); // IBC2 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc1Err",       0x10,  1,  4); // IBC1 error                    (0=no,1=yes)
    createStatusParam/disabled/("Ibc9TimeOv",    0x10,  1,  3); // Bad: IBC9 time overflow       (0=no,1=yes)
    createStatusParam/disabled/("Ibc8TimeOv",    0x10,  1,  2); // Bad: IBC8 time overflow       (0=no,1=yes)
    createStatusParam/disabled/("Ibc7TimeOv",    0x10,  1,  1); // Bad: IBC7 time overflow       (0=no,1=yes)
    createStatusParam/disabled/("Ibc6TimeOv",    0x10,  1,  0); // Bad: IBC6 time overflow       (0=no,1=yes)
*/
}

void AcpcFemPlugin::createConfigParams()
{
//     BLXXX:Det:FemXX:| sig name |                                | EPICS record description | (bi and mbbi description)
    createConfigParam("Ch9:Dis",      'E', 0x0,  1, 12, 0);     // Channel 9 disable            (0=enable,1=disable)
    createConfigParam("Ch8:Dis",      'E', 0x0,  1, 11, 0);     // Channel 8 disable            (0=enable,1=disable)
    createConfigParam("Ch7:Dis",      'E', 0x0,  1, 10, 0);     // Channel 7 disable            (0=enable,1=disable)
    createConfigParam("Ch6:Dis",      'E', 0x0,  1,  9, 0);     // Channel 6 disable            (0=enable,1=disable)
    createConfigParam("Ch5:Dis",      'E', 0x0,  1,  8, 0);     // Channel 5 disable            (0=enable,1=disable)
    createConfigParam("Ch4:Dis",      'E', 0x0,  1,  7, 0);     // Channel 4 disable            (0=enable,1=disable)
    createConfigParam("Ch3:Dis",      'E', 0x0,  1,  6, 0);     // Channel 3 disable            (0=enable,1=disable)
    createConfigParam("Ch2:Dis",      'E', 0x0,  1,  5, 0);     // Channel 2 disable            (0=enable,1=disable)
    createConfigParam("Ch1:Dis",      'E', 0x0,  1,  4, 0);     // Channel 1 disable            (0=enable,1=disable)
    createConfigParam("TxenCtrl",     'E', 0x0,  1,  3, 0);     // TXEN internal                (0=external,1=internal)
    createConfigParam("TsyncCtrl",    'E', 0x0,  1,  2, 0);     // TSYNC internal               (0=external,1=internal)
    createConfigParam("TclkCtrl",     'E', 0x0,  1,  1, 0);     // TCLK internal                (0=external,1=internal)
    createConfigParam("ResetCtrl",    'E', 0x0,  1,  0, 0);     // Reset external               (0=external,1=internal)
}
