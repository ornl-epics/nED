/* FileCircularBuffer.h
 *
 * Copyright (c) 2018 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#include "BaseCircularBuffer.h"

#include <array>
#include <epicsTime.h>
#include <limits>

class FileCircularBuffer : public BaseCircularBuffer {
    private:
        int m_fd = -1;
        std::array<uint8_t, 100000> m_buffer;
        uint32_t m_offset;
        double m_timeDiff = std::numeric_limits<double>::min();
        epicsTime m_startTime;
        bool m_reading = false;
        bool m_oldPackets = false;
        uint32_t m_maxPackets = 100;
        float m_speed = 1.0;

    public:
        /**
         * Opens a new file, closing any previous file descriptor.
         * 
         * @param path File path to be opened
         * @param error Error string on failure
         * @return true when file was opened, false otherwise.
         */
        bool open(const std::string &path, std::string &error);
        
        /**
         * Closes currently opened file descriptor if any.
         */
        void close();
       
        /**
         * Overloaded functionality resets file descriptor to beginning of file.
         */
        void clear() override
        {
            reset();
        }
        
        /**
         * Allows wait() function to proceed.
         */
        void start();
        
        /**
         * Stops wait() function to return any data - will always timeout.
         */
        void stop();
        
        /**
         * Resets file descriptor to beginning of file.
         */
        void reset();
        
        /**
         * Return true if currently reading the file.
         */
        bool isRunning()
        {
            return m_reading;
        }
        
        /**
         * Switches between old and new packet type.
         */
        void setOldPacketType(bool enable)
        {
            m_oldPackets = enable;
        }
        
        /**
         * Set maximum number of packets each time wait() function is called.
         */
        void setMaxPackets(uint32_t count)
        {
            m_maxPackets = count;
        }
        
        /**
         * Set replay speed, 1 for actual time, greater than 1 for faster, less than 1 for slower.
         */
        void setSpeed(float speed);
        
        /**
         * Waits for some data obeying the timestamps from packets.
         */
        int wait(void **data, uint32_t *len, double timeout=0.0) override;

        /**
         * Acknowledges some amount of data from previous wait().
         */
        int consume(uint32_t len) override;

        /**
         * Return true when no data is available in circular buffer.
         */
        bool empty() override
        {
            return (m_offset == 0);
        }

        /**
         * Return buffer used space in bytes.
         */
        uint32_t used() override
        {
            return m_offset;
        }
        /**
         * Return buffer size in bytes.
         */
        uint32_t size() override
        {
            return m_buffer.size();
        }

    private:
        /**
         * Tries to read one single packet and put it in buffer.
         * 
         * The maxTime defines the maximum time the packet must obey.
         * If time from packet is later than the maxTime, packet is
         * put back to file to be read next time. If time is lower, the
         * maxTime argument is modified with the actual packet time.
         * 
         * When successful, the function will put one complete packet into
         * the m_buffer and adjust the m_offset for the length of the packet.
         * If it fails, it may write partial data to m_buffer but the m_offset
         * does not change (so the next read will overwrite the m_buffer[m_offset].
         * Depending on the failure, the underlaying file descriptor might
         * get invalidated.
         */
        bool readPacket(epicsTime &maxTime);
};
