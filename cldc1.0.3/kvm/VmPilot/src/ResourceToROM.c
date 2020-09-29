/*
 *  Copyright (c) 200 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Platform-specific code
 * FILE:      ResourceToROM.c
 * OVERVIEW:  This file takes "jROM" resources associated with a
 *            Palm program, reads them in, and relocates them
 *            as necessary.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

/**
 * This file takes "jROM" resources associated with a PALM program, reads
 * them in, and relocates them.
 *
 * @author  Frank Yellin
 * @version 1.0 1/13/2000
 */

#include <global.h>
#include <rom.h>

#ifndef PILOT

/* These definitions are >>purely<< to allow us to find bugs in the
 * relocation code.  It is not intended that this code actually run on
 * any platform other than the Palm Pilot.
 */
#define MemHandleLock(x) (void *)x
#define MemHandleUnlock(x)  (void)x

#define MemPtrNew(x) malloc(x)
#define MemPtrFree(x) free(x)
#define MemHandleFree(x) free(x)
#define DmGetResource(type, int) UnixGetResource(int)
void *GetResource(int number);

#define MemPtrRecoverHandle(x) ((void*)(x))
#define MemHeapCompact(x) 

static void DmWrite(void *image, int offset, void *src, int size);

#define DmReleaseResource(x)
#define DmNextOpenResDatabase(x) NULL
#define DmNewHandle(x, size)  ((void)x, malloc(size))

typedef void*  DmOpenRef;
typedef void** VoidHand;

static void *UnixGetResource(int number);

#endif /* PILOT */

/* These are the variables that are normally declared in ROMjava.c.  For
 * simplicity, we declare them here.  
 */

#define ArraySize(x) (sizeof(x)/sizeof(x[0]))

HASHTABLE OldInternStringTable;
HASHTABLE OldClassTable;
HASHTABLE OldUTFStringTable;

HASHTABLE InternStringTable;
HASHTABLE ClassTable;
HASHTABLE UTFStringTable;

long *KVM_masterStaticDataPtr;
long *KVM_staticDataPtr;
long KVM_staticSize;

INSTANCE_CLASS JavaLangObject;
INSTANCE_CLASS JavaLangClass; 
INSTANCE_CLASS JavaLangString; 
INSTANCE_CLASS JavaLangThread; 
INSTANCE_CLASS JavaLangSystem; 
INSTANCE_CLASS JavaLangThrowable; 
INSTANCE_CLASS JavaLangError; 
ARRAY_CLASS    PrimitiveArrayClasses[T_LASTPRIMITIVETYPE + 1];

METHOD RunClinitMethod;
METHOD RunCustomCodeMethod;
NameTypeKey initNameAndType;
NameTypeKey clinitNameAndType; 
NameTypeKey runNameAndType;
NameTypeKey mainNameAndType;

extern long NativeRelocationCount;

/* This is information that we need to know about each resource in order
 * to read it in, relocate it, and process it.
 * An array of these is kept as part of the "environment".
 */

struct relocInfoType { 
    void *targetImage;          /* address of the resource */
    void *targetImageEnd;       /* address just beyond the end of the resource */

    void *currentImage;         /* if we've created a temporary copy of the
                                 * resource this contains that address.  
                                 * Otherwise, it is the same as targetAddress 
                                 */

    long postDelta;             /* Holds:  targetImage - <original address> */
    long copyInStatic:1;        /* Bit indicating copy is in static */
    long readOnly:1;            /* bit indicating we shouldn't modify this. */
};

/* This array holds the pointer address of all the resources that have been
 * read in. 
 */

void **ROMResources;

/* 
 * This structure is used for holding information during the location.
 * It contains relocation information about each of the resources, and the
 * contents of the first resource
 */

typedef struct environmentStruct { 
    ROMTableOfContentsPtr toc;    /* same as ROMResources[TOCResource] */
    bool_t                someResourceMoved;
    int                   firstMethodResource;
    int                   firstCodeResource;
    int                   unknownResource;
    int                   resourceCount;
    struct relocInfoType  relocInfo[1]; /* actually, one for each array */
} environmentStruct, *environmentPtr;


/* NOTE:
 *
 * Unless otherwise noted, all pointers are the address of the pointer
 * in the "targetImage", i.e. the address at which the resource is currently
 * located.  
 *
 * The macro UPDATE is used to access and modify a pointer.  It relocates 
 * a pointer from being a pointer in "old space" into a pointer in "new space".
 */

        
/* This macro updates a pointer.  The address of the pointer is in the
 * "from" resource.  The target of the pointer is either NULL, or in the
 * "to" resource.
 */

#define UPDATE(pointer, from, to) \
     updatePointer(env, &pointer, from ## Resource, to ## Resource)

#define findMethodResource(ptr) \
     findResource(env, ptr, env->firstMethodResource, env->firstCodeResource)

#define findCodeResource(ptr) \
     findResource(env, ptr, env->firstCodeResource, env->unknownResource)

#define isCodeResource(resource) \
     ((resource) >= env->firstCodeResource && (resource) < env->unknownResource)

/* Get the value of a non-pointer variable.  The resource indicates in which
 * resource the variable is located. 
 */

/* Change the value of the indicated location to the new value */
#define SET_VALUE(location, resource, newValuePtr) \
      setValue(env, &location, newValuePtr, sizeof(location), resource ## Resource)

#define SET_VALUE_IF(location, resource, newValuePtr) \
      if (location != *(newValuePtr)) SET_VALUE(location, resource, newValuePtr)


/* Update a resource's alignment, if necessary */
static void fixupAlignment(environmentPtr, ROMResourceType);

/* Initialize the relocation information for a resource */
static void setRelocationInfo(environmentPtr env, ROMResourceType thisResource);

/* Perform the actual relocation */
static void performRelocation(environmentPtr);

/* Relocate all the native methods */
static void 
relocateNativeMethods(environmentPtr env);

static int findResource(environmentPtr, void*, int, int);

/* Update a pointer, and return its new value */
static void *
updatePointer(environmentPtr, void *, ROMResourceType, ROMResourceType);

/* A relocation function is used to update each element of a hash table */
typedef void (*relocationFunction)(environmentPtr, void *object, bool_t first);

/* Update a hashtable, and call the relocation function on each of its 
 * entries 
 */
static void relocateHashTable(environmentPtr, HASHTABLE, ROMResourceType, 
                              int nextOffset, relocationFunction);

static HASHTABLE copyHashTable(HASHTABLE);

/* A relocation function used to update the strings in the InternString table 
 */
static void relocateInternString(environmentPtr, void *, bool_t first);

/* A relocation function used to update all the fields in a class */
static void relocateClass(environmentPtr, void *, bool_t first);
/* A relocation function used to update the fields in a class when non
 * of the resources have moved.  We only need to fix static fields. */
static void relocateClassQuick(environmentPtr, void *, bool_t first);

/* Relocate a single constant pool */
static void relocateConstantPool(environmentPtr, CONSTANTPOOL) ;
/* Relocate a single field table */
static void relocateFieldTable(environmentPtr, FIELDTABLE, bool_t);
/* Relocate a single method table */
static void relocateMethodTable(environmentPtr, METHODTABLE, int);

static void
setValue(environmentPtr env, 
         void *targetAddress, void *newValuePtr, int newValueSize,
         ROMResourceType fromResource);

/*=========================================================================
 * FUNCTION:      createROMImage();
 * INTERFACE 
 *   parameters:  none
 *   returns:     none
 * 
 * Loads in the resources representing each of the resources of the preloaded
 * classes.  Relocate them and modify them as necessary. 
 *=======================================================================*/

void
CreateROMImage()
{
    environmentPtr env;
    ROMTableOfContentsPtr toc = 
        MemHandleLock(DmGetResource('jROM', 1000 + TOCResource));
    int firstMethodResource = StaticDataResource + 1;
    int firstCodeResource = firstMethodResource + toc->methodTableResourceCount;
    int unknownResource = firstCodeResource + toc->codeResourceCount;
    int resourceCount = unknownResource + 1;
    int environmentSize = offsetof(environmentStruct, relocInfo) + 
                              resourceCount * sizeof(env->relocInfo[0]);
    int thisResource;

    env = MemPtrNew(environmentSize);
    memset(env, 0, environmentSize);

    env->toc = toc;
    env->firstMethodResource = firstMethodResource;
    env->firstCodeResource =  firstCodeResource;
    env->unknownResource = unknownResource;
    env->resourceCount = resourceCount;

    ROMResources = MemPtrNew(sizeof(void **) * resourceCount);
    
    ROMResources[TOCResource] = toc;
    ROMResources[unknownResource] = NULL;

    for (thisResource = 1; thisResource < env->resourceCount; thisResource++) { 
        if (thisResource == StaticDataResource) { 
            /* This resource is special.  It is initialized from the
             * contents of MasterStaticDataResource, and needs to remain
             * in dynamic memory */
            int size = env->toc->resources[thisResource].u.size;
            ROMResources[thisResource] = MemPtrNew(size);
        } else if (env->toc->resources[thisResource].startAddress != NULL) { 
            /* Read in the resource corresponding to the resource, and
             * unlock it. */
            VoidHand handle = DmGetResource('jROM', 1000 + thisResource); 
            ROMResources[thisResource] = 
            (handle == NULL) ? NULL : MemHandleLock(handle);
       } else { 
            /* Resource that is not yet used */
            ROMResources[thisResource] = NULL;
       }
    }

    /* If the former address of this resource is 0 mod 4, then this resource
     *   has two unused bytes at the end.
     * If the former address of this resource is 2 mod 4, then this resource
     *   has two unused bytes at the beginning.
     * If the address has changed from 0 mod 4 to 2 mod 4, or vice versa,
     *   then move the unused bytes from one end to the other, so that 
     *   the "real" data always starts on a 0 mod 4 boundary.
     */
    for (thisResource = 0; thisResource < env->resourceCount; thisResource++) {
        switch (thisResource) { 
        case TOCResource: case StaticDataResource: case MasterStaticDataResource:
            /* do nothing */
            break;
    
        default:
            if (ROMResources[thisResource] != NULL) { 
                fixupAlignment(env, thisResource);
            }
            break;
        }
    }
    
    /* Fill in the env structure for each resource. 
     */
    for (thisResource = 0; thisResource < env->resourceCount; thisResource++) { 
        setRelocationInfo(env, thisResource);
    }

    performRelocation(env);

    /* BEGIN CRITICAL SECTION
     *
     * It is import that either >>all<< of the following writes get executed
     * or that none of them do.  Otherwise, the system will be in an 
     * inconsistent state, and the image will probably never be able to run
     * again.
     */
    for (thisResource = 0; thisResource < env->resourceCount; thisResource++) {
        struct relocInfoType *relocInfo = &env->relocInfo[thisResource];
        void *targetImage = relocInfo->targetImage;
        void *currentImage = relocInfo->currentImage;
        if (targetImage != currentImage && thisResource != StaticDataResource) {
            /* The image has been modified */
            long size = env->toc->resources[thisResource].u.size;
            DmWrite(targetImage, 0, relocInfo->currentImage, size);
        }
    }
    /* END CRITICAL SECTION */

    /* The following code could have been merged into the above loop,
     * but we want to shorten, as much as possible, the time that the system
     * is in an inconsistent state 
     */
    for (thisResource = 0; thisResource < env->resourceCount; thisResource++) {
        struct relocInfoType *relocInfo = &env->relocInfo[thisResource];
        void *targetImage = relocInfo->targetImage;
        void *currentImage = relocInfo->currentImage;
        if (thisResource == StaticDataResource) {
            /* it needs to be zero before GC is set up */
            long size = env->toc->resources[thisResource].u.size;
            memset(relocInfo->targetImage, 0, size);
        } else if (targetImage == currentImage) { 
            /* Item was never relocated */
        } else { 
            if (relocInfo->copyInStatic) { 
                VoidHand currentHandle = MemPtrRecoverHandle(currentImage);
                MemHandleUnlock(currentHandle);
                MemHandleFree(currentHandle);
            } else { 
                MemPtrFree(relocInfo->currentImage);
            }
        }
    }

    /* We'd put these inside performRelocation, but since then they'd end
     * up fragmenting the heap.
     */
    MemPtrFree(env);
    MemHeapCompact(0);
    UTFStringTable    = copyHashTable(OldUTFStringTable);
    InternStringTable = copyHashTable(OldInternStringTable);
    ClassTable        = copyHashTable(OldClassTable);

    { 
        ROMTableOfContentsPtr toc = ROMResources[TOCResource];
        long newFlags = toc->flags | ROM_FLAGS_DIRTY;
        DmWrite(toc, offsetof(struct ROMTableOfContentsType, flags), 
                &newFlags, sizeof(long));
    }

}

/* If the former address of this resource is 0 mod 4, then this resource
 *      has two unused bytes at the end.
 * If the former address of this resource is 2 mod 4, then this resource
 *     has two unused bytes at the beginning.
 * If the address has changed from 0 mod 4 to 2 mod 4, or vice versa,
 *     then move the unused bytes from one end to the other, so that 
 *     the "real" data always starts on a 0 mod 4 boundary.
 * 
 * Note that we do the cleanup by moving the bytes in the targetImage, and
 * then modifying the information in the table of contents, so that it
 * represents the updated address of the low byte in the target image.
 */
static void
fixupAlignment(environmentPtr env, ROMResourceType thisResource)
{ 
    /* The old location of the image */
    void *oldImage = env->toc->resources[thisResource].startAddress;
    /* The current location of the image */
    void *targetImage = ROMResources[thisResource];

    int misalignment = ((long)targetImage  & 3) - ((long)oldImage & 3);
    
    if (misalignment != 0) { 
        /* Each block has two extra bytes, either at the beginning or at the
         * end.  So we shift the contents of targetImage so that they are
         * again aligned. */
        long dataSize = env->toc->resources[thisResource].u.size - 2;
        void *copy = MemPtrNew(dataSize);
        if (misalignment == 2) { 
            /* Old image was aligned, but current image isn't.  Move the
             * two junk bytes from the end to the beginning */
            memcpy(copy, targetImage, dataSize);
            DmWrite(targetImage, 2, copy, dataSize);
            /* The low byte of targetImage now represents this address */
            oldImage = (char *)oldImage - 2;
        } else if (misalignment == -2) { 
            /* Old image wasn't aligned, but the current image is.  Move
             * the junk bytes from the low end of the target to the end */
            memcpy(copy, (char *)targetImage + 2, dataSize);
            DmWrite(targetImage, 0, copy, dataSize);
            oldImage = (char *)oldImage + 2;
        } else { 
            fatalError("Unexpected misalignment");
        }
        MemPtrFree(copy);
        /* Update the table of contents */
        DmWrite(env->toc, offsetof(ROMTableOfContentsType, 
                                   resources[thisResource].startAddress), 
                                   &oldImage, sizeof(oldImage));
    }
}

/* Fill in the env structure for each resource.
 */

static void
setRelocationInfo(environmentPtr env, ROMResourceType thisResource) { 
    /* Where the image will finally end up. */
    void *targetImage = ROMResources[thisResource];
    /* The location of the image the last time that it ran */
    void *oldImage = env->toc->resources[thisResource].startAddress;
    /* The size of the image */
    long size = env->toc->resources[thisResource].u.size;
    /* A pointer to the structure that we will modify */
    struct relocInfoType *relocInfo = &env->relocInfo[thisResource];

    if (targetImage != NULL) { 
#ifdef PILOT
        if (size != MemPtrSize(targetImage)) { 
            sprintf(str_buffer, "Sizes do not match:  %ld != %ld", 
                size, (long)MemPtrSize(targetImage));
            fatalError(str_buffer);
        }
#endif
    } else { 
        size = 0;
    }

    relocInfo->currentImage = targetImage; /* no copy made yet */
    relocInfo->targetImage =  targetImage;
    relocInfo->targetImageEnd = (char *)targetImage + size;
    relocInfo->postDelta = (char *)targetImage - (char *)oldImage;
    relocInfo->readOnly = (thisResource == StaticDataResource || 
                           isCodeResource(thisResource));
    
    if (oldImage != targetImage) { 
        /* Change the table of contents to reflect the new address */
        UPDATE(env->toc->resources[thisResource].startAddress, TOC, this);
        if (thisResource != StaticDataResource) { 
            env->someResourceMoved = TRUE;
        }
    }
}

/* Perform all the relocations that need to be done, and get the values of
 * all the variables. 
 */

static void
performRelocation(environmentPtr env) { 
    int i;
    ROMTableOfContentsPtr toc = env->toc;
    ARRAY_CLASS *primitivePtr;
    
    /* keyValues[] contains the addresses of many key variables 
     * in the old image.  We can get these variables new values (and
     * update the table of contents at the same time!) but calling
     * UPDATE() on them.
     */
    

    /* Copies of the hash tables that live in static space */
    OldUTFStringTable =      UPDATE(toc->hashtables[0], TOC, UTF);
    OldInternStringTable =   UPDATE(toc->hashtables[1], TOC, String);
    OldClassTable =          UPDATE(toc->hashtables[2], TOC, ClassDefs);

    /* Important classes */
    JavaLangObject =      UPDATE(toc->classes[0],   TOC, ClassDefs);
    JavaLangClass  =      UPDATE(toc->classes[1],   TOC, ClassDefs);
    JavaLangString =      UPDATE(toc->classes[2],   TOC, ClassDefs);
    JavaLangThread =      UPDATE(toc->classes[3],   TOC, ClassDefs);
    JavaLangSystem =      UPDATE(toc->classes[4],   TOC, ClassDefs); 
    JavaLangThrowable =   UPDATE(toc->classes[5],   TOC, ClassDefs); 
    JavaLangError  =      UPDATE(toc->classes[6],   TOC, ClassDefs); 
    primitivePtr   =      UPDATE(toc->arrayClasses, TOC, ClassDefs); 

    for (i = 0; i < ArraySize(PrimitiveArrayClasses); i++) { 
        PrimitiveArrayClasses[i] = UPDATE(primitivePtr[i], ClassDefs, ClassDefs);
    }

    /* Static space, and an original copy of it */
    KVM_masterStaticDataPtr = ROMResources[MasterStaticDataResource];
    KVM_staticDataPtr       = ROMResources[StaticDataResource];
    KVM_staticSize =          env->toc->resources[StaticDataResource].u.size;

    /* We can update all the remaining pointers by updating three important
     * hashtables.  As a side effect, the function relocateHashTable()
     * creates a copy of the hashtable in dynamic memory 
     */

    if (env->someResourceMoved || ((toc->flags & ROM_FLAGS_DIRTY) != 0)) { 
        relocateHashTable(env, OldUTFStringTable, UTFResource,
                          offsetof(struct UTF_Hash_Entry, next), 
                          NULL);
    
        relocateHashTable(env, OldInternStringTable, StringResource,
                          offsetof(struct internedStringInstanceStruct, next), 
                          relocateInternString);
    
        relocateHashTable(env, OldClassTable, ClassDefsResource,
                          offsetof(struct classStruct, next), 
                          relocateClass);

    } else { 
        /* If no resources have moved, we only need to fix up pointers from
         * static data into the dynamic memory allocated for StaticDataResource
         */
        relocateHashTable(env, OldClassTable, ClassDefsResource,
                          offsetof(struct classStruct, next), 
                          relocateClassQuick);
    }

    relocateNativeMethods(env);
}

/* This function relocates a hashtable, and then calls the relocation
 * function on each entry in the hashtable.
 * 
 * This function returns a copy of the hashtable in dynamic memory.
 */

static void
relocateHashTable(environmentPtr env,
                  HASHTABLE table, 
                  ROMResourceType thisTableResource,
                  int nextOffset, 
                  relocationFunction functor)
{
    long bucketCount = table->bucketCount;
    int i;
    bool_t first = TRUE;
    for (i = 0; i < bucketCount; i++) { 
        /* Get the first pointer. */
        void *ptr = UPDATE(table->bucket[i], thisTable, thisTable) ;
        while (ptr != NULL) { 
            /* Get the address of the next pointer */
            void **nextPtrAddress = (void **)((char *)ptr + nextOffset);
            /* Call the relocation function to update the element. */
            if (functor != NULL) { 
                functor(env, ptr, first);
                first = FALSE;
            }
            /* Move on to the next element of this bucket */
            ptr = UPDATE(*nextPtrAddress, thisTable, thisTable);
        }
    }
}

/* Make a copy of a hashtable in dynamic memory. */

static HASHTABLE
copyHashTable(HASHTABLE table) 
{
    long bucketCount = table->bucketCount;

    /* The full size of the hashtable */
    long sizeInBytes = SIZEOF_HASHTABLE(bucketCount) << log2CELL;

    /* The copy of the hashtable, in dynamic memory, that we will return. */
    HASHTABLE result = (HASHTABLE)MemPtrNew(sizeInBytes);

    /* Copy the final result into the dynamic-memory copy, and return it */
    memcpy(result, table, sizeInBytes);
    return result;
}

/*  Relocate a string in the Intern string table */

void 
relocateInternString(environmentPtr env, void *object, bool_t first) 
{ 
    INTERNED_STRING_INSTANCE string = object;
    void * null = NULL;
    /* Update the INSTANCE_STRING's "array"  and "class" field */
    SHORTARRAY array  = UPDATE(string->array, String, String);
    INSTANCE_CLASS class = UPDATE(string->ofClass, String, ClassDefs);
    if (class != JavaLangString) { 
        fatalError("Bad class in string");
    }
    SET_VALUE_IF(string->mhc.address, String, &null);
    if (first) { 
        /* All the interned strings share a single underlying character
         * array.  We need to update this character array once.
         */
        ARRAY_CLASS aclass = UPDATE(array->ofClass, String, ClassDefs);
        if (aclass != PrimitiveArrayClasses[T_CHAR]) { 
            fatalError("Bad class in character array");
        }
        SET_VALUE_IF(array->mhc.address, String, &null);
    }
}   

/*  Relocate a class and all the stuff hanging off it */

void 
relocateClass(environmentPtr env, void *object, bool_t first) 
{
    CLASS clazz = object;
    void * null = NULL;
    /* Update all the pointers that hang off every class */
    UString packageName = UPDATE(clazz->packageName, ClassDefs, UTF);
    UString baseName = UPDATE(clazz->baseName, ClassDefs, UTF);
    UPDATE(clazz->ofClass, ClassDefs, ClassDefs);

    if (packageName != NULL) { 
        sprintf(str_buffer, "%s/%s",  
            UStringInfo(packageName), UStringInfo(baseName));
    } else { 
        sprintf(str_buffer, "%s", UStringInfo(baseName));
    }

    SET_VALUE_IF(clazz->mhc.address, ClassDefs, &null);

    if (!IS_ARRAY_CLASS(clazz)) { 
        /* Update all the pointers that hang off an instance class */
        INSTANCE_CLASS iclazz = (INSTANCE_CLASS)clazz;
        CONSTANTPOOL pool = UPDATE(iclazz->constPool, ClassDefs, ConstantPools);
        FIELDTABLE fieldTable = UPDATE(iclazz->fieldTable, ClassDefs, Fields);
        int methodsResource = findMethodResource(iclazz->methodTable);
        int unknownResource = env->unknownResource;
        short expectedStatus = 
            (iclazz->clazz.accessFlags & ACC_ROM_NON_INIT_CLASS) ? 
                   CLASS_READY : CLASS_VERIFIED;
        METHODTABLE methodTable = 
            UPDATE(iclazz->methodTable, ClassDefs, methods);
        UPDATE(iclazz->ifaceTable,   ClassDefs,  Interfaces);
        UPDATE(iclazz->staticFields, ClassDefs,  unknown); 
        UPDATE(iclazz->superClass,   ClassDefs,  ClassDefs);

        /* Perform additional relocation on those parts that have additional
         * pointers hanging off of them */
        if (pool != NULL) { 
            relocateConstantPool(env, pool);
        }
        if (fieldTable != NULL) {
            relocateFieldTable(env, fieldTable, TRUE);
        } 
        if (methodTable != NULL) { 
            relocateMethodTable(env, methodTable, methodsResource);
        }

        SET_VALUE_IF(iclazz->initThread, ClassDefs, &null);
        SET_VALUE_IF(iclazz->status, ClassDefs, &expectedStatus);
    } else { 
        /* Update all the pointers that hang of an array class */
        ARRAY_CLASS aclazz = (ARRAY_CLASS)clazz;
        if (aclazz->gcType == GCT_OBJECTARRAY) { 
            UPDATE(aclazz->u.elemClass, ClassDefs, ClassDefs);
        }
    }
}

void 
relocateClassQuick(environmentPtr env, void *object, bool_t first) 
{
    CLASS clazz = object;
    if (!IS_ARRAY_CLASS(clazz)) { 
        INSTANCE_CLASS iclazz = (INSTANCE_CLASS)clazz;
        FIELDTABLE fieldTable = iclazz->fieldTable;

        if (fieldTable != NULL) {
            relocateFieldTable(env, fieldTable, FALSE);
        } 
    }
}

/* Relocate all pointer entries in a constant pool */

static void relocateConstantPool(environmentPtr env, CONSTANTPOOL pool) 
{ 
    long length = CONSTANTPOOL_LENGTH(pool);
    unsigned char *tags = CONSTANTPOOL_TAGS(pool);
    int i;
    for (i = 1; i < length; i++) { 
        unsigned char type = tags[i];
        switch(type) { 
        case CONSTANT_Integer:
        case CONSTANT_Float:
            break;
            
        case CONSTANT_Long:
        case CONSTANT_Double:
            i++; break;
            
        case CONSTANT_String:
            UPDATE(pool->entries[i], ConstantPools, String);
            break;
            
        case CONSTANT_Class | CP_CACHEBIT:
            UPDATE(pool->entries[i], ConstantPools, ClassDefs);
            break;
            
        case CONSTANT_Fieldref | CP_CACHEBIT:
            UPDATE(pool->entries[i], ConstantPools, Fields);
            break;
            
        case CONSTANT_Methodref | CP_CACHEBIT:
        case CONSTANT_InterfaceMethodref | CP_CACHEBIT: {
            METHOD m = (METHOD)pool->entries[i].cache;
            int methodsResource = findMethodResource(m);
            UPDATE(pool->entries[i], ConstantPools, methods);
            break;
        }
            
        default:
            fprintf(stderr, "Unknown constant pool type %x\n", type);
            break;
        }
    }
}

/* Relocate all pointer entries in a field table */

static void 
relocateFieldTable(environmentPtr env, FIELDTABLE fieldTable, bool_t full)
{
    long length = fieldTable->length;
    FIELD field = fieldTable->fields;
    FIELD lastField = field + length;
    for ( ; field < lastField; field++) { 
        long flags = field->accessFlags;

        if (full) { 
            UPDATE(field->ofClass, Fields, ClassDefs);
        }
        if ((flags & ACC_STATIC) != 0) { 
            /* If it's static, we need to update the pointer. */
            void *staticAddr = 
                UPDATE(field->u.staticAddress, Fields, StaticData);
            if (staticAddr == NULL) { 
                fatalError("Huh?");
            } 
            if (full && ((flags & ACC_POINTER) != 0)) { 
                /* We also need to update the "master" copy of the variable
                 * that is sitting in master static data */
                long delta = (char*)staticAddr - (char*)KVM_staticDataPtr;
                void *masterAddr = (char *)KVM_masterStaticDataPtr + delta;
                UPDATE(*(void **)masterAddr, MasterStaticData, String);
            }
        }
    }
}

/* Relocate all the pointers in a method table */

static void relocateMethodTable(environmentPtr env, 
                                METHODTABLE methodTable, int methodsResource) 
{
    long length = methodTable->length;
    METHOD method = methodTable->methods; 
    METHOD lastMethod = method + length;
    for ( ; method < lastMethod; method++) { 
        UPDATE(method->ofClass, methods,  ClassDefs);
        if (!(method->accessFlags & ACC_NATIVE)) { 
            int codeResource = findCodeResource(method->u.java.code);
            UPDATE(method->u.java.code,      methods, code);
            UPDATE(method->u.java.handlers,  methods, Handlers);
            UPDATE(method->u.java.stackMaps.pointerMap, 
                   methods, Stackmaps);
        }
    }
}

static void 
relocateNativeMethods(environmentPtr env)
{ 
    struct NativeRelocationStruct *ptr = NativeRelocations;
    struct NativeRelocationStruct *end = ptr + NativeRelocationCount;

    for ( ; ptr < end; ptr++) { 
        int methodResource = ptr->resource + (env->firstMethodResource - 1);
        void *targetImage = env->relocInfo[methodResource].targetImage;
        void *table = (void *)(((long)targetImage + 2) & ~0x2);
        METHOD method = (METHOD)((char *)table + ptr->offset);
        if (!(method->accessFlags & ACC_NATIVE)) { 
            fatalError("Expected native method");
        } else { 
            SET_VALUE(method->u.native.code, method, &(ptr->function));
        }
    }
}

/* Reinitialize the virtual machine */

void InitializeROMImage() { 
    memcpy(KVM_staticDataPtr, KVM_masterStaticDataPtr, KVM_staticSize);
}

/* Perform any cleanup required of the virtual machine. 
 * Note that this is the simplest way of cleaning up the hash tables 
 */
void FinalizeROMImage() { 
#ifdef PILOT
    memcpy(UTFStringTable,    OldUTFStringTable,    MemPtrSize(UTFStringTable));
    memcpy(InternStringTable, OldInternStringTable, MemPtrSize(InternStringTable));
    memcpy(ClassTable,        OldClassTable,        MemPtrSize(ClassTable));
#endif
}

/* Indicate that we are completely done with the preloaded class tables, and
 * we are ready to return all the resources back to the system.
 */

void DestroyROMImage() { 
    ROMResourceType resource;
    
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    long newFlags = toc->flags & ~ROM_FLAGS_DIRTY;

    DmWrite(toc, offsetof(struct ROMTableOfContentsType, flags), &newFlags, sizeof(long));

    for (resource = 0;  ; resource++) {
        void *resourceAddr = ROMResources[resource];
        if (resourceAddr == NULL) { 
            break;
        } else if (resource == StaticDataResource) {
            MemPtrFree(resourceAddr);
        } else { 
            VoidHand hand = MemPtrRecoverHandle(resourceAddr);
            MemHandleUnlock(hand);
            DmReleaseResource(hand); 
        }
        ROMResources[resource] = NULL;
    }
    MemPtrFree(UTFStringTable);
    MemPtrFree(InternStringTable);
    MemPtrFree(ClassTable);
    MemPtrFree(ROMResources);
}

static int
findResource(environmentPtr env, void* address, int start, int end)
{
    int result = 0;
    int i;
    if (address == NULL) { 
        return env->unknownResource;
    }
    for (i = start; i < end; i++) { 
        struct relocInfoType *info = &env->relocInfo[i];
        void *oldImageStart = (char *)info->targetImage - info->postDelta;
        void *oldImageEnd = (char *)info->targetImageEnd - info->postDelta;
        if (oldImageStart <= address && address < oldImageEnd) { 
            if (result != 0) { 
                fatalError("Duplicate images??");
            }
            result = i;
        }
    }
    if (result == 0) { 
        fatalError("Cannot find resource");
    }
    return result;
}

/* This function does the hard part.  Given an address containing a pointer,
 * it returns the updated value of that pointer.  In addition, it modifies 
 * the original address the contain the updated value.
 *
 * This function should be called exactly >>once<< on each pointer.
 */

static void* 
updatePointer(environmentPtr env, 
              void *targetAddress, 
              ROMResourceType fromResource, ROMResourceType toResource) 
{

    /* Get relocation information on each of the resources */
    struct relocInfoType *fromInfo = &env->relocInfo[fromResource];
    struct relocInfoType *toInfo = &env->relocInfo[toResource];
    long postDelta = toInfo->postDelta;

    void *currentValue;

    /* Make sure that the address we're given is actually within range */
    if (targetAddress < fromInfo->targetImage 
          || targetAddress >= fromInfo->targetImageEnd) { 
        sprintf(str_buffer, "Address %lx not in range [%lx, %lx]\n", 
                (long)targetAddress, (long)fromInfo->targetImage, 
                (long)fromInfo->targetImageEnd);
        fatalError(str_buffer);
    }

    currentValue = *(void **)targetAddress;

    if (currentValue == NULL || postDelta == 0) { 
        /* We don't need to make any change to the value */
        return currentValue;
    } else { 
        /* currentValue is the pre-relocated value of this pointer.  It
         * indicates the previous value of the pointer.
         * postDelta contains the necessary relocation info to convert 
         * old pointers to new pointers */
        void *newValue = ((char *)currentValue) + postDelta;
        /* Sanity checking */
        if (newValue < toInfo->targetImage || 
            newValue > toInfo->targetImageEnd){
            sprintf(str_buffer, "Value %lx not in range [%lx, %lx]\n", 
                    (long)newValue, 
                    (long)toInfo->targetImage, (long)toInfo->targetImageEnd);
            fatalError(str_buffer);
        }
        setValue(env, targetAddress, &newValue, sizeof(void *), fromResource);
        return newValue;
    }
}

static void
setValue(environmentPtr env, 
         void *targetAddress, void *newValuePtr, 
         int newValueSize, ROMResourceType fromResource)
{
    struct relocInfoType *fromInfo = &env->relocInfo[fromResource];
    void *targetImage = fromInfo->targetImage;
    void *currentImage = fromInfo->currentImage;
    long offset = (char *)targetAddress - (char *)targetImage;
    
    if (currentImage == targetImage) { 
        /* Let's try making a copy. */
        long size = env->toc->resources[fromResource].u.size;
        /* This is the first time we've tried to modify this resource */
        if (fromInfo->readOnly) { 
            fatalError("Attempting to write read-only memory");
        } 
        currentImage = MemPtrNew(size);
        if (currentImage != NULL) { 
            fromInfo->copyInStatic = FALSE;
            memcpy(currentImage, targetImage, size);
        } else { 
            DmOpenRef me = DmNextOpenResDatabase(0);
            VoidHand currentHandle = DmNewHandle(me, size);
            currentImage = MemHandleLock(currentHandle);
            fromInfo->copyInStatic = TRUE;
            DmWrite(currentImage, 0, targetImage, size);
        }
        fromInfo->currentImage = currentImage;
    }
    if (fromInfo->copyInStatic) { 
        DmWrite(fromInfo->currentImage, offset, newValuePtr, newValueSize);
    } else { 
        memcpy(((char *)currentImage + offset), newValuePtr, newValueSize);
    }
}

/* Performs object->monitor = mid.  
 * Does special checking to see if "object" is in static memory.
 */

void setObjectMhcInternal(OBJECT object, long value) { 
    char *dmStart;
    int offset;
    if (isROMClass(object)) { 
        dmStart = ROMResources[ClassDefsResource];
    } else if (isROMString(object)) { 
        dmStart = ROMResources[StringResource];
    } else { 
        object->mhc.hashCode = value;
        return;
    }
    offset = (char *)&object->mhc.hashCode - dmStart;
    DmWrite(dmStart, offset, &value, sizeof(object->mhc.hashCode));
}

/* Performs clazz->initialThread = thread.
 * Does special checking to see if "thread" is in static memory.
 */

void setClassInitialThread(INSTANCE_CLASS clazz, THREAD thread) { 
    if (!isROMClass(clazz)) { 
        clazz->initThread = thread;
    } else { 
        char *dmStart = ROMResources[ClassDefsResource];
        int offset = (char *)&clazz->initThread - dmStart;
        DmWrite(dmStart, offset, &thread, sizeof(clazz->initThread));
    }
}

#if ENABLE_JAVA_DEBUGGER

static int
findCodeResourceforMethod(void* address, int start, int end)
{
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    int result = 0;
    int i;
    if (address == NULL) { 
        return 0;
    }
    for (i = start; i < end; i++) { 
        void *imageStart = toc->resources[i].startAddress;
        void *imageEnd = (char *)imageStart + toc->resources[i].u.size;
        if (imageStart <= address && address < imageEnd) { 
            return (i);
        }
    }
    if (result == 0) { 
        fprintf(stderr, "Cannot find resource to set breakpoint\n");
    }
    return result;
}

void setBreakpoint(METHOD method, long codeOffset, char * cpStart, unsigned char bk) { 
    char *dmStart;
    int offset;
    int codeResource;
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    int firstCodeResource = StaticDataResource + 1 +
        toc->methodTableResourceCount;

    if (isROMMethod(method)) { 

        if (toc->codeResourceCount == 1) {
            codeResource = firstCodeResource;
        } else {
            codeResource = findCodeResourceforMethod(method->u.java.code,
                firstCodeResource,
                firstCodeResource + toc->codeResourceCount);
        }
        if (codeResource == 0)
            return;
        dmStart = ROMResources[codeResource];
        offset = (char *)&method->u.java.code[codeOffset] - dmStart;
        DmWrite(dmStart, offset, &bk, 1);
        return;
    } else { 
        if (USESTATIC) {
            long offset1 = (char *)(&method->u.java.code[codeOffset]) - cpStart;
            modifyStaticMemory(cpStart, offset1, &bk, 1);
        } else {
            method->u.java.code[codeOffset] = bk;
            return;
        }
    }
}
#endif

/* Performs clazz->status = status.
 * Does special checking to see if "thread" is in static memory.
 */

void setClassStatus(INSTANCE_CLASS clazz, int status) { 
    if (!isROMClass(clazz)) { 
        clazz->status = status;
    } else { 
        char *dmStart = ROMResources[ClassDefsResource];
        /* This is ugly, but it saves us from needing to know the size
         * of status */
        struct instanceClassStruct temp;
        int offset = (char *)&clazz->status - dmStart;
        temp.status = status;
        DmWrite(dmStart, offset, &temp.status, sizeof(temp.status));
    }
}    


bool_t isROMMethod(void *pointer) { 
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    int firstMethodResource = StaticDataResource + 1;
    int size = toc->resources[firstMethodResource].u.size;
    char *startAddress = toc->resources[firstMethodResource].startAddress;
    return ((char *)pointer >= startAddress && 
        (char *)pointer < startAddress + size);
}

bool_t isROMString(void *pointer) { 
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    int size = toc->resources[StringResource].u.size;
    char *startAddress = toc->resources[StringResource].startAddress;
    return ((char *)pointer >= startAddress && 
        (char *)pointer < startAddress + size);
}

bool_t isROMClass(void *pointer) { 
    ROMTableOfContentsPtr toc = ROMResources[TOCResource];
    int size = toc->resources[ClassDefsResource].u.size;
    char *startAddress = toc->resources[ClassDefsResource].startAddress;
    return ((char *)pointer >= startAddress && 
        (char *)pointer < startAddress + size);

}

#ifndef PILOT

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

static void *UnixGetResource(int number)
{ 
    FILE *file;
    int length;
    void *result;
    
    sprintf(str_buffer, "/home/fy/kvm1.1/kvm/VmPilot/build/bin/PalmROM%d.bin",
            number);
    file = fopen(str_buffer, "rb+");
    if (file == NULL) {     
        fatalError("Cannot open resource");
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    result = mmap(0, length, PROT_READ, MAP_PRIVATE, fileno(file), 0);

    fclose(file);
    if (result == MAP_FAILED) { 
        fatalError("Cannot mmap resource");
    }
    return result;
}

static void 
DmWrite(void *image, int offset, void *src, int size)
{
    static int pageSize = 0;
    caddr_t start = (caddr_t)image + offset;
    caddr_t end = start + size;

    caddr_t pageStart, pageEnd;
    if (pageSize == 0) { 
        pageSize = getpagesize();
    }
    pageStart = (caddr_t)((long)start & ~(pageSize - 1));
    pageEnd = (caddr_t)(((long)end + pageSize - 1) & ~(pageSize - 1));
    if (mprotect(pageStart, pageEnd - pageStart, PROT_READ | PROT_WRITE) != 0) {
        fprintf(stderr, "Unable to set protection");
    }
    memcpy(start, src, size);
    mprotect(pageStart, pageEnd - pageStart, PROT_READ);
}

#endif /* PILOT */

