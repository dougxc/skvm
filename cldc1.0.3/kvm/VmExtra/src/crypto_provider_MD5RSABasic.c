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
 * SUBSYSTEM: Cryptographic Provider Implementation
 * FILE:      crypto_provider_md5rsabasic_.c
 * OVERVIEW:  This file provides a sample implementation of a Cryptographic
 *            Service Provider (CSP). It implements the standard RSA digital
 *            signature algorithm (i.e. hash with MD5, sign with RSA private
 *            key, verify with RSA public key). It also expects the following
 *            'Basic' encodings:
 *
 *            PublicKey {
 *                int   modulusSizeInBits,    // big-endian
 *                int   exponentSizeInBits,   // big-endian
 *                byte  modulus[],  // length = (modulusSizeInBits+7)/8
 *                byte  exponent[]  // length = (exponentSizeInBits+7)/8
 *            }
 *
 *            Signature {
 *                byte  signature[] // raw signature data
 *            }
 *                
 *            Both the modulus and exponent are multiple precision integers
 *            (MPIs) encoded in big-endian byte order. They are positive
 *            values and therefore have no sign bit extensions.
 *
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Type and forward declarations for rsaref2 crypto implementation.
 *=======================================================================*/
typedef unsigned char* POINTER;
#define MIN_RSA_MODULUS_BITS 508
#define MAX_RSA_MODULUS_BITS 1024
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)
#define MAX_SIGNATURE_LEN MAX_RSA_MODULUS_LEN
typedef struct {
  cell state[4];                                   /* state (ABCD) */
  cell count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

static void MD5Init (MD5_CTX *);
static void MD5Update (MD5_CTX *, unsigned char *, unsigned int);
static void MD5Final (unsigned char [16], MD5_CTX *);
typedef struct RSAPublicKeyStruct {
  unsigned int bits;                           /* length in bits of modulus */
  unsigned char modulus[MAX_RSA_MODULUS_LEN];                    /* modulus */
  unsigned char exponent[MAX_RSA_MODULUS_LEN];           /* public exponent */
}* RSA_PUBLIC_KEY;

static int RSAPublicDecrypt (unsigned char*, unsigned int*, unsigned char*,
                      unsigned int, RSA_PUBLIC_KEY);

void R_EncodeDigestInfo (unsigned char *, unsigned char *);
static unsigned char DIGEST_INFO_A[] = {
  0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x02
};
#define DIGEST_INFO_A_LEN sizeof (DIGEST_INFO_A)

static unsigned char DIGEST_INFO_B[] = { 0x05, 0x00, 0x04, 0x10 };
#define DIGEST_INFO_B_LEN sizeof (DIGEST_INFO_B)

#define DIGEST_INFO_LEN (DIGEST_INFO_A_LEN + 1 + DIGEST_INFO_B_LEN + 16)

#define R_memcpy(output,input,len) \
    if(len != 0) \
        memcpy(output,input,len);
#define R_memset(output,input,len) \
    if(len != 0) \
        memset(output,input,len);

/*=========================================================================
 * Implementation of interface defined in crypto_provider.h
 *=======================================================================*/
 
bool_t Crypto_VerifyCSP(const char* cspIdentifier, unsigned int length)
{
    return strncmp("MD5RSABasic",cspIdentifier,length) == 0;
}

/*
 * A macro to convert 4 bytes (in big endian order) to an unsigned int
 */
#define BYTES_TO_INT(b) \
    (unsigned int) ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3])

CryptoResultCode Crypto_DecodePublicKey(KEY_HANDLE keyH,
                                        unsigned int length,
                                        char* encodedKey)
{
    struct RSAPublicKeyStruct  publicKeyStruct;
    RSA_PUBLIC_KEY  publicKey = &publicKeyStruct;
    
    unsigned char* data = encodedKey;
    unsigned int modBits = BYTES_TO_INT(data);
    unsigned int expBits = BYTES_TO_INT((data+4));
    unsigned int modBytes = (modBits + 7)/8;
    unsigned int expBytes = (expBits + 7)/8;
    unsigned char* modulus  = &(data[8]);
    unsigned char* exponent = &(data[8+modBytes]);
    
    /*
     * check internal size of encoding
     */
    unsigned int encSize = (8 + modBytes + expBytes);
    if (length != encSize)
        return Crypto_InvalidEncoding;
    
    if (length > sizeof(struct RSAPublicKeyStruct) ||
        modBits > MAX_RSA_MODULUS_BITS ||
        expBits > MAX_RSA_MODULUS_BITS)
        return Crypto_UnsupportedKeySize;

    publicKey->bits = modBits;
    /*
     * need to account for fixed size of RSAREF structures by copying MPIs to
     * the end of the 128 character array used to store them. The leading
     * bytes are zero-padded.
     */
    memset(publicKey->modulus,0,MAX_RSA_MODULUS_LEN - modBytes);
    memcpy(publicKey->modulus + (MAX_RSA_MODULUS_LEN - modBytes),
           modulus,modBytes);
    memset(publicKey->exponent,0,MAX_RSA_MODULUS_LEN - expBytes);
    memcpy(publicKey->exponent + (MAX_RSA_MODULUS_LEN - expBytes),
           exponent,expBytes);
    /*
     * get SVM managed, in-memory copy of the key
     */
    unhand(keyH) = retrieveKey(sizeof(struct RSAPublicKeyStruct),
                           (unsigned char*)publicKey);
    return Crypto_OK;
}

CryptoResultCode
Crypto_DecodeSignature(SIGNATURE_HANDLE signatureH,
                       unsigned int length,
                       unsigned char* encodedSignature,
                       TRUSTED_CLASS tclazz)
{
    /*
     * EncodedSignatureStruct {
     *     unsigned char sig[]
     * }
     */
    if (length > MAX_SIGNATURE_LEN)
        return Crypto_UnsupportedSignatureSize;

    unhand(signatureH) = retrieveSignature(tclazz,length,encodedSignature);
    return Crypto_OK;
}

void
Crypto_DigestContent(DIGEST_HANDLE digestH, unsigned int length,
                     unsigned char* content)
{
    MD5_CTX   md5Ctx;
    MD5_CTX*  context = &md5Ctx;
    unsigned char digest[16];
    
    MD5Init(context);
    MD5Update(context,content,length);
    MD5Final(digest,context);
    
    unhand(digestH) = retrieveDigest(16,digest);
}

 
CryptoResultCode
Crypto_VerifySignature(SIGNATURE signature,
                       DIGEST digest,
                       KEY publicKey)
{
    unsigned char originalDigest[MAX_SIGNATURE_LEN];
    unsigned int  originalDigestLen = 0;
    RSA_PUBLIC_KEY rsaKey = (RSA_PUBLIC_KEY)publicKey->key;
    unsigned char digestInfo[DIGEST_INFO_LEN];
    
    R_EncodeDigestInfo(digestInfo, digest->digest);
    
    if (RSAPublicDecrypt(originalDigest,&originalDigestLen,
                         signature->signature,signature->size,rsaKey) != 0)
        return Crypto_VerifyFail;
    
    if ((originalDigestLen != DIGEST_INFO_LEN) ||
        memcmp((unsigned char*)originalDigest,digestInfo,16) != 0)
        return Crypto_VerifyFail;
    return Crypto_OK;
}

/*=========================================================================
 * MD5 implementation.
 * 
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.  
 *                                                                  
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.  
 *                                                                  
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.  
 *=======================================================================*/

/* Constants for MD5Transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


static void MD5Transform (cell [4], unsigned char [64]);
static void Encode (unsigned char *, cell *, unsigned int);
static void Decode (cell *, unsigned char *, unsigned int);
static void MD5_memcpy (POINTER, POINTER, unsigned int);
static void MD5_memset (POINTER, int, unsigned int);

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (cell)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + (cell)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + (cell)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + (cell)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
static void MD5Init (context)
MD5_CTX *context;                                        /* context */
{
  context->count[0] = context->count[1] = 0;

  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
     operation, processing another message block, and updating the
     context.
 */
static void MD5Update (context, input, inputLen)
MD5_CTX *context;                                        /* context */
unsigned char *input;                                /* input block */
unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((cell)inputLen << 3))
      < ((cell)inputLen << 3))
    context->count[1]++;
  context->count[1] += ((cell)inputLen >> 29);
  
  partLen = 64 - index;
  
  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    MD5_memcpy
      ((POINTER)&context->buffer[index], (POINTER)input, partLen);
    MD5Transform (context->state, context->buffer);
  
    for (i = partLen; i + 63 < inputLen; i += 64)
      MD5Transform (context->state, &input[i]);
    
    index = 0;
  }
  else
    i = 0;
  
  /* Buffer remaining input */
  MD5_memcpy 
    ((POINTER)&context->buffer[index], (POINTER)&input[i],
     inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
     the message digest and zeroizing the context.
 */
static void MD5Final (digest, context)
unsigned char digest[16];                         /* message digest */
MD5_CTX *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);
  
  /* Append length (before padding) */
  MD5Update (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);
  
  /* Zeroize sensitive information.
   */
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (state, block)
cell state[4];
unsigned char block[64];
{
  cell a = state[0], b = state[1], c = state[2], d = state[3], x[16];
  
  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

  /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  
  /* Zeroize sensitive information.
   */
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (cell) into output (unsigned char). Assumes len is
     a multiple of 4.
 */
static void Encode (output, input, len)
unsigned char *output;
cell *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (cell). Assumes len is
     a multiple of 4.
 */
static void Decode (output, input, len)
cell *output;
unsigned char *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((cell)input[j]) | (((cell)input[j+1]) << 8) |
      (((cell)input[j+2]) << 16) | (((cell)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */
static void MD5_memcpy (output, input, len)
POINTER output;
POINTER input;
unsigned int len;
{
  unsigned int i;
  
  for (i = 0; i < len; i++)
    output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (output, value, len)
POINTER output;
int value;
unsigned int len;
{
  unsigned int i;
  
  for (i = 0; i < len; i++)
    ((char *)output)[i] = (char)value;
}

/* DigestInfo encoding is DIGEST_INFO_A, then 2 or 5 (for MD2/MD5),
   then DIGEST_INFO_B, then 16-byte message digest.
 */

void R_EncodeDigestInfo (digestInfo, digest)
unsigned char *digestInfo;                           /* DigestInfo encoding */
unsigned char *digest;                                    /* message digest */
{
  R_memcpy 
    ((POINTER)digestInfo, (POINTER)DIGEST_INFO_A, DIGEST_INFO_A_LEN);
  
  digestInfo[DIGEST_INFO_A_LEN] = (unsigned char)5;

  R_memcpy 
    ((POINTER)&digestInfo[DIGEST_INFO_A_LEN + 1], (POINTER)DIGEST_INFO_B,
     DIGEST_INFO_B_LEN);
  
  R_memcpy 
    ((POINTER)&digestInfo[DIGEST_INFO_A_LEN + 1 + DIGEST_INFO_B_LEN],
     (POINTER)digest, 16);
}

/*=========================================================================
 * Mulitple precision arithmetic support.
 * 
 * Copyright (C) RSA Laboratories, a division of RSA Data Security,
 *    Inc., created 1991. All rights reserved.
 *
 *=======================================================================*/

/* Type definitions.
 */
typedef cell NN_DIGIT;
typedef unsigned short NN_HALF_DIGIT;

/* Constants.

   Note: MAX_NN_DIGITS is long enough to hold any RSA modulus, plus
   one more digit as required by R_GeneratePEMKeys (for n and phiN,
   whose lengths must be even). All natural numbers have at most
   MAX_NN_DIGITS digits, except for double-length intermediate values
   in NN_Mult (t), NN_ModMult (t), NN_ModInv (w), and NN_Div (c).
 */
/* Length of digit in bits */
#define NN_DIGIT_BITS 32
#define NN_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_NN_DIGITS \
  ((MAX_RSA_MODULUS_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_NN_DIGIT 0xffffffff
#define MAX_NN_HALF_DIGIT 0xffff

/* Macros.
 */
#define LOW_HALF(x) ((x) & MAX_NN_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((NN_DIGIT)(x)) << NN_HALF_DIGIT_BITS)
#define DIGIT_MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (NN_DIGIT_BITS - 2)) & 3)
#define NN_ASSIGN_DIGIT(a, b, digits) {NN_AssignZero (a, digits); a[0] = b;}
#define NN_EQUAL(a, b, digits) (! NN_Cmp (a, b, digits))
#define NN_EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1))


/* Computes a = b * c, where b and c are digits.

   Lengths: a[2].
 */
void NN_DigitMult (a, b, c)
NN_DIGIT a[2], b, c;
{
  NN_DIGIT t, u;
  NN_HALF_DIGIT bHigh, bLow, cHigh, cLow;

  bHigh = (NN_HALF_DIGIT)HIGH_HALF (b);
  bLow = (NN_HALF_DIGIT)LOW_HALF (b);
  cHigh = (NN_HALF_DIGIT)HIGH_HALF (c);
  cLow = (NN_HALF_DIGIT)LOW_HALF (c);

  a[0] = (NN_DIGIT)bLow * (NN_DIGIT)cLow;
  t = (NN_DIGIT)bLow * (NN_DIGIT)cHigh;
  u = (NN_DIGIT)bHigh * (NN_DIGIT)cLow;
  a[1] = (NN_DIGIT)bHigh * (NN_DIGIT)cHigh;
  
  if ((t += u) < u)
    a[1] += TO_HIGH_HALF (1);
  u = TO_HIGH_HALF (t);
  
  if ((a[0] += u) < u)
    a[1]++;
  a[1] += HIGH_HALF (t);
}

/* Sets a = b / c, where a and c are digits.

   Lengths: b[2].
   Assumes b[1] < c and HIGH_HALF (c) > 0. For efficiency, c should be
   normalized.
 */
void NN_DigitDiv (a, b, c)
NN_DIGIT *a, b[2], c;
{
  NN_DIGIT t[2], u, v;
  NN_HALF_DIGIT aHigh, aLow, cHigh, cLow;

  cHigh = (NN_HALF_DIGIT)HIGH_HALF (c);
  cLow = (NN_HALF_DIGIT)LOW_HALF (c);

  t[0] = b[0];
  t[1] = b[1];

  /* Underestimate high half of quotient and subtract.
   */
  if (cHigh == MAX_NN_HALF_DIGIT)
    aHigh = (NN_HALF_DIGIT)HIGH_HALF (t[1]);
  else
    aHigh = (NN_HALF_DIGIT)(t[1] / (cHigh + 1));
  u = (NN_DIGIT)aHigh * (NN_DIGIT)cLow;
  v = (NN_DIGIT)aHigh * (NN_DIGIT)cHigh;
  if ((t[0] -= TO_HIGH_HALF (u)) > (MAX_NN_DIGIT - TO_HIGH_HALF (u)))
    t[1]--;
  t[1] -= HIGH_HALF (u);
  t[1] -= v;

  /* Correct estimate.
   */
  while ((t[1] > cHigh) ||
         ((t[1] == cHigh) && (t[0] >= TO_HIGH_HALF (cLow)))) {
    if ((t[0] -= TO_HIGH_HALF (cLow)) > MAX_NN_DIGIT - TO_HIGH_HALF (cLow))
      t[1]--;
    t[1] -= cHigh;
    aHigh++;
  }

  /* Underestimate low half of quotient and subtract.
   */
  if (cHigh == MAX_NN_HALF_DIGIT)
    aLow = (NN_HALF_DIGIT)LOW_HALF (t[1]);
  else
    aLow = 
      (NN_HALF_DIGIT)((TO_HIGH_HALF (t[1]) + HIGH_HALF (t[0])) / (cHigh + 1));
  u = (NN_DIGIT)aLow * (NN_DIGIT)cLow;
  v = (NN_DIGIT)aLow * (NN_DIGIT)cHigh;
  if ((t[0] -= u) > (MAX_NN_DIGIT - u))
    t[1]--;
  if ((t[0] -= TO_HIGH_HALF (v)) > (MAX_NN_DIGIT - TO_HIGH_HALF (v)))
    t[1]--;
  t[1] -= HIGH_HALF (v);

  /* Correct estimate.
   */
  while ((t[1] > 0) || ((t[1] == 0) && t[0] >= c)) {
    if ((t[0] -= c) > (MAX_NN_DIGIT - c))
      t[1]--;
    aLow++;
  }
  
  *a = TO_HIGH_HALF (aHigh) + aLow;
}

static NN_DIGIT NN_AddDigitMult (NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *,
       unsigned int);
static NN_DIGIT NN_SubDigitMult 
  (NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int);

static int NN_Cmp (NN_DIGIT *, NN_DIGIT *, unsigned int);
static unsigned int NN_Digits (NN_DIGIT *, unsigned int);
static unsigned int NN_DigitBits (NN_DIGIT);

/* Decodes character string b into a, where character string is ordered
   from most to least significant.

   Lengths: a[digits], b[len].
   Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most
   significant bytes are truncated.)
 */
static void NN_Decode (a, digits, b, len)
NN_DIGIT *a;
unsigned char *b;
unsigned int digits, len;
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;
  
  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = 0;
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      t |= ((NN_DIGIT)b[j]) << u;
    a[i] = t;
  }
  
  for (; i < digits; i++)
    a[i] = 0;
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

   Lengths: a[len], b[digits].
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
   digits are truncated.)
 */
static void NN_Encode (a, len, b, digits)
NN_DIGIT *b;
unsigned char *a;
unsigned int digits, len;
{
  NN_DIGIT t;
  int j;
  unsigned int i, u;

  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = b[i];
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
      a[j] = (unsigned char)(t >> u);
  }

  for (; j >= 0; j--)
    a[j] = 0;
}

/* Assigns a = b.

   Lengths: a[digits], b[digits].
 */
static void NN_Assign (a, b, digits)
NN_DIGIT *a, *b;
unsigned int digits;
{
  unsigned int i;

  for (i = 0; i < digits; i++)
    a[i] = b[i];
}

/* Assigns a = 0.

   Lengths: a[digits].
 */
static void NN_AssignZero (a, digits)
NN_DIGIT *a;
unsigned int digits;
{
  unsigned int i;

  for (i = 0; i < digits; i++)
    a[i] = 0;
}

/* Computes a = b + c. Returns carry.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Add (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT ai, carry;
  unsigned int i;

  carry = 0;

  for (i = 0; i < digits; i++) {
    if ((ai = b[i] + carry) < carry)
      ai = c[i];
    else if ((ai += c[i]) < c[i])
      carry = 1;
    else
      carry = 0;
    a[i] = ai;
  }

  return (carry);
}

/* Computes a = b - c. Returns borrow.

   Lengths: a[digits], b[digits], c[digits].
 */
NN_DIGIT NN_Sub (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT ai, borrow;
  unsigned int i;

  borrow = 0;

  for (i = 0; i < digits; i++) {
    if ((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
      ai = MAX_NN_DIGIT - c[i];
    else if ((ai -= c[i]) > (MAX_NN_DIGIT - c[i]))
      borrow = 1;
    else
      borrow = 0;
    a[i] = ai;
  }

  return (borrow);
}

/* Computes a = b * c.

   Lengths: a[2*digits], b[digits], c[digits].
   Assumes digits < MAX_NN_DIGITS.
 */
static void NN_Mult (a, b, c, digits)
NN_DIGIT *a, *b, *c;
unsigned int digits;
{
  NN_DIGIT t[2*MAX_NN_DIGITS];
  unsigned int bDigits, cDigits, i;

  NN_AssignZero (t, 2 * digits);
  
  bDigits = NN_Digits (b, digits);
  cDigits = NN_Digits (c, digits);

  for (i = 0; i < bDigits; i++)
    t[i+cDigits] += NN_AddDigitMult (&t[i], &t[i], b[i], c, cDigits);
  
  NN_Assign (a, t, 2 * digits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_LShift (a, b, c, digits)
NN_DIGIT *a, *b;
unsigned int c, digits;
{
  NN_DIGIT bi, carry;
  unsigned int i, t;
  
  if (c >= NN_DIGIT_BITS)
    return (0);
  
  t = NN_DIGIT_BITS - c;

  carry = 0;

  for (i = 0; i < digits; i++) {
    bi = b[i];
    a[i] = (bi << c) | carry;
    carry = c ? (bi >> t) : 0;
  }
  
  return (carry);
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

   Lengths: a[digits], b[digits].
   Requires: c < NN_DIGIT_BITS.
 */
NN_DIGIT NN_RShift (a, b, c, digits)
NN_DIGIT *a, *b;
unsigned int c, digits;
{
  NN_DIGIT bi, carry;
  int i;
  unsigned int t;
  
  if (c >= NN_DIGIT_BITS)
    return (0);
  
  t = NN_DIGIT_BITS - c;

  carry = 0;

  for (i = digits - 1; i >= 0; i--) {
    bi = b[i];
    a[i] = (bi >> c) | carry;
    carry = c ? (bi << t) : 0;
  }
  
  return (carry);
}

/* Computes a = c div d and b = c mod d.

   Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
           dDigits < MAX_NN_DIGITS.
 */
static void NN_Div (a, b, c, cDigits, d, dDigits)
NN_DIGIT *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
  NN_DIGIT ai, cc[2*MAX_NN_DIGITS+1], dd[MAX_NN_DIGITS], t;
  int i;
  unsigned int ddDigits, shift;
  
  ddDigits = NN_Digits (d, dDigits);
  if (ddDigits == 0)
    return;
  
  /* Normalize operands.
   */
  shift = NN_DIGIT_BITS - NN_DigitBits (d[ddDigits-1]);
  NN_AssignZero (cc, ddDigits);
  cc[cDigits] = NN_LShift (cc, c, shift, cDigits);
  NN_LShift (dd, d, shift, ddDigits);
  t = dd[ddDigits-1];
  
  NN_AssignZero (a, cDigits);

  for (i = cDigits-ddDigits; i >= 0; i--) {
    /* Underestimate quotient digit and subtract.
     */
    if (t == MAX_NN_DIGIT)
      ai = cc[i+ddDigits];
    else
      NN_DigitDiv (&ai, &cc[i+ddDigits-1], t + 1);
    cc[i+ddDigits] -= NN_SubDigitMult (&cc[i], &cc[i], ai, dd, ddDigits);

    /* Correct estimate.
     */
    while (cc[i+ddDigits] || (NN_Cmp (&cc[i], dd, ddDigits) >= 0)) {
      ai++;
      cc[i+ddDigits] -= NN_Sub (&cc[i], &cc[i], dd, ddDigits);
    }
    
    a[i] = ai;
  }
  
  /* Restore result.
   */
  NN_AssignZero (b, dDigits);
  NN_RShift (b, cc, shift, ddDigits);

  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)cc, 0, sizeof (cc));
  R_memset ((POINTER)dd, 0, sizeof (dd));
}

/* Computes a = b mod c.

   Lengths: a[cDigits], b[bDigits], c[cDigits].
   Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.
 */
static void NN_Mod (a, b, bDigits, c, cDigits)
NN_DIGIT *a, *b, *c;
unsigned int bDigits, cDigits;
{
  NN_DIGIT t[2 * MAX_NN_DIGITS];
  
  NN_Div (t, a, b, bDigits, c, cDigits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_NN_DIGITS.
 */
static void NN_ModMult (a, b, c, d, digits)
NN_DIGIT *a, *b, *c, *d;
unsigned int digits;
{
  NN_DIGIT t[2*MAX_NN_DIGITS];

  NN_Mult (t, b, c, digits);
  NN_Mod (a, t, 2 * digits, d, digits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)t, 0, sizeof (t));
}

/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
   Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS.
 */

/* PGP 2.5's mpilib contains a faster modular exponentiation routine, mp_modexp.
   If USEMPILIB is defined, NN_ModExp is replaced in the PGP 2.5 sources with a 
   stub call to mp_modexp.  If USEMPILIB is not defined, we'll get a pure (albeit
   slower) RSAREF implementation.

   The RSAREF 2.0 license, clause 1(c), permits "...modify[ing] the Program in any
   manner for porting or performance improvement purposes..." */

#ifndef USEMPILIB
static void NN_ModExp (a, b, c, cDigits, d, dDigits)
NN_DIGIT *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
  NN_DIGIT bPower[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
  int i;
  unsigned int ciBits, j, s;

  /* Store b, b^2 mod d, and b^3 mod d.
   */
  NN_Assign (bPower[0], b, dDigits);
  NN_ModMult (bPower[1], bPower[0], b, d, dDigits);
  NN_ModMult (bPower[2], bPower[1], b, d, dDigits);
  
  NN_ASSIGN_DIGIT (t, 1, dDigits);

  cDigits = NN_Digits (c, cDigits);
  for (i = cDigits - 1; i >= 0; i--) {
    ci = c[i];
    ciBits = NN_DIGIT_BITS;
    
    /* Scan past leading zero bits of most significant digit.
     */
    if (i == (int)(cDigits - 1)) {
      while (! DIGIT_2MSB (ci)) {
        ci <<= 2;
        ciBits -= 2;
      }
    }

    for (j = 0; j < ciBits; j += 2, ci <<= 2) {
      /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
       */
      NN_ModMult (t, t, t, d, dDigits);
      NN_ModMult (t, t, t, d, dDigits);
      if ((s = DIGIT_2MSB (ci)) != 0)
        NN_ModMult (t, t, bPower[s-1], d, dDigits);
    }
  }
  
  NN_Assign (a, t, dDigits);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)bPower, 0, sizeof (bPower));
  R_memset ((POINTER)t, 0, sizeof (t));
}
#endif

/* Returns sign of a - b.

   Lengths: a[digits], b[digits].
 */
int NN_Cmp (a, b, digits)
NN_DIGIT *a, *b;
unsigned int digits;
{
  int i;
  
  for (i = digits - 1; i >= 0; i--) {
    if (a[i] > b[i])
      return (1);
    if (a[i] < b[i])
      return (-1);
  }

  return (0);
}

/* Returns the significant length of a in digits.

   Lengths: a[digits].
 */
static unsigned int NN_Digits (a, digits)
NN_DIGIT *a;
unsigned int digits;
{
  int i;
  
  for (i = digits - 1; i >= 0; i--)
    if (a[i])
      break;

  return (i + 1);
}

/* Computes a = b + c*d, where c is a digit. Returns carry.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_AddDigitMult (a, b, c, d, digits)
NN_DIGIT *a, *b, c, *d;
unsigned int digits;
{
  NN_DIGIT carry, t[2];
  unsigned int i;

  if (c == 0)
    return (0);

  carry = 0;
  for (i = 0; i < digits; i++) {
    NN_DigitMult (t, c, d[i]);
    if ((a[i] = b[i] + carry) < carry)
      carry = 1;
    else
      carry = 0;
    if ((a[i] += t[0]) < t[0])
      carry++;
    carry += t[1];
  }
  
  return (carry);
}

/* Computes a = b - c*d, where c is a digit. Returns borrow.

   Lengths: a[digits], b[digits], d[digits].
 */
static NN_DIGIT NN_SubDigitMult (a, b, c, d, digits)
NN_DIGIT *a, *b, c, *d;
unsigned int digits;
{
  NN_DIGIT borrow, t[2];
  unsigned int i;

  if (c == 0)
    return (0);

  borrow = 0;
  for (i = 0; i < digits; i++) {
    NN_DigitMult (t, c, d[i]);
    if ((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
      borrow = 1;
    else
      borrow = 0;
    if ((a[i] -= t[0]) > (MAX_NN_DIGIT - t[0]))
      borrow++;
    borrow += t[1];
  }
  
  return (borrow);
}

/* Returns the significant length of a in bits, where a is a digit.
 */
static unsigned int NN_DigitBits (a)
NN_DIGIT a;
{
  unsigned int i;
  
  for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)
    if (a == 0)
      break;
    
  return (i);
}
/*=========================================================================
 * RSA implementation.
 * 
 * Copyright (C) RSA Laboratories, a division of RSA Data Security,
 *   Inc., created 1991. All rights reserved.
 *
 *=======================================================================*/

static int RSAPublicBlock (unsigned char*, unsigned int*, unsigned char*,
                           unsigned int, RSA_PUBLIC_KEY );
/* Error codes.
 */
#define RE_CONTENT_ENCODING 0x0400
#define RE_DATA 0x0401
#define RE_DIGEST_ALGORITHM 0x0402
#define RE_ENCODING 0x0403
#define RE_KEY 0x0404
#define RE_KEY_ENCODING 0x0405
#define RE_LEN 0x0406
#define RE_MODULUS_LEN 0x0407
#define RE_NEED_RANDOM 0x0408
#define RE_PRIVATE_KEY 0x0409
#define RE_PUBLIC_KEY 0x040a
#define RE_SIGNATURE 0x040b
#define RE_SIGNATURE_ENCODING 0x040c
#define RE_ENCRYPTION_ALGORITHM 0x040d

/* RSA public-key decryption, according to PKCS #1.
 */
static int RSAPublicDecrypt (output, outputLen, input, inputLen, publicKey)
unsigned char *output;                                      /* output block */
unsigned int *outputLen;                          /* length of output block */
unsigned char *input;                                        /* input block */
unsigned int inputLen;                             /* length of input block */
RSA_PUBLIC_KEY publicKey;                              /* RSA public key */
{
  int status;
  unsigned char pkcsBlock[MAX_RSA_MODULUS_LEN];
  unsigned int i, modulusLen, pkcsBlockLen;
  
  modulusLen = (publicKey->bits + 7) / 8;
  if (inputLen > modulusLen)
    return FALSE;
  
  if ((status = RSAPublicBlock
      (pkcsBlock, &pkcsBlockLen, input, inputLen, publicKey)) != 0)
    return (status);
  
  if (pkcsBlockLen != modulusLen)
    return (RE_LEN);
  
  /* Require block type 1.
   */
  if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))
   return (RE_DATA);

  for (i = 2; i < modulusLen-1; i++)
    if (pkcsBlock[i] != 0xff)
      break;
    
  /* separator */
  if (pkcsBlock[i++] != 0)
    return (RE_DATA);
  
  *outputLen = modulusLen - i;
  
  if (*outputLen + 11 > modulusLen)
    return (RE_DATA);

  R_memcpy ((POINTER)output, (POINTER)&pkcsBlock[i], *outputLen);
  
  /* Zeroize potentially sensitive information.
   */
  R_memset ((POINTER)pkcsBlock, 0, sizeof (pkcsBlock));
  
  return (0);
}

/* Raw RSA public-key operation. Output has same length as modulus.

   Assumes inputLen < length of modulus.
   Requires input < modulus.
 */
static int RSAPublicBlock (output, outputLen, input, inputLen, publicKey)
unsigned char *output;                                      /* output block */
unsigned int *outputLen;                          /* length of output block */
unsigned char *input;                                        /* input block */
unsigned int inputLen;                             /* length of input block */
RSA_PUBLIC_KEY publicKey;                              /* RSA public key */
{
  NN_DIGIT c[MAX_NN_DIGITS], e[MAX_NN_DIGITS], m[MAX_NN_DIGITS],
    n[MAX_NN_DIGITS];
  unsigned int eDigits, nDigits;

  NN_Decode (m, MAX_NN_DIGITS, input, inputLen);
  NN_Decode (n, MAX_NN_DIGITS, publicKey->modulus, MAX_RSA_MODULUS_LEN);
  NN_Decode (e, MAX_NN_DIGITS, publicKey->exponent, MAX_RSA_MODULUS_LEN);
  nDigits = NN_Digits (n, MAX_NN_DIGITS);
  eDigits = NN_Digits (e, MAX_NN_DIGITS);
  
  if (NN_Cmp (m, n, nDigits) >= 0)
    return (RE_DATA);
  
  /* Compute c = m^e mod n.
   */
  NN_ModExp (c, m, e, eDigits, n, nDigits);

  *outputLen = (publicKey->bits + 7) / 8;
  NN_Encode (output, *outputLen, c, nDigits);
  
  /* Zeroize sensitive information.
   */
  R_memset ((POINTER)c, 0, sizeof (c));
  R_memset ((POINTER)m, 0, sizeof (m));

  return (0);
}
