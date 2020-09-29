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
 * FILE:      socketProtocol.c
 * OVERVIEW:  This file provides a default implementation of the native
 *            functions that are needed for supporting the "socket:" and
 *            "serversocket:" Generic Connection protocols.
 * AUTHOR(s): Nik Shaylor, Efren Serra
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
#include <winsock.h>
#else
#include <winsock2.h>
#endif
WSADATA wsaData;
int netError(void);
#define NONBLOCKING 0

/*=========================================================================
 * Protocol methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      setNonBlocking()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

void prim_com_sun_cldc_io_j2me_socket_Protocol_setNonBlocking(int fd)
{
    /* Do nothing this. This is a blocking only version */
}

/*=========================================================================
 * FUNCTION:      getIpNumber()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_getIpNumber(char *host)
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
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_open0(char *name, int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1, ipn, res;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET) {
        goto ioe;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof(int));

    ipn = prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(name);
    if (ipn == -1) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = ipn;

    do {
        res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));

#if INCLUDEDEBUGCODE
        if (tracenetworking) {
            fprintf(stdout, "socket::open0-connect ipn=%lx port=%ld res=%ld ne=%ld\n",
                    (long)ipn, (long)port, (long)res, (long)netError());
        }
#endif /* INCLUDEDEBUGCODE */

    } while ((res < 0) && (errno == WSAEINTR || errno == WSAEALREADY));

    if (res < 0 && errno == WSAEISCONN) {
        res = 0;
    }

    if (res >= 0) {
        goto done;
    }

ioe:
    *exception = "java/io/IOException";
    goto fail;

fail:
    if (fd >= 0) {
        closesocket(fd);
        fd = -1;
    }
    goto done;

done:

    return fd;
}

/*=========================================================================
 * FUNCTION:      read0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Read from a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_read0(int fd, char *p, int len)
{
    return recv(fd, p, len, 0);
}

/*=========================================================================
 * FUNCTION:      available0
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Return the number of bytes available
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_available0(int fd)
{
    int len;
    int res = ioctlsocket(fd, FIONREAD, &len);

    if (res < 0) {
        return 0;
    } else {
        return len;
    }
}

/*=========================================================================
 * FUNCTION:      write0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Write to a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_write0(int fd, char *p, int len)
{
    return send(fd, p, len, 0);
}

/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_close0(int fd)
{
    return closesocket(fd);
}

/*=========================================================================
 * FUNCTION:      open()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a listening TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_open0(int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1, res;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        goto ioe;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof (int));

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(fd, (struct sockaddr*)&addr, sizeof(addr));

    if (res == SOCKET_ERROR) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

    res = listen(fd, 3);
    if (res == SOCKET_ERROR) {
        goto ioe;
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
 * FUNCTION:      accept()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Accept and open a connection
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_accept(int fd)
{
    struct sockaddr sa;
    int saLen = sizeof (sa);
    return accept(fd, &sa, &saLen);
}

/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_close(int fd)
{
    return prim_com_sun_cldc_io_j2me_socket_Protocol_close0(fd);
}

/*=========================================================================
 * FUNCTION:      wsaInit()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Initialize the windows socket system
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void networkInit(void) {
    if (WSAStartup(0x0101, &wsaData ) != 0) {
        fatalError(KVM_MSG_WSASTARTUP_FAILURE);
    }
}

/*=========================================================================
 * FUNCTION:      netError()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      return fatal windows socket error
 * INTERFACE:
 *   parameters:  none
 *   returns:     int
 *=======================================================================*/

int netError(void) {
    return WSAGetLastError();
}


