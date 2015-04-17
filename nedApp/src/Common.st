/**
 * Common sequencer definitions.
 */

/*
 * Must match CmdRsp record of BaseModulePlugin
 */
#define RSP_SUCCESS             1
#define RSP_WAITING             2
#define RSP_TIMEOUT             3

/*
 * Macro to declare and assign PV. t is the type, n the variable name, and
 * s is the PV name.
 */
#define DECL(t,n,s)     \
    t n;                \
    assign n to s;

/*
 * Macro to declare a PV, assign, monitor.
 * t is the type, n the variable name, and s is the PV name.
 */
#define DECLMON(t,n,s)  \
        t n;            \
        assign n to s;  \
        monitor n;

/*
 * Macro to assign variable to new channel without monitor.
 *
 * ch is a new channel name as well as ending part of the PV name.
 * prefix is leading part of PV name.
 *
 * For example PV_ASSIGN(CmdReq, "BL100:Det:dsp1:") assigns
 * PV BL100:Det:dsp1:CmdReq to channel named CmdReq.
 */
#define PV_ASSIGN(ch, prefix) pvAssign(ch, pv_concat(prefix, #ch))

/*
 * Macro to assign a monitored variable to new channel.
 *
 * ch is a new channel name as well as ending part of the PV name.
 * prefix is leading part of PV name.
 *
 * For example PV_ASSIGN(CmdReq, "BL100:Det:dsp1:") assigns
 * PV BL100:Det:dsp1:CmdReq to channel named CmdReq.
 */
#define PV_ASSIGN_MON(ch, prefix) \
    pvAssign(ch, pv_concat(prefix, #ch)); \
    pvMonitor(ch)

/*
 * Macro to un-assign a monitored variable from current channel.
 */
#define PV_UNASSIGN(ch) if (pvAssigned(ch)) pvAssign(ch, "")

/*
 * Macro to un-assign a variable from current channel.
 */
#define PV_UNASSIGN_MON(ch) \
    if (pvAssigned(ch)) { \
        pvStopMonitor(ch); \
        pvAssign(ch, ""); \
    }

/*
 * set PV n to numeric value v and put.
 */
#define PVPUTNUM(n,v) { n = v; pvPut(n, SYNC); }

/*
 * set PV n to string value s and put.
   s is assumed to be a string, need not be quoted
 */
#define PVPUTSTR(n,s) {strcpy(n, s); pvPut(n); }

/*
 * Macro to declare a new synchronous command sending state.
 *
 * Send a single command and wait for response.
 * s is state name, cmd is command to be sent
 * onsuccess is next state on success,
 * onerror is next state on error,
 * ontimeout is next state on timeout received from nED.
 *
 * New state assumes nED will change CmdRsp to non-Waiting in 2 seconds.
 * If no change in 3 seconds, there's something wrong with nED and
 * next state is onerror.
 *
 * Assumes CmdReq and CmdRsp channels are assigned but not monitored.
 */
int statecmd_retries;
#define STATECMD(s, cmd, retries, onsuccess, onerror, ontimeout) \
    state s { \
        when () { \
            statecmd_retries = 0; \
        } state s##_send \
    } \
    state s##_send { \
        when () { \
            statecmd_retries += 1; \
            CmdReq = cmd; \
            pvPut(CmdReq, SYNC); \
        } state s##_wait \
    } \
    state s##_wait { \
        option -e; \
        option -t; \
        entry { \
            pvGet(CmdRsp, SYNC); \
        } \
        when (CmdRsp == RSP_SUCCESS) { \
        } state onsuccess \
        when (CmdRsp == RSP_TIMEOUT && statecmd_retries <= retries) { \
        } state s##_send \
        when (CmdRsp == RSP_TIMEOUT /* retries > retries */) { \
        } state ontimeout \
        when (CmdRsp != RSP_WAITING) { \
        } state onerror \
        when (delay(3.0)) { \
        } state ontimeout \
        when () { \
        } state s##_wait \
    }

/*
 * Concatenate two strings to form a valid PV name.
 * Put ':' in between unless a already ends with ':'.
 */
static char *pv_concat(const char *a, const char *b) {
    static char pvname[128];
    int len;
    strncpy(pvname, a, sizeof(pvname));
    len = strlen(pvname);
    if (len > 0 && pvname[len-1] != ':')
        strncat(pvname, ":", sizeof(pvname) - (++len));
    strncat(pvname, b, sizeof(pvname) - len);
    pvname[sizeof(pvname)-1] = '\0';
    return pvname;
}

