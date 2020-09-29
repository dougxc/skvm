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
 * SUBSYSTEM: Constant pool
 * FILE:      pool.c
 * OVERVIEW:  Constant pool management operations (see pool.h).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Frank Yellin, Sheng Liang (tightened the access 
 *            checks for JLS (Java Language Spec compliance)
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Constant pool access methods (low level)
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      cachePoolEntry()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Given an index to a pool entry, and a resolved value
 *                update that entry to have the given value.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index, resolved value 
 *   returns:     <nothing>
 * NOTE:          factoring out caching of resolved values like this means
 *                PalmOS specific code for writing to database memory is
 *                restricted.
 *=======================================================================*/

static void 
cachePoolEntry(CONSTANTPOOL constantPool, unsigned int cpIndex, cell* value)
{
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];

    if (!USESTATIC || inCurrentHeap(constantPool)) { 
        /* For !USESTATIC, the constant pool is always in the heap.
         * For USESTATIC, the constant pool can be in the heap when we're 
         *    setting the initial value of a final static String.
         */
        CONSTANTPOOL_TAG(constantPool, cpIndex) |= CP_CACHEBIT;
        thisEntry->cache = value;
    } else { 
        unsigned char *thisTagAddress = &CONSTANTPOOL_TAG(constantPool, cpIndex);
        int entryOffset = (char *)thisEntry - (char *)constantPool;
        int tagOffset =   (char *)thisTagAddress - (char *)constantPool;
        
        unsigned char                 newTag = *thisTagAddress | CP_CACHEBIT;
        union constantPoolEntryStruct newEntry;
        newEntry.cache = value;

        modifyStaticMemory(constantPool, entryOffset, &newEntry, sizeof(newEntry));
        modifyStaticMemory(constantPool, tagOffset,   &newTag,   sizeof(newTag));
    }
}

#if SVM

/*=========================================================================
 * FUNCTION:      classHasTrustedAccessToPublicOrProtectedMember
 * TYPE:          public instance-level operation
 * OVERVIEW:      Helper function for checking access to members in
 *                field/method resolution. 
 * INTERFACE:
 *   parameters:  currentClass - the class performing the access
 *                uClassAccessible - indiciates if the member is accesible
 *                    from untrusted classes (modulo standard Java access
 *                    semantics).
 *                isStatic - indicates if the member is static
 *                isConstructor - indicates if the member is a constructor
 *                cpIndex - the index into currentClass's constant pool
 *                    for the member's entry. If cpIndex == 0, then this
 *                    is a access via reflection.
 *                INSTANCE_CLASS memberClass - the class encapsulating the
 *                    member.
 *=======================================================================*/

static bool_t
classHasTrustedAccessToPublicOrProtectedMember(INSTANCE_CLASS currentClass, 
                       bool_t uClassAccessible,
                       bool_t isStatic,
                       bool_t isConstructor,
                       unsigned int cpIndex,
                       INSTANCE_CLASS memberClass)
{
    if (IS_TCLASS(memberClass)) {
        if (uClassAccessible)
            return TRUE;
        else
        if (isConstructor || isStatic)
        {
            if (!(IS_TCLASS(currentClass)))
                return FALSE;
            else {
                /*
                 * A trusted subclass always has access to its direct
                 * superclass.
                 */
                if (isConstructor && currentClass->superClass == memberClass)
                    return TRUE;
                /*
                 * Ensure that the current class has the necessary
                 * class resource access privelege.
                 */
                if (cpIndex != 0) {
                    verifyClassResourceAccessPrivilege(memberClass,
                            cpIndex,currentClass);
                } else {
                    verifyClassResourceAccessPrivilege_Reflection(memberClass,
                            currentClass);
                }
                return TRUE;
            }
        } else
            return IS_TCLASS(currentClass);
    }
    else
        return TRUE;
}

/*=========================================================================
 * FUNCTION:      classHasPackageAccessToClass
 * TYPE:          public instance-level operation
 * OVERVIEW:      Helper function for checking package private access
 *                rights in field/method resolution. This function assumes
 *                that the accessibility check for public access has
 *                already been performed.
 * INTERFACE:
 *   parameters:
 *   returns:
 *=======================================================================*/

bool_t classHasPackageAccessToClass(INSTANCE_CLASS clazz, CLASS targetClass) {
    
    INSTANCE_CLASS target;
    while (IS_ARRAY_CLASS(targetClass)) {
        ARRAY_CLASS arrayClass = (ARRAY_CLASS)targetClass;
        if (arrayClass->gcType == GCT_OBJECTARRAY)
            targetClass = arrayClass->u.elemClass;
        else
            /*
             * This is a primitive array and so is automatically accessibly
             * by anyone.
             */
            return TRUE;
    }

    target = (INSTANCE_CLASS)targetClass;
    /*
     * An untrusted class is never in the same package as a trusted class
     */
    if (IS_TCLASS(clazz) != IS_TCLASS(target))
        return FALSE;
    /*
     * Two untrusted classes must have matching package names. Two trusted
     * classes must also have matching primary domain keys.
     */
    return (clazz->clazz.packageName == target->clazz.packageName) &&
            ( (!IS_TCLASS(clazz)) ||
              (clazz->tclazz->domainKeys[0] == target->tclazz->domainKeys[0])
            );
    
}

#else

/*
 * These are the standard Java access semantics.
 */
#define classHasTrustedAccessToPublicOrProtectedMember(p1,p2,p3,p4,p5,p6) TRUE
#define classHasPackageAccessToClass(currentClass,targetClass) \
        (targetClass->packageName == currentClass->clazz.packageName)

#endif /* SVM */

/*=========================================================================
 * FUNCTION:      verifyClassAccess
 *                classHasAccessToClass
 *                classHasAccessToMember
 * TYPE:          public instance-level operation
 * OVERVIEW:      Helper functions for checking access rights
 *                in field/method resolution.
 * INTERFACE:
 *   parameters:  targetClass
 *                currentClass
 *   returns:     <nothing>
 *   throws:      IllegalAccessError on failure
 *=======================================================================*/

void verifyClassAccess(CLASS targetClass, INSTANCE_CLASS currentClass) 
{
    if (!classHasAccessToClass(currentClass, targetClass)) {
        START_TEMPORARY_ROOTS
            DECLARE_TEMPORARY_ROOT(char *, targetName, 
                                   getClassName((CLASS)targetClass));
            DECLARE_TEMPORARY_ROOT(char *, currentName, 
                                   getClassName((CLASS)currentClass));

            sprintf(str_buffer, 
                    KVM_MSG_CANNOT_ACCESS_CLASS_FROM_CLASS_2STRPARAMS,
                    targetName, currentName);
        END_TEMPORARY_ROOTS
        raiseExceptionCharMsg(IllegalAccessError,str_buffer);
    }
}

bool_t
classHasAccessToClass(INSTANCE_CLASS currentClass, CLASS targetClass) { 
    if (    currentClass == NULL 
        || ((CLASS)currentClass == targetClass)
        /* Note that array classes have the same package and access as
         * their base classes */
        || (targetClass->accessFlags & ACC_PUBLIC)
        || (classHasPackageAccessToClass(currentClass,targetClass))
        ) { 
        return TRUE;
    } else { 
        return FALSE;
    }
}

bool_t
classHasAccessToMember(INSTANCE_CLASS currentClass, 
                       int access,
#if SVM                       
                       bool_t uClassAccessible,
                       bool_t isConstructor,
                       CONSTANTPOOL cp,
                       unsigned int cpIndex,
#endif /* SVM */
                       INSTANCE_CLASS memberClass) { 
    if (   currentClass == NULL 
        || currentClass == memberClass 
        || ((ACC_PUBLIC & access) &&
            classHasTrustedAccessToPublicOrProtectedMember(currentClass,
                uClassAccessible,
                (access & ACC_STATIC),
                isConstructor,
                cpIndex,
                memberClass))) {
        return TRUE;
    } else if (ACC_PRIVATE & access) { 
        return FALSE;
    } else if (classHasPackageAccessToClass(currentClass,((CLASS)memberClass))){
        return TRUE;
    } else if ((ACC_PROTECTED & access) &&
            classHasTrustedAccessToPublicOrProtectedMember(currentClass,
                uClassAccessible,
                (access & ACC_STATIC),
                isConstructor,
                cpIndex,
                memberClass)) {
        /* See if currentClass is a subclass of memberClass */
        INSTANCE_CLASS cb;
        for (cb = currentClass->superClass; cb; cb = cb->superClass) {
            if (cb == memberClass) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*=========================================================================
 * Constant pool access methods (high level)
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      resolveClassReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Class, get the class
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index , current class
 *   returns:     class pointer
 *=======================================================================*/

CLASS resolveClassReference(CONSTANTPOOL constantPool,
                            unsigned int cpIndex,
                            INSTANCE_CLASS currentClass) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    CLASS thisClass;

    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        return (CLASS)(thisEntry->cache);
    }

    if (VERIFYCONSTANTPOOLINTEGRITY && 
        (thisTag & CP_CACHEMASK) != CONSTANT_Class) { 
        fatalError(KVM_MSG_ILLEGAL_CONSTANT_CLASS_REFERENCE);
    }
    thisClass = thisEntry->clazz;
    if (IS_ARRAY_CLASS(thisClass)) { 
        loadArrayClass((ARRAY_CLASS)thisClass);
    } else if (((INSTANCE_CLASS)thisClass)->status == CLASS_RAW) { 
        loadClassfile((INSTANCE_CLASS)thisClass);
    }
    verifyClassAccess(thisClass, currentClass);
    cachePoolEntry(constantPool, cpIndex, (cell*)thisClass);
    return thisClass;
}

/*=========================================================================
 * FUNCTION:      resolveFieldReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Fieldref, get the Field
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index, isStatic
 *   returns:     field pointer
 *   throws:      IllegalAccessError
 *                IncompatibleClassChangeError
 *=======================================================================*/

FIELD resolveFieldReference(CONSTANTPOOL constantPool, unsigned int cpIndex, 
                            bool_t isStatic, int opcode,
                            INSTANCE_CLASS currentClass) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    unsigned short classIndex;
    FIELD thisField;

    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        
        thisField = (FIELD)(thisEntry->cache);
    } else { 
        unsigned short nameTypeIndex = thisEntry->method.nameTypeIndex;
        CLASS thisClass;
        NameTypeKey nameTypeKey;
        
        classIndex = thisEntry->method.classIndex;
        thisClass = resolveClassReference(constantPool,
                                                classIndex,
                                                currentClass);
        nameTypeKey = 
            constantPool->entries[nameTypeIndex].nameTypeKey;

        thisField = NULL;
        if (  !IS_ARRAY_CLASS(thisClass) 
            && (((INSTANCE_CLASS)thisClass)->status != CLASS_ERROR)) { 
            thisField = lookupField((INSTANCE_CLASS)thisClass, nameTypeKey);
        }
    }

    /* Cache the value */
    if (thisField != NULL) {
        if (isStatic ? ((thisField->accessFlags & ACC_STATIC) == 0)
                     : ((thisField->accessFlags & ACC_STATIC) != 0)) {
            START_TEMPORARY_ROOTS
                DECLARE_TEMPORARY_ROOT(char *, fieldClassName, 
                    getClassName((CLASS)(thisField->ofClass)));
                sprintf(str_buffer, 
                        KVM_MSG_INCOMPATIBLE_CLASS_CHANGE_2STRPARAMS,
                        fieldClassName, 
                        fieldName(thisField));
                raiseExceptionCharMsg(IncompatibleClassChangeError,str_buffer);
            END_TEMPORARY_ROOTS
        }
        if ((thisField->accessFlags & ACC_FINAL) &&
            (opcode == PUTSTATIC || opcode == PUTFIELD) &&
            (thisField->ofClass != currentClass)) {
            START_TEMPORARY_ROOTS
                DECLARE_TEMPORARY_ROOT(char *, fieldClassName, 
                    getClassName((CLASS)(thisField->ofClass)));
                DECLARE_TEMPORARY_ROOT(char *, currentClassName, 
                    getClassName((CLASS)currentClass));
                    sprintf(str_buffer, 
                        KVM_MSG_CANNOT_MODIFY_FINAL_FIELD_3STRPARAMS,
                        fieldClassName, fieldName(thisField),
                        currentClassName);
            END_TEMPORARY_ROOTS
            raiseExceptionCharMsg(IllegalAccessError,str_buffer);
        }
        if (!(thisTag & CP_CACHEBIT)) { 
            /* Since access only depends on the class, and not on the
             * specific byte code used to access the field, we don't need
             * to perform this check if the constant pool entry
             * has already been resolved.
             */
            if (!classHasAccessToMember(currentClass, 
                                        thisField->accessFlags, 
#if SVM
                                        thisField->uClassAccessible,
                                        FALSE,
                                        constantPool,
                                        classIndex,
#endif /* SVM */
                                        thisField->ofClass)) { 
                START_TEMPORARY_ROOTS
                    DECLARE_TEMPORARY_ROOT(char *, fieldClassName, 
                        getClassName((CLASS)(thisField->ofClass)));
                    DECLARE_TEMPORARY_ROOT(char *, currentClassName, 
                        getClassName((CLASS)currentClass));
                    sprintf(str_buffer, 
                            KVM_MSG_CANNOT_ACCESS_MEMBER_FROM_CLASS_3STRPARAMS,
                            fieldClassName, fieldName(thisField), 
                            currentClassName);
                END_TEMPORARY_ROOTS
                raiseExceptionCharMsg(IllegalAccessError,str_buffer);
            }
            cachePoolEntry(constantPool, cpIndex, (cell*)thisField);
        }
    }
    return thisField;
}

/*=========================================================================
 * FUNCTION:      resolveMethodReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Methodref or 
 *                CONSTANT_InterfaceMethodref, get the Method
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer
 *                constant pool index
 *                isStatic - true if the caller expects the method to be
 *                           static
 *   returns:     method pointer
 *=======================================================================*/

METHOD resolveMethodReference(CONSTANTPOOL constantPool, unsigned int cpIndex, 
                              bool_t isStatic, INSTANCE_CLASS currentClass) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    unsigned short classIndex;
    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        return (METHOD)(thisEntry->cache);
    } else { 
        unsigned short nameTypeIndex = thisEntry->method.nameTypeIndex;
        CLASS thisClass;
        NameTypeKey nameTypeKey;
        METHOD thisMethod = NULL;

        classIndex = thisEntry->method.classIndex;
        thisClass = resolveClassReference(constantPool,
                                                classIndex,
                                                currentClass);

        if (  (((thisTag & CP_CACHEMASK) == CONSTANT_InterfaceMethodref) &&
               !(thisClass->accessFlags & ACC_INTERFACE))
            ||
              (((thisTag & CP_CACHEMASK) == CONSTANT_Methodref) &&
                (thisClass->accessFlags & ACC_INTERFACE))) {
            /* "Bad methodref" */
            raiseExceptionCharMsg(IncompatibleClassChangeError,
                KVM_MSG_BAD_FIELD_OR_METHOD_REFERENCE);
        }

        nameTypeKey = constantPool->entries[nameTypeIndex].nameTypeKey;
        if (IS_ARRAY_CLASS(thisClass) || 
            ((INSTANCE_CLASS)thisClass)->status != CLASS_ERROR) { 
            thisMethod = lookupMethod(thisClass, nameTypeKey, currentClass);
            if (nameTypeKey.nt.nameKey == initNameAndType.nt.nameKey) { 
                if (thisMethod != NULL 
                    && thisMethod->ofClass != (INSTANCE_CLASS)thisClass) { 
                    thisMethod = NULL;
                }
            }
        }
        if (thisMethod) {
            if (isStatic ? ((thisMethod->accessFlags & ACC_STATIC) == 0)
                         : ((thisMethod->accessFlags & ACC_STATIC) != 0)) {
                START_TEMPORARY_ROOTS
                    DECLARE_TEMPORARY_ROOT(char*, methodClassName, 
                        getClassName((CLASS)thisMethod->ofClass));
                    DECLARE_TEMPORARY_ROOT(char*, methodSignature, 
                        getMethodSignature(thisMethod));
                    sprintf(str_buffer, 
                            KVM_MSG_INCOMPATIBLE_CLASS_CHANGE_3STRPARAMS,
                            methodClassName, 
                            methodName(thisMethod), methodSignature);
                    raiseExceptionCharMsg(IncompatibleClassChangeError,
                        str_buffer);
                END_TEMPORARY_ROOTS
            }
            if (!classHasAccessToMember(currentClass, thisMethod->accessFlags, 
#if SVM
                                        thisMethod->uClassAccessible,
                                        thisMethod->nameTypeKey.nt.nameKey ==
                                            getUString("<init>")->key,
                                        constantPool,
                                        classIndex,
#endif /* SVM */
                                        thisMethod->ofClass)) { 
                START_TEMPORARY_ROOTS
                    DECLARE_TEMPORARY_ROOT(char*, className, 
                        getClassName((CLASS)currentClass));
                    DECLARE_TEMPORARY_ROOT(char*, methodClassName, 
                        getClassName((CLASS)thisMethod->ofClass));
                    sprintf(str_buffer, 
                        KVM_MSG_CANNOT_ACCESS_MEMBER_FROM_CLASS_3STRPARAMS,
                        methodClassName, methodName(thisMethod), 
                        className);
                END_TEMPORARY_ROOTS
                raiseExceptionCharMsg(IllegalAccessError,str_buffer);
            }
            cachePoolEntry(constantPool, cpIndex, (cell*)thisMethod);
        } else { 
            /* Some appropriate error message in debug mode */
        }
        return thisMethod;
    }
}

