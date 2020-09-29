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
 * SUBSYSTEM: Machine-specific functions
 * FILE:      runtime.h
 * OVERVIEW:  This file defines the machine-specific function
 *            prototypes necessary for implementing Java.
 *            Platform-specific implementations of these functions
 *            must be provided in a port-specific "runtime_md.c"
 *            file.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Function prototypes
 *=======================================================================*/

/* Any of these functions might be defined as macros
 * in the "machine_md.h" file for a particular implementation.
 */
#ifndef MAXCALENDARFLDS
#define MAXCALENDARFLDS 15
#endif

#ifndef AlertUser
void AlertUser(const char* message);
#endif

#ifndef allocateHeap
cell *allocateHeap(long *sizeptr, void **realresultptr);
#endif

#ifndef InitializeNativeCode
void InitializeNativeCode();
#endif

#ifndef FinalizeNativeCode
void FinalizeNativeCode(void);
#endif

#ifndef InitializeVM
void InitializeVM();
#endif

#ifndef FinalizeVM
void FinalizeVM(void);
#endif

#ifndef Calendar_md
unsigned long *Calendar_md(void);
#endif

#ifndef CurrentTime_md
ulong64 CurrentTime_md(void);
#endif

#ifndef RandomNumber_md
long RandomNumber_md();
#endif

#if ASYNCHRONOUS_NATIVE_FUNCTIONS

#ifndef Yield_md
void Yield_md(void);
#endif

#ifndef CallAsyncNativeFunction_md
struct asynciocb;
void CallAsyncNativeFunction_md(struct asynciocb *, void (*)(struct asynciocb *));
#endif

#ifndef InitalizeAsynchronousIO
void InitalizeAsynchronousIO(void);
#endif

#endif /* ASYNCHRONOUS_NATIVE_FUNCTIONS */

