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
 * SUBSYSTEM: Cryptographic Service Provider Interface (CSPI)
 * FILE:      crypto_provider.h
 * OVERVIEW:  Defines the functions that must be implemented by the
 *            Cryptographic Service Provider (CSP).
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * Error values that can be returned by the crypto routines.
 *=======================================================================*/

typedef enum CryptoResultCodeEnum {
    /*
     * Denotes a successful operation.
     */
    Crypto_OK,

    /*
     * Returned when a given key encoding represents a key
     * with an unsupported size. For example, the CSP may only
     * 512 and 1024 bit RSA keys but is given a 2048 bit key.
     */
    Crypto_UnsupportedKeySize,
        
    /*
     * Returned when a given key signature encoding is larger
     * than the signatures sizes supported by the CSP.
     */
    Crypto_UnsupportedSignatureSize,
        
    /*
     * Returned when the raw data for a public key or signature is encoded
     * in the format expected by the CSP.
     */
    Crypto_InvalidEncoding,
    
    /* Returned when signature verification fails. */
    Crypto_VerifyFail

} CryptoResultCode;

#define SVM_MSG_CRYPTO_MESSAGES_INITIALIZER                            \
{   /* 0 */    NULL,  /* SUCCESS */                                    \
    /* 1 */    "Unsupported key size",                                 \
    /* 2 */    "Unsupported signature size",                           \
    /* 3 */    "Invalid encoding for key/signature",                   \
    /* 4 */    "Signature verification failed"                         \
}

extern const char* CryptoResultCodeMessages[];
    
/*=========================================================================
 * Given that each CSP defines not only what digital signature algorithm it
 * implements but also the transport encoding of the public key and
 * signatures accepted by the implementation, each class must have been
 * developed for a specific CSP. This function tests whether or not the
 * CSP specified by a class identifies the CSP on the platform.
 *=======================================================================*/

bool_t Crypto_VerifyCSP(const char* cspIdentifier, unsigned int length);

/*
 * The CSP is responsible for decoding keys and signatures as
 * they come in a trusted classfile into the internal representation that will
 * be used in the crypto functions.
 */
CryptoResultCode Crypto_DecodePublicKey(KEY_HANDLE keyH,
                                        unsigned int length,
                                        char* encodedKey);

CryptoResultCode Crypto_DecodeSignature(SIGNATURE_HANDLE signatureH,
                                        unsigned int length,
                                        unsigned char* encodedSignature,
                                        TRUSTED_CLASS clazz);
/*
 * The process of verifying a digital signature is composed of 3 steps:
 * 
 *   1. Decode a signature into an implementation dependent structure.
 *   2. Compute hash on contents (e.g. class file) associated with signature
 *      and store this hash in an implementation dependent structure.
 *   3. Given a public key, verify that the signature was indeed computed
 *      on the previously digested contents.
 *
 * The functions below implement the last two steps. It is important to note
 * that the SVM does not require block digest computation and so the
 * Crypto_DigestContent function can compute the complete digest and store the
 * results at one point in time.
 */
void Crypto_DigestContent(DIGEST_HANDLE digestH,
                                      unsigned int length,
                                      unsigned char* content);
CryptoResultCode Crypto_VerifySignature(SIGNATURE signature,
                                        DIGEST digest,
                                        KEY publicKey);
