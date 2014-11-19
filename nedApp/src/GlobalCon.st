program globalCon

/*
 * Global control sequencer for nED. Supports the operations of global stop,
 * start, check, write config, and clear from soft control records. Global 
 * success is indicated to the operator and the operation may be retried, in 
 * which case it is performed only on those devices which failed before. This 
 * accumulation behavior is reset when a different operation or the clear 
 * operation is requested. Each device has a disable PV allowing it to be 
 * skipped in non-clear operations.
 */

%%#include <string.h>
%%#include <strings.h>		/* for rindex() */
%%#include <seqGrep.h>

/*
 * define valid values for CmdReq PVs
 */
#define RESETCMD	0
#define DISCOVERCMD 	1
#define READVERCMD 	2
#define READSTATCMD 	3
#define READCONFCMD	4
#define WRITECONFCMD	5
#define STARTACQCMD	6
#define STOPACQCMD	7
#define READCTRSCMD	8

/*
 * define valid values for CmdRsp PVs
 */
#define NORSP		0
#define SUCCESSRSP	1
#define WAITRESP	2
#define TIMEOUTRSP	3
#define ERRORRSP	4

/*
 * define valid values for Verified PVs
 */
#define NOTVER		0
#define TYPEOKVER	1
#define TYPEMISVER	2
#define VERSOKVER	3
#define VERSMISVER	4
#define BOTHOKVER	8

/*
 * define valid values for Acquiring PVs
 */
#define NOTACQUIRING	0
#define ACQUIRING	1

/*
 * define previous state values used in sequencer to allow cumulative success
 * retrying of previous operation.
 */
#define OP_CLEAR	0
#define OP_START	1
#define OP_STOP	 	2
#define OP_CHECK	3
#define OP_WRITE	4

/* 
 * MAXDEVICES is for static mem allocation and must be higher than number of
 * devices expected.
 */
#define MAXDEVICES 50	

/*
 * send a string indicating current state. No correspondence to "OP_" series 
 * above.  s is a string naming the state; not enumerated.
 */
#define SENDSTATE(s) {			\
        strcpy(statePV, #s);		\
        pvPut(statePV, SYNC); }

/*
 * Macro to declare and assign PV. t is the type, n the variable name, and 
 * s is the PV name.
 */
#define DECL(t,n,s)		\
	t n;			\
        assign n to #s;      

/*
 * Macro to declare an array of PVs and allocate memory for later runtime 
 * assignment. t is the type and n the variable name.
 */
#define PVARRAY(t,n) 		\
	t n[MAXDEVICES];	\
	assign n[0] to "";

/*
 * Macro to declare a PV, assign, monitor, and link with an event flag. 
 * t is the type, n the variable name, and s is the PV name.
 */
#define DECLEVENT(t,n,s)        \
        t n;                    \
        assign n to #s;         \
        monitor n;              \
        evflag n##Event;        \
        sync n n##Event;        

        
/*
 * These arrays correspond to groups of nED interface PVs, one per device
 */
PVARRAY(int, cmdReq)
PVARRAY(int, cmdRsp)
PVARRAY(int, verified)
PVARRAY(int, acquiring)

/*
 * This is a group of soft PVs, one per device
 */
PVARRAY(int, disabled)

/*
 * These are global request soft PVs. Treated as "busy" records in that they
 * are acted upon when set to "1" and reset when the operation completes.
 */
DECLEVENT(int, globalClear, {GPREFIX}:Clear)
DECLEVENT(int, globalStop, {GPREFIX}:Stop)
DECLEVENT(int, globalStart, {GPREFIX}:Start)
DECLEVENT(int, globalCheck, {GPREFIX}:Check)
DECLEVENT(int, globalWriteConf, {GPREFIX}:WriteConf)

/*
 * these are meant to indicate progress in sequencer operations on GUI
 */
DECL(string, currentPV, {PREFIX}:CurrentPV)
DECL(string, statePV, {PREFIX}:State)

/*
 * ReqSucceeded will be set true when one or a series of attempts to perform 
 * an operation succeeded for all devices
 */
DECL(int, requestSucceeded ,{PREFIX}:ReqSucceeded)

/*
 * Variables used by sequencer to keep track of success or failure of 
 * individual device operations.
 */
int cmdSucceeded[MAXDEVICES];
int cmdFailed[MAXDEVICES];

/*
 * List of devices found by looking for all :CmdReq PVs in IOC using seqGrep,
 * then truncating at ":". numFound indicates how many found
 */
string deviceNames[MAXDEVICES];
int numFound = 0;

/*
 * variable used by sequencer to keep track of what the previous operation was
 */
int lastRequest = 0;

/*
 * common variables
 */
int i;
string namebuf;

/*
 * Macros used in constructing state sets and states
 */

/*
 * Macro to produce several states to iterate through all devices sending
 * them all a command then waiting for responses from each of them. NAME is
 * the name of the entry state, COMMAND is the CmdReq command to send, and 
 * NEXT is the state to go to following completion.
 */
#define CMD_RSP(NAME, COMMAND, NEXT) 				\
    state NAME {						\
       when() {							\
          SENDSTATE(NAME)					\
          i = -1;						\
       } state NAME##2						\
    }								\
    state NAME##2 {						\
       option -e;						\
       entry {							\
          i++;							\
       }							\
       when (i >= numFound) { 				\
       } state NAME##3						\
       /*								\
          Note that cmdSucceeded[i] means it worked in the previous 	\
          attempt to initialize and cmdFailed[i] means something 	\
          failed already in this attempt.  In either case, skip it.	\
        */								\
       when (disabled[i] || cmdSucceeded[i] || cmdFailed[i]) 	{	\
       } state NAME##2							\
       when() {							\
          cmdReq[i] = DISCOVERCMD;				\
          pvPut(cmdReq[i], SYNC);				\
       } state NAME##2						\
    }								\
    state NAME##3 {						\
       when() {							\
          i = -1;						\
       } state NAME##4						\
    }								\
    state NAME##4 {						\
       option -e;						\
       entry {							\
          i = i+1;						\
       } 							\
       when (i >= numFound) { 				\
       } state NEXT						\
       when (disabled[i] || cmdSucceeded[i] || cmdFailed[i]) {	\
       } state NAME##4						\
       when () {						\
          strcpy(currentPV, deviceNames[i]);			\
          pvPut(currentPV,SYNC);				\
       } state NAME##5						\
    }								\
    state NAME##5 {						\
       when(cmdRsp[i] == SUCCESSRSP) {				\
       } state NAME##4							\
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||	\
             cmdRsp[i] == NORSP) { 					\
          cmdFailed[i] = 1;					\
       } state NAME##4						\
    }					

/*
 * Macro to produce several states to iterate through all devices testing
 * whether PV PVNAME is at state DESIRED. PV is assumed not to be monitored. 
 * NAME is the name of the entry state and NEXT is the state to go to after
 * completion.
 */
#define TEST_PV(NAME, PVNAME, DESIRED, NEXT)			\
    state NAME {						\
       when() {							\
          SENDSTATE(NAME)					\
          i = -1;						\
       } state NAME##2						\
    }								\
    state NAME##2 {						\
       option -e;						\
       entry {							\
          i++;							\
       }							\
       when (i >= numFound) { 					\
       } state NEXT						\
       /*								\
          Note that cmdSucceeded[i] means it worked in the previous 	\
          attempt and cmdFailed[i] means something failed already in 	\
          this attempt.  In either case, skip it.			\
        */								\
       when (disabled[i] || cmdSucceeded[i] || cmdFailed[i]) {		\
       } state NAME##2						\
       when() {							\
          pvGet(PVNAME[i], SYNC);				\
          if (PVNAME[i] != DESIRED) cmdFailed[i] = 1;		\
       } state NAME##2						\
    }	
/*
 * Macro to make an end-of-operation state.  This goes to the ready state
 * when done. NAME is the entry name of the state and EVNAME is the PV name
 * that invoked the state, ie, the EVNAME or the corresponding CMD_WHEN.
 */
#define OP_DONE(NAME, EVNAME) 						\
   state NAME {								\
      when() {								\
          requestSucceeded = 1;						\
          for (i = 0; i< numFound; i++) {				\
             if (cmdFailed[i]) {					\
                cmdFailed[i] = 0;	/* ready for next use */	\
                if (!disabled[i]) {					\
                   requestSucceeded = 0;				\
                }							\
             }								\
             else							\
             {								\
                if (!disabled[i])					\
                    cmdSucceeded[i] = 1;/* add succeeded device */	\
             }								\
          }								\
          pvPut(requestSucceeded, SYNC);				\
          EVNAME = 0;	/* release busy record */			\
          pvPut(EVNAME, SYNC);						\
          SENDSTATE(ready)						\
      } state ready							\
   }
/* 
 * Macro to define a global operation request-monitor when clause. EVNAME is
 * the base name of the event (ie, the variable name), OP_NAME is the operation
 * code from the "OP_xxx" command defines above, and NEXT is the destination
 * state to perform the operation.
 */
#define CMD_WHEN(EVNAME, OP_NAME, NEXT) 			\
       when (efTestAndClear(EVNAME##Event) && EVNAME == 1) {	\
          SENDSTATE(NEXT)					\
          if (lastRequest != OP_NAME) 				\
             for (i = 0; i< numFound; i++)			\
                cmdSucceeded[i] = 0;				\
          lastRequest = OP_NAME;				\
       } state NEXT

/*
 * Main state set.
 */
ss globalControl {
   state init {
      entry {
         SENDSTATE(init)
         numFound = seqGrep("mbbo", ".*[0-9]*:CmdReq", &deviceNames[0][0], 
                                                       MAXDEVICES);
         for (i=0; i<numFound; i++) {
            *rindex(deviceNames[i], ':') = 0;
            sprintf(namebuf, "%s:CmdReq", deviceNames[i]);
            pvAssign(cmdReq[i], namebuf);
            sprintf(namebuf, "%s:CmdRsp", deviceNames[i]);
            pvAssign(cmdRsp[i], namebuf);
            pvMonitor(cmdRsp[i]);
            sprintf(namebuf, "%s:Verified", deviceNames[i]);
            pvAssign(verified[i], namebuf);
            sprintf(namebuf, "%s:Acquiring", deviceNames[i]);
            pvAssign(acquiring[i], namebuf);
            pvMonitor(acquiring[i]);
            sprintf(namebuf, "%s:SeqDisable", deviceNames[i]);
            pvAssign(disabled[i], namebuf);
            pvMonitor(disabled[i]);
            cmdSucceeded[i] = 0;
            cmdFailed[i] = 0;
         }
       }
       when(numFound && pvConnectCount() == pvAssignCount()) {
          lastRequest = OP_CHECK;
       } state Discover
       when (delay(0.2)) {
       } state init
    }
    CMD_RSP(Discover, DISCOVERCMD, ReadVersion)
    CMD_RSP(ReadVersion, READVERCMD, CheckVerify)
    TEST_PV(CheckVerify, verified, 8, CheckReadStatus)
    CMD_RSP(CheckReadStatus, READSTATCMD, DoneDiscover)
    OP_DONE(DoneDiscover, globalCheck)
    state ready {
       CMD_WHEN(globalStop, OP_STOP, tryStopping)
       CMD_WHEN(globalStart, OP_START, tryStarting)
       CMD_WHEN(globalWriteConf, OP_WRITE, tryWriting)
       CMD_WHEN(globalCheck, OP_CHECK, Discover)
       CMD_WHEN(globalClear, OP_CLEAR, Clear)
    }
    CMD_RSP(tryStarting, STARTACQCMD, StartReadStatus)
    CMD_RSP(StartReadStatus, READSTATCMD, StartTestAcq)
    TEST_PV(StartTestAcq, acquiring, 1, DoneStarting)
    OP_DONE(DoneStarting, globalStart)
    CMD_RSP(tryStopping, STOPACQCMD, StopReadStatus)
    CMD_RSP(StopReadStatus, READSTATCMD, StopTestAcq)
    TEST_PV(StopTestAcq, acquiring, 1, DoneStopping)
    OP_DONE(DoneStopping, globalStop)
    CMD_RSP(tryWriting, WRITECONFCMD, DoneWriting)
    OP_DONE(DoneWriting, globalWriteConf)
    state Clear {
       when() {
          for (i = 0; i< numFound; i++)	{
                cmdSucceeded[i] = 1;	
                cmdFailed[i] = 0;	
          }
       } state DoneClear
    }
    OP_DONE(DoneClear, globalClear)
}       
       
