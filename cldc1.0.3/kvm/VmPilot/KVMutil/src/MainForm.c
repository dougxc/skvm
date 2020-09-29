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
 * SUBSYSTEM: User interface
 * FILE:      MainForm.c
 * OVERVIEW:  Main display of the KVMutil application
 * AUTHOR:    Mitch Butler
 *=====================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <KVMutil.h>

/*
 * Globals
 */

Short oldPort;

/*======================================================================
 * Functions
 *====================================================================*/

static void 
showDisplayLines(void)
{
    ControlPtr linesButton = GetObjectPtr(KVMutilMainDispLinesPushButton);
    char string[20];
    StrPrintF(string, "%ld  lines", KVMprefs.displayLines); 
    CtlSetLabel(linesButton, string);
}

static void 
changeDisplayLines(Int change)
{
    KVMprefs.displayLines += change;
    KVMprefsChanged = true;
    showDisplayLines();
}

static void 
showHeapSize(void)
{
    ControlPtr heapButton = GetObjectPtr(KVMutilMainHeapSizePushButton);
    char string[20];
    StrPrintF(string, "%ld  kb", KVMprefs.heapSize); 
    CtlSetLabel(heapButton, string);        
}
   
static void 
changeHeapSize(Int change)
{
    KVMprefs.heapSize += change;
    KVMprefsChanged = true;
    showHeapSize();
}

static void 
showCheckboxes(void)
{
    ControlPtr checkbox = GetObjectPtr(KVMutilMainSaveOutputCheckbox);
    CtlSetValue(checkbox, KVMprefs.saveOutput);        
    checkbox = GetObjectPtr(KVMutilMainShowHeapStatsCheckbox);
    CtlSetValue(checkbox, KVMprefs.showHeapStats);        
    checkbox = GetObjectPtr(KVMutilMainDebuggerEnableCheckbox);
    CtlSetValue(checkbox, KVMprefs.enableDebugger);        
}
   
static void 
showOutputButton(void)
{
    ControlPtr button = GetObjectPtr(KVMutilMainViewOutputButton);
    if (KVMstdoutDB == NULL) {
        CtlHideControl(button);                  
    } else {
        CtlShowControl(button);          
    }
}
   
static void 
disableInfoButtons(void)
{
    /* We use these buttons just to get centered text in a box.
       We don't want the user to be able to push them as buttons. */
    ControlPtr button = GetObjectPtr(KVMutilMainDispLinesPushButton);
    CtlSetEnabled(button, false);        
    button = GetObjectPtr(KVMutilMainHeapSizePushButton);
    CtlSetEnabled(button, false);        
}

static void
showPortfield(void)
{
    Char s[32];
    Handle newHandle;
    CharPtr newText;

    FieldPtr fld = GetObjectPtr(KVMutilMainPortField);
    oldPort = KVMDefaultDebuggerPort;
    StrIToA(s, oldPort); /* default value */
    if(KVMprefs.debuggerPort != 0) {
        oldPort = KVMprefs.debuggerPort;
        StrIToA(s, oldPort);
    } else {
        /* changed from 0 to default value */
        KVMprefs.debuggerPort = oldPort;
        KVMprefsChanged = true;
    }
    newHandle = MemHandleNew(StrLen(s) + 1);
    newText = MemHandleLock(newHandle);
    StrCopy(newText, s);
    MemHandleUnlock(newHandle);
    FldSetTextHandle(fld, newHandle);
}

static void
getPortField(void)
{
    CharPtr text;
    Short newPort;
    FieldPtr fld = GetObjectPtr(KVMutilMainPortField);
    text = FldGetTextPtr(fld);
    newPort = (Short)StrAToI(text);
    if (oldPort != newPort) {
        KVMprefs.debuggerPort = (Short)StrAToI(text);
        KVMprefsChanged = true;
    }
}
   
/*======================================================================
 * FUNCTION:    createDatabase
 *====================================================================*/

static Boolean 
createDatabase(void)
{
    VoidHand recHand;
    CharPtr recPtr;
    Err createErr;
    UInt atP;
    const char EMPTY[] = "empty";
 
    createErr = DmCreateDatabase(0, "KVMstdIO",
             KVMutilCreator, KVMstdoutDBType, false);
    if (!createErr) {             
        KVMstdoutDB = DmOpenDatabaseByTypeCreator(KVMstdoutDBType,
                 KVMutilCreator, dmModeReadWrite);
        for (atP = 0; atP <= 1; atP++) {
            recHand = DmNewRecord(KVMstdoutDB, &atP, MAXCHUNKSIZE);
            recPtr = MemHandleLock(recHand);
            DmWrite(recPtr, 0, EMPTY, StrLen(EMPTY)+1);  /* empty record */
            MemHandleUnlock(recHand);
            DmReleaseRecord(KVMstdoutDB, atP, false);
        }
        showOutputButton();
    }
}

/*======================================================================
 * Event handling public operations
 *====================================================================*/

/*======================================================================
 * FUNCTION:    MainFormInit
 *====================================================================*/

static void 
MainFormInit(void)
{
    FormPtr  form; 
    FieldPtr classNameField;
    VoidHand recHandle;
    CharPtr  recPtr;
    UInt     recSize;
    VoidHand memHandle;
    CharPtr  memPtr;
    
    /* set up form data and draw the form*/
    showCheckboxes();
    showOutputButton();
    disableInfoButtons();
    showPortfield();
    form = FrmGetActiveForm();
    FrmDrawForm(form);

    /* we must do these after form is drawn, otherwise they don't work */
    showDisplayLines();
    showHeapSize();
}

/*======================================================================
 * FUNCTION:    MainFormHandleEvent
 * DESCRIPTION: Handles processing of events for the main form.
 * PARAMETERS:  event - the most recent event.
 * RETURNS:     True if the event is handled, false otherwise.
 *====================================================================*/

Boolean MainFormHandleEvent(EventPtr event)
{
    Boolean    handled = false;
    FormPtr    form; 
    BitmapPtr  bitmap;

    CALLBACK_PROLOGUE; 

    form = FrmGetActiveForm();

    switch (event->eType) {
      /* The form was told to open */
      case frmOpenEvent:     
      case frmUpdateEvent:
        MainFormInit();
        handled = true;
        break;

      case frmCloseEvent:
        getPortField();
        FrmHandleEvent(form, event);
        handled = true;
        break;

      case winEnterEvent:
        /* do this on return from Output screen.
           we may need to turn off the output button. */
        if (returnFromOutput) {
            MainFormInit();
            returnFromOutput = false;
        }
        break;

      case menuEvent:
        /* First clear the menu status from the display */
        MenuEraseStatus(0);
        if (event->data.menu.itemID == OptionsAboutKVM) {
            /* Display the about box */
            FrmPopupForm(KVMutilAboutForm);
        } else {
            /* Reset the License preference */
            KVMprefs.licenseAccepted = false;
            KVMprefsChanged = true;
        }
        handled = true;
        break;
 
      case ctlEnterEvent:
        switch(event->data.ctlEnter.controlID) {
            case KVMutilMainFrameButton:
                handled = true;
                break;
        }
        break;

      case ctlSelectEvent:
        switch(event->data.ctlEnter.controlID) {
            case KVMutilMainViewOutputButton:
                FrmPopupForm(KVMutilOutputForm);
                handled = true;
                break;

            case KVMutilMainSaveOutputCheckbox:
                { 
                    ControlPtr saveOutputToggle = 
                    GetObjectPtr(KVMutilMainSaveOutputCheckbox);
                    KVMprefsChanged = true;
                    if (KVMprefs.saveOutput = CtlGetValue(saveOutputToggle)) {
                        createDatabase();
                    }
                    handled = true;
                }
                break;

            case KVMutilMainShowHeapStatsCheckbox:
                {
                    ControlPtr showHeapStatsToggle = 
                        GetObjectPtr(KVMutilMainShowHeapStatsCheckbox);
                    KVMprefs.showHeapStats = CtlGetValue(showHeapStatsToggle);
                    KVMprefsChanged = true;
                    handled = true;
                }
                break;

            case KVMutilMainDebuggerEnableCheckbox:
                {
                    ControlPtr enableDebuggerToggle =
                        GetObjectPtr(KVMutilMainDebuggerEnableCheckbox);
                    KVMprefs.enableDebugger = CtlGetValue(enableDebuggerToggle);
                    KVMprefsChanged = true;
                    handled = true;
                }
                break;
        }
        break;
        
      case ctlRepeatEvent:
        switch(event->data.ctlEnter.controlID) {
            case KVMutilMainDispLinesDecRepeating:
                if (KVMprefs.displayLines > 0) changeDisplayLines(-1);
                break;
            
            case KVMutilMainDispLinesIncRepeating:
                if (KVMprefs.displayLines < MAXDISPLAYLINES) 
                    changeDisplayLines(+1);
                break;
            
            case KVMutilMainHeapDecRepeating:
                if (KVMprefs.heapSize > 1) changeHeapSize(-1);
                break;
            
            case KVMutilMainHeapIncRepeating:
                if (KVMprefs.heapSize < maxHeapSize) changeHeapSize(+1);
                break;            
        }
        /* repeating controls don't repeat if handled = true */
        break;
        
    }

    CALLBACK_EPILOGUE;

    return handled;
}

