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
 * SUBSYSTEM: Global definitions
 * FILE:      global.c
 * OVERVIEW:  Global system-wide definitions.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Many others since then...
 *=======================================================================*/

/*=========================================================================
 * COMMENT: 
 * This file contains declarations that do not belong to any 
 * particular structure or subsystem of the VM. There are additional 
 * global variable declarations in other files.
 *=======================================================================*/

/*=========================================================================
 * Local include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Miscellaneous global variables
 *=======================================================================*/

/* Shared string buffer that is used internally by the VM */
/* NOTE: STRINGBUFFERSIZE is defined in main.h */
char str_buffer[STRINGBUFFERSIZE];

/* Requested heap size when starting the VM from the command line */
long RequestedHeapSize;    

/*=========================================================================
 * Global execution modes
 *=======================================================================*/

/*  Flags for toggling certain global modes on and off */
bool_t JamEnabled;
bool_t JamRepeat;

/*========================================================================
 * Runtime flags for choosing different tracing/debugging options.
 * All these options make the system very verbose. Turn them all off
 * for normal VM operation.
 *=======================================================================*/
                      
#define DEFINE_TRACE_VAR_AND_ZERO(varName, userName) int varName = 0;
FOR_EACH_TRACE_FLAG(DEFINE_TRACE_VAR_AND_ZERO)

/*=========================================================================
 * Error handling definitions
 *=======================================================================*/

static struct throwableScopeStruct ThrowableScopeStruct = {
   /* env =           */ NULL,
   /* throwable =     */ NULL,
   /* tmpRootsCount = */ 0,
   /* outer =         */ NULL
};
THROWABLE_SCOPE ThrowableScope = &ThrowableScopeStruct;
void* VMScope = NULL;
int   VMExitCode = 0;

