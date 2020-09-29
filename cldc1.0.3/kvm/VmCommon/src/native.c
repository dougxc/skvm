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
 * SUBSYSTEM: Native function interface
 * FILE:      native.c
 * OVERVIEW:  This file defines the interface for plugging in
 *            the native functions needed by the Java Virtual
 *            Machine. The implementation here is _not_ based
 *            on JNI (Java Native Interface), because JNI seems
 *            too expensive for small devices.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

METHOD CurrentNativeMethod;

/*=========================================================================
 * Operations on native functions
 *=======================================================================*/
int xstrcmp(const char *s1, const char *s2) {
    if (s1 == NULL) {
        s1 = "";
    }
    if (s2 == NULL) {
       s2 = "";
    }
    return strcmp(s1, s2);
}

/*=========================================================================
 * FUNCTION:      getNativeFunction()
 * TYPE:          lookup operation
 * OVERVIEW:      Given a function name as a string, try to find
 *                a corresponding native function and return a
 *                a pointer to it if found.
 * INTERFACE:
 *   parameters:  class name, method name
 *   returns:     function pointer or NIL if not found.
 *=======================================================================*/

NativeFunctionPtr
getNativeFunction(INSTANCE_CLASS clazz, const char* methodName, 
                                        const char *methodSignature)
{
#if !ROMIZING
    const ClassNativeImplementationType *cptr;
    const NativeImplementationType *mptr;
    UString UBaseName    = clazz->clazz.baseName;
    UString UPackageName = clazz->clazz.packageName;
    char* baseName;
    char* packageName;


    /* Package names can be NULL -> must do an explicit check */
    /* to ensure that string comparison below will not fail */
    if (UPackageName == NULL) {
        packageName = "";
    } else {
        packageName = UStringInfo(UPackageName);
    }

    if (UBaseName == NULL) {
        baseName = "";
    } else {
        baseName = UStringInfo(UBaseName);
    }
    

    for (cptr = nativeImplementations; cptr->baseName != NULL ; cptr++) {                 
        if (   (xstrcmp(cptr->packageName, packageName) == 0) 
            && (xstrcmp(cptr->baseName, baseName) == 0)) { 
            break;
        }
    }

    for (mptr = cptr->implementation; mptr != NULL ; mptr++) {
        const char *name = mptr->name;
        if (name == NULL) {
            return NULL;
        }
        if (strcmp(name, methodName) == 0) {
            const char *signature = mptr->signature;
            /* The signature is NULL for non-overloaded native methods. */
            if (signature == NULL || (xstrcmp(signature, methodSignature) == 0)){
                return mptr->implementation;
            }
        }
    }
#endif /* !ROMIZING */
    return NULL;
}

/*=========================================================================
 * FUNCTION:      invokeNativeFunction()
 * TYPE:          private operation
 * OVERVIEW:      Invoke a native function, resolving the native
 *                function reference if necessary.
 * INTERFACE:
 *   parameters:  method pointer
 *   returns:     <nothing>
 *   throws:      UnsatisfiedLinkError if the native method is not found.
 * NOTE:          Native functions are automatically synchronized, i.e.,
 *                they cannot be interrupted by other threads unless
 *                the native code happens to invoke the Java interpreter.
 *=======================================================================*/

void invokeNativeFunction(METHOD thisMethod)
{
    NativeFunctionPtr native = thisMethod->u.native.code;
#if INCLUDEDEBUGCODE
    
    START_TEMPORARY_ROOTS
    int saved_TemporaryRootsLength;
    /*
     * This is to ensure that the native method leaves the stack pointer as
     * expected according to it signature.
     */
    DECLARE_TEMPORARY_FRAME_ROOT(thisFP,getFP());
    int realStackSize = getSP() - (cell*)thisFP - SIZEOF_FRAME + 1;
    int expectedStackSize  = realStackSize - thisMethod->argCount;
    char* returnSig = strchr((change_Key_to_MethodSignature_inBuffer(
          thisMethod->nameTypeKey.nt.typeKey,str_buffer),str_buffer),')') + 1;
    if (*returnSig != 'V') {
        if (*returnSig == 'J' || *returnSig == 'D')
            expectedStackSize += 2;
        else
            expectedStackSize += 1;
    }
#endif

    if (native == NULL) {
        /*  Native function not found; throw error */

        /* The GC may get confused by the arguments on the stack */
        setSP(getSP() - thisMethod->argCount);

        START_TEMPORARY_ROOTS
            DECLARE_TEMPORARY_ROOT(char*, className, 
                     getClassName((CLASS)(thisMethod->ofClass)));
            sprintf(str_buffer,
                    KVM_MSG_NATIVE_METHOD_NOT_FOUND_2STRPARAMS,
                    className, methodName(thisMethod));
        END_TEMPORARY_ROOTS
        raiseExceptionCharMsg(UnsatisfiedLinkError,str_buffer);
    }

#if INCLUDEDEBUGCODE
    if (tracemethodcalls || tracemethodcallsverbose) {
        frameTracing(thisMethod, "=>", +1);
    }
    saved_TemporaryRootsLength = TemporaryRootsLength;

#endif

    /* Call the native function we are supposed to call */
    CurrentNativeMethod = thisMethod;
    native();
    CurrentNativeMethod = NULL;

#if INCLUDEDEBUGCODE
    {
        /*
         * Ensure that the execution stack has been left how the caller
         * expects it to be given that the native function executed in its
         * frame.
         */
        cell* thisSP = getSP();
        if (thisFP != getFP()) {
            /*
             * This will be the case for native functions (such as
             * java.lang.Class.forName) that will potentially push another
             * frame onto the execution stack.
             */
            FRAME currentFP = getFP();
            while (thisFP != currentFP) {
                thisSP = currentFP->previousSp;
                currentFP = currentFP->previousFp;
            }
        }
        realStackSize = thisSP - (cell*)thisFP - SIZEOF_FRAME + 1;
        if (expectedStackSize != realStackSize) {
            char buf[1024];
            getClassName_inBuffer((CLASS)thisMethod->ofClass,buf);
            sprintf(str_buffer,"Native method %s.%s corrupted frame of caller",
                    buf, methodName(thisMethod));
            fatalError(str_buffer);
        }
    }
    
    if (TemporaryRootsLength != saved_TemporaryRootsLength) { 
        START_TEMPORARY_ROOTS
            DECLARE_TEMPORARY_ROOT(char*, className, 
                     getClassName((CLASS)(thisMethod->ofClass)));
            sprintf(str_buffer,
                    KVM_MSG_NATIVE_METHOD_BAD_USE_OF_TEMPORARY_ROOTS,
                    className, methodName(thisMethod));
        END_TEMPORARY_ROOTS
        fatalError(str_buffer);
    }

    if (tracemethodcalls || tracemethodcallsverbose) {
        frameTracing(thisMethod, "<=", +1);
    }

    END_TEMPORARY_ROOTS

#endif

}
