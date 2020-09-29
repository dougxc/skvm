/*
 * Copyright (c) 1999-2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: Palm JAM
 * FILE:      Jam.h
 * OVERVIEW:  Palm JAM (Java Application Manager)
 * AUTHOR:    Kay Neuenhofen, Javasoft
 *            Frank Yellin
 * COMMENTS:  This implementation is based on Palm sample applications.
 *=======================================================================*/

/***********************************************************************
 *
 *   Internal Constants
 *
 ***********************************************************************/

#include <unix_stdarg.h>

#define appPrefID                  0x00
#define appPrefVersionNum          0x02

#define MAX_URL 256

#define LOCSIG (((long)'P' << 0) + ((long)'K' << 8) + (3L << 16) + (4L << 24))
#define CENSIG (((long)'P' << 0) + ((long)'K' << 8) + (1L << 16) + (2L << 24))
#define ENDSIG (((long)'P' << 0) + ((long)'K' << 8) + (5L << 16) + (6L << 24))

#define JAM_TEMP_RESOURCE_FILENAME "JamBoree"
#define JAM_TEMP_RESOURCE_TYPE     'Jamb'
#define JAM_APPL_RESOURCE_TYPE     'appl'

#define APPLICATION_NAME_TAG    "Application-Name"
#define APPLICATION_VERSION_TAG "Application-Version"
#define APPLICATION_ICON_TAG    "Application-Icon"
#define KVM_VERSION_TAG         "KVM-Version"
#define JAR_FILE_URL_TAG        "JAR-File-URL"
#define JAR_FILE_SIZE_TAG       "JAR-File-Size"
#define MAIN_CLASS_TAG          "Main-Class"
#define USE_ONCE_TAG            "Use-Once"

// Define the minimum OS version we support
#define ourMinVersion            sysMakeROMVersion(2,0,0,sysROMStageRelease,0)

typedef enum { FALSE, TRUE } bool_t;

typedef struct JamPreferencesType {
#define PREFERENCES_REPEAT_FLAG 0x01
#define PREFERENCES_INETLIB_FLAG 0x02
    UInt16 flags; /* JAM flags: repeat, use inetlib */
    UInt16 count; /* number of bytes for bookmark strings */
    char strings[1]; /* pointer to bookmark strings */
} JamPreferences;

void Alert(const char *msg, ...);
void setStatus(const char *msg, ...);
void setTitle(const char *msg);

typedef char* (*NetworkFetcherType)(const char *host, int port, const char *path);
typedef void  (*NetworkCloserType)(); 


bool_t Netlib_open(NetworkFetcherType*, NetworkCloserType*);
bool_t INetlib_open(NetworkFetcherType*, NetworkCloserType*);
bool_t INetlib_exists();

char *readCompletely(void *, Err (*reader)(void *, char *, long, ULong *));
