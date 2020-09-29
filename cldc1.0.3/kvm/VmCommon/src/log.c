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
 * SUBSYSTEM: Profiling
 * FILE:      log.c
 * OVERVIEW:  Operations for gathering and printing out diagnostic
 *            virtual machine execution information at runtime.
 * AUTHOR:    Daniel Blaukopf, based on code by Antero Taivalsaari
 *            and Doug Simon
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Functions
 *=======================================================================*/

/*=========================================================================
 * NOTE:
 * Documentation for each of the functions below is provided in log.h.
 *=======================================================================*/

static void 
log_uncaughtException(THROWABLE_INSTANCE exception) 
{
    fprintf(stderr, "Uncaught exception %s",
        getClassName((CLASS)exception->ofClass));
    if (exception->message != NULL) {
        fprintf(stderr,": %s",getStringContents(exception->message));
    }
    fprintf(stderr, ".\n");
}


/* The following functions are included only when really needed */
#if INCLUDEDEBUGCODE

static void log_enterMethod(METHOD method, const char *type) 
{ 
    ASSERTING_NO_ALLOCATION
        char className[256];
        char *methodName = methodName(method);
        getClassName_inBuffer((CLASS)method->ofClass, className);
        if (TERSE_MESSAGES) {
            fprintf(stdout, "=> %s.%s\n", className, methodName);
        } else {
            char methodSignature[256];
            unsigned short key = method->nameTypeKey.nt.typeKey;
            change_Key_to_MethodSignature_inBuffer(key, methodSignature);
            fprintf(stdout, "Invoking %s method '%s::%s%s'\n", 
                    type, className, methodName, methodSignature);
        }
    END_ASSERTING_NO_ALLOCATION
}

/*=========================================================================
 * FUNCTION:      TraceMethodExit()
 * OVERVIEW:      Trace a method exit
 * INTERFACE:
 *   parameters:  METHOD method
 *   returns:     <nothing>
 *=======================================================================*/

static void log_exitMethod(METHOD method) 
{
    ASSERTING_NO_ALLOCATION
        char className[256];
        char *methodName = methodName(method);
        getClassName_inBuffer((CLASS)method->ofClass, className);
        if (TERSE_MESSAGES) {
            fprintf(stdout, "<= %s.%s\n", className, methodName);
        } else { 
            fprintf(stdout, "Leaving %s::%s\n", className, methodName);
        }
    END_ASSERTING_NO_ALLOCATION
}

static void log_allocateObject(long pointer,
                               long cells,
                               int type,
                               long ID,
                               long memoryFree) {
    fprintf(stdout, "Allocated %ld bytes, address: %lx, type: %lx, free: %ld\n",
            cells*CELL, pointer, (long)type, memoryFree);
}

static void log_allocateHeap(long heapSize,
                             long heapBottom,
                             long heapTop) {
    fprintf(stdout, "Allocated a dynamic heap of %ld bytes\n", heapSize);
    fprintf(stdout, "Heap bottom: %lx, heap top: %lx\n",
            heapBottom, heapTop);
}

static void log_heapObject(long object,
                           const char *type,
                           long size,
                           int isMarked,
                           const char *className,
                           int isClass) {
    fprintf(stdout, "%lx: %s, size=%ld%s",
            object, type, size,
            isMarked ? ", MARKED" : "");
    if (className==NULL) {
        fprintf(stdout, "\n");
    } else if (isClass) {
        fprintf(stdout, ", class %s\n", className);
    } else {
        fprintf(stdout, ", instance of %s\n", className);
    }
}

static void log_heapSummary(int objectCount, int freeCount,
                            int liveByteCount, int freeByteCount,
                            int largestFreeObjectSize,
                            long heapBottom, long heapTop) {
    fprintf(stdout, "%d objects in heap (%d alive/%d free)\n",
            objectCount+freeCount, objectCount, freeCount);
    fprintf(stdout, "%d live bytes; %d bytes free (%d total)\n",
            liveByteCount, freeByteCount, liveByteCount+freeByteCount);
    fprintf(stdout, "Largest free object: %d bytes\n",
            largestFreeObjectSize);
    fprintf(stdout, "Heap bottom: %lx, heap top: %lx\n",
            heapBottom, heapTop);
}

static void log_horizontalRule(void) {
    fprintf(stdout, "\n====================\n");
}

#define log_startHeapScan log_horizontalRule
#define log_endHeapScan log_horizontalRule

static void log_heapWarning (long freeListCount,
                             long byteCount) {
    fprintf(stdout, "WARNING: %ld objects in the free list (%ld bytes)\n",
            freeListCount, byteCount);
}


static void log_loadClass(const char *className) {
    fprintf(stdout, "Loading class '%s'\n", className);
}
    
static void 
log_throwException(THROWABLE_INSTANCE exception) {
    fprintf(stdout, "Exception: %s\n", \
            getClassName((CLASS)exception->ofClass));
}

#endif /* INCLUDEDEBUGCODE */


static const struct KVMLogInterface_ logImplementation = {
    fprintf
    ,log_uncaughtException
#if INCLUDEDEBUGCODE
    ,log_enterMethod
    ,log_exitMethod
    ,log_allocateObject
    ,log_allocateHeap
    ,log_heapObject
    ,log_heapSummary
    ,log_startHeapScan
    ,log_endHeapScan
    ,log_heapWarning
    ,log_loadClass
    ,log_throwException
#endif /* INCLUDEDEBUGCODE */
};

const KVMLogInterface Log = (KVMLogInterface) &logImplementation;

