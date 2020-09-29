/*
 * Copyright (c) 1998-2001 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: Class file verifier (runtime part)
 * FILE:      verifier.h
 * OVERVIEW:  KVM has a new, two-phase class file verifier.  In order
 *            to run in KVM, class files must first be processed with
 *            a special "pre-verifier" tool. This phase is typically
 *            done on the development workstation.  During execution,
 *            the runtime verifier (defined in this file) of the KVM
 *            performs the actual class file verification based on 
 *            both runtime information and pre-verification information.
 * AUTHOR:    Sheng Liang
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Functions
 *=======================================================================*/

void InitializeVerifier(void);
void verifyClass(INSTANCE_CLASS c);

#if CACHE_VERIFICATION_RESULT
    bool_t checkVerifiedClassList(INSTANCE_CLASS);
    void appendVerifiedClassList(INSTANCE_CLASS);
#else 
#   define checkVerifiedClassList(class) FALSE
#   define appendVerifiedClassList(class)
#endif
