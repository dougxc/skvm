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
 * SUBSYSTEM: Common routines shared by KVM and KVMutil
 * FILE:      KVMcommon.c
 *=======================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <KVMcommon.h>

/*======================================================================
 * Auxiliary routines
 *====================================================================*/

/*======================================================================
 * FUNCTION:    RomVersionCompatible
 * DESCRIPTION: Check that the ROM version meets your
 *              minimum requirement.  Warn if the app was switched to.
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags indicating how the application was
 *                                launched.  A warning is displayed only if
 *                                these flags indicate that the app is 
 *                                launched normally.
 *              customAlert     - ID of the custom alert through which 
 *                                error messages are displayed.
 * RETURNS:     zero if rom is compatible else an error code
 *====================================================================*/

Err RomVersionCompatible(DWord requiredVersion, Word launchFlags,
                            Word customAlert) {
    #define errorMsg "This application requires Palm OS Version >= 3.0"
    DWord palmRomVersion;

    /* See if we're on in minimum required version of the ROM or later.
     * The system records the version number in a feature.  A feature is a
     * piece of information which can be looked up by a creator and feature
     * number.
     */

    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &palmRomVersion);
    if (palmRomVersion < requiredVersion) {
        /* If the user launched the app from the launcher, explain
         * why the app shouldn't run.  If the app was contacted for something
         * else, like it was asked to find a string by the system find, then
         * don't bother the user with a warning dialog.  These flags tell how
         * the app was launched to decided if a warning should be displayed.
         */
        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
            == (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {
            FrmCustomAlert(customAlert, errorMsg, NULL, NULL);
            if (palmRomVersion < version20) {
                AppLaunchWithCommand(sysFileCDefaultApp, 
                    sysAppLaunchCmdNormalLaunch, NULL);
            }
        }
        return sysErrRomIncompatible;
    }
    return 0;
}

/*======================================================================
 * FUNCTION:    ReturnToLauncher
 * DESCRIPTION: Queue an event to go back to the Palm launcher.
 *====================================================================*/

void ReturnToLauncher(void) {
    EventType event;
        
    /* Tell the system to change to the launcher */
    event.eType = keyDownEvent;
    event.data.keyDown.chr = launchChr;
    event.data.keyDown.keyCode = 0;  /* necessary? */
    event.data.keyDown.modifiers = commandKeyMask;
    EvtAddEventToQueue(&event);
}

/*======================================================================
 * FUNCTION:    getKVMprefs
 * RETURNS:     true if the license has been accepted
 *====================================================================*/

Boolean getKVMprefs(void) {
    Word prefSize = sizeof(KVMprefs);
    SWord version;

    version = PrefGetAppPreferences(KVMCreator, KVMPrefID,
                                    &KVMprefs, &prefSize, true);  
    if (version == noPreferenceFound
            || prefSize != sizeof(KVMprefs)
            || version != KVMPrefVersion) { 
        /* initialize the preferences struct */
        KVMprefs.licenseAccepted = 0;
        KVMprefs.saveOutput = 0;
        KVMprefs.displayLines = 0;
        KVMprefs.heapSize = 0;
        KVMprefs.showHeapStats = 0;
    }
    return (Boolean)KVMprefs.licenseAccepted;
}

/*======================================================================
 * FUNCTION:    getMaxHeapSize
 * RETURNS:     size in bytes of the largest chunk in the Palm heap
 *====================================================================*/

DWord getMaxHeapSize(void) {
    DWord freeSpace;
    DWord largestChunk;
    
    /*  Compact the dynamic C heap so that we can allocate */
    /*  as much memory as possible for the VM */
    MemHeapCompact(0);
    MemHeapFreeBytes(0, &freeSpace, &largestChunk);
    return freeSpace - OSRESERVEDSIZE;
}


