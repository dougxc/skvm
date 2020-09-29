/*
 * Copyright (c) 2001 Sun Microsystems, Inc. All Rights Reserved.
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

/* This file contains functions that are easier to do in C than in
 * assembler.
 */

#include <global.h>

long FastInterpret_idiv(long x, long y) {
    return x/y;
}

long FastInterpret_irem(long x, long y) {
    return x % y;
}

long64 FastInterpret_ldiv(long64 x, long64 y) {
    return x / y;
}

long64 FastInterpret_lrem(long64 x, long64 y) {
    return x % y;
}
