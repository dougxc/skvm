/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: IO Macros
 * FILE:      io.h
 * OVERVIEW:  Redefinition of IO functions for targets (such as the 
 *            Palm Pilot) that don't provide standard IO. All output
 *            must be done with the fprintf function.
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

#include <unix_stdarg.h>

/*
 * On Palm the following take the place of the standard C definitions and
 * allow us to handle logging in a platform-dependent way (see PALM_IO.c)
 */

#define LOGFILEPTR unsigned short
#define stdout 1
#define stderr 2
int fprintf(LOGFILEPTR file, const char *formatStr, ...);
void putchar(int c);

void InitializeStandardIO(void);
void FinalizeStandardIO(void);
