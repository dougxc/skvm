/*
 * Copyright (c) 1999-2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * FILE:      LicenseForm.c
 * OVERVIEW:  Event handling and view operations for the form 
 *            that is used to view the Click-thru License.
 * AUTHOR:    Mitch Butler
 * CREATED:   June 4, 1999
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=====================================================================*/

#include <KVMutil.h>

/*=======================================================================
 * Variables
 *=====================================================================*/

/*=======================================================================
 * View operations
 *=====================================================================*/

/*=======================================================================
 * FUNCTION:    LicenseFormUpdateScrollBar
 * DESCRIPTION: This routine updates the scroll bar.
 * PARAMETERS:  nothing
 * RETURNS:     nothing
 *=====================================================================*/

static void LicenseFormUpdateScrollBar()
{
    Word scrollPos;
    Word textHeight;
    Word fieldHeight;
    Short maxValue;
    FieldPtr fld;
    ScrollBarPtr bar;

    fld = GetObjectPtr(KVMutilLicenseLicenseTextField);
    bar = GetObjectPtr(KVMutilLicenseScrollScrollBar);

    FldGetScrollValues(fld, &scrollPos, &textHeight,  &fieldHeight);

    if (textHeight > fieldHeight)
        maxValue = textHeight - fieldHeight;
    else if (scrollPos)
        maxValue = scrollPos;
    else
        maxValue = 0;

    SclSetScrollBar(bar, scrollPos, 0, maxValue, fieldHeight-1);
}

/*=======================================================================
 * FUNCTION:    LicenseFormScrollLines
 * DESCRIPTION: Scroll the field a given amount of lines.
 * PARAMETERS:  linesToScroll - the number of lines to scroll. The
 *                 sign of this parameter determines the direction of
 *                 the scrolling.
 * RETURNS:     nothing
 *=====================================================================*/

static void LicenseFormScrollLines(Short linesToScroll)
{
    FieldPtr fld;

    fld = GetObjectPtr(KVMutilLicenseLicenseTextField);

    if (linesToScroll < 0)
        FldScrollField(fld, -linesToScroll, kWinUp);
    else if (linesToScroll > 0)
        FldScrollField(fld, linesToScroll, kWinDown);
}

/*=======================================================================
 * FUNCTION:    LicenseFormScrollPage
 * DESCRIPTION: This routine scrolls the view a page up or down.
 * PARAMETERS:  direction - up or down
 * RETURNS:     nothing
 *=====================================================================*/

static void LicenseFormScrollPage(kWinDirectionType direction)
{
    Short value;
    Short min;
    Short max;
    Short pageSize;
    Word linesToScroll;
    FieldPtr fld;
    ScrollBarPtr bar;

    fld = GetObjectPtr(KVMutilLicenseLicenseTextField);

    /* Scroll the field by a page in the given direction */
    linesToScroll = FldGetVisibleLines (fld) - 1;
    FldScrollField(fld, linesToScroll, direction);

    /* Update the scroll bar. */
    bar = GetObjectPtr(KVMutilLicenseScrollScrollBar);
    SclGetScrollBar(bar, &value, &min, &max, &pageSize);

    if (direction == kWinUp)
        value -= linesToScroll;
    else
        value += linesToScroll;

    SclSetScrollBar(bar, value, min, max, pageSize);
}

/*=======================================================================
 * View constructor
 *=====================================================================*/

/*======================================================================
 * FUNCTION:    LicenseFormInit
 * DESCRIPTION: Initializes the output form (view constructor).
 * PARAMETERS:  None.
 * RETURNS:     Nothing.
 *====================================================================*/

static void 
LicenseFormInit()
{
    FormPtr form = FrmGetActiveForm();
    FieldPtr fld = GetObjectPtr(KVMutilLicenseLicenseTextField);
    FldSetTextHandle (fld, (Handle)DmGetResource (strRsc, LicenseTextString));
    FrmDrawForm(form);

    /* Set the text handle and scroll position */
    FldSetScrollPosition(fld, 0);
    FldSetMaxChars(fld, FldGetTextLength(fld));

    /* Update the scroll bar */
    LicenseFormUpdateScrollBar();
}

/*=======================================================================
 * Controller (event handling) operations
 *=====================================================================*/

/*=======================================================================
 * FUNCTION:    LicenseFormHandleEvent
 * DESCRIPTION: The event handler for the form used to view the output
 *              buffers.
 * PARAMETERS:  event  - the event to handle
 * RETURNS:     true if the event has handled and should not be passed
 *              to a higher level handler.
 *=====================================================================*/

Boolean LicenseFormHandleEvent (EventPtr event)
{
    Boolean handled = false;

    CALLBACK_PROLOGUE; 

    switch (event->eType) { 

    case keyDownEvent:
        if (event->data.keyDown.chr == pageUpChr) {
            LicenseFormScrollPage(kWinUp);
            handled = true;
        } else if (event->data.keyDown.chr == pageDownChr) {
            LicenseFormScrollPage(kWinDown);
            handled = true;
        }
        break;

    case ctlSelectEvent: { 
        /* Unset the text handle for the field */
        if (event->data.ctlSelect.controlID == KVMutilLicenseAcceptButton) {
            KVMprefs.licenseAccepted = true;
            KVMprefsChanged = true;
            FrmGotoForm(KVMutilMainForm);
        } else {
            ReturnToLauncher();
        }
        handled = true;
        break;
    }

    case frmOpenEvent:
        LicenseFormInit();
        handled = true;
        break;

    case frmCloseEvent: { 
        FieldPtr fld = GetObjectPtr(KVMutilLicenseLicenseTextField);
        Handle oldHandle = FldGetTextHandle(fld); /* this is a resource */
        FldSetTextHandle(fld, NULL);
        DmReleaseResource(oldHandle);
        /* handled = false, so that default close handler gets run */
        break;
    }

    case sclRepeatEvent:
        LicenseFormScrollLines(event->data.sclRepeat.newValue - 
                               event->data.sclRepeat.value);
        break;

    default:
        break;

    }

    CALLBACK_EPILOGUE;

    return handled;
}


