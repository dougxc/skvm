/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: User interface
 * FILE:      KVM.h
 * OVERVIEW:  User interface for the KVM
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 * CREATED:   Feb 23, 1998 Initial version based on PalmPilot sample apps.
 *            Edited by Doug Simon 11/1998
 *=======================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <KVMcommon.h>
#include <KVM_rsrc.h> /* Application resource declarations */
#include <ExgMgr.h>
#include <NetMgr.h>

/*======================================================================
 * Definitions and declarations
 *====================================================================*/

#ifndef ERROR_CHECK_LEVEL
#define ERROR_CHECK_LEVEL 2
#endif

#define KVMExchangeType "class" /* We automatically get these */

/*  If this variable is set to 1, we try to take advantage of certain
 *  features available in the POSE simulator.  
 */
#define EMULATOR 1

/* A "user" event indicated that the application has received some beamed 
 * data.  This gets added to the event queue.
 */
#define beamEvent  firstUserEvent

/*=========================================================================
 * Command line parameter storage
 *=======================================================================*/

#define CMD_LINE_LEN  256 /*  the maximum length for command line parameters */

/*=========================================================================
 * Stuff needed by native
 *=======================================================================*/

extern WinHandle offScreenWindow;
extern Boolean useNetLib;

/* Needed as parameters for NetLib* calls. */
/* Berkeley style socket calls are actually */
/* nothing but macros for NetLib* calls, so */
/* we need to define AppNetRefNum and errno. */
extern Word AppNetRefnum;
extern long AppNetTimeout;
extern Err errno;

/*======================================================================
 * Global variables
 *====================================================================*/

extern DmOpenRef ClassfileDB;      /* Handle for classfile database */
extern DmOpenRef ClassfileAltDB;   /* Handle for classfile database */

/* This is actually a BYTEARRAY, but that type doesn't exist yet */
extern void*     PendingBeam;      /* Most recent beam to send to application */

/*======================================================================
 * Random functions that I can't figure out where else to put
 *====================================================================*/

void AlertUser(const char* message);
void ReceiveBeamData(ExgSocketPtr exgSocketP);

