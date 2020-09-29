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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Native function interface
 * FILE:      inetlib.c
 * OVERVIEW:  This file defines the native functions needed
 *            by the Java virtual machine. The implementation
 *            is _not_ based on JNI (Java Native Interface),
 *            because it seems too complicated for small devices.
 * AUTHOR:    Bill Pittore, Sun Labs 12/99
 * NOTE:      Many functions in this file are platform-specific,
 *            and require extra work when porting the system to
 *            run on another machine.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <nativeSpotlet.h>
#include <INetMgr.h>
#include <CTP.h>
#include <KVM.h>

/*=======================================================================
 * Globals
 *=======================================================================*/

static Word AppINetRefnum;
static Word AppINetTimeout = 2000;
static Handle AppInetH;
static Err lastErr;

/*=======================================================================
 * Declarations
 *=======================================================================*/

void Java_com_sun_cldc_io_palm_http_Inet_initProto(void);
void Java_com_sun_cldc_io_palm_http_Inet_showSignal(void);
void Java_com_sun_cldc_io_palm_http_Inet_openHTTPsocket(void);
void Java_com_sun_cldc_io_palm_http_Inet_HTTPCreateAndSendReq(void);
void Java_com_sun_cldc_io_palm_http_Inet_readHTTPsocket(void);
void Java_com_sun_cldc_io_palm_http_Inet_closeHTTPsocket(void);
void Java_com_sun_cldc_io_palm_http_Inet_getlasterror(void);
void Java_com_sun_cldc_io_palm_http_Inet_select(void);
void FinalizeINetLib(void);

void cvtEvent(EventType *);

Boolean INetLibWiCmd2(Word refNum, Word cmd, Int arg1, Int arg2)
    SYS_TRAP(inetLibTrapWiCmd);

#define firstINetLibEvent 4096

/*=======================================================================
 * Functions
 *=======================================================================*/

static int
smallEventLoop(void)
{
    INetEventType event;
    INetLibGetEvent(AppINetRefnum, AppInetH,
                   (INetEventType*)&event, SysTicksPerSecond() / 2);

    switch(event.eType) {
        case inetSockStatusChangeEvent:
            if (event.data.inetSockStatusChange.sockErr) {
                lastErr = event.data.inetSockStatusChange.sockErr;
                return -1;
            }
#if 0
            switch (event.data.inetSockStatusChange.status) {
                case inetStatusNew:
                        break;
                case inetStatusResolvingName:
                        break;
                case inetStatusNameResolved:
                        break;
                case inetStatusConnecting:
                        break;
                case inetStatusConnected:
                        break;
                case inetStatusSendingRequest:
                        break;
                case inetStatusWaitingForResponse:
                        break;
                case inetStatusReceivingResponse:
                        break;
                case inetStatusResponseReceived:
                        break;
                case inetStatusClosingConnection:
                        break;
                case inetStatusClosed:
                        break;
                case inetStatusAcquiringNetwork:
                        break;

                default:
                        break;
                }
#endif /* #if 0 */

                        return 0;

        case inetSockReadyEvent:
            return 1;

        case appStopEvent:
            /* Someone else needs to do the right thing */
            EvtAddEventToQueue((EventType*)&event);
            return -1;

        case nilEvent:
            return 0;

        default:
            cvtEvent((EventType*)&event);
            return 0;
    }
}

/*=========================================================================
 * FUNCTION:      initProto(V)I
 * CLASS:         com/sun/kjava/system/palm/protocol/http/Inet
 * TYPE:          native function
 * OVERVIEW:
 * INTERFACE (operand stack manipulation):
 *   parameters:  this (implicitly)
 *   returns:     inet handle ; -1 error
 *=======================================================================*/

void Java_com_sun_cldc_io_palm_http_Inet_initProto()
{
    Err err;
    Handle inetH;
    Word index;
    DWord conv = ctpConvNone;       /* no compression */
    UInt32 value;

    lastErr = 0;

    err = SysLibFind("INet.lib", &AppINetRefnum);
    if (err) {
        lastErr = err;
        AppINetRefnum = 0;
        AppInetH = 0;
        pushStack(-1);
        return;
    }

    err = INetLibConfigIndexFromName(AppINetRefnum,
              (INetConfigNamePtr)inetCfgNameCTPDefault, &index);
    if (err) {
        lastErr = err;
        AppINetRefnum = 0;
        AppInetH = 0;
        pushStack(-2);
        return;
    }

    if (AppInetH == 0) {
        err = INetLibOpen(AppINetRefnum, index, 0, NULL, 0, &inetH);
        if (err) {
            lastErr = err;
            AppINetRefnum = 0;
            pushStack(-3);
            return;
        }

        err = INetLibSettingSet(AppINetRefnum, inetH,
                  inetSettingConvAlgorithm,
                  &conv, sizeof(conv));
        if (err) {
            lastErr = err;
            INetLibClose(AppINetRefnum, inetH);
            AppINetRefnum = 0;
            pushStack(-4);
            return;
        }

        /* Set the max download size */
        value = 65500;
        err = INetLibSettingSet(AppINetRefnum, inetH, 
        inetSettingMaxRspSize, (BytePtr)&value, sizeof(value));
        if (err) {
            lastErr = err;
            INetLibClose(AppINetRefnum, inetH);
            AppINetRefnum = 0;
            pushStack(-5);
            return;
        }
        AppInetH = inetH;       /* Keep around for finalize */
    }
    pushStack(int)(AppInetH);
}

void Java_com_sun_cldc_io_palm_http_Inet_showSignal()
{
    int y = popStack();
    int x = popStack();

    /* Show signal strength meter */
    /* INetLibWiCmd (AppINetRefnum, wiCmdInit, 0, 0); */
    /* INetLibWiCmd (AppINetRefnum, wiCmdSetLocation, x, y); */
    /* INetLibWiCmd (AppINetRefnum, wiCmdSetEnabled, 1, 0); */

    if (AppINetRefnum == 0) {
        raiseException("com/sun/cldc/io/palm/http/InetException");
        return;
    }
    INetLibWiCmd2 (AppINetRefnum, 0, 1, 0);
    INetLibWiCmd2 (AppINetRefnum, 5, x, y);
    INetLibWiCmd2 (AppINetRefnum, 2, 1, 0);
    oneLess;
    return;
}

void Java_com_sun_cldc_io_palm_http_Inet_openHTTPsocket()
{
    Err err;
    Handle sockH;
    Handle inetH = (Handle)popStack();
    int scheme = popStack();

    lastErr = 0;

    err = INetLibSockOpen(AppINetRefnum, inetH, scheme, &sockH);
    if (err) {
        lastErr = err;
        topStack = -1;
        return;
    }
    topStack = (int)sockH;
}

void Java_com_sun_cldc_io_palm_http_Inet_HTTPCreateAndSendReq()
{
    INSTANCE referer = (INSTANCE)popStack();
    INSTANCE resource = (INSTANCE)popStack();
    BYTEARRAY data = (BYTEARRAY)popStack();  /* Data if POST */
    BytePtr ptr = NULL;
    INSTANCE verb = (INSTANCE)popStack();    /* probably 'GET' */
    unsigned char* resstr;
    unsigned char verbstr[256];
    Handle sockH = (Handle)popStack();
    Err err;

    int length;
    lastErr = 0;

    oneLess;
    START_TEMPORARY_ROOTS
    if (resource != NIL) {
        DECLARE_TEMPORARY_ROOT(STRING_INSTANCE, _resource, (STRING_INSTANCE)resource);

        long buflen = _resource->length + 1;
        DECLARE_TEMPORARY_ROOT(unsigned char *, buf,
                               (unsigned char *)mallocHeapObject((buflen+CELL-1)>>log2CELL,
                                                        GCT_NOPOINTERS));
        if (buf == NULL) {
            raiseException("java/lang/OutOfMemoryError");
            goto done;
        }

        resstr = (unsigned char *)getStringContentsSafely(_resource,
                                                          (char *)buf, buflen);
    } else {
        int resstrlen = 10 + 1;
        DECLARE_TEMPORARY_ROOT(unsigned char*, resstr,
                               (unsigned char*)mallocBytes(resstrlen));
        StrCopy((char *)resstr, "index.html");
    }

    if (verb != NIL) {
        getStringContentsSafely((STRING_INSTANCE)verb, (char *)verbstr, 256);
    } else {
        StrCopy((char *)verbstr, "GET");
    }

    err = INetLibSockHTTPReqCreate(AppINetRefnum, sockH, verbstr, resstr, NULL);
    if (err) {
        lastErr = err;
        raiseException("com/sun/cldc/io/palm/http/InetException");
        return;
    }

    if (data != NIL) {
        ptr = (unsigned char *)&data->bdata[0];  /* POST Data */
        length = data->length;
    } else {
        ptr = 0;
        length = 0;
    }

    err = INetLibSockHTTPReqSend(AppINetRefnum, sockH, ptr,
                                 length, AppINetTimeout);
    if (err) {
        lastErr = err;
        raiseException("com/sun/cldc/io/palm/http/InetException");
        return;
    }
done:
    END_TEMPORARY_ROOTS
}

void Java_com_sun_cldc_io_palm_http_Inet_readHTTPsocket()
{
    DWord count =  (DWord)popStack();
    Word offset =  popStack();
    BYTEARRAY buf = (BYTEARRAY)popStack();
    Handle sockH = (Handle)popStack();
    BytePtr ptr = (BytePtr)&buf->bdata[offset];
    DWord nleft = count;
    DWord nread;
    Err err;
    DWord total = 0;

    lastErr = 0;

    do {
        err = INetLibSockRead(AppINetRefnum, sockH, ptr, nleft, &nread,
                              AppINetTimeout);
        if (err) {
            lastErr = err;
            topStack = -2;
            return;
        }

        /* just return what we've read so far (may be zero, that's ok) */
        /* if we got zero back then the connection is closed so return */
        /* -1 so EOF gets set */
        if (nread == 0) {
            if (total > 0)
                topStack = total;
            else
                topStack = -1;

            return;
        }

        total += nread;
        nleft -= nread;
        ptr   += nread;
    } while (nleft);

    topStack = total;
}

void Java_com_sun_cldc_io_palm_http_Inet_closeHTTPsocket()
{
    Handle sockH = (Handle)popStack();
    Err err;

    lastErr = 0;

    oneLess;
    err = INetLibSockClose(AppINetRefnum, sockH);
    if (err) {
        lastErr = err;
        raiseException("com/sun/cldc/io/palm/http/InetException");
        return;
    }
}

void Java_com_sun_cldc_io_palm_http_Inet_getlasterror()
{
    pushStack(lastErr);
}

void Java_com_sun_cldc_io_palm_http_Inet_select()
{
    pushStack(smallEventLoop());
}

void FinalizeINetLib()
{
    lastErr = 0;

    if (AppINetRefnum) {
        lastErr = INetLibClose(AppINetRefnum, AppInetH);
        AppInetH = 0;
        AppINetRefnum = 0;
    }
}


