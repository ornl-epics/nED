/**
 * Helper sequence to apply configuration to a detector that is
 * acquiring. When triggered, state machine executes 4 simple steps:
 * - stop acquisition
 * - write configuration
 * - start acquisition
 * - refresh status
 *
 * State machine is triggered by writing detector id (like roc1) to
 * <seq prefix>:Detector PV.
 */

program ApplyConfig

%%#include <stdio.h>
%%#include <string.h>
%{
#include "Common.st"
}%

#define CMD_READ_STATUS         3
#define CMD_WRITE_CFG           5
#define CMD_START               6
#define CMD_STOP                7

DECL(int, CmdReq,               "")
DECL(int, CmdRsp,               "")
DECL(int, Acquiring,            "")
DECLMON(string, Detector,       "{PREFIX}{SEQ_ID}Detector")
DECL(string, StatusStr,         "{PREFIX}{SEQ_ID}StatusStr")

/**
 * Turn on Safe mode - thread race conditions otherwise
 * especially in CMDSTATE() macro since it's being processed a lot.
 */
option +s;

/**
 * variables
 */
string prefix;
int Acquiring;

ss ApplyConfig {
    state init {
        when () {
            PVPUTSTR(StatusStr, "Idle");
        } state ready
    }

    state ready {
        entry {
            PV_UNASSIGN(CmdReq);
            PV_UNASSIGN(CmdRsp);
        }
        when (Detector[0] != 0) {
            strcpy(prefix, pv_concat(macValueGet("PREFIX"), Detector));
            PV_ASSIGN(CmdReq, prefix);
            PV_ASSIGN(CmdRsp, prefix);
            PV_ASSIGN(Acquiring, prefix);

            // Only run transition once per change
            Detector[0] = 0;
            pvPut(Detector);
        } state wait_connected
        when (delay(10.0)) {
            PVPUTSTR(StatusStr, "Idle");
        } state ready
    }

    state wait_connected {
        when (pvConnectCount() == pvAssignCount()) {
            PVPUTSTR(StatusStr, "Check status");
        } state check_acq
        when (delay(2.0)) {
            PVPUTSTR(StatusStr, "Failed: PV(s) disconnected");
        } state ready
    }

    STATECMD(stop_acq,  CMD_STOP,        1, stopped, error, error)
    STATECMD(write_cfg, CMD_WRITE_CFG,   1, written, error, error)
    STATECMD(start_acq, CMD_START,       1, completed, error, error)

    state completed {
        when () {
            PVPUTSTR(StatusStr, "Succeeded");
        } state ready
    }

    state error {
        when () {
            PVPUTSTR(StatusStr, "Failed");
        } state read_status
    }

    STATECMD(read_status, CMD_READ_STATUS, 0, ready, ready, ready)
}
