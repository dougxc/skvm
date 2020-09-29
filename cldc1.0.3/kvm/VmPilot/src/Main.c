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
 * SUBSYSTEM: User interface
 * FILE:      Main.c
 * OVERVIEW:  User interface for the KVM
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 * CREATED:   Feb 23, 1998 Initial version based on PalmPilot sample apps.
 *            Edited by Doug Simon 11/1998
 *            Edited by Tasneem Sayeed 02/2000
 *=======================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <KVM.h>
#include <global.h>
#include <ErrorMgr.h>
#include <inetlib.h>
#include <KVMutil_rsrc.h>

#if ENABLE_JAVA_DEBUGGER
#include <debugger.h>
#endif

/*======================================================================
 * Global variables
 *====================================================================*/

Int16 strcmpnames(void* rec1, void* rec2, Int16 other, 
    SortRecordInfoPtr rec1SortInfo, SortRecordInfoPtr rec2SortInfo,
    VoidHand appInfoH);
    
/*======================================================================
 * Global variables
 *====================================================================*/

DmOpenRef  ClassfileDB;      /* Handle for resource database */
DmOpenRef  ClassfileAltDB;   /* Handle for classfile database */

struct KVMprefsStruct KVMprefs;
DmOpenRef KVMstdoutDB;

Boolean useNetLib;
Err errno;

/*======================================================================
 * Auxiliary routines
 *====================================================================*/

/*======================================================================
 * FUNCTION:    doAppStop
 *====================================================================*/

static void
doAppStop(void)
{
    EventType event;

    FrmCloseAllForms();
    ReturnToLauncher();

    /* wait for app stop event */
    do {
        EvtGetEvent(&event, evtWaitForever);
        /* This is boilerplate */
        SysHandleEvent(&event);
    } while (event.eType != appStopEvent);
}

/*======================================================================
 * FUNCTION:    launchKVMutil
 *====================================================================*/

static void launchKVMutil(Word cmd, Ptr cmdPBP) {
    DmSearchStateType searchState;
    LocalID utilID;
    UInt    utilCardNo;
    Word    err;

    err = DmGetNextDatabaseByTypeCreator(true, &searchState,
            sysFileTApplication, KVMutilCreator, true, &utilCardNo, &utilID);
    if (err) {
        FrmCustomAlert(CustomAlert, "Cannot find KVMutil on this device", NULL, NULL);
    } else {
        err = SysUIAppSwitch(utilCardNo, utilID, cmd, cmdPBP);
        if (!err) return;
        FrmCustomAlert(CustomAlert, "Cannot launch KVMutil", NULL, NULL);
    }
    doAppStop();
}


/*======================================================================
 * FUNCTION:    WriteCenteredText
 *====================================================================*/

static void
WriteCenteredText(CharPtr text, int y)
{
    int len = StrLen(text);
    int x = (160 - FntCharsWidth(text, len)) >> 1;
    WinDrawChars(text, len, x, y);
}

/*======================================================================
 * FUNCTION:    FinalizeVM
 * DESCRIPTION: Called after the virtual machine exits.
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *====================================================================*/

void FinalizeVM(void)
{
    /* Execution completed; clear the display, print */
    /* statistics and release the heap */
    WinResetClip();

    /* Erase the window */
    WinEraseWindow();

    if (offScreenWindow != NIL) {
        WinDeleteWindow(offScreenWindow, FALSE);
        offScreenWindow = NIL;

    }
}


/* compare function for DMQuickSort */
Int16 strcmpnames(void* rec1, void* rec2, Int16 other, 
    SortRecordInfoPtr rec1SortInfo, SortRecordInfoPtr rec2SortInfo,
    VoidHand appInfoH) 
{
    return StrCompare((CharPtr) rec1, (CharPtr) rec2);                 
}

/*======================================================================
 * Application startup & shutdown functions
 *====================================================================*/

/*======================================================================
 * FUNCTION:     StartApplication
 * DESCRIPTION:  This routine sets up the initial state of the application.
 *               It opens the application's database and sets up global
 *               variables.
 * PARAMETERS:   None.
 * RETURNS:      true if error (database couldn't be created)
 *====================================================================*/

static
RunKVM(Word cmd, UInt cardno, LocalID dbID)
{
    FormPtr form;
    VoidHand recHandle;
    CharPtr  recPtr;
    UInt     recSize;
    VoidHand memHandle;
    BitmapPtr  bitmap;
    char *argv[3] = {"KVM", NULL, NULL};
    int argc = 2;
    DmSearchStateType state;
    UInt16 card;
    LocalID currentDB = 0;
    UInt32 sorted;

    /* Try to open the class database */
    /* These days, the class database is used primarily */
    /* for storing some resource info -- classes are */
    /* typically romized into the KVM itself */
    errno = DmGetNextDatabaseByTypeCreator(true, &state, KVMclassDBType, 
                KVMCreator, false, &card, &currentDB);
    if (!errno && currentDB ) {
        DmDatabaseInfo (card, currentDB, NULL, NULL, NULL, NULL, NULL, NULL, 
            &sorted, NULL, NULL, NULL, NULL);

        if (sorted < 1337) {
            sorted = 1337;

            DmSetDatabaseInfo (card, currentDB, NULL, NULL, NULL, NULL, NULL, 
                NULL, &sorted, NULL, NULL, NULL, NULL);
            sorted = 0;
        }

        ClassfileAltDB = DmOpenDatabase(card, currentDB, sorted < 1337 ? 
                             dmModeReadWrite : dmModeReadOnly);
        if (sorted < 1337) DmQuickSort (ClassfileAltDB, strcmpnames, NULL);
    }

    ClassfileDB = DmOpenDatabase(cardno, dbID, dmModeReadOnly);
    ErrFatalDisplayIf(ClassfileDB == NULL, "Couldn't open resource");

    ERROR_TRY {
        /* check for networking */
        recHandle = DmGetResource('Flag', 1);
#if ENABLE_JAVA_DEBUGGER
        if (!KVMprefs.enableDebugger) {
            AppNetRefnum = 0;           /* indicates not loaded */
        } else {
            AppNetTimeout = -1;
        }
#else
        AppNetRefnum = 0;               /* indicates not loaded */
#endif /* ENABLE_JAVA_DEBUGGER */

        if (recHandle != NULL) {
            Word netIFerr;

            DmReleaseResource(recHandle);
            useNetLib = true;

            /* open Net.lib first */
            errno = SysLibFind("Net.lib", &AppNetRefnum);
            if (errno) {
                AlertUser("Net.lib not found");
                ERROR_THROW(-1);
            }
            errno = NetLibOpen(AppNetRefnum, &netIFerr);
            if (errno && errno != netErrAlreadyOpen) {
                StrPrintF(str_buffer, "Can't open Net.lib, error: %ld",
                          (long)errno);
                AlertUser(str_buffer);
                ERROR_THROW(-1);
            }
            if (netIFerr) {
                NetLibClose(AppNetRefnum, FALSE);
                StrPrintF(str_buffer, "Net.lib interface error: 0x%lx",
                          (long)netIFerr);
                AlertUser(str_buffer);
                ERROR_THROW(-1);
            }
            AppNetTimeout = -1;
        }

        InitializeStandardIO();

        /* find main class name */
        recHandle = DmGetResource('Main', 1);
        ErrFatalDisplayIf(recHandle == NULL, "'Main' resource not found");

        recPtr = MemHandleLock(recHandle);
        recSize = MemHandleSize(recHandle);

        memHandle = MemHandleNew(recSize + 1);
        argv[1] = MemHandleLock(memHandle);

        MemMove(argv[1], recPtr, recSize);
        argv[1][recSize] = 0;

        MemHandleUnlock(recHandle);
        DmReleaseResource(recHandle);

#if INCLUDEDEBUGCODE
        /* These are here so that you can set the ones you want to 1.  
         * Eventually, kvmutil should be able to deal with this.
         */
        tracememoryallocation = 0;
        tracegarbagecollection = 0;
        tracegarbagecollectionverbose = 0;
        traceclassloading = 0;
        traceclassloadingverbose = 0;
        traceverifier = 0;
        tracestackmaps = 0;
        tracebytecodes = 0;
        tracemethodcalls = 0;
        tracemethodcallsverbose = 0;
        tracestackchunks = 0;
        traceframes = 0;
        traceexceptions = 0;
        traceevents = 0;
        tracemonitors = 0;
        tracethreading = 0;
        tracenetworking = 0;
#endif
 
        /* Create empty form. Even though we don't use the standard Palm
           events and forms, it seems that we must create and draw upon
           a form during KVM execution. Otherwise, we get various kinds
           of crashes and problems when the user taps certain buttons.
         */
        form = FrmInitForm(EmptyForm);
        FrmDrawForm(form);
        FrmSetActiveForm(form);

        /* Show splash screen */
        WinResetClip();
        WriteCenteredText(argv[0], 28);
        bitmap = MemHandleLock(DmGetResource(bitmapRsc, SplashBitmap));
        WinDrawBitmap(bitmap, 50, 50);
        MemPtrUnlock(bitmap);
        DmReleaseResource(MemPtrRecoverHandle(bitmap));
        WriteCenteredText("Loading & verifying class files...", 120);

        /* Launch the VM */
        StartJVM(argc - 1, argv + 1);

        /* Release string containing class name */
        MemPtrFree(argv[1]);

        /* Close net.lib */
        if (useNetLib) {
            NetLibClose(AppNetRefnum, false);
        }
        FinalizeINetLib();
#if ENABLE_JAVA_DEBUGGER
        CloseDebugger();
#endif
    } ERROR_CATCH (error) {
        /* Do nothing for now */
    } ERROR_END_CATCH

    /* In case the application forgot to close a resource */
    DmResetRecordStates(ClassfileDB);   

    /* Close the databases */
    DmCloseDatabase(ClassfileDB);
    if (ClassfileAltDB != NULL) {
        DmCloseDatabase(ClassfileAltDB);
    }
    FinalizeStandardIO();
}

/*======================================================================
 * The main program
 *====================================================================*/

/*======================================================================
 * FUNCTION:    PilotMain
 * DESCRIPTION: This function is the equivalent of a main() function
 *              under standard C.  It is called by the Emulator to begin
 *              execution of this application.
 * PARAMETERS:  cmd - command specifying how to launch the application.
 *              cmdPBP - parameter block for the command.
 *              launchFlags - flags used to configure the launch.
 * RETURNS:     Any applicable error codes.
 *====================================================================*/

int stopEventCount;
bool_t calledFromJam;

DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags) {
    switch(cmd) {

    case sysAppLaunchCmdNormalLaunch:
        /* Run KVMutil instead */
        launchKVMutil(cmd, cmdPBP);
        break;

    case sysAppLaunchCmdGoTo: {
        UInt cardNo = ((GoToParamsPtr)cmdPBP)->dbCardNo;
        LocalID dbID = ((GoToParamsPtr)cmdPBP)->dbID;
        /* Program */
        UInt returnCardNo  = ((GoToParamsPtr)cmdPBP)->matchPos;
        LocalID returnDbID = ((GoToParamsPtr)cmdPBP)->matchCustom;

        calledFromJam = (returnDbID != 0); 

        if (launchFlags & sysAppLaunchFlagNewGlobals) {
            if (!getKVMprefs()) {
                /* License not accepted.  Create a copy of the goto
                 * parameter to pass to KVMUtil.  Future versions will
                 * be able to bounce it back to us.
                 */
                UInt size = MemPtrSize(cmdPBP);
                Ptr newCmdPBP = MemPtrNew(size);
                MemPtrSetOwner(newCmdPBP, 0);
                memcpy(newCmdPBP, cmdPBP, size);
                launchKVMutil(cmd, newCmdPBP);
            } else {
                Err error = RomVersionCompatible(version30,
                                                 launchFlags, CustomAlert);
                if (error) return error;
                stopEventCount = 0;
#if ENABLE_JAVA_DEBUGGER
                if (KVMprefs.enableDebugger) {
                    debuggerActive = TRUE;
                    debuggerPort = KVMprefs.debuggerPort;
                    suspend = TRUE;
                    WriteCenteredText("Waiting for debugger...", 120);
                }
#endif
                RunKVM(cmd, cardNo, dbID);  /* run the VM */
        
                if (returnDbID != 0 && stopEventCount == 0) { 
                    GoToParamsPtr gotoP = MemPtrNew(sizeof(GoToParamsType));
                    MemPtrSetOwner(gotoP, 0);
                    MemSet(gotoP, sizeof(GoToParamsType), 0);
                    gotoP->dbCardNo = returnCardNo;
                    gotoP->dbID = returnDbID; 
                    gotoP->matchPos = cardNo;
                    gotoP->matchCustom = dbID; 
                    SysUIAppSwitch(returnCardNo, returnDbID, 
                    sysAppLaunchCmdGoTo, (Ptr)gotoP);
                }

                doAppStop();
            }
        }
        break;
    }

    case sysAppLaunchCmdExgAskUser: {
        ExgAskParamPtr paramP = (ExgAskParamPtr)cmdPBP;
        ExgSocketPtr   socketP = paramP->socketP;

        /* ensure KVM is running & data name/description are as expected */
        if ( (launchFlags & sysAppLaunchFlagSubCall)
                && areAliveThreads()
                && !strcmp(socketP->name, "Application data")
                && !strcmp(socketP->name, socketP->description) ) {
            paramP->result = exgAskDialog;
        } else {
            FrmCustomAlert(CustomAlert,
                "KVM can receive \"Application data\" only if a Java application is running.", NULL, NULL);
            paramP->result = exgAskCancel;
        }
    }
    break;

    case sysAppLaunchCmdExgReceiveData:
        ReceiveBeamData((ExgSocketPtr)cmdPBP);
        break;
    }
    return 0;
}


