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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
# ifndef BSD_COMP
#  define BSD_COMP
# endif
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>

#define closesocket close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define HENT_BUF_SIZE 1024
#define NONBLOCKING (!ASYNCHRONOUS_NATIVE_FUNCTIONS)

int netError(void);

#ifdef __linux__
#include <sys/types.h>
typedef __u_long ulong_t;
#endif /* __linux__ */

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

   /*
    * Here is a total mystery to me. If I give solaris a port number of zero
    * then it allocated one somewhere above 50000. The problem is that when I
    * do a recvfrom() on a socket that is bound to a port > 32767 it never sees
    * any data.
    *
    * Temporary solution (seems to work all the time):
    * Start at port 6000 and just go upwards until a free port is found
    */
    if (i == 0) {
        i = 6000;
    }
    res = EADDRINUSE;
    do {
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons((short)i++);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        res = bind(fd, (struct sockaddr*)&addr, sizeof( addr ));
    } while (res == EADDRINUSE && port == 0 && i < 32767);

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
    return 0;
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
    return 0;
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

    struct hostent res;
    char buf[HENT_BUF_SIZE];
    int h_error=0;

#ifdef __GLIBC__
    gethostbyname_r(host, &res, buf, sizeof(buf), &hp, &h_error);
#else
    hp = gethostbyname_r(host, &res, buf, sizeof(buf), &h_error);
#endif /* __GLIBC__ */

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
#if NONBLOCKING

    int flgs = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flgs | O_NONBLOCK);

#endif /* NONBLOCKING */
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
    int res;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((short)port);
    addr.sin_addr.s_addr = ipnumber;

    res = sendto(fd, buffer, length, 0, (struct sockaddr*)&addr, sizeof(addr));

#if NONBLOCKING
    if (res == -1 && errno == EWOULDBLOCK) {
        res = 0;
    }
#endif /* NONBLOCKING */

    if (res == -1 && errno == EINTR) {
        res = -3;
    }

    return res;
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
    int res;

    res = recvfrom(fd, buffer, length, 0, (struct sockaddr*)&addr, &len);

#if NONBLOCKING
    if (res == -1 && errno == EWOULDBLOCK) {
        res = 0;
    }
#endif /* NONBLOCKING */

    if (res == -1 && errno == EINTR) {
        res = -3;
    }

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


