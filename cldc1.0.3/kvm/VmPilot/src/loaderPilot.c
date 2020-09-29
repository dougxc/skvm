/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 * FILE:      loaderPilot.c
 * OVERVIEW:  Structures and operations needed for loading on the Palm
 *            Java classfiles (class loader).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Frank Yellin
 * NOTE:      This file provides a Palm-specific implementation of
 *            the abstract class loading interface defined in 
 *            VmCommon/h/loader.h.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <KVM.h>
#include <Crc.h>

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

static Boolean
findClassFromDatabase(DmOpenRef db, const char* name,
                      VoidHand *recHandleP, UIntPtr recNumP);

static Boolean
findClassFromResource(DmOpenRef db, const char* name, ULong resourceType,
                      VoidHand *recHandleP, UIntPtr resIDP);

static FILEPOINTER
openFileInternal(const char *fileName, bool_t isClass);

static void
classFileCleanup(INSTANCE instance);

/*=========================================================================
 * Functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      loadByte(), loadShort(), loadCell()
 * TYPE:          private class file reading operations
 * OVERVIEW:      Read the next 1, 2 or 4 bytes from the
 *                given class file.
 * INTERFACE:
 *   parameters:  classfile pointer
 *   returns:     unsigned char, short or integer
 * NOTE:          For safety it might be a good idea to check
 *                explicitly for EOF in these operations.
 *=======================================================================*/

/* This structure is used for referring to open "files" when
 * loading classfiles (records) from PalmPilot databases.
 * It replaces the standard FILE* structure used in C/C++.
 */

/* FILEPOINTER */
struct filePointerStruct {
    char*   filePointer;        /* Pointer to current location in classfile */
    char*   endPointer;
    VoidHand recordHandle;      /* Handle for the database object */
    Boolean isResource;         /* Is this a resource or a database? */
    UInt16 resourceID;          /* remember ID of this resource (for
                                 * use by optimized Palm class loader) */
};

unsigned char
loadByte(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    unsigned char c;
    c = *ClassFile->filePointer;
    ClassFile->filePointer++;
    return c;
}

int
loadByteNoEOFCheck(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    if (ClassFile->filePointer == ClassFile->endPointer) {
        return -1;
    } else {
        unsigned char c = (unsigned char)*ClassFile->filePointer++;
        return c;
    }
}

unsigned short
loadShort(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    unsigned short c;
    unsigned char c1;
    unsigned char c2;


    if (ClassFile->filePointer + 2 > ClassFile->endPointer) {
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    }
    c1 = ClassFile->filePointer[0];
    c2 = ClassFile->filePointer[1];
    ClassFile->filePointer += 2;
    c  = c1 << 8 | c2;
    return c;
}

unsigned long
loadCell(FILEPOINTER_HANDLE ClassFileH)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    unsigned long c;
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;
    unsigned char c4;

    if (ClassFile->filePointer + 4 > ClassFile->endPointer) {
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    }
    c1 = ClassFile->filePointer[0];
    c2 = ClassFile->filePointer[1];
    c3 = ClassFile->filePointer[2];
    c4 = ClassFile->filePointer[3];
    ClassFile->filePointer += 4;
    c  = (long)c1 << 24 | (long)c2 << 16 | (long)c3 << 8 | (long)c4;
    return c;
}

void skipBytes(FILEPOINTER_HANDLE ClassFileH, unsigned int i)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    if (ClassFile->filePointer + i > ClassFile->endPointer) {
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    }
    ClassFile->filePointer += i;
}

void
loadBytes(FILEPOINTER_HANDLE ClassFileH, char *buffer, int length)
{
    FILEPOINTER ClassFile = unhand(ClassFileH);
    if (ClassFile->filePointer + length > ClassFile->endPointer) {
        raiseExceptionCharMsg(ClassFormatError,
                KVM_MSG_CLASSFILE_SIZE_DOES_NOT_MATCH);
    }
    memcpy(buffer, ClassFile->filePointer, length);
    ClassFile->filePointer += length;
}

/*=========================================================================
 * FUNCTION:      openClassfile()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Open a class file for loading.
 * INTERFACE:
 *   parameters:  properly formatted classfile name string
 *   returns:     File pointer referring to the classfile record,
 *                NIL if class was not found.
 *=======================================================================*/

FILEPOINTER
openClassfile(INSTANCE_CLASS clazz)
{
    FILEPOINTER result;
    START_TEMPORARY_ROOTS
    DECLARE_TEMPORARY_ROOT(char *, className, getClassName((CLASS)clazz));
        IS_TEMPORARY_ROOT(result, openFileInternal(className, true));
        registerCleanup((INSTANCE_HANDLE)&result, classFileCleanup);
    END_TEMPORARY_ROOTS
    return result;
}

/*=========================================================================
 * FUNCTION:      openResourcefile()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Open a >>Java<< resource file for loading.
 * INTERFACE:
 *   parameters:  properly formatted resource file name string
 *   returns:     File pointer referring to the resource file record,
 *                NIL if resource was not found.
 *
 * Note: Don't be confused.  "Resource" here means a Java resource. Java
 * resources,  like classes, can be read of the resource database.
 * Where classes have the resource type 'Clas' in the database, Java
 * resources have the resource type 'Rsrc'.
 *=======================================================================*/

FILEPOINTER foo;

FILEPOINTER openResourcefile(char* fileName)
{
    return openFileInternal(fileName, false);
}

static FILEPOINTER
openFileInternal(const char *fileName, bool_t isClass)
{
    VoidHand recordHandle;
    VoidPtr recPtr;
    FILEPOINTER ClassFile;
    Boolean isResource = true;
    UInt16 resourceID;

#if INCLUDEDEBUGCODE
    if (traceclassloadingverbose) {
        fprintf(stdout,"Opening '%s'\n", fileName);
    }
#endif /* INCLUDEDEBUGCODE */

    if (!findClassFromResource(ClassfileDB, fileName,
                               (isClass ? 'Clas' : 'Rsrc'),
                               &recordHandle, &resourceID)) {
        isResource = false;
        if (!isClass) {
            return NULL;
        }
        if (!findClassFromDatabase(ClassfileAltDB, fileName, 
                                   &recordHandle, NULL)) {
            return NULL;
        }
    }

    /*  Allocate an internal file pointer structure */
    ClassFile = (FILEPOINTER)mallocObject(SIZEOF_FILEPOINTER, GCT_NOPOINTERS);

    /* Get the record and a pointer to the contents */
    recPtr = MemHandleLock(recordHandle);
    /* Double-check that there is enough memory to load the class */
    if (recPtr == NIL) {
        fatalVMError("Not enough memory to load class");
    }
    ClassFile->recordHandle = recordHandle;
    ClassFile->filePointer =
     (char *)recPtr + StrLen((CharPtr)recPtr) + (isClass ? 5 : 1);
    ClassFile->endPointer = (char *)recPtr + MemPtrSize(recPtr);
    ClassFile->isResource = isResource;
    ClassFile->resourceID = resourceID;

    return ClassFile;
}

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
    if (traceclassloadingverbose)
        fprintf(stdout,"Closing classfile\n");
#endif /* INCLUDEDEBUGCODE */

    /*  Release the handle referring to the classfile */
    MemHandleUnlock(ClassFile->recordHandle);
    if (ClassFile->isResource) {
        DmReleaseResource(ClassFile->recordHandle);
    }
    ClassFile->recordHandle = NULL;
}

/*=========================================================================
 * FUNCTION:      classFileCleanup()
 * TYPE:          private class file load operation
 * OVERVIEW:      Close the given classfile during KVM cleanup. loader.c
 *                may have failed to close the classfile record if
 *                it encountered a fatalError() during class loading.
 * INTERFACE:
 *   parameters:  object instance pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void
classFileCleanup(INSTANCE instance)
{
    FILEPOINTER ClassFile = (FILEPOINTER)instance;

    if (ClassFile->recordHandle == NULL) {
        /* Already closed explicitly using closeClassfile() */
        return;
    } else {
        MemHandleUnlock(ClassFile->recordHandle);
        if (ClassFile->isResource) {
            DmReleaseResource(ClassFile->recordHandle);
        }
        ClassFile->recordHandle = NULL;
    }
}

/*=========================================================================
 * FUNCTION:      InitializeClassLoading()
 *                FinalizeClassLoading()
 * TYPE:          private class file loading operation
 * OVERVIEW:      Initialize and finalize the Palm-specific
 *                class loader. We don't actually have to do
 *                anything special on the Palm.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeClassLoading()
{ }

void FinalizeClassLoading()
{}

/*=========================================================================
 * FUNCTION:      findClassFromDatabase()
 * OVERVIEW:      Given a string, find the entry in the database that
 *                starts with this string. This can be used to find classes
 *                or any other entry that is identified by a string. The index
 *                of the first matching entry will be returned.
 * INTERFACE:
 *   parameters:  a null terminated string that identifies and entry
 *   returns:     record number or -1 if no entry was found with the
 *                given name.
 *=======================================================================*/

static Boolean
findClassFromDatabase(DmOpenRef db, const char* name,
                      VoidHand *recHandleP, UIntPtr recNumP)
{
    int low, high, current, compare;
    VoidHand recHandle;
    
    if (db == NULL) return false;

    low = 0;
    high = DmNumRecords(db) - 1;
    
    while (low <= high) {
        current = (low + high) / 2;
        
        recHandle = DmQueryRecord(db, current);   
        compare = StrCompare((CharPtr)MemHandleLock(recHandle), name);
        MemHandleUnlock(recHandle);
        
        if (compare < 0) low = current + 1;
        else if (compare > 0) high = current - 1;
        else {
            if (recHandleP) *recHandleP = recHandle;
            if (recNumP) *recNumP = high;
            return true;
       } 
    }
    
    return false;
}

static Boolean
findClassFromResource(DmOpenRef db, const char* name, ULong resourceType,
              VoidHand *recHandleP, UIntPtr resIDP) {
    UInt nameLength = StrLen(name);
    /*  Create a hash value from the name */
    UShort resourceID = Crc16CalcBlock((VoidPtr)name, nameLength, 0);
    for (; ; resourceID += nameLength) {
        Int index = DmFindResource(db, resourceType, resourceID, NULL);
        if (index >= 0) {
            VoidHand recHandle = DmGetResourceIndex(db, index);
            int compare = StrCompare(name, MemHandleLock(recHandle));
            MemHandleUnlock(recHandle);
            /* If they don't match, there is a rare chance of a collision */
            if (compare == 0) {
                if (recHandleP) {
                    /* The caller must release the resource if s/he
                     * doesn't want it */
                    *recHandleP = recHandle;
                    *resIDP = resourceID;
                } else {
                    DmReleaseResource(recHandle);
                }
                return true;
            }
        } else {
            return false;
        }
    }
}

