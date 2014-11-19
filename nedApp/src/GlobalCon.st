program globalCon

%%#include <string.h>

#define RESETCMD	0
#define DISCOVERCMD 	1
#define READVERCMD 	2
#define READSTATCMD 	3
#define READCONFCMD	4
#define WRITECONFCMD	5
#define STARTACQCMD	6
#define STOPACQCMD	7
#define READCTRSCMD	8

#define NORSP		0
#define SUCCESSRSP	1
#define WAITRESP	2
#define TIMEOUTRSP	3
#define ERRORRSP	4

#define NOTVER		0
#define TYPEOKVER	1
#define TYPEMISVER	2
#define VERSOKVER	3
#define VERSMISVER	4
#define BOTHOKVER	8

#define NOTACQUIRING	0
#define ACQUIRING	1

#define LASTRESET	0
#define LASTSTARTING	1
#define LASTSTOPPING	2
#define LASTCHECKING	3
#define LASTWRITING	4

/*
 this is the beam Include file

 NUMDEVICES must match number of strings in init of deviceNames[]
*/
#define NUMDEVICES 2

/*
Each of these devices must be a BaseModulePlugin
string deviceNames[NUMDEVICES] = { "BL99:Det:roc1" };
*/

string deviceNames[NUMDEVICES];
/*
end beam include file
*/

#define PVARRAY(t,n) 		\
	t n[NUMDEVICES];	\
	assign n[0] to "";

#define SENDSTATE(s)			\
        strcpy(prevStatePV, statePV);	\
        strcpy(statePV, #s);		\
        pvPut(prevStatePV);		\
        pvPut(statePV, SYNC);

#define DECLEVENT(t,n,s)        \
        t n;                    \
        assign n to #s;         \
        monitor n;              \
        evflag n##Event;        \
        sync n n##Event;        

        
PVARRAY(int, cmdReq)
PVARRAY(int, cmdRsp)
PVARRAY(int, verified)
PVARRAY(int, acquiring)
PVARRAY(int, disabled)

DECLEVENT(int, globalStop, {GPREFIX}:Stop)
DECLEVENT(int, globalStart, {GPREFIX}:Start)
DECLEVENT(int, globalCheck, {GPREFIX}:Check)
DECLEVENT(int, globalWriteConf, {GPREFIX}:WriteConf)

int cmdSucceeded[NUMDEVICES];
int cmdFailed[NUMDEVICES];

int ourstatus;
assign ourstatus to "{GPREFIX}:Status";

#define MIXED 0
#define STOPPED 1
#define STARTED 2

string currentPV;
assign currentPV to "{PREFIX}:CurrentPV";

int retrySeq;
assign retrySeq to "{PREFIX}:RetrySeq";
monitor retrySeq;
evflag retrySeqEvent;
sync retrySeq retrySeqEvent;

string statePV;
assign statePV to "{PREFIX}:State";

string prevStatePV;
assign prevStatePV to "{PREFIX}:PrevState";

int lastRequest = 0;
int requestFailed = 0;

int i;
string namebuf;

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
       when (i >= NUMDEVICES) { 				\
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
       when (i >= NUMDEVICES) { 				\
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
       when (i >= NUMDEVICES) { 				\
       } state NEXT						\
       /*								\
          Note that cmdSucceeded[i] means it worked in the previous 	\
          attempt to initialize and cmdFailed[i] means something 	\
          failed already in this attempt.  In either case, skip it.	\
        */								\
       when (disabled[i] || cmdSucceeded[i] || cmdFailed[i]) {		\
       } state NAME##2						\
       when() {							\
          pvGet(PVNAME[i], SYNC);				\
          if (PVNAME[i] != DESIRED) cmdFailed[i] = 1;		\
       } state NAME##2						\
    }	

#define CMD_WHEN(EVNAME, LASTNAME, NEXT) 			\
       when (efTestAndClear(EVNAME##Event) && EVNAME == 1) {	\
          EVNAME = 0;						\
          pvPut(EVNAME, SYNC);					\
          SENDSTATE(NEXT)					\
          if (lastRequest != LASTNAME) 				\
             for (i = 0; i< NUMDEVICES; i++)			\
                cmdSucceeded[i] = 0;				\
          lastRequest = LASTNAME;				\
       } state NEXT

ss globalControl {
   state init {
      entry {
         strcpy(statePV, "booted");
         pvPut(statePV);
         SENDSTATE(init)
         for (i=0; i<NUMDEVICES; i++) {
            sprintf(namebuf, "%s:CmdReq", deviceNames[i]);
            pvAssign(cmdReq[i], namebuf);
            sprintf(namebuf, "%s:CmdRsp", deviceNames[i]);
            pvAssign(cmdRsp[i], namebuf);
            sprintf(namebuf, "%s:Verified", deviceNames[i]);
            pvAssign(verified[i], namebuf);
            sprintf(namebuf, "%s:Acquiring", deviceNames[i]);
            pvAssign(acquiring[i], namebuf);
            sprintf(namebuf, "%s:SeqDisable", deviceNames[i]);
            pvAssign(disabled[i], namebuf);
            cmdSucceeded[i] = 0;
            cmdFailed[i] = 0;
         }
         pvMonitor(acquiring);
         pvMonitor(cmdRsp);
         pvMonitor(disabled);
       }
       when(pvConnectCount() == pvAssignCount()) {
          for (i=0; i<NUMDEVICES; i++) cmdFailed[i] = 0;
          lastRequest = LASTCHECKING;
       } state Discover
       when (delay(0.2)) {
       } state init
    }
    CMD_RSP(Discover, DISCOVERCMD, ReadVersion)
    CMD_RSP(ReadVersion, READVERCMD, CheckVerify)
    TEST_PV(CheckVerify, verified, 8, ReadStatus)
    CMD_RSP(ReadStatus, READSTATCMD, ready)
    state ready {
       entry {
          for (i = 0; i< NUMDEVICES; i++)
             if (cmdFailed[i]) {
                cmdFailed[i] = 0;		/* ready for next use */
                if (!disabled[i])
                   requestFailed = 1;
             }
             else
             {
                if (!disabled[i])
                    cmdSucceeded[i] = 1;	/* add succeeded device */
             }
          SENDSTATE(ready)
       }
       CMD_WHEN(globalStop, LASTSTOPPING, tryStopping)
       CMD_WHEN(globalStart, LASTSTARTING, tryStarting)
       CMD_WHEN(globalWriteConf, LASTWRITING, tryWriting)
       CMD_WHEN(globalCheck, LASTCHECKING, Discover)
    }
    CMD_RSP(tryStarting, STARTACQCMD, ReadStatus)
    CMD_RSP(tryStopping, STOPACQCMD, ReadStatus)
    CMD_RSP(tryWriting, WRITECONFCMD, ready)
}       
       
