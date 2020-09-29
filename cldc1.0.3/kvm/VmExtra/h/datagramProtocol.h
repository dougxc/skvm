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
 * SYSTEM:    KVM
 * SUBSYSTEM: networking (Generic Connection framework)
 * FILE:      datagramProtocol.h
 * OVERVIEW:  Prototypes for supporting using datagrams on the
 *            Generic Connection framework.  If a target platform
 *            wants to support these networking
 *            capabilities, a platform-specific implementation of the 
 *            functions below must be provided in the Vm<Port>/src directory,
 *            where <Port> is the name of your target platform
 *            (e.g., Win, Unix, Pilot).  A default implementation is
 *            provided in VmExtra/src.
 * AUTHOR:    Nik Shaylor
 *=======================================================================*/

#ifndef _DATAGRAM_PROTOCOL_H_
#define _DATAGRAM_PROTOCOL_H_

/*=========================================================================
 * Function prototypes for the native networking operations
 *=======================================================================*/

#ifndef __P
# ifdef __STDC__
#  define __P(p) p
# else
#  define __P(p) ()
# endif
#endif /* __P */

#ifndef MAX_HOST_LENGTH
# define MAX_HOST_LENGTH 256
#endif /* MAX_HOST_LENGTH */

extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_open0 __P ((int, char**));
extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_close __P ((int));

extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_send0 __P ((int, int, int, char*, int));
extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0 __P ((int, int*, int*, char*, int));

extern void prim_com_sun_cldc_io_j2me_datagram_Protocol_getHostByAddr __P ((int, char*));
extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber __P ((char*));
extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength __P ((void));
extern int prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength __P ((void));

extern void prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking __P ((int));

#endif /* _DATAGRAM_PROTOCOL_H_ */


