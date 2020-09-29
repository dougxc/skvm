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
 * SUBSYSTEM: Output viewing form
 * FILE:      OutputForm.c
 * OVERVIEW:  Form to view the output buffers after an execution.
 * AUTHOR:    Doug Simon, Sun Labs (original Spotless version)
 * CREATED:   Nov 24, 1998
 *            Mar 05, 2000 updates by Mitch Butler
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=====================================================================*/

#include <KVMutil.h>

/*=======================================================================
 * The stream whose buffer is currently being inspected
 *=====================================================================*/

VoidHand currentStream;
VoidHand stdoutStream;
VoidHand stderrStream;

/*=======================================================================
 * FUNCTION:    updateScrollBar
 * DESCRIPTION: This routine updates the scroll bar.
 * PARAMETERS:  nothing
 * RETURNS:     nothing
 *=====================================================================*/

static void 
updateScrollBar ()
{
    Word scrollPos;
    Word textHeight;
    Word fieldHeight;
    Short maxValue;
    FieldPtr fld;
    ScrollBarPtr bar;
    
    fld = GetObjectPtr (KVMutilOutputField);
    bar = GetObjectPtr (KVMutilOutputScrollBar);
    
    FldGetScrollValues (fld, &scrollPos, &textHeight,  &fieldHeight);

    if (textHeight > fieldHeight)
        maxValue = textHeight - fieldHeight;
    else if (scrollPos)
        maxValue = scrollPos;
    else
        maxValue = 0;

    SclSetScrollBar (bar, scrollPos, 0, maxValue, fieldHeight-1);
}

/***********************************************************************
 * FUNCTION:    refreshOutputField
 * DESCRIPTION: Sets the source for the field and draws it, ensuring
 *              that the scroll bar is updated as well.
 * PARAMETERS:  streamHand - source for field's text
 * RETURNS:     nothing
 ***********************************************************************/

static void 
refreshOutputField(VoidHand streamHand)
{
    FieldPtr fld = GetObjectPtr (KVMutilOutputField);
    CharPtr recPtr = MemHandleLock(streamHand);
    UInt textLen = StrLen(recPtr);
    CharPtr rolloverTag = StrStr(recPtr, ROLLOVERTAG);
    Word position = 0;
    
    /* Set the text handle and scroll position */
    FldSetTextHandle (fld, (Handle)streamHand);
    FldSetMaxChars(fld, textLen);
    if (rolloverTag) {
        Word linesToScroll = 9;
        position = rolloverTag - recPtr;
        FntWordWrapReverseNLines(recPtr, 160, &linesToScroll, &position);
    }
    FldSetScrollPosition (fld, position);
    MemHandleUnlock(streamHand);

    /* Update the scroll bar */
    updateScrollBar ();
    
    currentStream = streamHand;
}

/***********************************************************************
 * FUNCTION:    scrollLines
 * DESCRIPTION: Scroll the field a given amount of lines.
 * PARAMETERS:  linesToScroll - the number of lines to scroll. The
 *                 sign of this parameter determines the direction of
 *                 the scrolling.
 * RETURNS:     nothing
 ***********************************************************************/

static void 
scrollLines (Short linesToScroll)
{
    FieldPtr            fld;    
    fld = GetObjectPtr (KVMutilOutputField);

    if (linesToScroll < 0)
        FldScrollField (fld, -linesToScroll, kWinUp);
    else if (linesToScroll > 0)
        FldScrollField (fld, linesToScroll, kWinUp);
}

/***********************************************************************
 * FUNCTION:    scrollPage
 * DESCRIPTION: This routine scrolls the view a page up or down.
 * PARAMETERS:  direction - up or down
 * RETURNS:     nothing
 ***********************************************************************/

static void 
scrollPage (kWinDirectionType direction)
{
    Short value;
    Short min;
    Short max;
    Short pageSize;
    Word linesToScroll;
    FieldPtr fld;
    ScrollBarPtr bar;

    fld = GetObjectPtr (KVMutilOutputField);
    
    /* Scroll the field by a page in the given direction */
    linesToScroll = FldGetVisibleLines (fld) - 1;
    FldScrollField (fld, linesToScroll, direction);

    /* Update the scroll bar. */
    bar = GetObjectPtr (KVMutilOutputScrollBar);
    SclGetScrollBar (bar, &value, &min, &max, &pageSize);

    if (direction == kWinUp)
        value -= linesToScroll;
    else
        value += linesToScroll;
    
    SclSetScrollBar (bar, value, min, max, pageSize);
}

/***********************************************************************
 * FUNCTION:    clearOutputField
 ***********************************************************************/

static void 
clearOutputField (void)
{
    FieldPtr fld = GetObjectPtr(KVMutilOutputField);
    FldSetTextHandle(fld, NULL);
    FldSetScrollPosition (fld, 0);
    DmReleaseRecord(KVMstdoutDB, 0, false);
    DmReleaseRecord(KVMstdoutDB, 1, false);
    returnFromOutput = true;
}

/***********************************************************************
 * FUNCTION:    OutputFormHandleEvent
 * DESCRIPTION: The event handler for the form used to view the output
 *              buffers.
 * PARAMETERS:  event  - the event to handle
 * RETURNS:     true if the event has handled and should not be passed
 *              to a higher level handler.
 ***********************************************************************/

Boolean OutputFormHandleEvent (EventPtr event)
{
    FormPtr frm;
    FieldPtr fld;
    Boolean handled = false;
    UInt card;
    LocalID dbID;
    ControlPtr ctl;
    
    CALLBACK_PROLOGUE; 

    switch (event->eType) { 

      case keyDownEvent:
        if (event->data.keyDown.chr == pageUpChr) {
            scrollPage(kWinUp);
            handled = true;
        } else if (event->data.keyDown.chr == pageDownChr) {
            scrollPage(kWinDown);
            handled = true;
        }
        break;

      case ctlSelectEvent:
        switch (event->data.ctlSelect.controlID) { 
          case KVMutilOutputDeleteButton:
            clearOutputField();
            DmOpenDatabaseInfo(KVMstdoutDB, &dbID,
                               NULL, NULL, &card, NULL);
            DmCloseDatabase(KVMstdoutDB);
            KVMstdoutDB = NULL;
            DmDeleteDatabase(card, dbID);
            KVMprefs.saveOutput = false;
            KVMprefsChanged = true;
            FrmReturnToForm(KVMutilMainForm);
            handled = true;
            break;
            
          case KVMutilOutputCloseButton: 
            clearOutputField();
            FrmReturnToForm(KVMutilMainForm);
            handled = true;
            break;
            
          case KVMutilOutputSwitchButton:
            ctl = GetObjectPtr(KVMutilOutputSwitchButton);
            frm = FrmGetActiveForm();               
            if (currentStream == stdoutStream) {
                refreshOutputField(stderrStream);
                FrmSetTitle(frm,"KVM  stderr");
                CtlSetLabel(ctl,"View stdout");
            } else {
                refreshOutputField(stdoutStream);
                FrmSetTitle(frm,"KVM  stdout");
                CtlSetLabel(ctl,"View stderr");
            }
            handled = true;
        }
        break;

      case frmOpenEvent:
        frm = FrmGetActiveForm ();
        FrmDrawForm(frm);
        stdoutStream = DmGetRecord(KVMstdoutDB, 0);
        stderrStream = DmGetRecord(KVMstdoutDB, 1);
        refreshOutputField(stdoutStream);
        handled = true;
        break;

      case frmCloseEvent:
        clearOutputField();
        /* Do not set handled = true. */
        /* We still want default stuff to be done */
        break;

      case sclRepeatEvent:
        scrollLines(event->data.sclRepeat.newValue - 
                    event->data.sclRepeat.value);
        break;
    }

    CALLBACK_EPILOGUE;

    return handled;
}

