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
 * FILE:      crypto.h
 * OVERVIEW:  Internal runtime structures to provide the cryptographic
 *            support required by the SVM.
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Global definitions
 *=======================================================================*/

/*=========================================================================
 * IMPORTANT: If you change the order of any of the data elements
 * in these data structures, make sure that the order matches
 * with the corresponding data structures in 'rom.h'.
 *=======================================================================*/


/*=========================================================================
 * The data structures defined below are essentially opaque wrappers around
 * the Cryptographic Service Provider's implementation structures. This
 * provides an abstract interface between the CSP and the parts of
 * the SVM that use the provided cryptographic support.
 *=======================================================================*/

/* DIGEST */
struct digestStruct {
    unsigned int size;        /* opaque data size (bytes) */
    unsigned char  digest[1]; /* opaque digest data */
};

/* KEY */
struct keyStruct {
    KEY next;                 /* next KEY in KeyTable hash table */
    unsigned int  size;       /* opaque data size (bytes) */
    char key[1];              /* opaque key data */
};

/* SIGNATURE */
struct signatureStruct {
    unsigned int size;           /* opaque data size (bytes) */
    unsigned char  signature[1]; /* opaque signature data */
};

/*=========================================================================
 * A permit represents a privilege obtained by a class (the requestor) to
 * access another trusted class (the grantor) in a particular way (e.g. the
 * requestor wants to subclass the grantor). Permits are implemented as
 * digital signatures of the requestor (i.e. its class file), signed by the
 * grantor. Exactly which parts of a class file are subjected to the signing
 * process depend on the type of privilege they represent. The permits for a
 * class are attached to its class file at development time. The SVM runtime
 * must accept these permits at load time and discharge (i.e. verify the
 * signatures) the permits before the privilege represented is granted to the
 * runtime class. Ideally, all permits would be discharged at the time the
 * permit is loaded and thus remove the need to keep the permit in memory.
 * This is not possible given that the point at which the permit is loaded
 * is not close to the point at which we are guaranteed to have the grantor
 * class loaded. As such, the 'pending' permits must be stored with the
 * class until such time that they can be discharged.
 *
 * The permit array in PENDING_PERMITS is ordered as follows. The first
 * entry is the subclass permit (if any) followed by permits for
 * implementing trusted interfaces. The remainder of the array is dedicated
 * to instantiation permits. This ordering only applies between the time
 * all the permits are loaded and the time at which the first permits is
 * discharged. The verification functions re-order the array for
 * efficiency.
 *=======================================================================*/


/* PERMIT */
struct permitStruct {
    INSTANCE_CLASS grantor;
    SIGNATURE signature;
};

/* PENDING_PERMITS */
struct pendingPermitsStruct {
    DIGEST digest;
    SIGNATURE subclassPermit;
    unsigned short interfacePermitCount;
    unsigned short count;
    PERMIT permits[1];
};

/*=========================================================================
 * Operations for manipulating PENDING_PERMITS and PERMITs. Note that some
 * operations are conditionally defined as macros depending on whether
 * or not this is a debug build.
 *=======================================================================*/

/*
 * The subclass permit will be the only one that has a NULL value in the field
 * specifying its grantor.
 */
#define IS_SUBCLASS_PERMIT(permit) (permit->grantor == NULL)

/*
 * These are operations for adding or removing PERMITs to a PENDING_PERMITS
 * struct or for indexing into such a struct. If INCLUDEDEBUGCODE is defined,
 * there are defined as functions which include bounds checks. Otherwise they
 * are defined as macros which omit the bounds checking for extra speed. The
 * bounds will only be violated if there is an error in the SVM code, not by
 * malicious classes.
 */
#if !INCLUDEDEBUGCODE
#define permitAt(ppermits,i)         (ppermits->permits[i])
#define addPermit(ppermits, permit, isInterfacePermit)  \
    (ppermits->permits[ppermits->count++] = permit, \
     ppermits->interfacePermitCount += (isInterfacePermit?1:0), \
     ppermits->count-1)
#define removePermit(ppermits, index) \
    (--(ppermits->count) == 0  ? NULL : \
     (index != ppermits->count ? (ppermits->permits[index] = \
                                  ppermits->permits[ppermits->count],ppermits)\
                               : ppermits))
#else
PERMIT permitAt(PENDING_PERMITS ppermits, unsigned int index);
unsigned int addPermit(PENDING_PERMITS ppermits, PERMIT permit,
                       bool_t isInterfacePermit);
PENDING_PERMITS removePermit(PENDING_PERMITS ppermits, unsigned int index);
#endif

/*
 * This macro defines a convenient way to traverse all the non-null PERMITs
 * in a given PENDING_PERMITS.
 */
#define FOR_ALL_PERMITS(__ppermits__,__permit__,__idx__) \
    {                                                                     \
        for (__idx__ = 0;                                                 \
             __idx__ != (__ppermits__->count); ++__idx__)                 \
        {                                                                 \
            PERMIT __permit__ = __ppermits__->permits[__idx__];
            

#define END_FOR_ALL_PERMITS \
        }                   \
    }

        
/*=========================================================================
 * These operations are used by the CSP to store the implementation
 * dependent runtime structures in an opaque wrapper in the SVM memory
 * system. The CSP need not be concerned with the particulars of how these
 * structures are stored.
 *=======================================================================*/

KEY       retrieveKey(unsigned int length, unsigned char* opaqueData);
SIGNATURE retrieveSignature(TRUSTED_CLASS tclazz,
                            unsigned int length,
                            unsigned char* opaqueData);
DIGEST    retrieveDigest(unsigned int length,
                         unsigned char* opaqueData);

/*=========================================================================
 * Size of above variable-length structures
 *=======================================================================*/

#define SIZEOF_KEY(n) \
    ByteSizeToCellSize(offsetof(struct keyStruct,key) + n)
#define SIZEOF_SIGNATURE(n) \
    ByteSizeToCellSize(offsetof(struct signatureStruct, signature) + n)
#define SIZEOF_DIGEST(n) \
    ByteSizeToCellSize(offsetof(struct digestStruct, digest) + n)
#define SIZEOF_PERMIT          StructSizeInCells(permitStruct)
#define SIZEOF_PENDING_PERMITS(n) \
    ByteSizeToCellSize(offsetof(struct pendingPermitsStruct,permits) + \
        (n << log2CELL))

