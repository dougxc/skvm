/*
 * Copyright (c) 2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * SYSTEM:    KVMutil
 * SUBSYSTEM: KVMutil program
 * FILE:      KVMutil.h
 * OVERVIEW:  This application allows the Palm user to set various
 *            KVM execution options.
 * AUTHOR:    Mitch Butler
 *=======================================================================*/

#include <KVMcommon.h>
#include <KVMutil_rsrc.h>

/*======================================================================
 * External variables
 *====================================================================*/

extern Boolean returnFromOutput;
extern Boolean KVMprefsChanged;
extern Int maxHeapSize;

/*======================================================================
 * Function prototypes
 *====================================================================*/

Boolean MainFormHandleEvent(EventPtr event);
Boolean LicenseFormHandleEvent (EventPtr event);
Boolean AboutFormHandleEvent (EventPtr event);
Boolean OutputFormHandleEvent (EventPtr event);

VoidPtr GetObjectPtr(Int objectID);
FieldPtr GetFocusObjectPtr(void);
