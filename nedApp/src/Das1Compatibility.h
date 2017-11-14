/* Das1Compatibility.h
 *
 * Copyright (c) 2017 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef OLD_DAS_PACKET_CONVERTER_H
#define OLD_DAS_PACKET_CONVERTER_H

#include "BasePlugin.h"

/**
 * Converts between old DAS packets and internal packets in both ways.
 *
 * The DAS 1.0 compatibility plugin converts old packet format into internal
 * structure and vise-versa. On downstream path it intercepts packets that
 * begin with upper 4 bits as zero and passes through other packets intact. 
 * Upstream DasCmdPacket is converted to DAS 1.0 format.
 */
class Das1Compatibility : public BasePlugin {
    public:
        /**
         * Constructor
         */
        Das1Compatibility(const char *portName, const char *parentPlugins);

        /**
         * Intercept DAS 1.0 packets and convert them, pass thru the rest.
         */
        void recvDownstream(int type, PluginMessage *msg);

    private:
        std::vector<uint32_t> m_rtdlSources;        //!< Mapping between DSP address and 8-bit number
        
        DasRtdlPacket *old2new_rtdl(const DasPacket *packet);
        DasCmdPacket *old2new_cmd(const DasPacket *packet);
        DasPacket *new2old_cmd(const DasCmdPacket *packet, bool lvds);
};

#endif // OLD_DAS_PACKET_CONVERTER_H
