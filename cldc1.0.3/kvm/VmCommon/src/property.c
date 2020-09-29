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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Property support
 * FILE:      property.c
 * OVERVIEW:  System dependent property lookup operations and 
 *            definitions.
 * AUTHOR:    Nik Shaylor Sun C&E
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getSystemProperty()
 * TYPE:          public operation
 * OVERVIEW:      Return a value of a property key
 * INTERFACE:
 *   parameters:  char* key (as a C string (ASCIZ))
 *   returns:     char* result (as a C string (ASCIZ))
 *
 * NOTE:          In the future, if the number of properties becomes
 *                larger, we should create an internal hashtable for
 *                properties.  Also, we should make it easy to extend
 *                the set of system properties in port-specific files
 *                without editing VmCommon files.
 *=======================================================================*/

char* getSystemProperty(char *key) {
    char* value = NULL;

    /*
     * Currently, we define properties simply by going
     * through a set of if statements.  If the number of
     * properties becomes any larger, we should really
     * use an internal hashtable for the key/value pairs.
     */

    if (strcmp(key, "microedition.configuration") == 0) {
        /* Important: This value should reflect the */
        /* version of the CLDC Specification supported */
        /* -- not the version number of the implementation */
        value = "CLDC-1.0";
        goto done;
    }

    if(strcmp(key, "microedition.platform") == 0) {
#ifdef PLATFORMNAME
        value = PLATFORMNAME;
        goto done;
#else
#ifdef PILOT
        value = "palm";
        goto done;
#endif
#endif
    }

    if (strcmp(key, "microedition.encoding") == 0) {
        value = "ISO8859_1";
        goto done;
    }

done:
    return value;
}

