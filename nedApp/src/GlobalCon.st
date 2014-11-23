program globalCon

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
#define MAXDEVICES 10	

/*
 * set PV n to numeric value v and put. 
 */
#define PVPUTNUM(n,v) { n = v; pvPut(n); }	

/*
 * set PV n to string value s and put.
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

/*
 * Groups of soft PVs, one per device
 */
PVARRAY(int, disable)
PVARRAY(int, cmdSucceeded)

/*
 * These are global request soft PVs. Treated as "busy" records in that they
 * are acted upon when set to "1" and reset when the operation completes.
 */
DECLEVENT(int, globalClear, {GPREFIX}Clear)
DECLEVENT(int, globalStop, {GPREFIX}Stop)
DECLEVENT(int, globalStart, {GPREFIX}Start)
DECLEVENT(int, globalCheck, {GPREFIX}Check)
DECLEVENT(int, globalWriteConf, {GPREFIX}WriteConf)

/*
 * these are meant to indicate progress in sequencer operations on GUI
 */
DECL(string, currentDev, {GPREFIX}CurrentDev)
DECL(string, statePV, {GPREFIX}State)

/*
 * ReqSucceeded will be set true when one or a series of attempts to perform 
 * an operation succeeded for all devices. pctSucceeded will give a 
 * percentage of devices that succeeded.
 */
DECL(int, requestSucceeded ,{GPREFIX}ReqSucceeded)
DECL(float, pctSucceeded, {GPREFIX}PctSucceeded)

/*
 * Variables used by sequencer to keep track of success or failure of 
 * individual device operations.
 */
int cmdFailed[MAXDEVICES];
int numSucceeded = 0;
int numEnabled = 0;

/*
 * List of devices found by looking for all :CmdReq PVs in IOC using seqGrep,
 * then truncating at ":". numFound indicates how many found
 */
string deviceNames[MAXDEVICES];
string acqNames[MAXDEVICES];
int numFound;
int acqsFound;
int acqsAssigned;

/*
 * variable used by sequencer to keep track of what the previous operation was
 */
int lastRequest = 0;

/*
 * common variables
 */
int i;
int j;
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
          PVPUTSTR(statePV,NAME)					\
          for (i = 0; i < numFound; i++) {			\
             /*								\
                Note that cmdSucceeded[i] means it worked in the 	\
                previous attempt and cmdFailed[i] means something  	\
                failed already in this attempt. In either case, skip it.\
              */							\
             if (!disable[i] && !cmdSucceeded[i] && !cmdFailed[i]) {	\
                cmdReq[i] = COMMAND;				\
                pvPut(cmdReq[i], SYNC);				\
             }							\
          }							\
          i = -1;						\
       } state NAME##1						\
    }								\
    state NAME##1 {						\
       option -e;						\
       entry {							\
          i = i+1;						\
       } 							\
       when (i >= numFound) { 					\
       } state NEXT						\
       when (disable[i] || cmdSucceeded[i] || cmdFailed[i]) {	\
       } state NAME##1						\
       when () {						\
          strcpy(currentDev, deviceNames[i]);			\
          pvPut(currentDev);					\
       } state NAME##2						\
    }								\
    state NAME##2 {						\
       when(cmdRsp[i] == SUCCESSRSP) {				\
          printf("state %s dev %d success = %d\n", #NAME, i, cmdRsp[i]);		\
       } state NAME##1							\
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||	\
             cmdRsp[i] == NORSP) { 					\
          printf("state %s dev %d failure = %d\n", #NAME, i, cmdRsp[i]);		\
          cmdFailed[i] = 1;					\
       } state NAME##1						\
    }					

/*
 * Macro to produce a state to iterate through all devices testing
 * whether PV PVNAME is at state DESIRED. PV is assumed not to be monitored. 
 * NAME is the name of the entry state and NEXT is the state to go to after
 * completion.
 */
#define TEST_PV(NAME, PVNAME, DESIRED, NEXT)			\
    state NAME {						\
       when() {							\
          PVPUTSTR(statePV,NAME)					\
          for (i = 0; i < numFound; i++) {			\
             /* 							\
                Note that some PVs such as Acquiring are not present in \
                all cases and then will not be assigned.		\
              */							\
             if (!disable[i] && !cmdSucceeded[i] && !cmdFailed[i] &&	\
                       pvAssigned(PVNAME[i])) {				\
                pvGet(PVNAME[i], SYNC);					\
                if (PVNAME[i] != DESIRED) cmdFailed[i] = 1;		\
             }							\
          }							\
       } state NEXT						\
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
                if (!disable[i]) {					\
                   requestSucceeded = 0;				\
                   numEnabled++;					\
                }							\
             }								\
             else							\
             {								\
                if (!disable[i]) {					\
                   PVPUTNUM(cmdSucceeded[i],1)				\
                   numSucceeded++;					\
                   numEnabled++;					\
                }							\
             }								\
          }								\
          pvPut(requestSucceeded, SYNC);				\
          if (requestSucceeded) {					\
             pctSucceeded = 100.0;					\
          } else {							\
             if (numEnabled == 0)					\
                pctSucceeded = 0.0;					\
             else							\
                pctSucceeded = (100.0 * numSucceeded)/numEnabled;	\
          }								\
          pvPut(pctSucceeded, SYNC);					\
          EVNAME = 0;	/* release busy record */			\
          pvPut(EVNAME);						\
          PVPUTSTR(statePV,ready)						\
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
          PVPUTSTR(statePV,NEXT)					\
          if (lastRequest != OP_NAME) {				\
             for (i = 0; i< numFound; i++)			\
                PVPUTNUM(cmdSucceeded[i], 0)			\
             numSucceeded = 0;					\
             numEnabled = 0;					\
          }							\
          lastRequest = OP_NAME;				\
       } state NEXT

/*
 * Main state set.
 */
ss globalControl {
   state init {
      entry {
         PVPUTSTR(statePV,init)
         /* What we want here is to find the baseModulePlugin devices */
         numFound = seqGrep("stringin", ".*:HwId", &deviceNames[0][0], 
                                                       MAXDEVICES);
         for (i=0; i<numFound; i++) {
            *strrchr(deviceNames[i], ':') = 0;
            sprintf(namebuf, "%s:CmdReq", deviceNames[i]);
            pvAssign(cmdReq[i], namebuf);
            sprintf(namebuf, "%s:CmdRsp", deviceNames[i]);
            pvAssign(cmdRsp[i], namebuf);
            pvMonitor(cmdRsp[i]);
            sprintf(namebuf, "%s:Verified", deviceNames[i]);
            pvAssign(verified[i], namebuf);
            sprintf(namebuf, "%s:disable", deviceNames[i]);
            pvAssign(disable[i], namebuf);
            pvMonitor(disable[i]);
            sprintf(namebuf, "%s:cmdSucceeded", deviceNames[i]);
            pvAssign(cmdSucceeded[i], namebuf);
            PVPUTNUM(cmdSucceeded[i], 0)
            cmdFailed[i] = 0;
         }
         /* not all devices have the Acquiring PV */
         acqsFound = seqGrep("bi", ".*:Acquiring", &acqNames[0][0], 
                                                       MAXDEVICES);
         for (i=0; i<acqsFound; i++) {
            *strrchr(acqNames[i], ':') = 0;
         }
         i = 0; j = 0; acqsAssigned = 0;
         while (j < acqsFound) {
            while (i < numFound) {
               if (!strcmp(acqNames[j], deviceNames[i])) {
                  sprintf(namebuf, "%s:Acquiring", deviceNames[i]);
                  pvAssign(acquiring[i], namebuf);
                  acqsAssigned++;
                  i++;
                  break;
               }
               i++;
            }
            j++;
         }
         if (acqsAssigned != acqsFound) 
            printf("Warning: acqsFound = %d but acqsAssigned = %d\n", 
                     acqsFound, acqsAssigned);
       }
       when(numFound && pvConnectCount() == pvAssignCount()) {
          lastRequest = OP_CHECK;
       } state Discover
       when (delay(2)) {
          printf("pvConnectCount = %ld but pvAssignCount = %ld\n",
                  pvConnectCount(), pvAssignCount());
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
                PVPUTNUM(cmdSucceeded[i],1)
                cmdFailed[i] = 0;	
          }
       } state DoneClear
    }
    OP_DONE(DoneClear, globalClear)
}       
       
