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
 * SUBSYSTEM: resource access (Generic Connection framework)
 * FILE:      resource.c
 * OVERVIEW:  This file implements the native functions for
 *            a Generic Connection protocol that is used for 
 *            accessing external resources.
 * AUTHOR:    Nik Shaylor
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Forward declarations
 *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_open(void);
void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_close(void);
void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_read(void);

/*=========================================================================
 * Functions
 *=======================================================================*/

 /*=========================================================================
  * FUNCTION:      Object open(String) (STATIC)
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Open resource stream
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_open(void) {
    STRING_INSTANCE string = popStackAsType(STRING_INSTANCE);
    char           *name;
    FILEPOINTER     fp;

    START_TEMPORARY_ROOTS
        DECLARE_TEMPORARY_ROOT(STRING_INSTANCE, _string, string);
        long buflen = _string->length + 1;
        DECLARE_TEMPORARY_ROOT(char *, buf,
                               (char *)mallocHeapObject((buflen+CELL-1)>>log2CELL,
                               GCT_NOPOINTERS));

        name = getStringContentsSafely(_string, buf, buflen);
        if (buf == NULL) {
            raiseException("java/lang/OutOfMemoryError");
        } else {
            fp = openResourcefile(name);
            if (fp == NULL) {
                raiseException("java/io/IOException");
            } else { 
                pushStackAsType(FILEPOINTER, fp);
            }
        }
    END_TEMPORARY_ROOTS
}

 /*=========================================================================
  * FUNCTION:      void close(Object) static   [Object is fp]
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the resource
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_close(void) {
    START_TEMPORARY_ROOTS
        DECLARE_TEMPORARY_ROOT(FILEPOINTER, fp, popStackAsType(FILEPOINTER));
        if (fp == NULL) {
            raiseException("java/io/IOException");
        } else {
            closeClassfile(&fp);
        }
    END_TEMPORARY_ROOTS
    /* Java code will set the handle to NULL */
}

 /*=========================================================================
  * FUNCTION:      read()I (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the resource
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_read(void) {
    START_TEMPORARY_ROOTS
        int result;
        DECLARE_TEMPORARY_ROOT(FILEPOINTER, fp, popStackAsType(FILEPOINTER));
        if (fp == NULL) {
            raiseException("java/io/IOException");
        } else { 
            result = loadByteNoEOFCheck(&fp);
            pushStack(result);
        }
    END_TEMPORARY_ROOTS
}


