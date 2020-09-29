/*
 * Copyright (c) 2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: JAM
 * FILE:      bookmarks.c
 * OVERVIEW:  bookmark functions for the Palm JAM
 * AUTHOR:    Kay Neuenhofen, Javasoft
 * COMMENTS:  Please see the header file for function descriptions
 *=======================================================================*/

#include <CWCompatibility.h>
#include <unix_string.h>

#include <bookmarks.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define MAX_BOOKMARKS 20

typedef enum {CLOSED, OPEN} BMStateType;

static char *url[MAX_BOOKMARKS] = {0};
static UInt16 urlCount = 0;
static BMStateType bm_state = CLOSED;
static void *listPtr = NULL;
static char *queued_strings = NULL;

static VoidPtr GetObjectPtr(UInt16 objectID) {
    FormPtr frmP = FrmGetActiveForm();
    return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}

static void BookmarksFree() {
    int i;

    for (i=0; i<urlCount; i++) {
        if (url[i] != NULL) MemPtrFree(url[i]);
    }
}

int BookmarksInit(char *ptr, UInt16 count) {
    int i;

    if (bm_state != CLOSED) return 1;
    if (count >= MAX_BOOKMARKS) return 1;

    for (i=0; i<count; i++) {
        url[i] = MemPtrNew(strlen(ptr) + 1);
        if (url[i] == NULL) {
            BookmarksFree();
            return 1;
        }

        strcpy(url[i], ptr);
        urlCount++;

        ptr += strlen(ptr) + 1;
    }

    bm_state = OPEN;
    return 0;
}

void BookmarksUpdate(void *list_ptr) {
    if (list_ptr != NULL) listPtr = list_ptr;
    LstSetListChoices(listPtr, url, urlCount);
    LstDrawList(listPtr);
}

int BookmarkAdd(char *entry) {
    int i;
    char *ptr = NULL;

    if (bm_state != OPEN) return 1;
    if (urlCount >= MAX_BOOKMARKS) return 1;

    for (i=0; i<urlCount; i++) {
        if (!strcmp(entry, url[i])) {
            BookmarkMakeTop(i);
            return 0;
        }
    }

    ptr = MemPtrNew(strlen(entry) + 1);
    if (ptr == NULL) return 1;
    strcpy(ptr, entry);

    for (i=urlCount; i>0; --i) {
        url[i] = url[i-1];
    }

    url[0] = ptr;
    urlCount++;
    BookmarksUpdate(NULL);

    return 0;
}

int BookmarkRemoveString(char *entry) {
    int i;

    if (bm_state != OPEN) return 1;
    if (entry == NULL || strlen(entry) == 0) return 1;

    for (i=0; i<urlCount; i++) {
        if (!strcmp(entry, url[i])) {
            BookmarkRemoveIndex(i);
            return 0;
        }
    }

    return 1;
}

int BookmarkRemoveIndex(UInt16 index) {
    int i;

    if (bm_state != OPEN) return 1;
    if (index >= urlCount) return 1;

    i = index;
    MemPtrFree(url[i++]);
    for (;i<urlCount; i++) {
        url[i-1] = url[i];
    }
    url[--urlCount] = NULL;
    BookmarksUpdate(NULL);

    return 0;
}

char *BookmarkFromIndex(UInt16 index) {
    if (bm_state != OPEN) return NULL;
    if (index >= urlCount) return NULL;
    return url[index];
}

int BookmarkMakeTop(UInt16 index) {
    char *ptr;
    int i;

    if (bm_state != OPEN) return 1;
    if (index >= urlCount) return 1;

    if (index == 0) return 0;

    ptr = url[index];
    for (i=index; i>0; i--) {
        url[i] = url[i-1];
    }
    url[0] = ptr;
    BookmarksUpdate(NULL);

    return 0;
}

int BookmarksCleanup(char **packed, UInt16 *count) {
    char *ptr;
    int result = 1;
    UInt32 totalSize = urlCount;
    int i;

    if (bm_state != OPEN) return 1;

    *packed = NULL;
    *count = 0;

    for (i=0; i<urlCount; i++) {
        totalSize += strlen(url[i]);
    }

    queued_strings = MemPtrNew(max(totalSize, 1));
    if (queued_strings == NULL) return 1;

    ptr = queued_strings;

    if (urlCount == 0) {
        ptr[0] = '\0';
    } else {
        for (i=0; i<urlCount; i++) {
            strcpy(ptr, url[i]);
            ptr += strlen(ptr) + 1;
        }
    }

    BookmarksFree();
    bm_state = CLOSED;

    *packed = queued_strings;
    *count = urlCount;
    return 0;
}

