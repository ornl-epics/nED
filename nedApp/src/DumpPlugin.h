/* DumpPlugin.h
 *
 * Copyright (c) 2014 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Klemen Vodopivec
 */

#ifndef DUMP_PLUGIN_H
#define DUMP_PLUGIN_H

#include "BasePlugin.h"

#include <string>

/**
 * Dummy dump plugin writes all received OCC data into a file.
 *
 * The plugin dumps raw data received from OCC to a file. File path needs to be
 * set through FilePath parameter and plugin needs to be enabled to actually
 * open a file and save data. When plugin is disabled, file gets closed.
 *
 * After experiencing long delays on open() and write() system calls, this
 * plugin is now completly non-blocking. Not saved packets are acounted and
 * reported through PV. Partially written packet are deleted from the file
 * unless the output file is a named pipe, in which case data was already
 * transfered to client's side of the pipe. In such case the corruption offset
 * is reported. Saved and not saved packet counter as well as corruption offset
 * are reset when a new file is opened.
 */
class DumpPlugin : public BasePlugin {
    private: // variables
        int m_fd;           //!< File handle for an opened file, or -1
        bool m_fdIsPipe;    //!< true when opened file is a named pipe

    public: // functions
        /**
         * Constructor
         *
         * Only initialize parameters in constructor, don't open the file.
         *
         * @param[in] portName asyn port name.
         * @param[in] dispatcherPortName Name of the dispatcher asyn port to connect to.
         * @param[in] blocking Flag whether the processing should be done in the context of caller thread or in background thread.
         */
        DumpPlugin(const char *portName, const char *dispatcherPortName, int blocking);

        /**
         * Destructor
         *
         * Closes the dump file to make sure unsynced data get flushed to filesystem.
         */
        ~DumpPlugin();

        /**
         * Overloaded asynInt32 write handler
         *
         * Overload behavior of Enable parameter. When plugin gets enabled, open the
         * file if path is set. When disabled, close the file descriptor in non-blocking
         * way - don't wait for data synchronization to complete.
         */
        asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);

        /**
         * Overloaded asynOctet write handler
         *
         * When new file path is set and the plugin is enabled, the old file is closed
         * and new path is opened. If plugin is not enabled, remember the file path but
         * only open the file when plugin gets enabled.
         */
        asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t nChars, size_t *nActual);

        /**
         * Overloaded function to receive all OCC data.
         */
        void processData(const DasPacketList * const packetList);

    private: // variables
        bool m_rtdlEn;
        bool m_neutronEn;
        bool m_metadataEn;
        bool m_cmdEn;
        bool m_unknwnEn;

    private: // functions
        /**
         * Switch to new dump file.
         *
         * If for some reason new file can not be opened, previous state
         * remains in effect - old file, if any, remains the one to be written to.
         *
         * @param[in] path Absolute file path of a new file.
         * @return true on success, false otherwise.
         */
        bool openFile(const std::string &path);

        /**
         * Close file handle and disable further writes.
         *
         * Switch file synchronization mode to non-blocking, then close the file and
         * let the OS complete data synchronization.
         */
        void closeFile();

    private: // asyn parameters
        #define FIRST_DUMPPLUGIN_PARAM FilePath
        int FilePath;       //!< Path to file where to save all received data
        int RtdlPktsEn;     //!< Switch for RTDL packets
        int NeutronPktsEn;  //!< Switch for neutron packets
        int MetadataPktsEn; //!< Switch for metadata packets
        int CmdPktsEn;      //!< Switch for command packets
        int UnknwnPktsEn;   //!< Switch for unrecognized packets
        int SavedCount;     //!< Num saved packets to file
        int NotSavedCount;  //!< Num not saved packets due error
        int CorruptOffset;  //!< Corrupted data absolute offset in file
        #define LAST_DUMPPLUGIN_PARAM CorruptOffset
};

#endif // DUMP_PLUGIN_H
