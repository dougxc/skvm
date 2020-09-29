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
 */

/**
 * This implements the network protocols for Windows
 *
 * @author  Nik Shaylor
 * @version 1.0 1/17/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <async.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#define NOMSG
#include <windows.h>
#include <process.h>
#ifdef GCC
#include <errno.h>
#include <winsock.h>
#else
#include <winsock2.h>
#endif
WSADATA wsaData;
int netError(void);

#ifndef ulong_t
typedef unsigned long ulong_t;
#endif /* ulong_t */

/*=========================================================================
 * Protocol Methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_open0(int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1;
    int res, i;
    int gsnlen = sizeof( addr );

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == INVALID_SOCKET) {
        goto ioe;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof(int));

    i = port;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((short)i++);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(fd, (struct sockaddr*)&addr, sizeof( addr ));

    getsockname(fd, (struct sockaddr*)&addr, &gsnlen);

    if (res == SOCKET_ERROR) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

    goto done;

ioe:
    *exception = "java/io/IOException";
    goto fail;

fail:
    if (fd >= 0) {
        closesocket(fd);
    }
    fd = -1;
    goto done;

done:

    return fd;
}

/*=========================================================================
 * FUNCTION:      getMaximumLength()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram max length
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength(void)
{
    return wsaData.iMaxUdpDg;
}

/*=========================================================================
 * FUNCTION:      getNominalLength()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram nominal length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength(void)
{
    return wsaData.iMaxUdpDg;
}

/*=========================================================================
 * FUNCTION:      getHostByAddr()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate an ip address into a host name
 *=======================================================================*/

void
prim_com_sun_cldc_io_j2me_datagram_Protocol_getHostByAddr(int ipn, char* host)
{
    int h_len;
    ulong_t addr = (ulong_t)ipn;
    struct hostent *hp;

    hp = gethostbyaddr((char *)&addr, sizeof (addr), AF_INET);

    if (hp  == NULL) {
#if INCLUDEDEBUGCODE
        if (tracenetworking) {
            fprintf(stdout,
                    "getHostByAddr ip addr='%lx' host information not found\n",
                    ipn);
        }
#endif /* INCLUDEDEBUGCODE */
    } else {
        h_len = strlen(hp->h_name);
        (void)memmove(host, hp->h_name, h_len < MAX_HOST_LENGTH ? h_len :
                      MAX_HOST_LENGTH);
    }

#if INCLUDEDEBUGCODE
    if (tracenetworking) {
        fprintf(stdout, "getHostByAddr ip addr='%lx' host='%s'\n",
                (long)ipn, host);
    }
#endif /* INCLUDEDEBUGCODE */
}

/*=========================================================================
 * FUNCTION:      getIpNumber()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(char *host)
{
    struct sockaddr_in sin;
    struct sockaddr_in* sa = &sin;
    struct hostent *hp;
    struct in_addr addr;
    int result;

    if ((hp = gethostbyname(host)) == NULL ) {
#if INCLUDEDEBUGCODE
        if (tracenetworking) {
            fprintf(stdout, "getIpNumber host='%s' res=-1\n", host);
        }
#endif /* INCLUDEDEBUGCODE */
        result = -1;
    } else {
        memset( sa, 0, sizeof(struct sockaddr_in) );
        memcpy(&addr, hp->h_addr_list[0], hp->h_length);

#if INCLUDEDEBUGCODE
        if (tracenetworking) {
            fprintf(stdout, "getIpNumber host='%s' res=%lx\n", host,
                    (long)addr.s_addr);
        }
#endif /* INCLUDEDEBUGCODE */

        result = addr.s_addr;
    }

    return result;
}

/*=========================================================================
 * FUNCTION:      setNonBlocking()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

void prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(int fd)
{
    /* Do nothing this. This is a blocking only version */
}

/*=========================================================================
 * FUNCTION:      send0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Send a datagram
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(int fd, int ipnumber,
    int port, char *buffer, int length)
{
    struct sockaddr_in addr;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((short)port);
    addr.sin_addr.s_addr = ipnumber;

    return sendto(fd, buffer, length, 0, (struct sockaddr*)&addr, sizeof(addr));
}

/*=========================================================================
 * FUNCTION:      receive0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Receive a datagram
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(int fd, int *ipnumber,
    int *port, char *buffer, int length)
{
    struct sockaddr_in addr;
    int len = sizeof(struct sockaddr_in);

    int res = recvfrom(fd, buffer, length, 0, (struct sockaddr*)&addr, &len);

    *ipnumber = (long)addr.sin_addr.s_addr;
    *port     = htons(addr.sin_port);

    return res;
}

/*=========================================================================
 * FUNCTION:      close()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_close(int fd)
{
    return closesocket(fd);
}


