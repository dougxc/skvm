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
 * SUBSYSTEM: infrared
 * FILE:      infrared
 * OVERVIEW:  Operations needed for reading and writing class files via
 *            infrared.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Removed beaming functionality from Class Manager UI,
 *            Tasneem Sayeed, February 2000
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <KVM.h>
#include <Crc.h>

/*=========================================================================
 * Infrared beaming routines
 *=======================================================================*/

/*======================================================================
 * FUNCTION:     ReceiveBeamedPacket
 * DESCRIPTION:  Receive a packet of data from another Palm device
 *               via infrared port.
 * PARAMETERS:   exgSocketP: socket pointer received from the
 * RETURNS:      handle to the data area that was allocated
 *               to hold the received data, indirectly: the
 *               real length of the data
 *====================================================================*/

static Handle 
ReceiveBeamedPacket(ExgSocketPtr exgSocketP, int* dataLen) {
    Err     err;
    Handle  dataH;
    BytePtr dataP;
    Word    size;
    Int     len;

    *dataLen = 0;

    /* Read the size of the record to be received */
    if (exgSocketP->length) 
         size = exgSocketP->length;
    else size = 100;

    /* Allocate a buffer, ensuring that there is enough memory */
    /* Note: we cannot allocate the buffer in static memory, */
    /* since it is not possible to write to static memory */
    /* without using DmWrite. */
    dataH = (Handle)MemHandleNew(size);
    if (!dataH) return NIL;

    /* ExgAccept will open a beam progress dialog and  */
    /* wait for the receive commands */
    err = ExgAccept(exgSocketP);
    if (!err) {
        dataP = MemHandleLock((VoidHand)dataH);
        do {
            len = ExgReceive(exgSocketP, &dataP[*dataLen], size-*dataLen, &err);
            if (len && !err) {
                *dataLen += len;
                /* Enlarge data block as necessary */
                if (*dataLen >= size) {
                    MemHandleUnlock((VoidHand)dataH);
                    err = MemHandleResize((VoidHand)dataH, size+100);
                    dataP = MemHandleLock((VoidHand)dataH);
                    if (!err) size += 100;
                }
            }
        } while (len && !err); /* Reading 0 bytes means end of file */
        /* Close transfer dialog */
        ExgDisconnect(exgSocketP, err);

        MemHandleUnlock((VoidHand)dataH);
    }

    /* Check for errors */
    if (err) {
        AlertUser("Data transfer failed");
        MemHandleFree((VoidHand)dataH);
        dataH = NIL;
    }

    return dataH;
}

/*======================================================================
 * FUNCTION:     forwardBeamedDataToSpotlet
 * DESCRIPTION:  Forward the beam-received data packet to
 *               the 'beamReceive' event handler of the current
 *               Spotlet (if any).
 * PARAMETERS:   handle to the data area that contains the
 *               data, length of data (may be different
 *               from the data area length)
 * RETURNS:      0 if successful, error code otherwise
 *====================================================================*/

void *PendingBeam;
bool_t PendingBeamInitialized;

static void 
forwardBeamedDataToSpotlet(Handle dataH, int dataLen) {
    EventType evt;
    CharPtr dataP;
    ARRAY_CLASS bac;
    BYTEARRAY byteArray;
    int i;

    if (!PendingBeamInitialized) { 
        PendingBeam = NULL;
        makeGlobalRoot((cell **)&PendingBeam);
        PendingBeamInitialized = TRUE;
    }

    if (PendingBeam != NULL) { 
        AlertUser("Previous beam not handled yet.  Ignoring.");
        return;
    }

    /* Create a byte array object that can be passed as a  */
    /* parameter to the event handler */
    bac = (ARRAY_CLASS)getClass("[B");
    byteArray = (BYTEARRAY)instantiateArray(bac, dataLen);

    /* Copy data from the infrared buffer to the bytearray */
    /* Lock the record to get a real pointer to data */
    dataP = MemHandleLock((VoidHand)dataH);

    /* Copy data from record to bytearray */
    MemMove(byteArray->bdata, dataP, dataLen);

    /* release memory */
    MemHandleUnlock((VoidHand)dataH);
    MemHandleFree((VoidHand)dataH);

    /**
     * Danger:  We would like to just include the byteArray in the
     * evt item.  But this causes all sorts of GC problems.  For now,
     * we'll just keep the pending beam array in a global.
     */
    PendingBeam = byteArray;
    evt.eType = beamEvent;
    EvtAddEventToQueue(&evt);
}

/*======================================================================
 * FUNCTION:     ReceiveBeamData
 * DESCRIPTION:  Receive a classfile or Java application data from
 *               another Palm device via infrared port.
 * PARAMETERS:   exgSocketP: socket pointer received from the
 *               application launch code sysAppLaunchCmdExgReceiveData,
 * NOTE:         This operation decides dynamically whether the
 *               data should be forwarded to the Java application
 *               that is currently running (if any) or if the
 *               received data contains a classfile that should
 *               be added to the class database.
 *====================================================================*/

void ReceiveBeamData(ExgSocketPtr exgSocketP) {
    Err     err;
    Handle  dataHand;
    int     dataLen = 0;
    CharPtr dataName;
    CharPtr typeInfo;

    /* Read a packet of data via infrared */
    dataHand = ReceiveBeamedPacket(exgSocketP, &dataLen);
    
    /* If reading failed, return immediately */
    if (dataHand == 0 || dataLen == 0) return;

    /* Extract the data name and type information from the socket struct */
    dataName = exgSocketP->name;
    typeInfo = NIL;  /* Not available (PalmOS does not seem to forward */
    /* any other fields than 'name' to receiver) */

    forwardBeamedDataToSpotlet(dataHand, dataLen);
}

/*======================================================================
 * FUNCTION:     beamSend([B)V (STATIC)
 * CLASS:        com.sun.kjava.Spotlet
 * TYPE:         static native function
 * DESCRIPTION:  Send data to another Palm device via
 *               infrared port.
 * INTERFACE (operand stack manipulation):
 *   parameters: byteArray: data to send as a byte array
 *   returns:    true if operation succeeded, false if failed
 *====================================================================*/

void
Java_com_sun_kjava_Spotlet_beamSend(void) {
    ExgSocketType exgSocket;
    Err err = 0;

    /* Read the bytearray parameter */
    BYTEARRAY byteArray = topStackAsType(BYTEARRAY);

    /* Get the size of the bytearray to send */
    ULong size = byteArray->length;

    /* Important to init structure to zeros */
    MemSet(&exgSocket, sizeof(exgSocket), 0);
    exgSocket.description = "Application data";
    exgSocket.name        = "Application data";
    exgSocket.length  = size;
    exgSocket.target  = KVMCreator;

    /* Put data to destination */
    err = ExgPut(&exgSocket);
    if (!err) {
        ExgSend(&exgSocket, &byteArray->bdata, size, &err);
        ExgDisconnect(&exgSocket, err);
    }

    /* Store return value */
    topStack = err ? false : true;
}

