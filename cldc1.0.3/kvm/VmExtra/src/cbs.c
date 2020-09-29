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
 * SUBSYSTEM: Capabilities Based Security
 * FILE:      cbs.c
 * OVERVIEW:  Native operations of the SVM Capabilities Based Security API
 * AUTHOR:    Doug Simon
 *=======================================================================*/

/*=======================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * FUNCTION:      loadTrustedClass(Ljava/lang/String;BII)Ljava/lang/Class;
 *                (STATIC)
 * CLASS:         javax.microedition.cbs.TrustedLoader
 * TYPE:          static native function (reflection)
 * OVERVIEW:      Secure dynamic class loading.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a string object
 *   returns:     a class object
 *=======================================================================*/

void Java_javax_microedition_cbs_TrustedLoader_nativeLoadTrustedClass(void)
{
    unsigned int len = popStackAsType(unsigned int);
    unsigned int off = popStackAsType(unsigned int);
    BYTEARRAY b   = popStackAsType(BYTEARRAY);
    STRING_INSTANCE string = topStackAsType(STRING_INSTANCE);
    if (string == NULL) {
        raiseException(NullPointerException);
    } else {
    START_TEMPORARY_ROOTS
        const char* className = getStringContents((STRING_INSTANCE)string);
        CLASS thisClass = NULL;
        IS_TEMPORARY_ROOT(b,b);
        if (!strchr(className, '/')) {
            replaceLetters((char*)className,'.','/');
            if (isValidName(className, LegalClass)) {
                thisClass = getRawClassX(&className,0,strlen(className));
                /*
                 * The class must not already exist in the system and must
                 * not be an array class.
                 */
                if (((INSTANCE_CLASS)thisClass)->status != CLASS_RAW) {
                    raiseExceptionCharMsg(IllegalSubclassError,
                            "Class already exists");
                }
                /*
                 * Register the classfile content with the classfile
                 * manager. If the registration is unsuccessful, then the
                 * class is already defined by an existing classfile and so
                 * can not be (re)defined by the given classfile.
                 */
                if (setTrustedClassfile((INSTANCE_CLASS)thisClass,
                            &b,off,len) == FALSE) {
                    raiseExceptionCharMsg(IllegalSubclassError,
                            "Cannot override existing classfile");
                }
                /*
                 * Now load the class.
                 */
                thisClass = getClass(className);
                /* The specification does not require that the current
                 * class have "access" to thisClass */
                if (thisClass) {
                    /*
                     * The loaded class must be a trusted class.
                     */
                    if (IS_ARRAY_CLASS(thisClass) ||
                        !IS_TCLASS((INSTANCE_CLASS)thisClass)) {
                        /*
                         * "Remove" this class from the system and then
                         * raise appropriate exception.
                         */
                        revertToRawClass((INSTANCE_CLASS)thisClass);
                        raiseExceptionCharMsg(IllegalSubclassError,
                                "Classfile does not define a trusted class");
                    }
                    topStackAsType(CLASS) = thisClass;
                    if (!CLASS_INITIALIZED((INSTANCE_CLASS)thisClass)) {
                        initializeClass((INSTANCE_CLASS)thisClass);
                    }
                }
            }
        }
        if (thisClass == NULL) {
            raiseException("java/lang/ClassNotFoundException");
        }
    END_TEMPORARY_ROOTS
    }
}


