/*
 * Copyright (c) 2000-2001 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: networking
 * FILE:      commProtocol.c
 * OVERVIEW:  Operations to support serial communication ports
 *            (native implementation of the 'comm:' protocol).
 *
 *            These functions are mapped onto platform-specific
 *            functions defined in Vm<Port>/src directories,
 *            where <Port> indicates the name of the target
 *            platform (e.g., Win, Unix, Pilot).
 * AUTHOR:    Stephen Flores, Nik Shaylor
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <commProtocol.h>

/*=======================================================================
 * Forward declarations
 *=======================================================================*/

void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1openByNumber();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1openByName();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1open();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1close();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1readBytes();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1available();
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1writeBytes();

/*=======================================================================
 * Protocol implementation functions
 *=======================================================================*/

/*
 * static native int native_open(int port, int baud, int flags,
 *     int blocking) throws IOException;
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1openByNumber() {
    long  blocking   = popStack();
    long  flags      = popStack();
    long  baud       = popStack();
    long  port       = popStack();
    long  hPort      = 0;
    char* pszError   = NULL;

    hPort = openPortByNumber(&pszError, port, baud, flags, blocking);
    if (pszError != NULL) {
        freePortError(pszError);
        pushStack(0);
        raiseExceptionCharMsg("java/io/IOException", pszError);
    }

    pushStack(hPort);
}

/*
 * static native int native_open(int port, int baud, int flags,
 *     int blocking) throws IOException;
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1openByName() {
    long            blocking      = popStack();
    long            flags         = popStack();
    long            baud          = popStack();
    STRING_INSTANCE pName = popStackAsType(STRING_INSTANCE);
    char            szName[80];
    long            hPort = 0;
    unsigned int    i;
    char*           pszError = NULL;

    if (pName->length >= sizeof (szName)) {
        pushStack(0);
        raiseExceptionCharMsg("java/io/IOException",
                                 KVM_MSG_PROTOCOL_NAME_TOO_LONG);
    }

    /* device names are in ASCII */
    for (i = 0; i < pName->length; i++) {
        szName[i] = (char)pName->array->sdata[i + pName->offset];
    }

    szName[pName->length] = 0;
    hPort = openPortByName(&pszError, szName, baud, flags, blocking);
    if (pszError != NULL) {
        freePortError(pszError);
        pushStack(0);
        raiseExceptionCharMsg("java/io/IOException", pszError);
    }

    pushStack(hPort);
}

/*
 * native void native_close(int hPort) throws IOException;
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1close() {
    long hPort = popStack();

    closePort(hPort);
}

/*
 * native int native_readBytes(int hPort, byte b[], int off, int len)
 *     throws IOException;
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1readBytes() {
    long      length = popStack();
    long      offset = popStack();
    BYTEARRAY buffer = popStackAsType(BYTEARRAY);
    long      hPort  = popStack();
    long      bytesRead = 0;
    char*     pszError = NULL;

    bytesRead = readFromPort(&pszError, hPort,
                             (char*)((buffer->bdata) + offset), length);
    if (pszError != NULL) {
        freePortError(pszError);
        pushStack(0);
        raiseExceptionCharMsg("java/io/IOException", pszError);
    }

    pushStack(bytesRead);
}

/*
 * static native int native_available(int hPort) throws IOException;
 *
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1available()
{
    long hPort = popStack();
    long n = 0;

    n = peekFromPort(hPort);

    pushStack(n);
}

/*
 * native void native_writeBytes(int hPort, byte b[], int off, int len)
 *     throws IOException;
 */
void Java_com_sun_cldc_io_j2me_comm_Protocol_native_1writeBytes() {
    long      length = popStack();
    long      offset = popStack();
    BYTEARRAY buffer = popStackAsType(BYTEARRAY);
    long      hPort  = popStack();
    char*     pszError = NULL;
    int       bytesWritten = 0;

    bytesWritten = writeToPort(&pszError, hPort,
                               (char*)((buffer->bdata) + offset), length);
    if (pszError != NULL) {
        freePortError(pszError);
        raiseExceptionCharMsg("java/io/IOException", pszError);
    }

    if (bytesWritten != length) {
        raiseExceptionCharMsg("java/io/IOException", 
                                 KVM_MSG_COMM_WRITE_INCOMPLETE);
    }
}

