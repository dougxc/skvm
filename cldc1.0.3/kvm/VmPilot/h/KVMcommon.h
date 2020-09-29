/*
 * Copyright (c) 1998-2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: VM internals
 * FILE:      KVMcommon.h
 * OVERVIEW:  Declarations shared between KVM and KVMutil
 *=======================================================================*/

/*======================================================================
 * Application resource declarations
 *====================================================================*/

#include <machine_md.h>

/*======================================================================
 * Definitions and declarations
 *====================================================================*/

#define version20       0x02000000  /* PalmOS 2.0 version number */
#define version30       0x03000000  /* PalmOS 3.0 version number */

#define KVMCreator      'kJav'      /* Creator for KVM */

#define KVMclassDBType  'Data'      /* Type for database */
#define KVMDBName       "Classes"   /* Name of database */

#define KVMstdoutDBType 'Stdo'      /* Type for database */


#define KVMXferType     'Xfer'      /* Type for database updates */

#define KVMutilCreator  'kJax'      /* Creator for KVMutil */

#define KVMPrefID       0           /* ID # of the preferences */

#define MAXDISPLAYLINES 13

#define MAXCHUNKSIZE 65400
#define ROLLOVERTAG "\216\216\n"

#define KVMDefaultDebuggerPort 2800    /* default port for debugger */

#define KVMPrefVersion 105
struct  KVMprefsStruct {
    // It is incredibly important that this data structure uses only
    // Palm-specific types, since things like "int", "boolean", "long" 
    // can differ from compiler to compiler!!
    Long heapSize;
    Long displayLines;
    Byte licenseAccepted;
    Byte saveOutput;
    Byte showHeapStats;
    Byte enableDebugger;
    Short debuggerPort;
};


/*======================================================================
 * External variables 
 *====================================================================*/

extern struct KVMprefsStruct KVMprefs;
extern DmOpenRef KVMstdoutDB;

/*======================================================================
 * Function prototypes
 *====================================================================*/

Err RomVersionCompatible(DWord requiredVersion, Word launchFlags,
                            Word customAlert);
void ReturnToLauncher();
Boolean getKVMprefs(void);
DWord getMaxHeapSize(void);

/* This is a workaround for a bug in the current version of gcc:
   gcc assumes that no one will touch %a4 after it is set up in crt0.o.
   This isn't true if a function is called as a callback by something
   that wasn't compiled by gcc (like FrmCloseAllForms()).  
*/

#if __GNUC__
register void *reg_a4 asm("%a4");
#  define CALLBACK_PROLOGUE \
      void *save_a4 = reg_a4; asm("move.l %%a5,%%a4; sub.l #edata,%%a4" : :);
#  define CALLBACK_EPILOGUE reg_a4 = save_a4;
#else
#  define CALLBACK_PROLOGUE 
#  define CALLBACK_EPILOGUE
#endif /* __GNUC__ */
