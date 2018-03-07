/* PvaNeutronsPlugin.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef PVA_NEUTRONS_H
#define PVA_NEUTRONS_H

#include "BasePlugin.h"
#include <pv/sharedVector.h>

/**
 * Gather and present statistical information of the incoming data
 */
class PvaNeutronsPlugin : public BasePlugin {
    public: // functions

        enum {
            STATUS_READY        = 0,
            STATUS_BAD_CONFIG   = 1,
            STATUS_INIT_ERROR   = 2,
            STATUS_SEND_ERROR   = 3,
        };

        /**
         * Constructor
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] pvName name of a PVA record used to export RTDL data
         */
        PvaNeutronsPlugin(const char *portName, const char *parentPlugins, const char *pvName);

        /**
         * Process downstream data packets
         */
        void recvDownstream(const DasDataPacketList &packets);

    private:
        class PvaRecordAcpc;
        class PvaRecordAroc;
        class PvaRecordBnl;
        class PvaRecordCroc;
        class PvaRecordLpsd;
        class PvaRecordPixel;

        std::tr1::shared_ptr<PvaRecordAcpc>  m_acpcRecord;
        std::tr1::shared_ptr<PvaRecordAroc>  m_arocRecord;
        std::tr1::shared_ptr<PvaRecordBnl>   m_bnlRecord;
        std::tr1::shared_ptr<PvaRecordCroc>  m_crocRecord;
        std::tr1::shared_ptr<PvaRecordLpsd>  m_lpsdRecord;
        std::tr1::shared_ptr<PvaRecordPixel> m_pixelRecord;
        std::tr1::shared_ptr<PvaRecordPixel> m_metaRecord;

        // asyn parameters
        int Status;             // See PvaNeutronsPlugin::STATUS_*
        int AcpcPvaName;        // PV name for ACPC diagnostic data
        int AcpcNumEvents;      // Number of events sent on ACPC channel
        int AcpcEnable;         // Enable ACPC channel
        int ArocPvaName;        // PV name for AROC diagnostic data
        int ArocNumEvents;      // Number of events sent on AROC channel
        int ArocEnable;         // Enable AROC channel
        int BnlPvaName;         // PV name for BNL diagnostic data
        int BnlNumEvents;       // Number of events sent on BNL channel
        int BnlEnable;          // Enable BNL channel
        int CrocPvaName;        // PV name for CROC diagnostic data
        int CrocNumEvents;      // Number of events sent on CROC channel
        int CrocEnable;         // Enable CROC channel
        int LpsdPvaName;        // PV name for LPSD diagnostic data
        int LpsdNumEvents;      // Number of events sent on LPSD channel
        int LpsdEnable;         // Enable LPSD channel
        int PixelPvaName;       // PV name for neutrons
        int PixelNumEvents;     // Number of events sent on Neutrons channel
        int PixelEnable;        // Enable Neutrons channel
        int MetaPvaName;        // PV name for meta
        int MetaNumEvents;      // Number of events sent on meta channel
        int MetaEnable;         // Enable meta channel
};

#endif // PVA_NEUTRONS_H
