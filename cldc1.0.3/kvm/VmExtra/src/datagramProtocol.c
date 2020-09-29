/*
 *  Copyright (c) 1999-2001 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*=========================================================================
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: networking (Generic Connection framework)
 * FILE:      datagramProtocol.c
 * OVERVIEW:  This file provides a default implementation of the native
 *            functions that are needed for supporting the "datagram:"
 *            Generic Connection protocols.
 * AUTHOR:    Nik Shaylor
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <async.h>

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

#if INCLUDEDEBUGCODE
#define NDEBUG0(fmt)                         if (tracenetworking) { fprintf(stdout, fmt);                         }
#define NDEBUG1(fmt, p1)                     if (tracenetworking) { fprintf(stdout, fmt, p1);                     }
#define NDEBUG2(fmt, p1, p2)                 if (tracenetworking) { fprintf(stdout, fmt, p1, p2);                 }
#define NDEBUG3(fmt, p1, p2, p3)             if (tracenetworking) { fprintf(stdout, fmt, p1, p2, p3);             }
#define NDEBUG4(fmt, p1, p2, p3, p4)         if (tracenetworking) { fprintf(stdout, fmt, p1, p2, p3, p4);         }
#define NDEBUG5(fmt, p1, p2, p3, p4, p5)     if (tracenetworking) { fprintf(stdout, fmt, p1, p2, p3, p4, p5);     }
#define NDEBUG6(fmt, p1, p2, p3, p4, p5, p6) if (tracenetworking) { fprintf(stdout, fmt, p1, p2, p3, p4, p5, p6); }
#else
#define NDEBUG0(fmt)                         /**/
#define NDEBUG1(fmt, p1)                     /**/
#define NDEBUG2(fmt, p1, p2)                 /**/
#define NDEBUG3(fmt, p1, p2, p3)             /**/
#define NDEBUG4(fmt, p1, p2, p3, p4)         /**/
#define NDEBUG5(fmt, p1, p2, p3, p4, p5)     /**/
#define NDEBUG6(fmt, p1, p2, p3, p4, p5, p6) /**/
#endif

/*=========================================================================
 * Protocol methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      setSocketHandle()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Set the contents of the handle field
 * INTERFACE:
 *   parameters:  instance, value
 *   returns:     <nothing>
 *=======================================================================*/

static void setSocketHandle(ASYNCIOCB *aiocb, long value)
{
    aiocb->instance->data[0].cell = value; /* 'handle' must be slot 0 */

    NDEBUG1("setSocketHandle handle=%ld\n",
            (long)aiocb->instance->data[0].cell);
}

/*=========================================================================
 * function:      getSocketHandle()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Get the contents of the handle field
 * INTERFACE:
 *   parameters:  instance
 *   returns:     value
 *=======================================================================*/

static int getSocketHandle(ASYNCIOCB *aiocb)
{
    NDEBUG1("getSocketHandle handle=%ld\n",
            (long)aiocb->instance->data[0].cell);

    return aiocb->instance->data[0].cell; /* 'handle' must be slot 0 */
}

/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, mode, append, port
 *   returns:     none
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_open0)
{
    long            port      = ASYNC_popStack();
    long            append    = ASYNC_popStack();
    long            mode      = ASYNC_popStack();
    INSTANCE        instance  = ASYNC_popStackAsType(INSTANCE);
    char           *exception = NULL;
    int fd;

    (void)mode;
    (void)append;

    aiocb->instance = instance; /* Save instance in ASYNCIOCB */

    NDEBUG1("datagram::open0 p=%ld\n", port);

    ASYNC_enableGarbageCollection();
    fd = prim_com_sun_cldc_io_j2me_datagram_Protocol_open0(port, &exception);
    ASYNC_disableGarbageCollection();

    NDEBUG4("datagram::open0 p=%ld fd=%ld exception='%s' ne=%ld\n",
             port, (long)fd, (exception == NULL) ? "null" : exception,
            (long)netError());

    if (exception == NULL) {
        /* For non-blocking systems only */
        prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(fd);
        goto done;
    }

    ASYNC_raiseException(exception);
    goto fail;
fail:
    fd = -1;
done:
    setSocketHandle(aiocb, fd);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      getMaximumLength0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram max length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength0)
{
    INSTANCE instance = ASYNC_popStackAsType(INSTANCE);
    int len;
    (void)instance;

    ASYNC_enableGarbageCollection();
    len = prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength();
    ASYNC_disableGarbageCollection();

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    if(len > ASYNC_BUFFER_SIZE) {
        len = ASYNC_BUFFER_SIZE;
    }
#endif

    NDEBUG1("datagram::getMaximumLength len=%ld\n", (long)len);

    ASYNC_pushStack(len);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      getNominalLength0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram nominal length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength0)
{
    INSTANCE instance = ASYNC_popStackAsType(INSTANCE);
    int len;
    (void)instance;

    ASYNC_enableGarbageCollection();
    len = prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength();
    ASYNC_disableGarbageCollection();

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    if(len > ASYNC_BUFFER_SIZE) {
        len = ASYNC_BUFFER_SIZE;
    }
#endif

    NDEBUG1("datagram::getNominalLength len=%ld\n", (long)len);

    ASYNC_pushStack(len);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      getHostByAddr()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translates an ip address into a host name
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, int, byte[]
 *   returns:     void
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_getHostByAddr)
{
    BYTEARRAY barray = ASYNC_popStackAsType(BYTEARRAY);
    int ipn = ASYNC_popStack();
    INSTANCE instance  = ASYNC_popStackAsType(INSTANCE);
    char* buf = (char*)&barray->bdata[0];

    (void)instance;

    ASYNC_enableGarbageCollection();

    prim_com_sun_cldc_io_j2me_datagram_Protocol_getHostByAddr(ipn, buf);
    NDEBUG2("datagram::getHostByAddr ipn='%lx' host='%s'\n",
            (long)ipn, buf);

    ASYNC_disableGarbageCollection();
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      getIpNumber()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, string
 *   returns:     int
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber)
{
    STRING_INSTANCE string    = ASYNC_popStackAsType(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_popStackAsType(INSTANCE);
    char            name[MAX_HOST_LENGTH];
    int ipn;
    long stringlen;

    (void)instance;

    ASYNC_enableGarbageCollection();

    stringlen = string->length + 1;
    (void)getStringContentsSafely(string, name, stringlen);

    ipn = prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(name);
    NDEBUG2("datagram::getIpNumber name='%s' ipn=%lx\n", name,
            (long)ipn);

    ASYNC_disableGarbageCollection();

    ASYNC_pushStack(ipn);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      send0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Send a datagram
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, iaddr, port, buffer, offset, length
 *   returns:     length
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_send0)
{
    long            length   = ASYNC_popStack();
    long            offset   = ASYNC_popStack();
    BYTEARRAY       array    = ASYNC_popStackAsType(BYTEARRAY);
    long            port     = ASYNC_popStack();
    long            ipnumber = ASYNC_popStack();
    INSTANCE        instance = ASYNC_popStackAsType(INSTANCE);

    long fd;
    int  res;
    aiocb->instance = instance;
    fd = getSocketHandle(aiocb);

    NDEBUG6("datagram::send0 b=%lx o=%ld l=%ld p=%ld ip=%lx fd=%ld\n",
            (long)array, offset, length, port, ipnumber, fd);

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    {
        char buffer[ASYNC_BUFFER_SIZE];
       /*
        * If necessary reduce the length to that of the buffer. The
        * Java code will call back with more I/O operations if needed.
        */
        if(length > ASYNC_BUFFER_SIZE) {
            ASYNC_raiseException("java/io/IllegalArgumentException");
        } else {
            memcpy(buffer, (char *)array->bdata + offset, length);

            ASYNC_enableGarbageCollection();
            res = prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(fd,
                                      ipnumber, port, buffer, length);
            ASYNC_disableGarbageCollection();
        }
    }
#else
    res = prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(fd,
           ipnumber, port, (char*)(array->bdata)+offset, length);
#endif /* ASYNCHRONOUS_NATIVE_FUNCTIONS */

    NDEBUG2("datagram::send0 res=%ld ne=%ld\n",
             (long)res, (long)netError());

    if (res < 0) {
        if (res == -3) {
            ASYNC_raiseException("java/io/InterruptedIOException");
        } else {
            ASYNC_raiseException("java/io/IOException");
        }
    }

    ASYNC_pushStack(res);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      receive0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Receive a datagram
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, buffer, offset, length
 *   returns:     none
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_receive0)
{
    long            length   = ASYNC_popStack();
    long            offset   = ASYNC_popStack();
    BYTEARRAY       array    = ASYNC_popStackAsType(BYTEARRAY);
    INSTANCE        instance = ASYNC_popStackAsType(INSTANCE);

    int ipnumber, port, res;
    ulong64 lres;
    long fd;

    aiocb->instance = instance;
    fd = getSocketHandle(aiocb);

    NDEBUG4("datagram::receive0 b=%lx o=%ld l=%ld fd=%ld\n",
            (long)array, offset, length, fd);

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    {
        char buffer[ASYNC_BUFFER_SIZE];
       /*
        * If necessary reduce the length to that of the buffer. The
        * Java code will call back with more I/O operations if needed.
        */
        if (length > ASYNC_BUFFER_SIZE) {
            ASYNC_raiseException("java/io/IllegalArgumentException");
        } else {
            aiocb->array = array;

            ASYNC_enableGarbageCollection();
            res = prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(fd,
                                         &ipnumber, &port, buffer, length);
            ASYNC_disableGarbageCollection();

            if (res > 0) {
                memcpy((char *)aiocb->array->bdata + offset, buffer, length);
            }
        }
    }
#else
    res = prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(fd,
          &ipnumber, &port, (char *)(array->bdata)+offset, length);
#endif /* ASYNCHRONOUS_NATIVE_FUNCTIONS */

    NDEBUG4("datagram::receive0 res=%ld ip=%lx p=%ld ne=%ld\n",
            (long)res, (long)ipnumber, (long)port, (long)netError());

    if (res < 0) {
        if (res == -3) {
            ASYNC_raiseException("java/io/InterruptedIOException");
        } else {
            ASYNC_raiseException("java/io/IOException");
        }
    }

    if (res > 0xFFFF) {
        fatalError(KVM_MSG_CANNOT_RECEIVE_DATAGRAMS_LONGER_THAN_65535);
    }

#if COMPILER_SUPPORTS_LONG
    lres = (((ulong64)ipnumber) << 32) + (unsigned)((port&0xFFFF) << 16) + (res&0xFFFF);
#else
    lres.high = ipnumber;
    lres.low = (unsigned)((port&0xFFFF) << 16) + (res&0xFFFF);
#endif /* COMPILER_SUPPORTS_LONG */

    ASYNC_pushLong(lres);
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *=======================================================================*/

ASYNC_FUNCTION_START(Java_com_sun_cldc_io_j2me_datagram_Protocol_close0)
{
    INSTANCE instance = ASYNC_popStackAsType(INSTANCE);
    long fd;
    int  res = 0;
    aiocb->instance = instance;
    fd = getSocketHandle(aiocb);

    if (fd != -1L) {
        setSocketHandle(aiocb, -1);
        ASYNC_enableGarbageCollection();
        res = prim_com_sun_cldc_io_j2me_datagram_Protocol_close(fd);
        ASYNC_disableGarbageCollection();
    }

    NDEBUG3("datagram::close res=%ld fd=%ld ne=%ld\n",
            (long)res, (long)fd, (long)netError());

    if (res < 0) {
        ASYNC_raiseException("java/io/IOException");
    }
}
ASYNC_FUNCTION_END

/*=========================================================================
 * FUNCTION:      registerCleanup
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *=======================================================================*/

static void
datagramCleanup(INSTANCE instance)
{ 
    long fd = instance->data[0].cell;
    if (fd != -1L) {
        prim_com_sun_cldc_io_j2me_datagram_Protocol_close(fd);
    }
}

void Java_com_sun_cldc_io_j2me_datagram_Protocol_registerCleanup() 
{
    START_TEMPORARY_ROOTS
        DECLARE_TEMPORARY_ROOT(INSTANCE, instance, popStackAsType(INSTANCE));
        registerCleanup(&instance, datagramCleanup);
    END_TEMPORARY_ROOTS
}


