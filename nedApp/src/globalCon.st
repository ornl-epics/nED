program globalCon

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


# this is the beam Include file

# NUMDEVICES must match number of strings in init of deviceNames[]
#define NUMDEVICES 1

# Each of these devices must be a BaseModulePlugin
string deviceNames[NUMDEVICES] = { "BL99:Det:roc1" };

#end beam include file

# declare an array of type t named n for later PV assignment and allocate memory
#define PVARRAY(t,n) 		\
	t n[NUMDEVICES];	\
	assign n[0] to "";

#define SENDSTATE(s)			\
        strcpy(prevStatePV, statePV);	\
        strcpy(statePV, #s);		\
        pvPut(prevStatepv);		\
        pvPut(statePV, SYNC);
        
PVARRAY(int, cmdReq)
PVARRAY(int, cmdRsp)
PVARRAY(int, verified)
PVARRAY(int, acquiring)

int globalCmd;
assign globalCmd to "{GPREFIX}:CmdReq"

int globalRsp;
assign globalRsp to "{GPREFIX}:CmdRsp"
monitor globalRsp;

string currentPV;
assign currentPV to "{PREFIX}:CurrentPV";

int retrySeq;
assign retrySeq to "{PREFIX}:RetrySeq"
monitor retrySeq;
evflag retrySeqEvent;
sync retrySeq retrySeqEvent;

string statePV;
assign statePV to "{PREFIX}:State";

string prevStatePV;
assign prevStatePV to "{PREFIX}:PrevState";

int starting = 0;
int stopping = 0;

ss startStop {
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
       }
       when (pvConnectCount() == pvAssignCount) {
       } state check
       when (delay(0.2)) {
       } state init
    }
    state check {
       entry {
          SENDSTATE(check)
       }
       when() {
          i = 0;
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV,SYNC);
       } state check1
    }
    state check1 {
       entry {
          SENDSTATE(check1)
          cmdReq[i] = DISCOVERCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdRsp[i]);
       } 
       when(cmdRsp[i] == SUCCESSRSP) {
          cmdReq[i] = READVERCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdResp[i]);
       } state check3
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state checkError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state check1
    }
    state check3 {
       entry {
          SENDSTATE(check3)
       }
       when(cmdRsp[i] == SUCCESSRSP) {
          pvGet(verified[i]);
       } state check4
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state checkError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state check3
    }
    state check4 {
       entry {
          SENDSTATE(check4)
       }
       when(verified[i] == BOTHOKVER) {
          cmdReq[i] = READSTATCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdResp[i]);
       } state check5
       when() {
       } state checkError
    }
    state check5 {
       entry {
          SENDSTATE(check5)
       }
       when (cmdRsp[i] == SUCCESSRSP) {
          pvGet(acquiring[i]);
       } state checkIncr
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state checkError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state check5
    }
    state checkIncr {
       entry {
          i=i+1;
       }
       when (i< NUMDEVICES) {
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV);
       } state check1
       when() {
          pvMonitor(acquiring);
       } state ready
    }
    state checkError {
       entry {
          SENDSTATE(checkError)
       }
       when (efTestAndClear(retrySeqEvent) && retrySeq == 1) {
          retrySeq == 0;
          pvPut(retrySeq, SYNC);
       } state check1
    }
    state ready {
       entry {
          SENDSTATE(ready)
       }
       when (starting) { 
          SENDSTATE(starting)
          i = -1
       } state starting1
       when (efTestAndClear(globalStopEvent) && globalStop == 1) {
          globalStart = 0;
          pvPut(globalStart, SYNC);
          SENDSTATE(tryStopping)
          stopping = 1;
       } state tryStopping
       when (efTestAndClear(globalStartEvent) && globalStart == 1) {
          globalStop = 0;
          pvPut(globalStop, SYNC);
          SENDSTATE(tryStarting)
          starting = 1;
       } state tryStarting
       exit {
          i = -1;
       }
    }
    state tryStarting {
       option -e;
       entry {
          i = i+1;
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV);
       }
       when (i >= NUMDEVICES) {
       } state check
       when (acquiring[i] == NOTACQUIRING) {
       } state tryStarting
       when() {
          starting = 0;
          ourstate = mixed;
          pvPut(ourstatus);
       } state ready
    }
    state tryStopping {
       option -e;
       entry {
          i = i+1;
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV);
       }
       when (i >= NUMDEVICES) {
          ourstatus = stopped;
          pvPut(ourstatus);
       } state ready
       when (acquiring[i] == NOTACQUIRING) {
       } state tryStopping
       when (acquiring[i] == ACQUIRING) {
          cmdReq[i] = STOPACQCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdRsp[i]);
       } state tryStopping1
     }
     state tryStopping1 {
       when (cmdRsp[i] == SUCCESSRSP) {
          cmdReq[i] = READSTATCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdRsp[i]);
       } state tryStopping2
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state readyError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state tryStopping1
     }
     state tryStopping2 {
       when (cmdRsp[i] == SUCCESSRSP) {
          pvGet(acquiring[i]);
       } state tryStopping3
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state readyError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state tryStopping2
     }
     state tryStopping3 {
        when(acquiring[i] == NOTAQUIRING) {
        } state tryStopping
        when () {
        } state readyError
     }
     state starting1 {
       option -e;
       entry {
          i = i+1;
       }
       when (i >= NUMDEVICES) {
          i = -1;
       } state starting3
       when () {
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV);
          cmdReq[i] = WRITECONFCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdResp[i]);
       } state starting2
    }
    state starting2 {
       when (cmdRsp[i] == SUCCESSRSP) {
       } state starting1
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state readyError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state starting2
     }
     state starting3 {
       option -e;
       entry {
          i = i+1;
       }
       when (i >= NUMDEVICES) {
       } state running
       when () {
          strcpy(currentPV, deviceNames[i]);
          pvPut(currentPV);
          cmdReq[i] = STARTAQCCMD;
          pvPut(cmdReq[i], SYNC);
          pvGet(cmdResp[i]);
       } state starting4
    }
    state starting4 {
       when (cmdRsp[i] == SUCCESSRSP) {
       } state starting3
       when (cmdRsp[i] == ERRORRSP || cmdRsp[i] == TIMEOUTRSP ||
             cmdRsp[i] == NORSP) { } state readyError
       when(delay(0.2)) {
          pvGet(cmdRsp[i]);
       } state starting4
     }
    state running {
       entry {
          SENDSTATE(Running)
       }
       when (efTestAndClear(globalStopEvent) && globalStop == 1) {
          globalStart = 0;
          pvPut(globalStart, SYNC);
          SENDSTATE(tryStopping)
          stopping = 1;
       } state tryStopping
   }
}       
       


