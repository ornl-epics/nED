program devCon ("instance=test:Det:roc32")

#define RESETCMD        0
#define DISCOVERCMD     1
#define READVERCMD      2
#define READSTATCMD     3
#define READCONFCMD     4
#define WRITECONFCMD    5
#define STARTACQCMD     6
#define STOPACQCMD      7
#define READCTRSCMD     8

#define NORSP           0
#define SUCCESSRSP      1
#define WAITRESP        2
#define TIMEOUTRSP      3
#define ERRORRSP        4
#define INVALIDRSP	5

#define DONE	0

#define PRINTFAILED(m) \
    sprintf(failed, "%s: Cmd %d returned %d", #m, cmdReq, cmdRsp);	\
    pvPut(failed);

#define DECLEVENT(t,n,s)        \
        t n;                    \
        assign n to #s;         \
        monitor n;              \
        evflag n##Event;        \
        sync n n##Event;        \
        t n##Last;

#define DECLMON(t,n,s)        	\
        t n;                    \
        assign n to #s;         \
        monitor n; 

#define DECL(t,n,s)        	\
        t n;                    \
        assign n to #s;  

DECLEVENT(int, startPV, {instance}:Start) 
DECLEVENT(int, startRsp, {instance}:StartRsp) 
DECLEVENT(int, stopPV, {instance}:Stop) 
DECLEVENT(int, stopRsp, {instance}:StopRsp) 
/*
PV that says global sequencer is ready
*/
DECLEVENT(int, startedOK, {instance}:StartedOK)
DECLMON(int, acquiring, {instance}:Acquiring)

DECL(int, cmdReq, {instance}:CmdReq)
DECLMON(int, cmdRsp, {instance}:CmdRsp)

DECL(string, failed, {instance}:FailedCmd)

ss devCon1 {
   state initial {
      when(efTestAndClear(startedOKEvent) && startedOK) {
      } state waitForCmd
   }
   state waitForCmd {
      when (efTestAndClear(startPVEvent) && startPV) {
      } state starting
      when (efTestAndClear(stopPVEvent) && stopPV) {
      } state stopping
   }
   state stopping {
      when () {
         cmdReq = STOPACQCMD; pvPut(cmdReq, SYNC);
      } state stopping1
   }
   state stopping1 {
      when (cmdRsp == SUCCESSRSP) { 
         cmdReq = READSTATCMD; pvPut(cmdReq, SYNC);
      } state stopping2          
      when (cmdRsp == ERRORRSP || cmdRsp == TIMEOUTRSP || 
            cmdRsp == NORSP) { 
         PRINTFAILED(stopping)
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
   }
   state stopping2 {
      when (cmdRsp == SUCCESSRSP) { 
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCommand          
      when (cmdRsp == ERRORRSP || cmdRsp == TIMEOUTRSP || 
            cmdRsp == NORSP) { 
         PRINTFAILED(stopping)
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
   }
   state starting {
      when (acquiring) { 
         startRsp = INVALIDRSP; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd
      when () {
         cmdReq = WRITECONFCMD; pvPut(cmdReq, SYNC);
      } state starting1
   }
   state starting1 {
      when (cmdRsp == SUCCESSRSP) { 
         cmdReq = STARTACQCMD; pvPut(cmdReq, SYNC);
      } state starting2          
      when (cmdRsp == ERRORRSP || cmdRsp == TIMEOUTRSP || 
            cmdRsp == NORSP) { 
         PRINTFAILED(starting)
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
   }
   state starting2 {
      when (cmdRsp == SUCCESSRSP) { 
         cmdReq = READSTATCMD; pvPut(cmdReq, SYNC);
      } state starting3          
      when (cmdRsp == ERRORRSP || cmdRsp == TIMEOUTRSP || 
            cmdRsp == NORSP) { 
         PRINTFAILED(starting)
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
   }
   state starting3 {
      when (cmdRsp == SUCCESSRSP) {
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
      when (cmdRsp == ERRORRSP || 
            cmdRsp == TIMEOUTRSP || cmdRsp == NORSP) { 
         PRINTFAILED(starting)
         startRsp = cmdRsp; pvPut(startRsp, SYNC);
         startPV = DONE; pvPut(startPV, SYNC);
      } state waitForCmd  
   }
}


   
   
      
       


