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
 * SYSTEM:    SVM
 * SUBSYSTEM: Internal runtime structures for cryptographic operations
 * FILE:      crypto.c
 * OVERVIEW:  
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

const char* CryptoResultCodeMessages[] = SVM_MSG_CRYPTO_MESSAGES_INITIALIZER;

/*=========================================================================
 * Operations on pending permits
 *=======================================================================*/

SIGNATURE
retrieveSignature(TRUSTED_CLASS tclazz,
                   unsigned int length,
                   unsigned char* data)
{
/*static unsigned int total = 0, unique = 0;*/
    SIGNATURE sig;
/*printf("Signatures:\ttotal %d, unique %d\n",total++,unique);*/
    if (tclazz && tclazz->ppermits)
    {
        unsigned int index;
        FOR_ALL_PERMITS(tclazz->ppermits,permit,index)
            sig = permit->signature;
            if (sig != NULL &&
                sig->size == length &&
                sig->signature[0] == data[0] &&
                memcmp(data,sig->signature,length) == 0)
                return sig;
        END_FOR_ALL_PERMITS
    }
/*unique++;*/
    /*
     * now have to allocate a new SIGNATURE
     */
    sig = (SIGNATURE)mallocObject(SIZEOF_SIGNATURE(length),GCT_NOPOINTERS);
    sig->size = length;
    memcpy(sig->signature,data,length);
    return sig;
}

/*
 * Allocate, populate and return a new digest.
 */
DIGEST
retrieveDigest( unsigned int length,
                unsigned char* data)
{
    DIGEST digest = (DIGEST)mallocObject(SIZEOF_DIGEST(length),GCT_NOPOINTERS);
    digest->size = length;
    memcpy(digest->digest,data,length);
    return digest;
}

/*=========================================================================
 * Operations on PENDING_PERMITS
 *=======================================================================*/

#if INCLUDEDEBUGCODE

#define PERMIT_BOUNDS_CHECK(ppermits,i)                   \
    if ((unsigned)i >= (getObjectSize((cell*)ppermits) - 2))     \
        fatalError(SVM_MSG_PERMIT_ARRAY_INDEX_OUT_OF_BOUNDS);


PERMIT
permitAt(PENDING_PERMITS ppermits, unsigned int index)
{
    PERMIT_BOUNDS_CHECK(ppermits,index)
    return ppermits->permits[index];
}
        
unsigned int
addPermit(PENDING_PERMITS ppermits, PERMIT permit, bool_t isInterfacePermit)
{
    PERMIT_BOUNDS_CHECK(ppermits,ppermits->count)
    ppermits->permits[ppermits->count++] = permit;
    if (isInterfacePermit)
        ppermits->interfacePermitCount++;
    return ppermits->count - 1;
}

PENDING_PERMITS
removePermit(PENDING_PERMITS ppermits, unsigned int index)
{
    PERMIT_BOUNDS_CHECK(ppermits,index)
    if (--(ppermits->count) == 0) {
        return NULL;
    }
    else
    {
        if (index != ppermits->count)
            ppermits->permits[index] = ppermits->permits[ppermits->count];
        return ppermits;
    }
}
#endif
