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
 * SUBSYSTEM: Native function interface
 * FILE:      nativePilot.c
 * OVERVIEW:  This file defines the native functions needed
 *            by the KVM on the Palm device.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 * NOTE:      Many functions in this file are platform-specific,
 *            and require extra work when porting the system to
 *            run on another machine.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <nativeSpotlet.h>
#include <KVM.h>
#include <TimeMgr.h>
#include <SoundMgr.h>
#include <NetMgr.h>

#define _STDLIB
#define __string_h
#define __stdarg_h
#define __stdio_h

#define IGNORE_STDIO_STUBS
#include <sys_socket.h>
#undef IGNORE_STDIO_STUBS

/*=========================================================================
 * Declarations
 *=======================================================================*/

void Java_com_sun_kjava_Graphics_initialize(void);
void Java_com_sun_kjava_Database_create(void);
void Java_com_sun_kjava_Database_open(void);
void Java_com_sun_kjava_Database_close(void);
void Java_com_sun_kjava_Database_readRecordToBuffer(void);
void Java_com_sun_kjava_Database_writeRecordFromBuffer(void);
void cvtEvent(EventType *);

/*=========================================================================
 * variables
 *=======================================================================*/

WinHandle offScreenWindow = NIL;
extern int wantPalmSysKeys; /* true if Spotlet wants ALL Palm hard buttons */

/*=========================================================================
 * Native functions of class com.sun.kjava.Graphics
 *=======================================================================*/

/* Constants used in the drawing functions. Should correlate with those */
/* defined in com.sun.kjava.Graphics */
#define MODE_PLAIN  1
#define MODE_GRAY   2
#define MODE_ERASE  3
#define MODE_INVERT 4

/*=========================================================================
 * FUNCTION:      initialize()V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      initialize the graphics sub system.
 * INTERFACE (operand stack manipulation):
 *   parameters:  none
 *   returns:     <nothing>
 *=======================================================================*/

void Java_com_sun_kjava_Graphics_initialize()
{
    /* nothing to do. */
}

/*=========================================================================
 * FUNCTION:      drawLine(IIIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a line. The action performed will depend on
 *                the mode parameter
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX, srcY - the src point of the line
 *                dstX, dstY - the end point of the line
 *                mode - an integer describing the type of drawing action
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_drawLine()
{
    SWord mode = popStack();
    SWord dstY = popStack();
    SWord dstX = popStack();
    SWord srcY = popStack();
    SWord srcX = popStack();

    switch (mode) {
    case MODE_ERASE:
        WinEraseLine(srcX,srcY,dstX,dstY); return;
    case MODE_GRAY:
        WinDrawGrayLine(srcX,srcY,dstX,dstY); return;
    case MODE_INVERT:
        WinInvertLine(srcX,srcY,dstX,dstY); return;
    case MODE_PLAIN:
    default:
        WinDrawLine(srcX,srcY,dstX,dstY); return;
    }
}

/*=========================================================================
 * FUNCTION:      drawRectangle(IIIIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a rectangle. The action performed will depend on
 *                the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                cornerDiam - diameter or circle used to give rounded corners
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_drawRectangle() {
    RectangleType rt;
    int cornerDiam = popStack();
    int mode       = popStack();
    rt.extent.y    = popStack();
    rt.extent.x    = popStack();
    rt.topLeft.y   = popStack();
    rt.topLeft.x   = popStack();

    switch (mode) {
    case MODE_ERASE:
        WinEraseRectangle(&rt,cornerDiam); return;
    case MODE_GRAY:     {
        static CustomPatternType gray_pattern =
            { 0xAA33, 0xAA33, 0xAA33, 0xAA33 };
        CustomPatternType current;
        kWinGetPattern(current);
        kWinSetPattern(gray_pattern);
        WinFillRectangle(&rt,cornerDiam);
        kWinSetPattern(current);
        return;
    }
    case MODE_INVERT:
        WinInvertRectangle(&rt,cornerDiam); return;
    case MODE_PLAIN:
    default:
        WinDrawRectangle(&rt,cornerDiam); return;
    }
}

/*=========================================================================
 * FUNCTION:      drawBorder(IIIIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a border around a rectangle. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the upper left corner coordinates
 *                width,height - the width and height of the rectangle
 *                mode - an integer describing the type of drawing action
 *                borderType - the type of the border
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_drawBorder()
{
    RectangleType rt;
    FrameType frameType = popStack();
    int mode       = popStack();
    rt.extent.y    = popStack();
    rt.extent.x    = popStack();
    rt.topLeft.y   = popStack();
    rt.topLeft.x   = popStack();

    switch (mode) {
    case MODE_GRAY:
        WinDrawGrayRectangleFrame(frameType,&rt); return;
    case MODE_ERASE:
        WinEraseRectangleFrame(frameType,&rt); return;
    case MODE_INVERT:
        WinInvertRectangleFrame(frameType,&rt); return;
    case MODE_PLAIN:
    default:
        WinDrawRectangleFrame(frameType,&rt);
    }
}

/*=========================================================================
 * FUNCTION:      drawString(Ljava/lang/String;III)I (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw or erase a string at a given position. The action
 *                performed will depend on the mode parameter.
 * INTERFACE (operand stack manipulation):
 *   parameters:  text - the String to draw
 *                left,top - the top left bound of the first character
 *                mode - an integer describing the type of drawing action
 *   returns:     the left parameter plus the width of the string in pixels
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_drawString()
{
    int mode = popStack();
    SWord y  = popStack();
    SWord x  = popStack();
    STRING_INSTANCE string = topStackAsType(STRING_INSTANCE);
    char* str;
    int strlen;

    if (string == NULL) { 
        /* Don't do anything.  Just return 0. */
        topStack = 0;
        return;
    } 
    str = getStringContents(string);
    strlen = StrLen(str);

    switch (mode) {
    case MODE_ERASE:
        WinEraseChars(str,strlen,x,y);
        break;
    case MODE_INVERT:
        WinDrawInvertedChars(str,strlen,x,y);
        break;
    case MODE_PLAIN:
    default:
        WinDrawChars(str,strlen,x,y);
        break;
    }

    topStack = (cell)x+FntCharsWidth(str, strlen);
}

/*=========================================================================
 * FUNCTION:      getWidth(Ljava/lang/String;)I (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Return the width of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the width of the string in pixels
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_getWidth()
{
    STRING_INSTANCE string = topStackAsType(STRING_INSTANCE);
    if (string == NULL) { 
        topStack = 0;
    } else { 
        char* str = getStringContents(string);
        topStack = (cell)FntCharsWidth(str,StrLen(str));
    }
}

/*=========================================================================
 * FUNCTION:      getHeight(Ljava/lang/String;)I (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Return the height of a String in pixels.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a String object
 *   returns:     the height of the string in pixels
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_getHeight()
{
    topStack = (cell)FntCharHeight();
}

/*=========================================================================
 * FUNCTION:      setDrawRegion(IIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Set the clipping rectangle.
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - the top left corner of the clipping rectangle
 *                width,height - the dimensions of the clipping rectangle
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_setDrawRegion()
{
    RectangleType rt;
    rt.extent.y    = popStack();
    rt.extent.x    = popStack();
    rt.topLeft.y   = popStack();
    rt.topLeft.x   = popStack();

    WinSetClip(&rt);
}

/*=========================================================================
 * FUNCTION:      resetDrawRegion()V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Reset the clipping rectangle to be the whole window.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_resetDrawRegion()
{
    WinResetClip();
}

/*=========================================================================
 * FUNCTION:      copyRegion(IIIIIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Copy a region to another region (possibly the same location)
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX,srcY - the origin of the copy
 *                width,height - the dimensions of the region to be copied
 *                dstX,dstY - the destination of the copy
 *                mode - the copy mode to be used
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_copyRegion()
{
    WinHandle winHand = WinGetDisplayWindow();
    ScrOperation mode = (ScrOperation)popStack();
    SWord dstY        = popStack();
    SWord dstX        = popStack();
    RectangleType src;
    src.extent.y    = popStack();
    src.extent.x    = popStack();
    src.topLeft.y   = popStack();
    src.topLeft.x   = popStack();

    WinCopyRectangle(winHand, winHand, &src, dstX, dstY, mode);
}

/*=========================================================================
 * FUNCTION:      copyOffScreenRegion(IIIIIIIII)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Copy a region to another region (possibly the same location,
 *                  possibly between windows)
 * INTERFACE (operand stack manipulation):
 *   parameters:  srcX,srcY - the origin of the copy
 *                width,height - the dimensions of the region to be copied
 *                dstX,dstY - the destination of the copy
 *                mode - the copy mode to be used
 *                scrWin,dstWin - the windows to copy to/from
 *   returns:     <nothing>
 *=======================================================================*/

#define ONSCREEN_WINDOW   0
#define OFFSCREEN_WINDOW  1

/* Helper routine for copyRegion */
static WinHandle
getWindow(int windowCode)
{
    WinHandle displayWindow = WinGetDisplayWindow();
    if (!windowCode) return displayWindow;
    if (offScreenWindow == NIL) {
        Word error;
        RectangleType fullScreen =  {{ 0, 0}, {160, 160}};
        offScreenWindow = WinCreateOffscreenWindow(160, 160,
                                                   screenFormat, &error);
        if (error) {
            AlertUser("Unable to allocate offscreen window");
            ErrThrow(true);
        }
        WinSetDrawWindow(offScreenWindow);
        WinEraseRectangle(&fullScreen, 0);
        WinSetDrawWindow(displayWindow);
    }
    return offScreenWindow;
}

void
Java_com_sun_kjava_Graphics_copyOffScreenRegion(void)
{
    WinHandle dstWin  = getWindow(popStack());
    WinHandle srcWin  = getWindow(popStack());
    ScrOperation mode = (ScrOperation)popStack();
    SWord dstY        = popStack();
    SWord dstX        = popStack();

    RectangleType src;
    src.extent.y      = popStack();
    src.extent.x      = popStack();
    src.topLeft.y     = popStack();
    src.topLeft.x     = popStack();

    WinCopyRectangle(srcWin, dstWin, &src, dstX, dstY, mode);
}

/*=========================================================================
 * FUNCTION:      drawBitmap(IILcom.sun.kjava.Bitmap;)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Draw a bitmap
 * INTERFACE (operand stack manipulation):
 *   parameters:  left,top - top left corner of bitmap
 *                bitmap - the bitmap to be drawn
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_drawBitmap()
{
    INSTANCE object = (INSTANCE)popStack();
    int y        = popStack();
    int x        = popStack();

    ARRAY bitmapData = (ARRAY)object->data[0].cellp;
    BitmapPtr bitmap = (BitmapPtr)bitmapData->data;
    WinDrawBitmap(bitmap,x,y);
}

/*=========================================================================
 * FUNCTION:      playSound(I)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Play a sound
 * INTERFACE (operand stack manipulation):
 *   parameters:  the code number of the Palm sound to play
 *   returns:     <nothing>
 * NOTE:          This method doesn't really belong in the Graphics 
 *                class, but we don't want to create a new Java class
 *                just for this one method.
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_playSound(void)
{
    SndPlaySystemSound((SndSysBeepType)popStack());
}

/*=========================================================================
 * FUNCTION:      playSoundHz(III)V (STATIC)
 * CLASS:         com.sun.kjava.Graphics
 * TYPE:          static native function
 * OVERVIEW:      Play a sound
 * INTERFACE (operand stack manipulation):
 *   parameters:  Hz, millisecs, volume
 *   returns:     <nothing>
 * NOTE:          This method doesn't really belong in the Graphics 
 *                class, but we don't want to create a new Java class
 *                just for this one method.
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_playSoundHz(void)
{
    SndCommandType cmd;
    cmd.cmd        = sndCmdFreqDurationAmp;
    cmd.param3        = popStack();
    cmd.param2    = popStack();
    cmd.param1        = popStack();
    
    SndDoCmd(0, &cmd, true);
}

/*=========================================================================
 * FUNCTION:      playSMF([B)V (STATIC)
 * CLASS:         com.sun.kjava..Graphics
 * TYPE:          static native function
 * OVERVIEW:      Play a MIDI file
 * INTERFACE (operand stack manipulation):
 *   parameters:  MIDI sequence
 *   returns:     <nothing>
 * NOTE:          This method doesn't really belong in the Graphics 
 *                class, but we don't want to create a new Java class
 *                just for this one method.
 *=======================================================================*/

void
Java_com_sun_kjava_Graphics_playSMF(void)
{
        BYTEARRAY buf = (BYTEARRAY)popStack();
        BytePtr ptr = (BytePtr)&buf->bdata;

        SndPlaySmf(NULL, sndSmfCmdPlay, ptr, NULL, NULL, NULL, false);
}

/*=========================================================================
 * Flash ID function
 *=======================================================================*/

/*======================================================================
 * FUNCTION:     getFlashID()Ljava.lang.String
 * CLASS:        com.sun.kjava.Spotlet
 * TYPE:         static native function
 * OVERVIEW:     Get the Palm Flash ID.
 * INTERFACE (operand stack manipulation):
 *   parameters: <none>
 *   returns:    String containing flashID
 *====================================================================*/

void
Java_com_sun_kjava_Spotlet_getFlashID(void) {
    Word retVal;
    Word bufLen;
    CharPtr bufP;
    Short count;
    char *theID = 0;
    STRING_INSTANCE result;

    retVal = SysGetROMToken(0, sysROMTokenSnum, (BytePtr*) &bufP, &bufLen);

    if ((!retVal) && (bufP) && ((Byte) *bufP != 0xFF)) {
        /* valid serial number; make a string out of it */
        theID = mallocBytes(bufLen+1);
        for (count = 0; count<bufLen; count++) {
            theID[count] = bufP[count];
        }
        theID[bufLen] = 0;
    }

    result = theID ? instantiateString(theID, strlen(theID)) : NULL;
    pushStackAsType(STRING_INSTANCE, result);
}

/*=========================================================================
 * Native functions of class com.sun.kjava.Database
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      create(ILjava/lang/String;IIZ)I
 * CLASS:         com.sun.kjava.Database
 * TYPE:          static native function
 * OVERVIEW:      Open a Palm database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  PalmOS card number, database name, creator (application)
 *                id, database type id, flag telling if this is a
 *                resource database or not.
 *   returns:     true if database could be created, false otherwise
 *=======================================================================*/

void
Java_com_sun_kjava_Database_create(void) {

    int resourceDB = popStack();
    int typeID     = popStack();
    int creatorID  = popStack();
    STRING_INSTANCE name  = (STRING_INSTANCE)popStack();
    int cardNumber = topStack;

    char* rawName = getStringContents(name);
    Err err;

    err = DmCreateDatabase(cardNumber, rawName, creatorID, typeID, resourceDB);

    if (err == 0)
         topStack = TRUE;
    else topStack = FALSE;
}

/*=========================================================================
 * FUNCTION:      open(III)I
 * CLASS:         com.sun.kjava.Database
 * TYPE:          static native function
 * OVERVIEW:      Open a Palm database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  database type id, creator (application) id,
 *                read/write flag (see Database class definition)
 *   returns:     PalmOS database reference number
 *=======================================================================*/

void
Java_com_sun_kjava_Database_open(void) {

    long mode      = popStack();
    long creatorID = popStack();
    long typeID    = topStack;

    long dbRef = (long)DmOpenDatabaseByTypeCreator(typeID, creatorID, mode);

    topStack = dbRef;
}

/*=========================================================================
 * FUNCTION:      close()V
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      Close a Palm database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly)
 *   returns:     <nothing>
 *=======================================================================*/

void
Java_com_sun_kjava_Database_close(void) {

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)popStack();

    /* Read the first instance variable (dbRef) */
    DmOpenRef dbRef = (DmOpenRef)database->data[0].cell;

    /* Close the database */
    if (dbRef) {
        DmCloseDatabase(dbRef);
    }
}

/*=========================================================================
 * FUNCTION:      getNumberOfRecords()I
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      Get the number of records in the database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly)
 *   returns:     number of records
 *=======================================================================*/

void
Java_com_sun_kjava_Database_getNumberOfRecords(void) {

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Read the number of records */
    if (dbRef) {
         topStack = DmNumRecords((DmOpenRef)dbRef);
    } else {
        topStack = 0;
    }
}

/*=========================================================================
 * FUNCTION:      getRecord(I)[B
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      Get a record from the database
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly), record number
 *   returns:     byte array containing the record contents, or
 *                NIL if record could not be found
 * NOTE:          Record numbering starts from 0.
 *=======================================================================*/

void
Java_com_sun_kjava_Database_getRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    VoidHand recHandle = DmGetRecord((DmOpenRef)dbRef, recordNumber);

    /* Skip deleted/non-existing records */
    if (recHandle) {

        /* Get the length of the record */
        int length = MemHandleSize(recHandle);

        /* Instantiate new bytearray */
        ARRAY_CLASS bac = (ARRAY_CLASS)getClass("[B");
        BYTEARRAY byteArray = (BYTEARRAY)instantiateArray(bac, length);

        /* Lock the record to get a real pointer to data */
        CharPtr recPtr = MemHandleLock(recHandle);

        /* Copy data from record to bytearray */
        int i;
        for (i = 0; i < length; i++) {
            byteArray->bdata[i] = recPtr[i];
        }

        /* Unlock the record handle and release the record */
        MemHandleUnlock(recHandle);
        DmReleaseRecord((DmOpenRef)dbRef, recordNumber, false);

        topStack = (cell)byteArray;

    } else {
        topStack = NIL;
    }
}

/*=========================================================================
 * FUNCTION:      setRecord(I[B)Z
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      Set the contents of the given record in the database,
 *                either by overriding a current record, or by
 *                adding a new record to the end of the database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly), record number, byte array
 *                containing the data to be written
 *   returns:     true if operation was successful, false otherwise
 * NOTE:          Record numbering starts from 0.
 *=======================================================================*/

void
Java_com_sun_kjava_Database_setRecord(void) {

    BYTEARRAY byteArray = (BYTEARRAY)popStack();
    unsigned short recordNumber = (unsigned short)popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Read the length of the bytearray parameter */
    int length = byteArray->length;

    Err err;
    VoidHand handle;
    VoidHand oldHandle = NIL;
    CharPtr ptr;
    int numberOfRecords;

    /* Assume that operation may fail */
    topStack = false;

    /* Ensure that database is open and that bytearray */
    /* parameter is of correct type and size */
    if (!dbRef || (byteArray->ofClass != (ARRAY_CLASS)getClass("[B")) || length < 0)
        return;

    /* Also ensure that given record number is within bounds */
    /* or dmMaxRecordIndex */
    numberOfRecords = DmNumRecords((DmOpenRef)dbRef);
    if (recordNumber < 0 ||
        ((recordNumber >= numberOfRecords) && (recordNumber != dmMaxRecordIndex)))
        return;

    /* Allocate a new persistent record */
    handle = DmNewHandle((DmOpenRef)dbRef, length);
    if (!handle) return;

    /* Copy data from bytearray parameter to persistent record */
    ptr = MemHandleLock(handle);
    DmWrite(ptr, 0, byteArray->bdata, length);
    MemHandleUnlock(handle);

    /* Attach the persistent record to the database */
    if (recordNumber == dmMaxRecordIndex) {
        /* Insert the record to the database */
        err = DmAttachRecord((DmOpenRef)dbRef, &recordNumber,
                             (Handle)handle, NIL);
    } else {
        /* Replace an existing record */
        err = DmAttachRecord((DmOpenRef)dbRef, &recordNumber,
                             (Handle)handle, (Handle*)&oldHandle);
    }
    if (err) {
        MemHandleFree(handle);
        return;
    }

    /* Delete the possible record that was previously located  */
    /* in the current location in the database */
    if (oldHandle) MemHandleFree(oldHandle);

    /* If we got here, then operation was successful */
    topStack = true;
}

/*=========================================================================
 * FUNCTION:      moveRecord(II)Z
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      insert the selected record at the to index
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly), from index, to index
 *   returns:     true if deletion was successful, false otherwise
 * NOTE:          Record numbering starts from 0.
 *=======================================================================*/

/* 16 bit linking problems occur when this is included -- wrb */

#ifdef DONOTINCLUDE
void
Java_com_sun_kjava_Database_moveRecord(void) {
    Err err;
    int toIndex = popStack();
    int fromIndex = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Ensure that database is open */
    if (!dbRef) {
        topStack = false;
        return;
    }

    /* Move the record in the database */
    err = DmMoveRecord((DmOpenRef)dbRef, fromIndex, toIndex);
    if (err) {
         topStack = false;
    } else {
        topStack = true;
    }
}
#endif

/*=========================================================================
 * FUNCTION:      deleteRecord(I)Z
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:      Delete (completely remove) a record from the database.
 * INTERFACE (operand stack manipulation):
 *   parameters:  this pointer (implicitly), record number
 *   returns:     true if deletion was successful, false otherwise
 * NOTE:          Record numbering starts from 0.
 *=======================================================================*/

void
Java_com_sun_kjava_Database_deleteRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    Err err;

    /* Assume that operation may fail */
    topStack = false;

    /* Ensure that database is open */
    if (!dbRef) return;

    /* Ensure record exists */
    if (!DmQueryRecord((DmOpenRef)dbRef, recordNumber)) return;

    /* Remove the record from the database */
    err = DmRemoveRecord((DmOpenRef)dbRef, recordNumber);
    topStack = (err) ? false : true;
}


/*=========================================================================
 * FUNCTION:      readRecordToBuffer(int recordNumber, int readOffset,
 *                int length, byte[] buffer, int writeOffset);
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:
 * INTERFACE (operand stack manipulation):
 *   parameters:
 *   returns:
 * NOTE:          Not yet implemented
 *=======================================================================*/

void Java_com_sun_kjava_Database_readRecordToBuffer(void) {
    fatalError("not implemented");
}

/*=========================================================================
 * FUNCTION       int writeRecordFromBuffer(int recordNumber,
 *                      int writeOffset, int length,
 *                      byte[] buffer, int readOffset);
 * CLASS:         com.sun.kjava.Database
 * TYPE:          virtual native function
 * OVERVIEW:
 * INTERFACE (operand stack manipulation):
 * NOTE:          Not yet implemented
 *=======================================================================*/

void Java_com_sun_kjava_Database_writeRecordFromBuffer(void) {
    fatalError("not implemented");
}

extern int stopEventCount; 

void cvtEvent(EventType *event) {
    switch (event->eType) {
        case penDownEvent:
            StoreKVMEvent(penDownKVMEvent, 2, 
                          (cell)event->screenX, (cell)event->screenY);
            break;

        case penUpEvent:
            StoreKVMEvent(penUpKVMEvent, 2, 
                          (cell)event->screenX, (cell)event->screenY);
            break;

        case penMoveEvent:
            StoreKVMEvent(penMoveKVMEvent, 2, 
                          (cell)event->screenX, (cell)event->screenY);
            break;

        case keyDownEvent:
            StoreKVMEvent(keyDownEvent, 1, (cell)event->data.keyDown.chr);
            break;

        case appStopEvent:  
            stopEventCount++;
            StoreKVMEvent(appStopEvent, 0);
            EvtAddEventToQueue(event);
            break;

        case beamEvent:
            StoreKVMEvent(beamReceiveKVMEvent, 1, (cell)PendingBeam);
            PendingBeam = NULL;
            break;
    }
}

/*=========================================================================
 * FUNCTION:      Get the next event
 * TYPE:          event handler
 * OVERVIEW:      Wait for an external event.
 * INTERFACE
 *   parameters:  evt:   a KVMEventType structure into which to stuff the
 *                         the return event.
 *                forever: if TRUE, this function should wait until
 *                         the next native event arrives (and try to 
 *                         conserve battery meanwhile.) If FALSE, the 
 *                         function should wait at most 'waitUntil'
 *                         milliseconds for the next event.
 *                waitUntil: The time (as returned by CurrentTime_md()) at
 *                         which this function should give up waiting for
 *                         events.
 *
 *   returns:     TRUE if an event was found, false otherwise.
 *=======================================================================*/

void
GetAndStoreNextKVMEvent(bool_t forever, ulong64 waitUntil)
{
    Long timeout;
    EventType event;

    if (forever) {
        timeout = evtWaitForever;
    } else {
        if (ll_zero_eq(waitUntil)) {
            /* This test is just an optimization to avoid having to call
             * CurrentTime_md() below */
            timeout = 0;
        } else {
            ulong64 now = CurrentTime_md();
            if (ll_compare_ge(now, waitUntil)) {
                timeout = 0;
            } else {
                ulong64 delay;
                ulong64 delta = waitUntil;
                ll_uint_to_long(delay, 1000000);
                ll_dec(delta, now);
                if (ll_compare_ge(delta, delay)) {
                    /* More than 1000 sec.  Just use 1000 */
                    timeout = 1000L * SysTicksPerSecond();
                } else {
                    ll_long_to_uint(delta, timeout);
                    timeout = timeout * SysTicksPerSecond() / 1000;
                }
            }
        }
    }

    while (eventCount == 0) {
        /* Note: Palm OS will automatically take */
        /* care of power conservation for us here */
        EvtGetEvent(&event, timeout);
        if (!wantPalmSysKeys || event.eType != keyDownEvent) {
            if (SysHandleEvent(&event)) continue;
        }
        switch (event.eType) {
        case penDownEvent:
        case penUpEvent:
        case penMoveEvent:
        case beamEvent:
        case appStopEvent:  
            cvtEvent(&event);
            return;
        case keyDownEvent:
            /*  only keyDown events that aren't processed by the system */
            /*  event handler (e.g. the page up, page down, menu icon and */
            /*  calculator icon keys) or are a result of system event */
            /*  processing (e.g. Graffiti characters) are processed here. */
            if (event.data.keyDown.chr == 0) { 
                continue;
            }
            cvtEvent(&event);
            return ;

        case nilEvent:
            if (!forever) {
                return ;
            }
            break;

        default:
            break;
        }
    }
}

