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
 * FILE:      loader_md.c
 * OVERVIEW:  This files provides the structures and operations needed for
 *            storing and loading classfiles on a DCryptor device.
 *            We basically use the JAR layout to initialially put classes
 *            onto the system through an fx_load. At VM startup, this
 *            jar file is moved into a RAM based structure so that standard
 *            pointer arithemtic can be performed to read its contents. If
 *            it is shown that the Flash ROM on the DCryptor devices also
 *            supports pointer arithmetic for read-only operations, then
 *            the jar file can remain in ROM.
 * AUTHOR:    Doug Simon
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <jar.h>

#include <flash.h>

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

/*
 * All files are entries in a jar file
 */
struct filePointerStruct {
    long dataLen;          /* length of data stream */
    long dataIndex;        /* current position for reading */
    unsigned char data[1];
};

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/*=========================================================================
 * Variables
 *=======================================================================*/

/*
 * All classes are in a single jar file.
 */
static struct jarInfoStruct TheJarFile;

#if SVM
static FILEPOINTER TrustedClassfile;
#if INCLUDEDEBUGCODE
static INSTANCE_CLASS TrustedClass;
#endif /* INCLUDEDEBUGCODE */
#endif /* SVM */

/*=========================================================================
 * Static operations (used only in this file)
 *=======================================================================*/

static FILEPOINTER openClassfileInternal(BYTES_HANDLE);
static BYTES       classToFilename(INSTANCE_CLASS clazz);

/*=========================================================================
 * Class loading file operations
 *=======================================================================*/

/*=========================================================================
 * NOTE:      The functions below provide an implementation of
 *            the abstract class loading interface defined in 
 *            VmCommon/h/loader.h.  The implementation here is
 *            suitable only for those target platforms that have
 *            a conventional file system.
 *=======================================================================*/

#if SVM

/*=========================================================================
 * FUNCTION:      setTrustedClassfile
 * TYPE:          class reading
 * OVERVIEW:      Registers the classfile content for a trusted class that
 *                is about to be loaded due to a call to
 *                java.lang.TrustedLoader.loadTrustedClass().
 *                The following preconditions must exist on entry to this
 *                function (they are not tested):
 *                   i.   clazz->status == CLASS_RAW
 *                   ii.  b->length >= (offset + length)
 * INTERFACE:
 *   parameters:  clazz:  Class defined by the classfile
 *                b:      the bytes that make up the class data
 *                offset: the start offset of the class data
 *                length: the length of the class data
 *   returns:     TRUE if the class does not correspond to an existing
 *                file known by the class loader, FALSE otherwise.
 *=======================================================================*/

bool_t setTrustedClassfile(INSTANCE_CLASS clazz, BYTEARRAY_HANDLE b,
                    unsigned int offset, unsigned int length)
{
    char* fileName = classToFilename(clazz);
    if (openClassfileInternal(&fileName) != NULL)
        /* Class file is already on the system - can't be replaced by
         * dynamic class. */
        return FALSE;
    
#if INCLUDEDEBUGCODE
    TrustedClass = clazz;
#endif
    TrustedClassfile = (FILEPOINTER)
        mallocBytes(sizeof(struct filePointerStruct) + length);
    TrustedClassfile->dataLen = length;
    TrustedClassfile->dataIndex = 0;
    memcpy(&(TrustedClassfile->data[0]),&(unhand(b)->bdata[offset]),length);
    return TRUE;
}

#endif /* SVM */

/*=========================================================================
 * FUNCTION:      openClassFile
 * TYPE:          class reading
 * OVERVIEW:      Returns a FILEPOINTER object to read the bytes of the 
 *                given class name
 * INTERFACE:
 *   parameters:  clazz:  Clazz to open
 *   returns:     a FILEPOINTER, or error if none exists
 *=======================================================================*/

FILEPOINTER
openClassfile(INSTANCE_CLASS clazz) { 
    FILEPOINTER ClassFile;
#if SVM
    /*
     * Return the registered classfile and then reset it.
     */
    if (TrustedClassfile != NULL) {
        FILEPOINTER fp = TrustedClassfile;
#if INCLUDEDEBUGCODE
        /*
         * Error in implementation.
         */
        if (TrustedClass != clazz)
            fatalError("Trusted class mismatch.");
        TrustedClass = NULL;
#endif /* INCLUDEDEBUGCODE */
        TrustedClassfile = NULL;
        return fp;
    }
#endif /* SVM */
    
    START_TEMPORARY_ROOTS
        DECLARE_TEMPORARY_ROOT(char *, fileName, classToFilename(clazz));

        ClassFile = openClassfileInternal(&fileName);   
        if (ClassFile == NULL) { 
#if INCLUDEDEBUGCODE
            if (traceclassloadingverbose) {
                DECLARE_TEMPORARY_ROOT(char *, className, 
                    getClassName((CLASS)clazz));
                sprintf(str_buffer, KVM_MSG_CLASS_NOT_FOUND_1STRPARAM, className);
                fprintf(stdout, str_buffer);
            }
#endif /* INCLUDEDEBUGCODE */
        }
    END_TEMPORARY_ROOTS
    return ClassFile;
}

/*=========================================================================
 * FUNCTION:      openResourceFile
 * TYPE:          resource reading
 * OVERVIEW:      Returns a FILEPOINTER object to read the bytes of the 
 *                given resource name
 * INTERFACE:
 *   parameters:  resourceName:  Name of resource to read
 *   returns:     a FILEPOINTER, or error if none exists
 *=======================================================================*/

FILEPOINTER
openResourcefile(BYTES resourceNameArg) { 
    FILEPOINTER ClassFile;
    START_TEMPORARY_ROOTS
        DECLARE_TEMPORARY_ROOT(BYTES, resourceName, resourceNameArg);
        ClassFile = openClassfileInternal(&resourceName);
        if (ClassFile == NULL) {
#if INCLUDEDEBUGCODE
            if (traceclassloadingverbose) {
                sprintf(str_buffer, 
                        KVM_MSG_RESOURCE_NOT_FOUND_1STRPARAM, resourceName);
                fprintf(stdout, str_buffer);
            }
#endif /* INCLUDEDEBUGCODE */
        }
    END_TEMPORARY_ROOTS
    return ClassFile;
}

/*=========================================================================
 * FUNCTION:      classToFilename()
 * TYPE:          class reading
 * OVERVIEW:      Internal function used by openClassfile().
 *                It returns the file name that a given class corresponds to.
 * INTERFACE:
 *   parameters:  clazz:  the class for which a file name must be found.
 *   returns:     char*: the heap allocated file name
 *=======================================================================*/
static char* classToFilename(INSTANCE_CLASS clazz) {
    UString UPackageName = clazz->clazz.packageName;
    UString UBaseName    = clazz->clazz.baseName;
    int     baseLength = UBaseName->length;
    int     packageLength = UPackageName == 0 ? 0 : UPackageName->length;
    int     length = packageLength + 1 + baseLength + 7;
    char*   fileName = (char*)mallocBytes(length);
    char    *to;

    ASSERTING_NO_ALLOCATION
        to = fileName;

        if (UPackageName != NULL) {
            memcpy(to, UStringInfo(UPackageName), packageLength);
            to += packageLength;
            *to++ = '/';
        }
        memcpy(to, UStringInfo(UBaseName), baseLength);
        to += baseLength;
        strcpy(to, ".class");
    END_ASSERTING_NO_ALLOCATION
    return fileName;
}

/*=========================================================================
 * FUNCTION:      openClassfileInternal()
 * TYPE:          class reading
 * OVERVIEW:      Internal function used by openClassfile().
 *                It returns a FILE* for reading the class if a file exists.
 *                NULL otherwise.
 * INTERFACE:
 *   parameters:  fileName:  Name of class file to read
 *   returns:     FILE* for reading the class, or NULL.
 *=======================================================================*/

static FILEPOINTER 
openClassfileInternal(BYTES_HANDLE filenameH) { 
    FILEPOINTER fp = NULL;
    long length;
    struct filePointerStruct *result = (struct filePointerStruct*)
           loadJARFileEntry(&TheJarFile, unhand(filenameH),&length,
                             offsetof(struct filePointerStruct, data[0]));
    if (result != NULL) { 
        result->dataLen = length;
        result->dataIndex = 0;
        fp = (FILEPOINTER)result;
    }
    return fp;
}

/*=========================================================================
 * FUNCTION:      loadByte(), loadShort(), loadCell()
 *                loadBytes, skipBytes()
 * TYPE:          private class file reading operations
 * OVERVIEW:      Read the next 1, 2, 4 or n bytes from the 
 *                given class file.
 * INTERFACE:
 *   parameters:  classfile pointer
 *   returns:     unsigned char, short or integer
 * NOTE:          For safety it might be a good idea to check
 *                explicitly for EOF in these operations.
 *=======================================================================*/

int
loadByteNoEOFCheck(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    struct filePointerStruct *ds = (struct filePointerStruct *)ClassFile;
    if (ds->dataIndex < ds->dataLen) { 
        return ds->data[ds->dataIndex++];
    } else { 
        return -1;
    }
}

unsigned char 
loadByte(FILEPOINTER_HANDLE ClassFileH)
{
    int c = loadByteNoEOFCheck(ClassFileH);
    if (c == -1) {
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    }
    return (unsigned char)c;
}

unsigned short 
loadShort(FILEPOINTER_HANDLE ClassFileH)
{
    unsigned char c1 = loadByte(ClassFileH);
    unsigned char c2 = loadByte(ClassFileH);
    unsigned short c  = c1 << 8 | c2;
    return c;
}

unsigned long
loadCell(FILEPOINTER_HANDLE ClassFileH)
{
    unsigned char c1 = loadByte(ClassFileH);
    unsigned char c2 = loadByte(ClassFileH);
    unsigned char c3 = loadByte(ClassFileH);
    unsigned char c4 = loadByte(ClassFileH);
    unsigned int c;
    c  = c1 << 24 | c2 << 16 | c3 << 8 | c4;
    return c;
}

void 
loadBytes(FILEPOINTER_HANDLE ClassFileH, char *buffer, int length) 
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    struct filePointerStruct *ds = (struct filePointerStruct *)ClassFile;
    int avail = ds->dataLen - ds->dataIndex;
    if (avail < length) { 
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    } else { 
        memcpy(buffer, &ds->data[ds->dataIndex], length);
        ds->dataIndex += length;
    }
}

void
skipBytes(FILEPOINTER_HANDLE ClassFileH, unsigned int length)
{ 
    FILEPOINTER ClassFile = unhand(ClassFileH);
    struct filePointerStruct *ds = (struct filePointerStruct *)ClassFile;
    int avail = ds->dataLen - ds->dataIndex;
    if (avail < length) { 
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    } else { 
        ds->dataIndex += length;
    }
}

#if SVM
/*=========================================================================
 * FUNCTION:      fseekFile(), readFile(), ftellFile()
 * TYPE:          private class file reading operations
 * OVERVIEW:      Faster and unsafer functions for reading from a file and
 *                manipulating the file position pointer. These should only
 *                be used on portions of a file that have already been
 *                manipulated with the safer load* operations above.
 *                They are primarily provided for fast reconstruction of a
 *                trusted class file to its original form as it was before it
 *                had the Trusted attribute injected.
 * INTERFACE:
 *   parameters:  file pointer
 *   returns:     <nothing>
 *=======================================================================*/

void
setFilePosition(FILEPOINTER_HANDLE ClassFileH, unsigned int offset)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    /* Once again, we assume that we are seeking to an existing part of
     * the file.
     */
    if (ClassFile->dataLen <= offset)
        fatalError(SVM_MSG_FILE_ERROR);
    ClassFile->dataIndex = offset;
}

unsigned int   getFilePosition(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    return ClassFile->dataIndex;
}

#endif /* SVM */

/*=========================================================================
 * FUNCTION:      closeClassfile()
 * TYPE:          private class file load operation
 * OVERVIEW:      Close the given classfile. Ensure that we have 
 *                reached the end of the classfile.
 * INTERFACE:
 *   parameters:  file pointer
 *   returns:     <nothing>
 *=======================================================================*/

void closeClassfile(FILEPOINTER_HANDLE ClassFileH)
{
#if INCLUDEDEBUGCODE
    if (traceclassloadingverbose) {
        fprintf(stdout, "Closing classfile\n");
    }
#endif /* INCLUDEDEBUGCODE */
}

/*=========================================================================
 * FUNCTION:      InitializeClassLoading()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Set up the dynamic class loader.
 *                Opens the single JAR file from a statically defined (in
 *                the JAR_FLASH_BLOCK macro) block of the Flash ROM on the
 *                DCryptor device.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     nothing directly, but the operation initializes
 *                the global classpath variables.
 *=======================================================================*/
#if MMAPPED_JAR
#ifndef SIMULATOR
/*
 * On the real device, this variable is defined by a 'map' statement in the
 * spec file used to build the KVM image.
 */
extern volatile unsigned char  JarFlashBlock[MAX_JAR_SIZE];
#else
/*
 * When simulating, this variable is just apointer into the dynamically
 * allocated memory simulating the flash.
 */
extern volatile unsigned char* JarFlashBlock;
#endif
void InitializeClassLoading()
{
    unsigned int   jarFileSize = 0, read = 0, numReads = 0;
    unsigned char* jarFile = (char*)JarFlashBlock;
    /*
     * Map the jar file that was fx_load'ed to a fixed block in memory
     * and then call openJARFile with that address. The resulting jar file
     * pointer is the single classpath entry.
     */
    numReads = sscanf(jarFile,"%d\n%n",&jarFileSize,&read);
    if (numReads != 1 || jarFileSize > MAX_JAR_SIZE) {
        /*
         * This can occur for a number of reasons:
         * 
         *   1. JAR has not been loaded onto the device
         *   2. Corrupt JAR was loaded or load failed
         *
         * Note that the JAR itself cannot be too big otherwise it would
         * not have been successfully loaded by fx_load.
         */
        fatalError("JAR file is missing or corrupted"); 
    }
    /*
     * Open the JAR file for reading. This simply extracts the table of
     * contents of the JAR but doesn't actually do anything with the
     * contents.
     */
    if (openJARFile(jarFile+read, jarFileSize, &TheJarFile) != TRUE)
        fatalError("Error reading JAR file.");
#if SVM
    /*
     * Register the TrustedClassfile handle as a global root (which will be
     * NULL for most of the time).
     */
#if INCLUDEDEBUGCODE
    TrustedClass = NULL;
#endif /* INCLUDEDEBUGCODE */
    TrustedClassfile = NULL;
    makeGlobalRoot((cell**)&TrustedClassfile);
#endif /* SVM */
}

#else /* ! MMAPPED_JAR */

void InitializeClassLoading()
{
    int   alignedBuffer[FLASH_READ_SIZE / 4];
    char* flashBuffer = (char*)alignedBuffer;
    unsigned char* jarFileContents;
    unsigned int   jarFileSize, read, i, offset, numReads;

    /*
     * Map the jar file that was fx_load'ed to a fixed block into memory
     * and then call openJARFile with that address. The resulting jar file
     * pointer is the single classpath entry.
     */
    flash_read(JAR_FLASH_ADDRESS, flashBuffer);
    numReads = sscanf(flashBuffer,"%d\n%n",&jarFileSize,&read);
    if (numReads != 1 || jarFileSize > MAX_JAR_SIZE)
        fatalError("JAR file is malformed or too big"); 
    numReads = (jarFileSize + (read - 1)) / FLASH_READ_SIZE;

    /*
     * The jar file's contents must be loaded into permanent, immovable
     * memory as the JAR_INFO struct simply contains pointers into the
     * contents.
     */
    jarFileContents = (unsigned char*)
          callocPermanentObject((jarFileSize + FLASH_READ_SIZE) >> log2CELL);
    /*
     * Copy remainder of first block into jar file.
     */
    offset = MIN((FLASH_READ_SIZE-read),jarFileSize);
    memcpy(jarFileContents,flashBuffer+read,offset);
    /*
     * Now copy rest of jar file from remaining blocks.
     */
    for (i = 1; i <= numReads; i++) {
        if(flash_read(JAR_FLASH_ADDRESS+(FLASH_READ_SIZE*i),
                                flashBuffer) != 0)
            fatalError("Error reading JAR file.");
        memcpy(jarFileContents+offset,flashBuffer,FLASH_READ_SIZE);
        offset += FLASH_READ_SIZE;
    }
    /*
     * Open the JAR file in memory.
     */
    if (openJARFile(jarFileContents, jarFileSize, &TheJarFile) != TRUE) {
        fatalError("Error reading JAR file.");
    }
#if SVM
    /*
     * Register the TrustedClassfile handle as a global root (which will be
     * NULL for most of the time).
     */
#if INCLUDEDEBUGCODE
    TrustedClass = NULL;
#endif /* INCLUDEDEBUGCODE */
    TrustedClassfile = NULL;
    makeGlobalRoot(&TrustedClassfile);
#endif /* SVM */
}
#endif /* MMAPPED_JAR */

/*=========================================================================
 * FUNCTION:      FinalizeClassLoading()
 * TYPE:          destructor (kind of)
 * OVERVIEW:      Shut down the dynamic class loader.
 *                Actual implementation of this function is
 *                platform-dependent.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing> (but closes JAR files as a side-effect)
 *=======================================================================*/

void FinalizeClassLoading() 
{ 
}
