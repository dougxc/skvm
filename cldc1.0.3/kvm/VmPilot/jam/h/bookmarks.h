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
 * FILE:      bookmarks.h
 * OVERVIEW:  bookmark function prototypes.
 * AUTHOR:    Kay Neuenhofen, Javasoft
 *=======================================================================*/

#ifndef BOOKMARKS__H_
#define BOOKMARKS__H_

/***********************************************************************
 * FUNCTION:    BookmarksInit
 * DESCRIPTION: Initialize the bookmarking functionality.
 * PARAMETERS:  ptr - a pointer to a block of memory that
 *                    holds the list of null-terminated
 *                    bookmark strings
 *              count - the number of bookmark entries
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarksInit(char *ptr, UInt16 count);

/***********************************************************************
 * FUNCTION:    BookmarksUpdate
 * DESCRIPTION: Sync the UI with the functional bookmarks.
 * PARAMETERS:  list_ptr - pointer to the UI component
 * RETURNED:    nothing.
 ***********************************************************************/
void BookmarksUpdate(void *list_ptr);

/***********************************************************************
 * FUNCTION:    BookmarkAdd
 * DESCRIPTION: Add a new entry to the bookmark list.
 * PARAMETERS:  url - pointer to the string representing the entry
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarkAdd(char *url);

/***********************************************************************
 * FUNCTION:    BookmarkRemoveIndex
 * DESCRIPTION: Remove an entry to the bookmark list.
 * PARAMETERS:  index - the index of the entry to be removed
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarkRemoveIndex(UInt16 index);

/***********************************************************************
 * FUNCTION:    BookmarkRemoveString
 * DESCRIPTION: Remove an entry to the bookmark list.
 * PARAMETERS:  url - the entry to be removed
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarkRemoveString(char *url);

/***********************************************************************
 * FUNCTION:    BookmarkFromIndex
 * DESCRIPTION: Return the entry corresponding to a given index
 *              into the bookmark list.
 * PARAMETERS:  index - the index of the entry to be returned
 * RETURNED:    pointer to the entry if successful, NULL otherwise.
 ***********************************************************************/
char *BookmarkFromIndex(UInt16 index);

/***********************************************************************
 * FUNCTION:    BookmarkMakeTop
 * DESCRIPTION: Shuffle the bookmark entries such that the one
 *              indicated by index will bubble to the top (used for
 *              implementing a most-recently-used ordering).
 * PARAMETERS:  index - the index of the entry to be shuffled to the top
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarkMakeTop(UInt16 index);

/***********************************************************************
 * FUNCTION:    BookmarksCleanup
 * DESCRIPTION: bookmark clean-up.
 * PARAMETERS:  packed - *packed will hold the list of null-terminated
 *              bookmark entries. THE CALLER NEEDS TO FREE THE ALLOCATED
 *              MEMORY!
 *              count - *count will hold the number of entries in the list
 * RETURNED:    0 if successful, != 0 otherwise.
 ***********************************************************************/
int BookmarksCleanup(char **packed, UInt16 *count);

#endif /* BOOKMARKS__H_ */

