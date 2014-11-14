program devCon ("instance=test:Det:roc32")

#define DECLEVENT(t,n,s)        \
        t n;                    \
        assign n to #s;         \
        monitor n;              \
        evflag n##Event;        \
        sync n n##Event;        \
        t lastval;


DECLMON(int, startPV, {instance}:Start) 
DECLMON(int, stopPV, {instance}:Stop) 

   state initial {
