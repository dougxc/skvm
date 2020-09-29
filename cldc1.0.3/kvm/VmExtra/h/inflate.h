/*
 * Copyright (c) 1999-2001 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: JAR file reader / inflater.
 * FILE:      inflate.h
 * OVERVIEW:  Public header file for the JAR inflater module. 
 * AUTHOR:    Tasneem Sayeed, Frank Yellin
 *=======================================================================*/

#ifndef _INFLATE_H_
#define _INFLATE_H_

/*=========================================================================
 * Forward declarations for JAR file reader
 *=======================================================================*/

typedef int    (*JarGetByteFunctionType)(void *);

bool_t inflate(void *data, JarGetByteFunctionType, int compLen, 
               UNSIGNED_CHAR_HANDLE outFileH, int decompLen);

/* Any caller to inflate must ensure that it is safe to read at least
 * this many bytes beyond compData + compLen
 */
#define INFLATER_EXTRA_BYTES 4

#endif /* _INFLATE_H_ */


