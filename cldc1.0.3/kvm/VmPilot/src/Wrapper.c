/*
 * Copyright © 1999-2000 Sun Microsystems, Inc., 901 San Antonio Road,
 * Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 * 
 * Sun Microsystems, Inc. has intellectual property rights relating
 * to the technology embodied in this software.  In particular, and
 * without limitation, these intellectual property rights may include
 * one or more U.S. patents, foreign patents, or pending
 * applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 * and all Sun-based and Java-based marks are trademarks or
 * registered trademarks of Sun Microsystems, Inc.  in the United
 * States and other countries.
 * 
 * This software is distributed under licenses restricting its use,
 * copying, distribution, and decompilation.  No part of this
 * software may be reproduced in any form by any means without prior
 * written authorization of Sun and its licensors, if any.
 * 
 * FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 * Subject to Standard License Terms and Conditions
 * 
 */

#include <KVMcommon.h>
#include <Wrapper_rsrc.h>

/** DANGER:
 *
 *  Make sure that the CustomAlert resource ID used by this wrapper code does
 *  not conflict with IDs used by the core VM. 
 *
 *  The core VM will open up this wrapper code as a resource, and will become
 *  confused if there are two different Alerts with the same ID (and will
 *  probably bring up the wrong one. 
 * 
 *  REGNAD
 */
 
static void LaunchVM(void) { 
    DmSearchStateType searchState; 
    GoToParamsPtr gotoPtr;
    LocalID kvmID;
    UInt    kvmCardNo;
    Word    err;

    err = DmGetNextDatabaseByTypeCreator(true, &searchState, 
                sysFileTApplication, KVMCreator, true, &kvmCardNo, &kvmID);
    if (err) { 
        FrmCustomAlert(CustomAlert, "Cannot find KVM on this device", NULL, NULL);
        return;
    }

    /* The following code is stolen from the Palm FAQ. */
    gotoPtr = MemPtrNew(sizeof(GoToParamsType));
    MemPtrSetOwner(gotoPtr, 0);
    MemSet(gotoPtr, sizeof(GoToParamsType), 0); /* can't hurt */
    
    /* Find the card number and LocalID of this wrapper */
    DmOpenDatabaseInfo(DmNextOpenResDatabase(0), 
                       &gotoPtr->dbID, 0, 0, 
                       &gotoPtr->dbCardNo, 0);                     

    err = SysUIAppSwitch(kvmCardNo, kvmID, 
                         sysAppLaunchCmdGoTo, (Ptr)gotoPtr);
    if (err) { 
        FrmCustomAlert(CustomAlert, "Cannot launch KVM", NULL, NULL);
    }
}


DWord 
PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    if (cmd == sysAppLaunchCmdNormalLaunch) {
        LaunchVM();
    }

    return 0;
}




