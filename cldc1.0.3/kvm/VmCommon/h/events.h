/*
 * Copyright (c) 1998-2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Event handling support
 * FILE:      events.h
 * OVERVIEW:  This file defines the macros and operations for
 *            binding the interpreter to the event handling
 *            mechanisms of the host operating system in a 
 *            portable fashion.
 * AUTHOR:    Nik Shaylor 4/20/00
 *            Original Palm OS specific implementation by Doug Simon 1998
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Event system macros
 *=======================================================================*/

/*=========================================================================
 * Thread scheduling macros.
 * These are here to isolate the scheduling code out of interpret.c.
 * Note that the thread switching macros are normally used also for
 * driving the event handling/polling mechanism,
 *=======================================================================*/

/*
 * Indicate task switching is necessary
 * (enforce a thread switch)
 */
#define signalTimeToReschedule() (Timeslice = 0)

/*
 * Determine if it is time to reschedule
 * (by decrementing the time slice and comparing
 * it to zero)
 */
#define isTimeToReschedule() (Timeslice-- == 0)

/*
 * Check that the time slice value is not junk
 * (should never happen)
 */
#if INCLUDEDEBUGCODE
#define checkRescheduleValid() if (Timeslice < 0) \
    { fatalError(KVM_MSG_INVALID_TIMESLICE); }
#else
#define checkRescheduleValid() /**/
#endif /* INCLUDEDEBUGCODE */

/*
 * InterpreterHandleEvent
 *
 * This function ties the virtual machine with events coming
 * from the host operating system.  Since CLDC itself does 
 * not define any event handling capabilities, the function
 * pretty much empty in those builds that don't include any
 * additional classes beyond CLDC.
 *
 */
void InterpreterHandleEvent(ulong64);

/*
 * __ProcessDebugCmds()
 *
 * This function ties the virtual machine with the new
 * Java-level debugger interface.
 */
#if ENABLE_JAVA_DEBUGGER
#define __ProcessDebugCmds(x) ProcessDebugCmds(x);
#else
#define __ProcessDebugCmds(x)
#endif /* ENABLE_JAVA_DEBUGGER */

/*
 * Reschedule
 *
 * This routine is called from inside the interpreter when
 * it is time to perform thread switching.
 *
 */
#define reschedule()                                                \
     do  {                                                          \
        ulong64 wakeupTime;                                         \
        if (!areAliveThreads()) {                                   \
            return;   /* end of program */                          \
        }                                                           \
        checkTimerQueue(&wakeupTime);                               \
        InterpreterHandleEvent(wakeupTime);                         \
        __ProcessDebugCmds(0);                                      \
    } while (!SwitchThread());


/*=========================================================================
 * Data structures needed for event handling
 *=======================================================================*/

#define appStopKVMEvent -1

/*=========================================================================
 * Event handling functions
 *=======================================================================*/

extern int eventCount;                 /* Number of events on event queue */

void InitializeEvents(void);

void  StoreKVMEvent(cell type, int argCount,  /* cell cell cell */ ... );

/*
 * GetAndStoreNextKVMEvent
 *
 * Platform-specific function for reading events from the host
 * operating system.  This function typically needs to be ported 
 * to use the low-level event handling mechanisms of the host
 * platform.
 */

void
GetAndStoreNextKVMEvent(bool_t forever, ulong64 waitUntil);


