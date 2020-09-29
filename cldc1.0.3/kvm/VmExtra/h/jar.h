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

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: JAR file reader.
 * FILE:      jar.h
 * OVERVIEW:  Declarations for reading JAR files and their entries.
 * AUTHOR:    Ioi Lam, Tasneem Sayeed, Frank Yellin
  *=======================================================================*/

#ifndef JAR_FILES_USE_STDIO
#define JAR_FILES_USE_STDIO  1
#endif

typedef struct jarInfoStruct {
    union {
        struct {
            void /* FILE  */ *file;  /* We may not have stdio loaded */
            unsigned long locOffset; /* Offset of local directory */
            unsigned long cenOffset; /* Offset of central directory */
        } jar;
        struct {
            const unsigned char *base;
            const unsigned char *locPtr;
            const unsigned char *cenPtr;
        } mjar;
    } u;
} *JAR_INFO, **JAR_INFO_HANDLE;

bool_t openJARFile(void *nameOrAddress, int length, JAR_INFO entry);

void closeJARFile(JAR_INFO entry);

void *
loadJARFileEntry(JAR_INFO, const char *filename, long *length, 
                 int extraBytes);


typedef bool_t (*JARFileTestFunction)(const char *name, int nameLength, 
                                      int *extraBytes, void *info);
typedef void (*JARFileRunFunction)(const char *name, int nameLength, 
                                   void *value, long length, void *info);

void
loadJARFileEntries(JAR_INFO entry, 
                   JARFileTestFunction, JARFileRunFunction, void*);


