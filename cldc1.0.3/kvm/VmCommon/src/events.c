/*
 *  Copyright (c) 1998-2001 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Event handling support
 * FILE:      events.c
 * OVERVIEW:  This file defines the operations for binding the
 *            interpreter to the event handling mechanisms of
 *            the host operating system.
 * AUTHOR:    Nik Shaylor 4/20/00
 *            Bill Pittore (debugging support)
 *            Frank Yellin
 *=======================================================================*/

/*
 * This file implements the "events:" protocol
 *
 * @author  Nik Shaylor
 * @version 1.0 1/15/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#if ENABLE_JAVA_DEBUGGER
#include <debugger.h>
#endif

#ifndef PILOT
#include <stdarg.h>
#endif

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

#define MAXPARMLENGTH 20

/*=========================================================================
 * Local variables
 *=======================================================================*/

static THREAD waitingThread;
static int    opened;

static cell   eventBuffer[MAXPARMLENGTH];
static int    eventInP;
       int    eventCount;

/*=========================================================================
 * Event handling functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      InitializeEvents
 * TYPE:          global function
 * OVERVIEW:      Initialize the event system.
 *                We have to explicitly initialize the event system
 *                so that the KVM can shutdown and restart cleanly.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void InitializeEvents()
{
    waitingThread = 0;
    makeGlobalRoot((cell **)&waitingThread);
    opened     = FALSE;

    eventInP = 0;
    eventCount = 0;

#if INCLUDEDEBUGCODE
    if (traceevents) {
        fprintf(stdout, "Event system initialized\n");
    }
#endif /* INCLUDEDEBUGCODE */
}

/*=========================================================================
 * FUNCTION:      StoreKVMEvent
 * TYPE:          global function
 * OVERVIEW:      Callback to indicate an event has occurred

 * INTERFACE:
 *   parameters:  type:      event type
 *                argCount:  number of parameters taken by event
 *                .....:     argCount parameters
 *   returns:     none
 *=======================================================================*/

void 
StoreKVMEvent(cell type, int argCount, ...) 
{
    int inP;
    int i;
    va_list  args;  
    if (eventCount == 0) { 
        /* Reset the in pointer to 0 if we can */
        eventInP = 0;
    } else if (eventInP > (MAXPARMLENGTH - 1 - argCount)) { 
        /* Ignore the event, for now, since we don't have any space to
         * store it.
         */
        return;
    }
    
#if INCLUDEDEBUGCODE
    if (traceevents) {
        fprintf(stdout, "Event %ld received\n", (long)type);
    }
#endif

    if (type == appStopKVMEvent) { 
        /* Indicate the end */
        VM_EXIT(0);
    }

    inP = eventInP;             /* cache this locally */
    eventBuffer[inP++] = type;

    va_start(args, argCount);
        for (i = 0; i < argCount; i++) {
            eventBuffer[inP++] = va_arg(args, cell);
        }
    va_end(args);

    /* Update the count and the pointer to where to put the next argument */
    eventCount += (inP - eventInP);
    eventInP = inP;
}

/*=========================================================================
 * FUNCTION:      getKVMEvent
 * TYPE:          Local function
 * OVERVIEW:      Find next event or parameter on the input queue.
 * INTERFACE:
 *   parameters:  forever:   if TRUE, wait forever, if FALSE wait
 *                           until waitUntil
 *                waitUntil: wait at most until this time (if !forever).
 *                result:    location to put the next event 
 *   returns:     boolean    indicating whether an event or parameter
 *                           is available
 *=======================================================================*/

static bool_t
getKVMEvent(bool_t forever, ulong64 waitUntil, cell *result) { 
    if (eventCount == 0) { 
        /* Try and get the machine's system to give us an event */
        GetAndStoreNextKVMEvent(forever, waitUntil);
    }
    if (eventCount > 0) { 
        /* We have an event */
        *result = eventBuffer[eventInP - eventCount];
        eventCount--;
        return TRUE;
    } else { 
        /* No event available */
        return FALSE;
    }
}

 /*=========================================================================
  * FUNCTION:      InterpreterHandleEvent
  * TYPE:          Global C function
  * OVERVIEW:      Wait for a system event
  * INTERFACE:
  *   parameters:  time to wait if no events are available immediately;
  *                zero value indicates that the host platform can go
  *                to sleep and/or conserve battery until a new native
  *                event occurs
  *   returns:     nothing
  *=======================================================================*/

void InterpreterHandleEvent(ulong64 wakeupTime) {
    bool_t forever = FALSE;     /* The most common value */
    
    if (areActiveThreads()) { 
        /* Indicate that we don't wait for an event */
        ll_setZero(wakeupTime);
    } else if (ll_zero_ne(wakeupTime)) { 
        /* wakeupTime already has the right value.  But change it to
         * be at most 20ms from now if the debugger is available. 
         */
#if (ENABLE_JAVA_DEBUGGER || ASYNCHRONOUS_NATIVE_FUNCTIONS)
        ulong64 max = CurrentTime_md();
        ll_inc(max, 20);
        if (ll_compare_ge(wakeupTime, max)) { 
            wakeupTime = max;
        }
#endif
    } else { 
        /* We wait forever, unless the debugger is available, in which
         * case we wait for at most 20ms.
         */
#if (ENABLE_JAVA_DEBUGGER || ASYNCHRONOUS_NATIVE_FUNCTIONS)
        wakeupTime = CurrentTime_md();
        ll_inc(wakeupTime, 20);
#else
        forever = TRUE;
        ll_int_to_long(wakeupTime, -1);
#endif
    } 

    if (!opened || waitingThread == NULL) { 
        /* Nothing to do but sleep */
        if (ll_zero_eq(wakeupTime)) { 
            return;
        } else {
#if ENABLE_JAVA_DEBUGGER
            long64 delta64 = *(long64*)&wakeupTime;
            ll_dec(delta64, CurrentTime_md());
            if (ll_zero_gt(delta64)) { 
                unsigned long delta;
                ll_long_to_uint(delta64, delta);

                ProcessDebugCmds(delta);
            } 
#else
            SLEEP_UNTIL(wakeupTime);
#endif
        }
    } else { 
        cell result;
        if (getKVMEvent(forever, wakeupTime, &result)) { 
            (void)popStackForThread(waitingThread);
            pushStackForThread(waitingThread, result);
            resumeThread(waitingThread);
            waitingThread = NULL;
        }
    }
}

/*=========================================================================
 * Protocol implementation methods ("events:" protocol)
 *=======================================================================*/

 /*=========================================================================
  * FUNCTION:      open()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_open(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (opened) {
        raiseException("java/lang/IllegalAccessException");
    }
    opened = TRUE;
}

 /*=========================================================================
  * FUNCTION:      close()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_close(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (!opened) {
        raiseException("java/lang/IllegalAccessException");
    }
    opened = FALSE;
}

 /*=========================================================================
  * FUNCTION:      int    readInt() 
  * FUNCTION:      byte[] readByteArray() 
  * FUNCTION       String readUTF()
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

static void readOneEvent() { 
    ulong64 wakeupTime;
    
    /* The "instance" on the top of the stack is trash */
    ll_setZero(wakeupTime);

    if (!getKVMEvent(FALSE, wakeupTime, &topStack)) { 
        waitingThread = CurrentThread;
        topStack = 0;
        suspendThread();
    } 
 }

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readInt(void) {
    readOneEvent();
}

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readByteArray(void) {
    readOneEvent();
}

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readUTF(void) {
    readOneEvent();
}


