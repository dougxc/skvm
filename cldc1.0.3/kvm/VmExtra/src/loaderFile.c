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
 * FILE:      loaderFile.c
 * OVERVIEW:  Structures and operations needed for loading
 *            Java classfiles from the file system.  This file
 *            (loaderFile.c) provides the default file system
 *            operations that are typically very useful on all those
 *            target platforms that have a regular file system.
 *
 *            These operations have been separated from the actual
 *            class loading operations (VmCommon/src/loader.c), since
 *            the mapping between the KVM and the Java classfile
 *            storage mechanisms of the host operating system can vary
 *            considerably in different embedded devices.
 * AUTHOR:    Tasneem Sayeed, Consumer & Embedded division
 *            Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stdio.h>
#include <sys/stat.h>
#include <jar.h>

#if !JAR_FILES_USE_STDIO
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

struct filePointerStruct {
    /* If set to true, indicates a JAR file */
    bool_t isJarFile;
};

struct stdioPointerStruct { 
    bool_t isJarFile;      /* Always FALSE */
    FILE   *file;          /* Pointer to class file */
};

struct jarPointerStruct { 
    bool_t isJarFile;      /* always TRUE */
    long dataLen;          /* length of data stream */
    long dataIndex;        /* current position for reading */
    unsigned char data[1];
};

typedef struct classPathEntryStruct { 
    union { 
        struct jarInfoStruct jarInfo; /* if it's a jar file */
        /* Leave possibility of other types, for later */
    } u;
    char  type;
    char  name[1];
} *CLASS_PATH_ENTRY, **CLASS_PATH_ENTRY_HANDLE;

/*=========================================================================
 * Variables
 *=======================================================================*/

/* A table for storing the individual directory paths along classpath.
 * Each entry is one element in the list.
 *
 * An entry consists of the letter 'j' or 'd' followed by more information.
 * 'd' means this is a directory.  It is immediately followed by the name
 *      of the directory.
 * 'j' means a zip file.  It is followed by 3 ignored spaces, then 4 bytes
 *      giving the location of the first local header, 4 bytes giving
 *      the location of the first central header, and then the name of the
 *      zip file.
 */

/* Remember: This table contains object pointers (= GC root) */
static POINTERLIST ClassPathTable = NIL;

/* Set in main() to classpath environment */
char* UserClassPath = NULL;

static unsigned int MaxClassPathTableLength = 0;

#if SVM
static struct jarPointerStruct* TrustedClassfile;
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
        FILEPOINTER fp = (FILEPOINTER)TrustedClassfile;
#if INCLUDEDEBUGCODE
        /*
         * Error in implementation.
         */
        if (TrustedClass != clazz)
            fatalVMError("Trusted class mismatch.");
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
    int filenameLength = strlen(unhand(filenameH));
    int paths = ClassPathTable->length;
    int i;
    FILE *file = NULL;
    FILEPOINTER fp = NULL;

    START_TEMPORARY_ROOTS
        int fullnameLength = MaxClassPathTableLength + filenameLength + 2;
        DECLARE_TEMPORARY_ROOT(char*, fullname, 
            (char*)mallocBytes(fullnameLength));
        DECLARE_TEMPORARY_ROOT(CLASS_PATH_ENTRY, entry, NULL);
        for (i = 0; i < paths && fp == NULL; i++) { 
            entry = (CLASS_PATH_ENTRY)ClassPathTable->data[i].cellp;
            switch (entry->type) { 
            case 'd':                      /* A directory */
                sprintf(fullname, "%s/%s", entry->name, unhand(filenameH));
                file = fopen(fullname, "rb");
                if (file != NULL) {
                    fp = (FILEPOINTER)
                         mallocBytes(sizeof(struct stdioPointerStruct));
                    fp->isJarFile = FALSE;
                    ((struct stdioPointerStruct *)fp)->file = file;
                }
                break;

            case 'j':  {                   /* A JAR file */
                long length;
                struct jarPointerStruct *result = (struct jarPointerStruct*)
                    loadJARFileEntry(&entry->u.jarInfo, unhand(filenameH), 
                                     &length,
                                     offsetof(struct jarPointerStruct, data[0]));
                if (result != NULL) { 
                    result->isJarFile = TRUE;
                    result->dataLen = length;
                    result->dataIndex = 0;
                    fp = (FILEPOINTER)result;
                }
                break;
            }
            }
        }
    END_TEMPORARY_ROOTS
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
    if (!ClassFile->isJarFile) {
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        return getc(file);
    } else {
        struct jarPointerStruct *ds = (struct jarPointerStruct *)ClassFile;
        if (ds->dataIndex < ds->dataLen) { 
            return ds->data[ds->dataIndex++];
        } else { 
            return -1;
        }
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
    int n;
    FILEPOINTER ClassFile = unhand(ClassFileH);
    if (!ClassFile->isJarFile) {
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        n = fread(buffer, 1, length, file);
        if (n != length) {
            raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
        }
    } else { 
        struct jarPointerStruct *ds = (struct jarPointerStruct *)ClassFile;
        int avail = ds->dataLen - ds->dataIndex;
        if (avail < length) { 
            raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
        } else { 
            memcpy(buffer, &ds->data[ds->dataIndex], length);
            ds->dataIndex += length;
        }
    }
}

void
skipBytes(FILEPOINTER_HANDLE ClassFileH, unsigned int length)
{ 
    FILEPOINTER ClassFile = unhand(ClassFileH);
    if (!ClassFile->isJarFile) {
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        /* We originally used fseek, but it doesn't give any easy to
         * recognize EOF warning.
         */
        while (length-- > 0) { 
            int c = getc(file);
            if (c == EOF) {
                raiseExceptionCharMsg(ClassFormatError,
                    KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
            }
        }
    } else {
        struct jarPointerStruct *ds = (struct jarPointerStruct *)ClassFile;
        int avail = ds->dataLen - ds->dataIndex;
        if (avail < length) { 
            raiseExceptionCharMsg(ClassFormatError,
                    KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
        } else { 
            ds->dataIndex += length;
        }
    }
}

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
    TrustedClassfile = (struct jarPointerStruct*)
        mallocBytes(sizeof(struct jarPointerStruct) + length);
    TrustedClassfile->isJarFile = TRUE;
    TrustedClassfile->dataLen = length;
    TrustedClassfile->dataIndex = 0;
    memcpy(&(TrustedClassfile->data[0]),&(unhand(b)->bdata[offset]),length);
    return TRUE;
}

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
    if (!ClassFile->isJarFile) {
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        /* We can use fseek as we assume that this operation will only be used
         * to seek to a part of the file that we know exists.
         */
        if (fseek(file,offset,SEEK_SET) == -1)
            fatalError(SVM_MSG_FILE_ERROR);
    } else {
        struct jarPointerStruct *ds = (struct jarPointerStruct *)ClassFile;
        /* Once again, we assume that we are seeking to an existing part of
         * the file.
         */
        if (ds->dataLen >= offset)
            fatalError(SVM_MSG_FILE_ERROR);
        ds->dataIndex = offset;
    }
}

unsigned int   getFilePosition(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    unsigned int result;
    if (!ClassFile->isJarFile) {
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        if((result = ftell(file)) == -1)
            fatalError(SVM_MSG_FILE_ERROR);
    } else {
        struct jarPointerStruct *ds = (struct jarPointerStruct *)ClassFile;
        result = ds->dataIndex;
    }
    return result;
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
    FILEPOINTER ClassFile = unhand(ClassFileH);

#if INCLUDEDEBUGCODE
    if (traceclassloadingverbose) {
        fprintf(stdout, "Closing classfile\n");
    }
#endif /* INCLUDEDEBUGCODE */

    if (ClassFile->isJarFile) {
        /* Close the JAR datastream.   Don't need to do anything */
    } else {
        /* Close the classfile */
        FILE *file = ((struct stdioPointerStruct*)ClassFile)->file;
        if (file != NULL) { 
            fclose(file);
            ((struct stdioPointerStruct*)ClassFile)->file = NULL;
        }
    }
}

/*=========================================================================
 * FUNCTION:      InitializeClassLoading()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Set up the dynamic class loader.
 *                Read the optional environment variable CLASSPATH
 *                and set up the class path for classfile loading
 *                correspondingly. Actual implementation of this
 *                function is platform-dependent.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     nothing directly, but the operation initializes
 *                the global classpath variables.
 *=======================================================================*/

void InitializeClassLoading()
{
    char *classpath = UserClassPath;
    int length, pathCount, i;

    int tableIndex;
    int previousI;

    /*  Count the number of individual directory paths along CLASSPATH */
    length = strlen(classpath);
    pathCount = 1;

    for (i = 0; i < length; i++) {
        if (classpath[i] == PATH_SEPARATOR) pathCount++;
    }

    /* We use callocObject() since we'll be allocating new objects to fill
     * up the table */
    ClassPathTable = (POINTERLIST)callocObject(SIZEOF_POINTERLIST(pathCount), 
                                               GCT_POINTERLIST); 
    makeGlobalRoot((cell **)&ClassPathTable);

    ClassPathTable->length = pathCount;
    MaxClassPathTableLength = 0;

    tableIndex = 0;
    previousI = 0;

    for (i = 0; i <= length; i++) {
       if (classpath[i] == PATH_SEPARATOR || classpath[i] == 0) {
           struct stat sbuf;
           /* Create a new string containing the individual directory path. 
            * We prepend either a 'j' (jar file) or 'd' (directory) indicating
            * the type of classpath element this is. 
            */
           unsigned int length = i - previousI;
           CLASS_PATH_ENTRY result = 
               (CLASS_PATH_ENTRY)mallocBytes(
                   sizeof(struct classPathEntryStruct) + length);

           /* Copy the name into the result buffer */
           memcpy(result->name, classpath + previousI, length);
           result->name[length] = '\0';
           result->type = '\0'; /* indicates a problem */
   
           if (stat(result->name, &sbuf) < 0) { 
               /* No need to do anything */
           } else if (S_ISDIR(sbuf.st_mode)) { 
               /* This is a directory */
               /*  Store the newly created string to the classpath table */
               if (length > MaxClassPathTableLength) { 
                   MaxClassPathTableLength = length;
               }
               result->type = 'd';
           }
#if JAR_FILES_USE_STDIO
           else if (openJARFile(result->name, 0, &result->u.jarInfo)) { 
               result->type = 'j';
           }
#else 
           else { 
               /* This is something we only do for debugging */
               long fileLength = sbuf.st_size;
               FILE *file = fopen(result->name, "rb");
               char *data = mmap(0, fileLength, PROT_READ, MAP_SHARED, 
                                 fileno(file), 0);
               if (openJARFile(data, fileLength, &result->u.jarInfo)) { 
                   result->type = 'j';
               } else { 
                   /* Not a 'jar' file.  Unmap it */
                   if (data != NULL) { 
                       munmap(data, fileLength);
                   }
               }
           }
#endif /* JAR_FILES_USE_STDIO */

           if (result->type == '\0') { 
               /* bad entry  */
               ClassPathTable->length--;
           } else { 
               ClassPathTable->data[tableIndex].cellp = (cell*)result;
               tableIndex++;
           }
           previousI = i+1;
       }
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
    makeGlobalRoot((cell**)&TrustedClassfile);
#endif /* SVM */

}

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
    int paths = ClassPathTable->length;
    int i;
    for (i = 0; i < paths; i++) { 
        CLASS_PATH_ENTRY entry = 
            (CLASS_PATH_ENTRY)ClassPathTable->data[i].cellp;
        if (entry->type == 'j') { 
            closeJARFile(&entry->u.jarInfo);
        }
    }
}

