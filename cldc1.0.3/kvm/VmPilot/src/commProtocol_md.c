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
 * FILE:      commProtocol_md.c
 * OVERVIEW:  Operations to support serial communication ports on
 *            the Palm OS (native Palm support for the 'comm:' protocol)
 * AUTHOR:    Nik Shaylor
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <KVM.h>
#include <CWcompatibility.h>

/*=======================================================================
 * Definitions and declarations
 *=======================================================================*/

UInt serref = 0;
static SerSettingsType serial_settings;
UInt ticksToWait = -1;
int firstTime = TRUE;
#define POP_STACK popStack
#define POP_STACK_AS_TYPE popStackAsType
#define PUSH_STACK pushStack

void prim_realOpen(INSTANCE instance, char **exception, int baud, 
                   int flags, int ctsTimeout, int receiveTimeout);
int prim_readBytes(INSTANCE instance, char **exception, 
                   char *buffer, int offset, int length);
void prim_writeBytes(INSTANCE instance, char **exception, 
                     char *buffer, int offset, int length);

void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1realOpen(void);
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1close(void);
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1readBytes(void);
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1writeBytes(void);

/*=======================================================================
 * Protocol implementation functions
 *=======================================================================*/

/*
 * prim_readBytes
 */
void prim_realOpen(INSTANCE instance, char **exception, int baud, 
                   int flags, int ctsTimeout, int receiveTimeout) {

    Err errout;
    ULong starttime;
    ULong bytecount;


    if (firstTime) {
        if ((errout = SysLibFind("Serial Library", &serref)) != 0) {
            *exception = "java/io/IOException";
            return;
        }
        firstTime = FALSE;
    }

    serial_settings.baudRate   = baud;
    serial_settings.ctsTimeout = serDefaultCTSTimeout;
    serial_settings.flags =
        serSettingsFlagBitsPerChar8 |
        serSettingsFlagStopBits1 |
        serSettingsFlagCTSAutoM |
        serSettingsFlagRTSAutoM;

    if (flags != -1) {
        serial_settings.flags = flags;
    }

    if (ctsTimeout != -1) {
        serial_settings.ctsTimeout = ctsTimeout;
    }

    if ((errout = SerOpen(serref, 0, serial_settings.baudRate)) != 0 &&
        errout != serErrAlreadyOpen) {
        *exception = "java/io/IOException";
        return; /* perhaps net is connected */
    }

    if ((errout = SerSetSettings(serref, &serial_settings)) != 0) {
        SerClearErr(serref);
        *exception = "java/io/IOException";
        return;
    }

    starttime = TimGetSeconds();
    while (starttime + 2 > TimGetSeconds()){
        /* Just spin */
    }

    if (SerReceiveCheck(serref, &bytecount) != 0) {
        SerClearErr(serref);
    }

    if (bytecount > 0) {
        SerReceiveFlush(serref, 1);
    }

    ticksToWait = receiveTimeout;

    return;
}

/*
 * native void prim_realOpen(int baud, int flags, int ctsTimeout, int receiveTimeout) throws IOException;
 */
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1realOpen(void) {
    long            receiveTimeout = POP_STACK();
    long            ctsTimeout     = POP_STACK();
    long            flags          = POP_STACK();
    long            baud           = POP_STACK();
    INSTANCE        instance       = POP_STACK_AS_TYPE(INSTANCE);
    char           *exception      = NULL;

    prim_realOpen(instance, &exception, baud, flags, ctsTimeout, receiveTimeout);

    if (exception) {
        raiseException(exception);
    }
}

/*
 * native void prim_close() throws IOException;
 */
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1close(void) {
    INSTANCE        instance  = POP_STACK_AS_TYPE(INSTANCE);

    SerClose(serref);
}

/*
 * prim_readBytes
 */
int prim_readBytes(INSTANCE instance, char **exception, char *buffer, int offset, int length) {

    Long bytecount = 0;
    ULong readcount, countread;
    Err error = 0;
    int i;

    for (i = 0; i < 5; i++) {
        if ((error =
            SerReceiveCheck(serref, (ULong *)&bytecount)) != 0) {
            SerClearErr(serref);
        } else {
            break;
        }
    }

    if (bytecount <= 0)
        return 0;

    length = SerReceive(serref, buffer+offset, length, ticksToWait, &error);

#if INCLUDEDEBUGCODE
    if (tracenetworking) {
        fprintf(stdout, "Read bytes from comm port (res = %d error = %d)\n", 
                length, error);
    }
#endif /* INCLUDEDEBUGCODE */

    if (error){
        SerClearErr(serref);
        *exception = "java/io/IOException";
    }

    return length;
}

/*
 * native int prim_readBytes(byte b[], int off, int len) throws IOException;
 */
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1readBytes(void) {
    long            length = POP_STACK();
    long            offset = POP_STACK();
    BYTEARRAY       buffer = POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE        instance = POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    int v = prim_readBytes(instance, &exception, (char*)(buffer->bdata),
                           offset, length);

    if (exception) {
        raiseException(exception);
    }
    PUSH_STACK(v);
}

/*
 * prim_writeBytes
 */
void prim_writeBytes(INSTANCE instance, char **exception, char *buffer, int offset, int length) {

    Err error = 0;

    SerSend(serref, buffer+offset, length, &error);
    if (error) {
        *exception = "java/io/IOException";
    } else {
        SerSendWait(serref, -1);
    }

#if INCLUDEDEBUGCODE
    if (tracenetworking) {
        fprintf(stdout, 
                "Wrote bytes to comm port (res = %d error = %d)\n",
                length, error);
    }
#endif /* INCLUDEDEBUGCODE */

}

/*
 * native int prim_writeBytes(byte b[], int off, int len) throws IOException;
 */
void Java_com_sun_cldc_io_palm_comm_Protocol_prim_1writeBytes(void) {
    long            length = POP_STACK();
    long            offset = POP_STACK();
    BYTEARRAY       buffer = POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE        instance = POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    prim_writeBytes(instance, &exception, (char*)(buffer->bdata), 
                    offset, length);

    if (exception) {
        raiseException(exception);
    }
}
