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

/* We have two separate workspaces for building on the Palm.
 * One includes this file, and the other doesn't.
 */

#ifndef _KVM_PREDEFINE_H_INCLUDED
#define _KVM_PREDEFINE_H_INCLUDED

#define PILOT_PRECOMPILED_HEADERS_OFF 0

/*
#define PLATFORMNAME          "j2me"
*/

/* Turn debugging and profiling code off and use
 * terse messages since Palm has a limited screen 
 */
#define INCLUDEDEBUGCODE      0
#define ENABLEPROFILING       0
#define TERSE_MESSAGES        1
#define PALM                  1
#define PILOT                 1
#define ENABLE_JAVA_DEBUGGER  0
#define JAR_FILES_USE_STDIO   0
#define USE_JAM               0
#define ROMIZING              1
#define PADTABLE              1

#endif /* _KVM_PREDEFINE_H_INCLUDED */


