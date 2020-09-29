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
 * SYSTEM:    KVMutil
 * SUBSYSTEM: User interface for KVMutil
 * FILE:      Main.c
 * OVERVIEW:  User interface main program for the KVMutil tool
 *            that allows the Palm user to set various KVM
 *            execution options.
 * AUTHOR:    Mitch Butler
 *=======================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <KVMutil.h>

/*======================================================================
 * Global variables
 *====================================================================*/

struct KVMprefsStruct KVMprefs;
DmOpenRef KVMstdoutDB;

Boolean returnFromOutput = false;
Boolean KVMprefsChanged = false;
Int maxHeapSize;        /* in kb */

/*======================================================================
 * Auxiliary routines
 *====================================================================*/

/*======================================================================
 * FUNCTION:    GetObjectPtr
 * DESCRIPTION: Returns a pointer to an object in the active form.
 * PARAMETERS:  objectID - id of the object
 * RETURNS:     pointer to the object's data structure
 *====================================================================*/

VoidPtr GetObjectPtr(Int objectID)
{
    FormPtr frm = FrmGetActiveForm();
    return FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectID));
}

/*======================================================================
 * FUNCTION:    GetFocusObjectPtr
 * DESCRIPTION: This routine returns a pointer to the field object, in 
 *              the current form, that has the focus.
 * PARAMETERS:  nothing
 * RETURNS:     pointer to a field object or NULL of there is no focus
 *====================================================================*/

FieldPtr GetFocusObjectPtr(void)
{
    FormPtr frm;
    Word focus;
    
    /*  Get a pointer to tha active form and 
     * the index of the form object with the focus. 
     */
    frm = FrmGetActiveForm();
    focus = FrmGetFocus(frm);
    
    /* If no object has the focus return NULL pointer. */
    if (focus == noFocus) return(NULL);
        
    /* Return a pointer to the object with focus. */
    return FrmGetObjectPtr(frm, focus);
}

/*======================================================================
 * Application startup & shutdown functions
 *====================================================================*/

/*======================================================================
 * FUNCTION:     StartApplication
 * DESCRIPTION:  This routine sets up the initial state of the application.
 *               It opens the application's database and sets up global variables. 
 * PARAMETERS:   None.
 *====================================================================*/

static void 
StartApplication(void) 
{
   Int startView = KVMutilMainForm;    /* show the main form */
    
    maxHeapSize = (getMaxHeapSize() + 1023) >> 10;  /* convert to kb */
    
    /* open stdout DB, if it exists */
    KVMstdoutDB = DmOpenDatabaseByTypeCreator(KVMstdoutDBType, KVMutilCreator,
                                              dmModeReadOnly);  
    if (!getKVMprefs()) {
        /* License not accepted - show license form */
        startView = KVMutilLicenseForm;
        /* initialize the heapSize if it is 0 */
        if (!KVMprefs.heapSize) KVMprefs.heapSize = maxHeapSize;
    } else if (KVMprefs.saveOutput && !KVMstdoutDB) {
        KVMprefs.saveOutput = false;
        KVMprefsChanged = true;
    }
    FrmGotoForm(startView);
}


/*======================================================================
 * FUNCTION:    StopApplication
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
 * PARAMETERS:  nothing
 * RETURNS:     nothing
 *====================================================================*/

static void 
StopApplication(void) 
{
    /* Close all open forms.  This sends every open form a frmCloseEvent */
    FrmCloseAllForms();
    
    /* save preferences if they were changed */
    if (KVMprefsChanged) {
        PrefSetAppPreferences(KVMCreator, KVMPrefID, KVMPrefVersion,
                                &KVMprefs, sizeof(KVMprefs), true);
        }

    /* close stdout DB if it is open */
    if (KVMstdoutDB != NULL) {
        DmCloseDatabase(KVMstdoutDB);
    }
}


/*======================================================================
 * Global application event handling
 *====================================================================*/

/*======================================================================
 * FUNCTION:    ApplicationHandleEvent
 * DESCRIPTION: This routine loads a form resource and sets the event 
 *              handler for the form.
 * PARAMETERS:  event - a pointer to an EventType structure
 * RETURNS:     True if the event has been handled and should not be
 *              passed to a higher level handler.
 *====================================================================*/

static Boolean 
ApplicationHandleEvent(EventPtr event)
{
    FormPtr frm;
    Int     formId;
    Boolean handled = false;

    CALLBACK_PROLOGUE; 

    if (event->eType == frmLoadEvent) {
        /* Load the form resource specified in the event 
         * then activate the form. 
         */
        formId = event->data.frmLoad.formID;
        frm = FrmInitForm(formId);
        FrmSetActiveForm(frm);

        /* Set the event handler for the form.  The handler of the currently 
         * active form is called by FrmDispatchEvent each time it receives 
         * an event.   */
        switch (formId) {
            case KVMutilMainForm:
                FrmSetEventHandler(frm, MainFormHandleEvent);
                break;

            case KVMutilAboutForm:
                FrmSetEventHandler(frm, AboutFormHandleEvent);
                break;
                
            case KVMutilLicenseForm:
                FrmSetEventHandler(frm, LicenseFormHandleEvent);
                break;
                
            case KVMutilOutputForm:
                FrmSetEventHandler(frm, OutputFormHandleEvent);
                break;
        }
        handled = true;
    }
    
    CALLBACK_EPILOGUE;

    return handled;
}

/*======================================================================
 * FUNCTION:    EventLoop
 * DESCRIPTION: A simple loop that obtains events from the Event
 *              Manager and passes them on to various applications and
 *              system event handlers before passing them on to
 *              FrmDispatchEvent for default processing.
 * PARAMETERS:  None.
 * RETURNS:     Nothing.
 *====================================================================*/

static void 
EventLoop(void)
{
    EventType event;
    Word error;
    
    do {
        /* Get the next available event. */
        EvtGetEvent(&event, evtWaitForever);
        
        /* This is boilerplate */
        if (!SysHandleEvent(&event))
            if (!MenuHandleEvent(0, &event, &error))
                if (!ApplicationHandleEvent(&event))
                    FrmDispatchEvent(&event);
    } while (event.eType != appStopEvent);
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

DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags) {
    Err error;
    
    switch(cmd) {

    case sysAppLaunchCmdNormalLaunch:
    NormalLaunch:
        error = RomVersionCompatible(version30, launchFlags, CustomAlert);
        if (error) return error;
        StartApplication(); 
        EventLoop();
        StopApplication();
        break;
    
    case sysAppLaunchCmdGoTo: { 
        UInt cardNo = ((GoToParamsPtr)cmdPBP)->dbCardNo;
        LocalID dbID = ((GoToParamsPtr)cmdPBP)->dbID;
        if (launchFlags & sysAppLaunchFlagNewGlobals) goto NormalLaunch;
        break;   
    }

    }
    return 0;
}

