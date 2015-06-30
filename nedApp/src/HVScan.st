program HVScan("winterval=0.1")




/* HVScan.st
 *
 * Copyright (c) 2015 Oak Ridge National Laboratory.
 * All rights reserved.
 * See file LICENSE that is included with this distribution.
 *
 * @author Carl Lionberger
 */


/*
 * HVScan reads the ROC HV voltage readbacks a fixed period apart to 
 * prevent locking other clients out by the scan.
 * 
 * The list of devices to operate on is obtained with seqGrep(), which looks
 * at the EPICS database in the IOC memory. seqGrep() is part of the 
 * pvMediator app.
 */

%%#include <stdlib.h>
%%#include <string.h>
%%#include <seqGrep.h>

option -c;	/* do not wait for PVs to connect */

/* 
 * HVSCAN_MAXDEVICES is for static mem allocation and must be higher than 
 * number of devices expected. 
 */
#define HVSCAN_MAXDEVICES 120	

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
#define PVARRAY(t,n) 			\
	t n[HVSCAN_MAXDEVICES];	\
	assign n[0] to "";

/*
 * These arrays correspond to groups of nED interface PVs, one per device
 */
PVARRAY(int, readVolt)

/*
 * List of devices found in IOC using seqGrep. numFound indicates number found.
 */
string deviceNames[HVSCAN_MAXDEVICES];

/*
 * common variables
 */
int i;
int numFound;
double delayval;

/*
 * Main state set.
 */
ss HVScan {
   state init {
      entry {
         /* find set of devices we are interested in */
         numFound = seqGrep("ai", ".*:Volt", &deviceNames[0][0], 
						HVSCAN_MAXDEVICES);
         printf("Found %d Voltages\n", numFound);
         for (i=0; i<numFound; i++) {
            strcat(deviceNames[i], ".PROC");
            pvAssign(readVolt[i], deviceNames[i]);
	    readVolt[i] = 1; /* just for show */
         }
         delayval = strtod(macValueGet("winterval"), 0);
         if (delayval > -0.001 && delayval < 0.001) {
            printf("Defaulting delay to 0.1 seconds\n");
            delayval = 0.1;
         }
       }
       when(numFound && pvConnectCount() == pvAssignCount()) {
           i = 0;
       } state scan
       when (delay(5)) {
          printf("pvConnectCount = %ld pvAssignCount = %ld numFound = %d\n",
                  pvConnectCount(), pvAssignCount(), numFound);
      } state init
   }
   state scan {
      when (i < numFound) {
         pvPut(readVolt[i], SYNC);
      } state wait
      when () {
         i = 0;
      } state scan
   }
   state wait {
      when (delay(delayval)) {
          i = i + 1;
      } state scan
   }
}       
       
