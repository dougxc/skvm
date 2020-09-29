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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: IO Macros
 * FILE:      Pilot_IO.c
 * OVERVIEW:  Redefinition of IO functions for the Palm Pilot which 
 *            doesn't provide the standard means for IO.
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

/*======================================================================
 * Include files
 *====================================================================*/

#include <global.h>
#include <KVM.h>
#include <TimeMgr.h>

/*======================================================================
 * Definitions and declarations
 *====================================================================*/

#if EMULATOR
#define sysTrapHostControl (sysTrapBase + 836)
#include <HostControl.h>

CharPtr hostFilename[2] = {"STDOUT.txt", "STDERR.txt"};
HostFILE* hostFile[2];    /* stdout/stderr files */

#endif /* EMULATOR */

/*  Temporary buffer used to construct the string for fprintf. */
#define TMP_BUFFER_SIZE   512

Boolean streamsOpened;
UInt stdrecOffset[2];    /* offset into stdout/stderr records */
UChar headerLen;
UChar rolloverTagLen;
UChar displayX;
UChar displayY;

/*======================================================================
 * Functions
 *====================================================================*/

/*======================================================================
 * FUNCTION:    testFormatStr
 * DESCRIPTION: Subroutine determining if a format string is supported by
 *              the Pilot's string formatting functions (StrPrintF,
 *              StrVPrintF). Currently, only %ld, %i, %u, %x and %s are
 *              implemented and field length or format
 *              specifications except for the l (long) modifier are not
 *              accepted.
 * PARAMETERS:  the format string to be checked
 * RETURNS:     true if the format string is valid
 * NOTE:        This test will not catch all cases of unsupported format
 *              strings. For example "\\%08x" will not be caught. However,
 *              the most common cases are caught.
 *              The test can be avoided by ensuring that invalid format
 *              strings simply aren't used in the code. However, keep in
 *              mind that StrPrintF and StrVPrintF crash the Pilot
 *              if handed an invalid format string.
 *====================================================================*/

static int testFormatStr(const char* str)
{
    const char *ptr;
    for (ptr = str; ; ptr++) { 
        switch(*ptr) {
            case '\0':
                /* End of the string reached successfully */
                return TRUE;
            case '\\':  
                /* Skip the next character (unless end of string) */
                if (*++ptr == '\0')
                    return FALSE;
                break;
            case '%': {
                char mod = *++ptr;
                if (mod == 'l' || mod == 'L')
                    mod = *++ptr;
                if (mod == '\0'|| !StrChr("diuxsDIUXS", mod))
                    return FALSE;
                break;
            }
        }
    }
}

/*======================================================================
 * FUNCTION:    InitializeStandardIO
 *====================================================================*/

void InitializeStandardIO(void) { 
    streamsOpened = FALSE;
}

/*======================================================================
 * FUNCTION:    FinalizeStandardIO
 *====================================================================*/

void FinalizeStandardIO(void) {

    if (streamsOpened) { 
#if EMULATOR
        int stream;
        for (stream = 0; stream <= 1; stream++) { 
            HostFILE* file = hostFile[stream];
            if ((file != NULL)) {
                HostFClose(file);
            }
        }
#endif /* EMULATOR */
        if (KVMstdoutDB != NULL) {
            DmCloseDatabase(KVMstdoutDB);
        }
        streamsOpened = FALSE;
    }
}

/*======================================================================
 * FUNCTION:    openStandardIOStreams
 *====================================================================*/

static void openStandardIOStreams(void) {

#if EMULATOR
    int version = HostGetHostVersion();
    int id = HostGetHostID();
#endif
    char headerStr[50];
    char *headerPtr = headerStr;
    DateTimeType dateTime;
    UInt stream;

    StrCopy(headerPtr, "KVM stdout: ");
    headerPtr += strlen(headerPtr);

    TimSecondsToDateTime(TimGetSeconds(), &dateTime);
    DateToAscii(dateTime.month, dateTime.day, dateTime.year, dfDMYLong, 
                headerPtr);
    headerPtr += strlen(headerPtr);    

    *headerPtr++ = ',';
    *headerPtr++ = ' ';
    TimeToAscii(dateTime.hour, dateTime.minute, tfColon24h, headerPtr);
    headerPtr += strlen(headerPtr);    

    *headerPtr++ = '\n';
    headerLen = headerPtr - headerStr;
    
    if (KVMprefs.saveOutput) { 
        KVMstdoutDB = DmOpenDatabaseByTypeCreator(KVMstdoutDBType,
                                                  KVMutilCreator, 
                                                  dmModeReadWrite);
        if (KVMstdoutDB == NULL) {
            AlertUser("Cannot open KVM stdout database");
            KVMprefs.saveOutput = false;
        }
    } else { 
        KVMstdoutDB = NULL;
    }

    for (stream = 0; stream <= 1; stream++) {
        if (KVMprefs.saveOutput) { 
            VoidHand recHand = DmGetRecord(KVMstdoutDB, stream);
            CharPtr recPtr = MemHandleLock(recHand);
            DmSet(recPtr, 0, MemHandleSize(recHand), 0); /* set entire rec to null */
            DmWrite(recPtr, 0, headerStr, headerLen);
            MemHandleUnlock(recHand);
            DmReleaseRecord(KVMstdoutDB, stream, true);
            stdrecOffset[stream] = headerLen;
            rolloverTagLen = StrLen(ROLLOVERTAG);
        }
#if EMULATOR
        if ((version != hostPlatformPalmOS) || (id == hostIDPalmOSEmulator)) {
            HostFILE* file = hostFile[stream] = HostFOpen(hostFilename[stream], "w");
            HostFWrite(headerStr, 1, headerLen, file);
            HostFFlush(file);
        }
#endif /* EMULATOR */
        MemMove(&headerStr[7], "err", 3); /* change header text */
    }
    
    if (KVMprefs.displayLines) {
        /* clear the display area of the screen */
        RectangleType rect;
        UChar height = FntCharHeight();
        
        displayX = 160;
        displayY = (KVMprefs.displayLines - 1) * height;
        rect.topLeft.x = 0;
        rect.topLeft.y = 0;
        rect.extent.x  = 160;
        rect.extent.y  = displayY + height;
        WinEraseRectangle(&rect, 0 /*square corner*/);
    }
    streamsOpened = TRUE;
}

/*======================================================================
 * FUNCTION:    putchar for Palm
 *====================================================================*/

void putchar(int c) {
    fprintf(stdout, NULL, c);  /* Special case for "%c" (see fprintf) */
}

/*======================================================================
 * FUNCTION:    fprintf for Palm
 * DESCRIPTION: Implements the standard fprintf functionality with a few
 *              exceptions. Each stream is written to an output buffer
 *              located in static memory. The streams can also be
 *              displayed in a scrolling windows during execution.
 * PARAMETERS:  printf parameters
 * RETURNS:     number of characters output
 *
 * SPECIAL CASE: For some reason, the Palm StrVPrintF does not support
 *              the character format type %c. Because we need to print
 *              chars, this fprintf treats a null formatStr as "%c".
 *====================================================================*/

int fprintf(LOGFILEPTR stream, const char *formatStr, ...) {

#if EMULATOR
    HostFILE* file;
#endif
    va_list  args;      /*  Stores the variable argument list */
    char     tmpBuffer[TMP_BUFFER_SIZE]; /*  Buffer used by StrPrintF & StrVPrintF */
    int      strLen;   /*  Length of string data */

    if ((stream == 0) ||
      (!EMULATOR && !KVMprefs.saveOutput && !KVMprefs.displayLines)) { 
        return 0;
    }

    stream--;   /* adjust index of stdout/stderr from 1/2 to 0/1 */
    
    if (formatStr) {
        /*  Format the new text to the string buffer after ensuring that the 
         *  format string is valid and that there is sufficient space for it. 
         *  Given that we can't know exactly the length of the string resulting 
         *  created by StrVPrintF, we use the length of format string + 20 as 
         *  a rough estimate.    */
        strLen = StrLen(formatStr);
        if (testFormatStr(formatStr)) {
            strLen = strLen + 20;
            if (strLen > TMP_BUFFER_SIZE) { 
                return 0;
            }
            /*  Use the variable argument macros and format the printf string */
            va_start(args, formatStr);
            StrVPrintF(tmpBuffer, formatStr, args);
            va_end(args);
        } else {
            char *errMsg = "<Invalid format string: \"%s\">\n";
            strLen += (StrLen(errMsg) + 1); 
            if (strLen > TMP_BUFFER_SIZE) { 
                return 0;
            }
            StrPrintF(tmpBuffer, errMsg, formatStr);
        }
        strLen = StrLen(tmpBuffer);
        if (strLen == 0) {
            return 0;
        }
    } else {
        /* Special case: if formatStr==NULL, assume it would have been "%c" */
        va_start(args, formatStr);
        tmpBuffer[0] = va_arg(args, int);
        va_end(args);
        tmpBuffer[strLen = 1] = 0;
    }
    
    if (!streamsOpened) { 
        openStandardIOStreams();
    }

    if (KVMprefs.saveOutput) {
        VoidHand recHand = DmGetRecord(KVMstdoutDB, stream);
        UInt chunkSize = MemHandleSize(recHand);
        CharPtr recPtr = MemHandleLock(recHand);
        UInt offset = stdrecOffset[stream];
        UInt freeSpace = chunkSize - offset;
        UInt writeLen = strLen + rolloverTagLen;
        
        MemMove(&tmpBuffer[strLen], ROLLOVERTAG, rolloverTagLen);
        if (freeSpace < writeLen) {
            DmSet(recPtr, offset, freeSpace, 0); /* clear remainder of record */
            offset = headerLen;     /* roll over */
        }
        DmWrite(recPtr, offset, &tmpBuffer, writeLen);
        MemHandleUnlock(recHand);
        DmReleaseRecord(KVMstdoutDB, stream, true);
        stdrecOffset[stream] = offset + strLen;
    }

#if EMULATOR
    if ((file = hostFile[stream])) {
        HostFWrite(tmpBuffer, 1, strLen, file);
        if (strchr(tmpBuffer, '\n') != NULL) { 
            HostFFlush(file);
        }
    }
#endif

    if (KVMprefs.displayLines) {
        Word index = 0;
        do {
            if (tmpBuffer[index] == '\n') {
                displayX = 160;
            } else {
                UChar width = FntCharWidth(tmpBuffer[index]);
                if ((displayX + width) > 160) {
                    /* scroll up */
                    WinHandle displayWindow = WinGetDisplayWindow();
                    UChar height = FntCharHeight();
                    RectangleType rect;
                    
                    rect.topLeft.x = displayX = 0;
                    rect.topLeft.y = height;
                    rect.extent.x  = 160;
                    rect.extent.y  = displayY;
                    WinCopyRectangle(displayWindow, displayWindow,
                                     &rect, 0, 0, scrCopy);
                    rect.topLeft.y = displayY;
                    rect.extent.y  = height;
                    WinEraseRectangle(&rect, 0 /*square corner*/);
                }
                WinDrawChars(&tmpBuffer[index], 1, displayX, displayY);
                displayX += width;
            }
        } while (++index != strLen);
    }    
    return strLen;
}
