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
 * FILE:      commProtocol.h
 * OVERVIEW:  Prototypes for supporting serial communication ports.
 *            If a target platform wants to support serial ports,
 *            a platform-specific implementation of the functions
 *            must be provided in the Vm<Port>/src directory,
 *            where <Port> is the name of your target platform
 *            (e.g., Win, Unix, Pilot).
 * AUTHOR:    Stephen Flores, Nik Shaylor
 *=======================================================================*/

#ifndef _COMM_PROTOCOL_H_
#define _COMM_PROTOCOL_H_

#ifndef __P
# ifdef __STDC__
#  define __P(p) p
# else
#  define __P(p) ()
# endif
#endif /* __P */

/* COMM options */
#define STOP_BITS_2     0x01
#define ODD_PARITY      0x02
#define EVEN_PARITY     0x04
#define AUTO_RTS        0x10
#define AUTO_CTS        0x20
#define BITS_PER_CHAR_7 0x80
#define BITS_PER_CHAR_8 0xC0

/*=========================================================================
 * Serial protocol prototypes (each platform/port must supply definitions
 * of these prototypes)    
 *=======================================================================*/

extern long openPortByNumber __P ((char** ppszError, long port, long baudRate, long options, long blocking));
extern long openPortByName __P ((char** ppszError, char* pszDeviceName, long baudRate, long options, long blocking));

extern long peekFromPort __P ((long hPort));
extern long readFromPort __P ((char** ppszError, long hPort, char* pBuffer, long nNumberOfBytesToRead));

extern long writeToPort __P ((char** ppszError, long hPort, char* pBuffer, long nNumberOfBytesToWrite));

extern void closePort __P ((long hPort));
extern void freePortError __P ((char* pszError));

#endif /* _COMM_PROTOCOL_H_ */


