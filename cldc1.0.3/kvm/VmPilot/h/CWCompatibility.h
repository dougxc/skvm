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

#ifndef _CWCompatibility_H_INCLUDED
#define _CWCompatibility_H_INCLUDED

/* CodeWarrior IDE version */
#define CWVERSION 7


#if CWVERSION == 7

#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <SerialMgrOld.h>
#define kWinSetPattern(x) WinSetPattern(&(x))
#define kWinGetPattern(x) WinGetPattern(&(x))
#define kWinUp winUp
#define kWinDown winDown
#define kWinDirectionType WinDirectionType

#else

#include <Pilot.h>
#include <SerialMgr.h>
#define kWinSetPattern(x) WinSetPattern(x)
#define kWinGetPattern(x) WinGetPattern(x)
#define kWinUp up
#define kWinDown down
#define kWinDirectionType DirectionType
#define sysTrapHostControl sysTrapSysGremlins

#endif

#endif
