program GlobalCon

/*
 * Global control sequencer for nED. Supports the operations of global stop,
 * start, check, write config, and clear from soft control records. Global 
 * success is indicated to the operator and the operation may be retried, in 
 * which case it is performed only on those devices which failed before. This 
 * accumulation behavior is reset when a different operation or the clear 
 * operation is requested. Each device has a disable PV allowing it to be 
 * skipped in non-clear operations.
 * 
 * The list of devices to operate on is obtained with seqGrep(), which looks
 * at the EPICS database in the IOC memory. seqGrep() is part of the 
 * pvMediator app.
 */

%%#include <string.h>
%%#include <seqGrep.h>

option -c;	/* do not wait for PVs to connect */

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
#define SKIPPEDRSP	5


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

#define PVSEVROK	0
/* 
 * MAXDEVICES is for static mem allocation and must be higher than number of
 * devices expected.
 */
#define MAXDEVICES 100	

/*
 * set PV n to numeric value v and put. 
 */
#define PVPUTNUM(n,v) { n = v; pvPut(n); }	

/*
 * set PV n to string value s and put.
   s is assumed to be a string, need not be quoted 
 */
#define PVPUTSTR(n,s) {strcpy(n, #s); pvPut(n); }

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
PVARRAY(string, hwtype)
PVARRAY(int, cfgSection)

/*
 * Groups of soft PVs, one per device
 */
PVARRAY(int, disable)
PVARRAY(int, cmdSucceeded)
PVARRAY(string, failMsg)

/*
 * These are global request soft PVs. Treated as "busy" records in that they
 * are acted upon when set to "1" and reset when the operation completes.
 */
DECLEVENT(int, globalEnable, {GPREFIX}Enable)
DECLEVENT(int, globalDisable, {GPREFIX}Disable)
DECLEVENT(int, globalStop, {GPREFIX}Stop)
DECLEVENT(int, globalStart, {GPREFIX}Start)
DECLEVENT(int, globalCheck, {GPREFIX}Check)
DECLEVENT(int, globalWriteConf, {GPREFIX}WriteConf)

/*
 * these are meant to indicate progress in sequencer operations on GUI.
 * prevReq is used in the success accumulation algorithm.
 */
DECL(string, statePV, {GPREFIX}State)
DECL(string, prevReq, {GPREFIX}PrevReq)

/*
 * ReqSucceeded will be set true when one or a series of attempts to perform 
 * an operation succeeded for the percentage of devices (pctSucceeded) 
 * specified by pctCriterion. 
 */
DECL(int, requestSucceeded ,{GPREFIX}ReqSucceeded)
DECL(float, pctSucceeded, {GPREFIX}PctSucceeded)
DECL(float, pctCriterion, {GPREFIX}PctCriterion)

/*
 * for benefit of run control, a single bit
 */
DECL(int, running, {GPREFIX}Running)

/*
 * Variables used by sequencer to keep track of success or failure of 
 * individual device operations.
 */
int cmdFailed[MAXDEVICES];
int numSucceeded = 0;
int numEnabled = 0;

/*
 * List of devices found in IOC using seqGrep. numFound indicates number found.
 */
string deviceNames[MAXDEVICES];
DECL(int, numFound, {GPREFIX}NumFound)

/*
 * common variables
 */
int i;
int timedout;
string namebuf;
int severity;

/*
 * Macros used in constructing states and combinations of states
 */
/*
 * SET_DISABLES used for setting all modules enabled or disabled
 */
#define SET_DISABLES(NAME, VAL) 			\
    state NAME {					\
       when() {						\
          for (i = 0; i< numFound; i++) {		\
             disable[i] = VAL; pvPut(disable[i]);	\
          }						\
       } state Ready					\
    }  							\
/*
 * CMD_BEGIN() and CMD_END() are a pair of macros that together make two states
 * that send COMMAND to all devices. The same NAME must be passed to each, and 
 * NEXT should be the NAME passed to the RSP_BEGIN, RSP_END pair of macros,
 * which should follow.  NEXT must differ from NAME. CMD_BEGIN is called first 
 * and may be followed by any additional when() clauses before CMD_END is 
 * called. See the macro definitions for CMD_RSP() and CMD_RSP_IFNOT() below.
 */
#define CMD_BEGIN(NAME, NEXT) 					\
    state NAME {						\
       when() {							\
          i = -1;						\
       } state NAME##0						\
    }								\
    state NAME##0 {						\
       option -e;						\
       entry {							\
          i = i+1;						\
       } 							\
       when (i >= numFound) { 					\
       } state NEXT						\
       when (disable[i] || cmdFailed[i]) {	\
       } state NAME##0	

#define CMD_END(NAME, COMMAND)					\
       when() {							\
          cmdReq[i] = COMMAND;					\
          pvPut(cmdReq[i], SYNC);				\
          severity = pvSeverity(cmdReq[i]);			\
          if (severity != PVSEVROK) {				\
             cmdFailed[i] = 1;					\
             sprintf(failMsg[i], "Failed %s PV sev %d", 	\
                                  #NAME, severity);		\
             pvPut(failMsg[i]);					\
          }							\
       } state NAME##0						\
    }	
/*
 * RSP_BEGIN() and RSP_END() are a pair of macros that together make two states
 * that check the responses of all devices to the last command. The same NAME 
 * must be passed to each, and NEXT should be the next state. RSP_BEGIN is 
 * called first and may be followed by any additional when() clauses before 
 * RSP_END is called. See the macro definitions for CMD_RSP() and 
 * CMD_RSP_IFNOT() below.
 */
#define RSP_BEGIN(NAME, NEXT)					\
    state NAME {						\
       when() {							\
          i = -1; timedout = 0;					\
       } state NAME##0						\
    }								\
    state NAME##0 {						\
       option -e;						\
       entry {							\
          i = i+1;						\
       } 							\
       when (i >= numFound) { 					\
       } state NEXT						\
       when (disable[i] || cmdFailed[i]) {	\
       } state NAME##0	


#define RSP_END(NAME)						\
       when () {						\
       } state NAME##1						\
    }								\
    state NAME##1 {						\
       when(cmdRsp[i] == SUCCESSRSP) {				\
       } state NAME##0							\
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||	\
             cmdRsp[i] == NORSP) { 					\
          cmdFailed[i] = 1;					\
          sprintf(failMsg[i], "Failed %s %d", #NAME, cmdRsp[i]);	\
          pvPut(failMsg[i]);					\
       } state NAME##0						\
       when (timedout) {					\
          cmdFailed[i] = 1;					\
          sprintf(failMsg[i], "Timeout %s %d", #NAME, cmdRsp[i]);	\
          pvPut(failMsg[i]);					\
       } state NAME##0						\
       when (delay(3.0)) {					\
          cmdFailed[i] = 1;					\
          sprintf(failMsg[i], "Timeout %s %d", #NAME, cmdRsp[i]);	\
          pvPut(failMsg[i]);					\
          timedout = 1;						\
       } state NAME##0						\
    }					

/*
 * SKIP_WHEN() is meant to be inserted between CMD_BEGIN() and CMD_END() or
 * between RSP_BEGIN() and RSP(END), or even between PV_TEST_BEGIN() 
 * PV_TEST_END() (see below). NAME must the the same as the NAME in the 
 * enclosing macros, and TYPE is meant to a hardware type from hwtypes[].
 * The generated when() will skip the operation for that hardware type.
 */

#define SKIP_WHEN(NAME, TYPE) 					\
      when (0 == strcmp(hwtype[i], #TYPE)) {			\
      } state NAME##0

/*
 * In the beginning, there was CMD_RSP().  It now survives as a combination
 * of the macros into which it was divided.
 */
#define CMD_RSP(CRNAME, COMMAND, CRNEXT)				\
    CMD_BEGIN(CRNAME, CRNAME##R)					\
    CMD_END(CRNAME, COMMAND)						\
    RSP_BEGIN(CRNAME##R, CRNEXT)					\
    RSP_END(CRNAME##R)

/*
 * Analog of CMD_RSP(), but with SKIP_WHEN() clauses for types of hardware 
 * that do not support start and stop
 */

#define CMD_RSP_IFNOT(CRNAME, COMMAND, CRNEXT)				\
    CMD_BEGIN(CRNAME, CRNAME##R)					\
    SKIP_WHEN(CRNAME, FEM)						\
    SKIP_WHEN(CRNAME, ACPC FEM)						\
    CMD_END(CRNAME, COMMAND)						\
    RSP_BEGIN(CRNAME##R, CRNEXT)					\
    SKIP_WHEN(CRNAME##R, FEM)						\
    SKIP_WHEN(CRNAME##R, ACPC FEM)					\
    RSP_END(CRNAME##R)

/* 
 * TEST_PV_BEGIN() and TEST_PV_END() are a pair of macros that produce two 
 * states that iterate through all the devices, testing a PV for a desired
 * numeric value.  NAME is the base name of the states and NEXT is the state
 * to go to after the tests are complete. PVNAME is the name of the PV to 
 * test and DESIRED is the desired value. TEST_PV_BEGIN() must be called first,
 * followed if desired by any when() clauses desired and then TEST_PV_END(). 
 * the two macros must share the same NAME.  See macro definitions for 
 * TEST_PV() and TEST_PV_IFNOT() below.
 */
#define TEST_PV_BEGIN(NAME, NEXT)				\
    state NAME {						\
       when() {							\
          i = -1;						\
       } state NAME##0						\
    }								\
    state NAME##0 {						\
       option -e;						\
       entry {							\
          i = i+1;						\
       } 							\
       when (i >= numFound) { 					\
       } state NEXT						\
       when (disable[i] || cmdFailed[i]) {	\
       } state NAME##0	

#define TEST_PV_END(NAME, PVNAME, DESIRED)			\
       when() {							\
          pvGet(PVNAME[i], SYNC);				\
          if (PVNAME[i] != DESIRED) {				\
             cmdFailed[i] = 1;					\
             sprintf(failMsg[i], "%s %d", #NAME, cmdRsp[i]);	\
             pvPut(failMsg[i]);					\
          }							\
       } state NAME##0						\
    }	

/* 
 * In the beginning there was TEST_PV(); it survives as a combination of the
 * macros into which it was divided.
 */
#define TEST_PV(NAME, PVNAME, DESIRED, NEXT)			\
     TEST_PV_BEGIN(NAME, NEXT)					\
     TEST_PV_END(NAME, PVNAME, DESIRED)

/*
 * Analogous to TEST_PV(), but with when() clause added to skip on hardware
 * type for types that dont support start and stop.
 */
#define TEST_PV_IFNOT(NAME, PVNAME, DESIRED, NEXT)		\
     TEST_PV_BEGIN(NAME, NEXT)					\
     SKIP_WHEN(NAME, FEM)					\
     SKIP_WHEN(NAME, ACPC FEM)					\
     TEST_PV_END(NAME, PVNAME, DESIRED)

/*
 * Macro to make an end-of-operation state.  This goes to the Ready state
 * when done. NAME is the entry name of the state and EVNAME is the PV name
 * that invoked the state, ie, the EVNAME or the corresponding CMD_WHEN.
 */
#define OP_DONE(NAME, EVNAME) 						\
   state NAME {								\
      when() {								\
          numSucceeded = 0; numEnabled = 0;				\
          for (i = 0; i< numFound; i++) {				\
             if (!disable[i]) {						\
                numEnabled++;						\
                if (cmdFailed[i]) {					\
                   cmdFailed[i] = 0;	/* ready for next use */	\
                } else {						\
                   failMsg[i][0] = 0;					\
                   pvPut(failMsg[i]);					\
                   PVPUTNUM(cmdSucceeded[i],1)				\
                   numSucceeded++;					\
                }							\
             }		   						\
          }								\
          if (numEnabled == 0)						\
             pctSucceeded = 0.0;					\
          else 								\
             pctSucceeded = (100.0 * numSucceeded)/numEnabled;		\
          pvPut(pctSucceeded);						\
          pvGet(pctCriterion);						\
          requestSucceeded = (pctSucceeded >= pctCriterion)? 1 : 0;	\
          pvPut(requestSucceeded,SYNC);					\
          strcpy(prevReq, statePV); pvPut(prevReq);			\
          running = (strcmp(prevReq, "Starting") == 0 && 		\
                                  requestSucceeded == 1);		\
          pvPut(running, SYNC);						\
          EVNAME = 0;	/* release busy record */			\
          pvPut(EVNAME);						\
          PVPUTSTR(statePV,Ready)					\
      } state Ready							\
   }
/* 
 * Macro to define a global operation request-monitor when clause. EVNAME is
 * the base name of the event (ie, the variable name), OP_NAME is the operation
 * code from the "OP_xxx" command defines above, and NEXT is the destination
 * state to perform the operation.
 */
#define CMD_WHEN(EVNAME, NEXT) 			\
       when (efTestAndClear(EVNAME##Event) && EVNAME == 1) {	\
          PVPUTSTR(statePV,NEXT)				\
          if (strcmp(prevReq, #NEXT)) {				\
             for (i = 0; i< numFound; i++)			\
                PVPUTNUM(cmdSucceeded[i], 0)			\
          }							\
       } state NEXT

/*
 * Main state set.
 */
ss GlobalControl {
   state init {
      entry {
         PVPUTSTR(statePV,init)
         /* find set of devices we are interested in */
         numFound = seqGrep("stringin", ".*:FailMsg", &deviceNames[0][0], 
                                                       MAXDEVICES);
         pvPut(numFound);
         for (i=0; i<numFound; i++) {
            pvAssign(failMsg[i], deviceNames[i]);
            *strrchr(deviceNames[i], ':') = 0;
            sprintf(namebuf, "%s:CfgSection", deviceNames[i]);
            pvAssign(cfgSection[i], namebuf);
            cfgSection[i] = 0;	/* should be and stay that way: Full config */
            sprintf(namebuf, "%s:HwType", deviceNames[i]);
            pvAssign(hwtype[i], namebuf);
            pvGet(hwtype[i]);
            sprintf(namebuf, "%s:CmdReq", deviceNames[i]);
            pvAssign(cmdReq[i], namebuf);
            sprintf(namebuf, "%s:CmdRsp", deviceNames[i]);
            pvAssign(cmdRsp[i], namebuf);
            pvMonitor(cmdRsp[i]);
            sprintf(namebuf, "%s:Verified", deviceNames[i]);
            pvAssign(verified[i], namebuf);
            sprintf(namebuf, "%s:Acquiring", deviceNames[i]);
            pvAssign(acquiring[i], namebuf);
            sprintf(namebuf, "%s:Disable", deviceNames[i]);
            pvAssign(disable[i], namebuf);
            pvMonitor(disable[i]);
            sprintf(namebuf, "%s:CmdSucceeded", deviceNames[i]);
            pvAssign(cmdSucceeded[i], namebuf);
            PVPUTNUM(cmdSucceeded[i], 0)
            sprintf(namebuf, "%s:FailMsg", deviceNames[i]);
            pvAssign(failMsg[i], namebuf);
            cmdFailed[i] = 0;
         }
       }
       when(numFound && pvConnectCount() == pvAssignCount()) {
          PVPUTSTR(prevReq,statePV)	
          strcpy(prevReq, statePV); pvPut(prevReq);
          PVPUTSTR(statePV,Discover)	
       } state Discover
       when (delay(5)) {
          printf("pvConnectCount = %ld pvAssignCount = %ld numFound = %d\n",
                  pvConnectCount(), pvAssignCount(), numFound);
       } state init
    }
    CMD_RSP(Discover, DISCOVERCMD, ReadVersion)
    CMD_RSP(ReadVersion, READVERCMD, CheckVerify)
    TEST_PV(CheckVerify, verified, BOTHOKVER, CheckReadStatus)
    CMD_RSP(CheckReadStatus, READSTATCMD, DoneDiscover)
    OP_DONE(DoneDiscover, globalCheck)
    state Ready {
       CMD_WHEN(globalEnable, Enabling)
       CMD_WHEN(globalEnable, Disabling)
       CMD_WHEN(globalStop, Stopping)
       CMD_WHEN(globalStart, Starting)
       CMD_WHEN(globalWriteConf, WritingConf)
       CMD_WHEN(globalCheck, Discover)
    }		
    SET_DISABLES(Enabling, 0)
    SET_DISABLES(Disabling, 1)
    CMD_RSP_IFNOT(Starting, STARTACQCMD, StartReadStatus)
    CMD_RSP(StartReadStatus, READSTATCMD, StartTestAcq)
    TEST_PV_IFNOT(StartTestAcq, acquiring, ACQUIRING, DoneStarting)
    OP_DONE(DoneStarting, globalStart)
    CMD_RSP_IFNOT(Stopping, STOPACQCMD, StopReadStatus)
    CMD_RSP(StopReadStatus, READSTATCMD, StopTestAcq)
    TEST_PV_IFNOT(StopTestAcq, acquiring, NOTACQUIRING, DoneStopping)
    OP_DONE(DoneStopping, globalStop)
    state WritingConf {
       when() {
          for (i = 0; i< numFound; i++) {	
             if (!disable[i]) {			
                cfgSection[i]= 0; pvPut(cfgSection[i], SYNC);		
             }
          }
       } state DoWriting
    }
    CMD_RSP(DoWriting, WRITECONFCMD, DoneWriting)
    OP_DONE(DoneWriting, globalWriteConf)
}       
       
