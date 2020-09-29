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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * FILE:      machine_md.h (for Palm)
 * OVERVIEW:  This file is included in every compilation.  It contains
 *            definitions that are specific to the Palm version of KVM.
 * AUTHOR:    Frank Yellin, Antero Taivalsaari
 * NOTE:      This file overrides many of the default compilation
 *            flags and macros defined in VmCommon/h/main.h.
 *=======================================================================*/

#ifndef __MACHINE_MD_H__
#define __MACHINE_MD_H__

/*=========================================================================
 * Platform definition
 *=======================================================================*/

#if !COMPILING_ROMJAVA
#pragma stack_cleanup on
#endif

/*=========================================================================
 * Include files
 *=======================================================================*/

#if !COMPILING_ROMJAVA
# include <CWCompatibility.h>         /* All the system toolbox headers */
# include <KeyMgr.h>
# include <SysEvtMgr.h>
# include <SysEvtMgr.h>     /* Needed for search for EvtSysEventAvail */
# include <FeatureMgr.h>    /* Needed to get the ROM version */
# include <DateTime.h>      /* Needed for DateTimeType Time Manager */
# include <Pilot_IO.h>
#else
# include <stdio.h>
#endif

/*=========================================================================
 * Platform-specific datatype definitions
 *=======================================================================*/

/* MetroWerks compiler requires "forward static" declarations to
 * be defined as "external"
 */
#if __MWERKS__
#define FORWARD_STATIC_DECLARATION extern
#endif

#if !COMPILING_ROMJAVA
typedef long long           long64;
typedef unsigned long long  ulong64;
#else 
/* These are never used in the ROM image, so it doesn't matter what we
 * define them to be.  But they are used as part of other data structures,
 * so we just make them into something harmless.
 */
typedef void*  long64; 
typedef void*  ulong64;         
#endif

/*=========================================================================
 * Compilation flags and macros that override values defined in main.h
 *=======================================================================*/

/* Palm is a big-endian target platform */
#define BIG_ENDIAN 1

/* Support for long integers is available */
#define COMPILER_SUPPORTS_LONG 1

/* Utilize the feature informally known as "Simonizing":
 * in order to conserve precious Palm dynamic RAM, copy
 * all the appropriate immutable data structures to 
 * static "storage" RAM.
 */
#define USESTATIC         1

/* Use the segmented heap model which allows us to
 * allocate more heap space on the Palm.
 */
#define CHUNKY_HEAP       1

/* This interpreter option seems to slow down
 * the interpreter on the Palm architecture.
 * Turn it off.
 */
#define SPLITINFREQUENTBYTECODES 0

/* Utilize ROMizing if the makefile requests so.
 * A separate romizing.h file will be include if we are actually romizing.
 * We set RELOCATABLE_ROM to be whatever value ROMIZING has.
 */
#ifndef ROMIZING
#define ROMIZING 0
#endif
#define RELOCATABLE_ROM ROMIZING

/*=========================================================================
 * Palm-specific memory allocation definitions
 *=======================================================================*/

/*  This constant defines the amount of dynamic RAM that should left for
 *  the OS as well as for any other structures used by the VM that are not
 *  allocated from the GC heap.
 *
 *  The OS requires space for its background processes
 *  such as the "Find" dialog, IrDA Beam receive (on the Palm III), TCP stack
 *  and other things like hacks that may have been added to the OS. The only
 *  structure the VM currently allocates dynamically from the C stack is the
 *  GC heap itself. The Palm OS documentation from 3Com recommends leaving
 *  about 15K for the OS. However, this is a very conservative figure and
 *  requests for more accurate information have not yielded much so far.
 *  For the time being, we leave 8K for the OS.
 */
#define OSRESERVEDSIZE       8192   /*  8 kb */
#define NETLIBRESERVEDSIZE  32768   /* 32 kb */

/*=========================================================================
 * Platform-specific macros and function prototypes
 *=======================================================================*/

#define InitializeVM()

void freeHeap(void *theHeap);
#define RandomNumber_md() SysRandom(0)

#define DOUBLE_REMAINDER(a,b) (fatalError("no float remainder"), a+b)

/* EOF needs to be defined for Palm */
#if !defined(EOF)
#  define EOF -1
#endif

#if !COMPILING_ROMJAVA
    /* Unix-isms that are defined slightly differently on the pilot */
    #define strcmp(s1,s2) StrCompare(s1,s2)
    #define strncmp(s1,s2,n) StrNCompare(s1,s2,n)
    #define strlen(s) StrLen(s)
    #define strcat(s1,s2) StrCat(s1,s2)
    #define strchr(s,c) StrChr(s,c)
    #define strncpy(s1,s2,n) StrNCopy(s1,s2,n)
    #define strcpy(s1,s2) StrCopy(s1,s2)
    #define sprintf StrPrintF
    #define atoi(s) StrAToI(s)
    /* Why are the arguments to this one different than everywhere else? */
    #define memset(s,c,n) MemSet(s,n,c)
    #define memmove(s1,s2,n) MemMove(s1,s2,n)
    #define memcpy(s1,s2,n) MemMove(s1,s2,n)
    #define memcmp(s1,s2,n) MemCmp(s1,s2,n)
    #define setjmp(buf) ErrSetJump(buf)
    #define longjmp(buf,res) ErrLongJump(buf,res)
    #define jmp_buf ErrJumpBuf
#endif

#endif /*  __MACHINE_MD_H__ */

