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
 * SUBSYSTEM: Class loader
 * FILE:      loader.h
 * OVERVIEW:  Internal definitions needed for loading 
 *            Java class files (class loader). 
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Sheng Liang, Frank Yellin, many others...
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines a JVM Specification compliant classfile reader.
 * It is capable of reading any standard Java classfile, and generating
 * the necessary corresponding VM runtime structures.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

extern char* UserClassPath; /* set in main() or elsewhere */

#ifndef PATH_SEPARATOR
#   define PATH_SEPARATOR ':'
#endif

/*=========================================================================
 * Classfile loading data structures
 *=======================================================================*/

/*  This structure is used for referring to open "files" when  */
/*  loading Java classfiles from the storage system of the host */
/*  operating system. It replaces the standard FILE* structure */
/*  used in C program. We need our own structure, since many of */
/*  our target devices don't have a regular file system. */

/*  FILEPOINTER */
struct filePointerStruct;

#define SIZEOF_FILEPOINTER       StructSizeInCells(filePointerStruct)

/*=========================================================================
 * Class file verification operations (performed during class loading)
 *=======================================================================*/

enum validName_type {LegalMethod, LegalField, LegalClass};
bool_t isValidName(const char* name, enum validName_type);

/*=========================================================================
 * Class loading operations
 *=======================================================================*/

void loadClassfile(INSTANCE_CLASS CurrentClass);
void loadArrayClass(ARRAY_CLASS);

/*=========================================================================
 * Generic class file reading operations
 *=======================================================================*/

/*
 * NOTE: The functions below define an abstract interface for
 * reading data from class files.  The actual implementations
 * of these functions are platform-dependent, and therefore are
 * not provided in VmCommon. An implementation of these functions
 * must be provided for each platform. A sample implementation 
 * can be found in VmExtra/src/loaderFile.c.
 */
int            loadByteNoEOFCheck(FILEPOINTER_HANDLE);
unsigned char  loadByte(FILEPOINTER_HANDLE);
unsigned short loadShort(FILEPOINTER_HANDLE);
unsigned long  loadCell(FILEPOINTER_HANDLE);
void           loadBytes(FILEPOINTER_HANDLE, char *buffer, int length);
void           skipBytes(FILEPOINTER_HANDLE, unsigned int i);
#if SVM
/* These functions are used to reconstruct the original class file from a
 * trusted class file that has had the Trusted attribute injected into it.
 * They can be much faster than the above functions as they operate on
 * sections of a file that have already been read with the above functions
 * (and so no bounds checking is required).
 */
void           setFilePosition(FILEPOINTER_HANDLE, unsigned int);
unsigned int   getFilePosition(FILEPOINTER_HANDLE);
#endif /* SVM */

void           InitializeClassLoading(void);
void           FinalizeClassLoading();

FILEPOINTER    openClassfile(INSTANCE_CLASS clazz);
FILEPOINTER    openResourcefile(BYTES resourceName);
void           closeClassfile(FILEPOINTER_HANDLE);

#if SVM
/*
 * A trusted class can be dynamically downloaded into the system. This
 * function provides the mechanism for the platform independent part of
 * this restructed type of dynamic class loading to interface to the
 * platform dependent class file manager.
 */
bool_t         setTrustedClassfile(INSTANCE_CLASS clazz, BYTEARRAY_HANDLE b,
                    unsigned int offset, unsigned int length);
#endif /* SVM */

/*=========================================================================
 * Helper functions
 *=======================================================================*/

char* replaceLetters(char* string, char c1, char c2);

