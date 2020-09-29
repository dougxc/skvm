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
 * FILE:      Main.c
 * OVERVIEW:  Palm JAM main application
 * AUTHOR:    Kay Neuenhofen, Javasoft
 *            Frank Yellin
 * COMMENTS:  This implementation is based on Palm sample applications.
 *=======================================================================*/

#include <KVMCommon.h>
#include <SysEvtMgr.h>
#include <netmgr.h>
#include <Crc.h>
#define NON_INTERNATIONAL
#include <CharAttr.h>

typedef unsigned char **UNSIGNED_CHAR_HANDLE;

#include <Jam.h>
#include <JamRsrc.h>
#include <jar.h>
#include <inflate.h>
#include <wrapper.h>
#include <bookmarks.h>

#include <unix_stdarg.h>

UInt16 prefsFlags;
ULong   myCreator;       /* Creator of this application */
UInt    myCardNo;        /* Card number of this application */

/***********************************************************************
 * Internal Functions
 ***********************************************************************/

static Err RomVersionCompatible(DWord requiredVersion, Word launchFlags, Word);
static void LaunchVM(UInt cardNo, LocalID dbID);
static VoidPtr GetObjectPtr(Word objectID);
static Err AppStart(void);
static void AppStop(void);
static void AppEventLoop(void);
static Boolean AppHandleEvent( EventPtr eventP);
static Boolean MainFormHandleEvent(EventPtr eventP);

static void pushTheRunButton(void);

static bool_t JamDownloadURL(char *url, LocalID *dbIDP, bool_t useINetLib);
static bool_t createResourceDB(DmOpenRef *databaseP, LocalID *databaseIdP, char *fileName, bool_t useOnce);
static unsigned char *convertBitmapToIconFormat(unsigned char *buf);
static bool_t getJarURL(char *, int, char *host, int *port, char *path);
static bool_t addClassesToResource(unsigned char *jar, char *jamContent, DmOpenRef resourceFile);

static bool_t parseURL(char * url, int length,
                  char *host, int *port, char *path);

static char *JamGetProp(char *buffer, char * name, int *length);

static void debug(const char *format, ...);

static bool_t isUseOnceApp(UInt cardNo, LocalID dbID) {
    VoidHand recHandle;
    UInt16 resIndex;
    DmOpenRef db_ref;
    bool_t useOnce = FALSE;
    Ptr recPtr;

    if (dbID == 0) return FALSE;

    db_ref = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
    if (db_ref == NULL) goto use_once_done;
    resIndex = DmFindResource(db_ref, 'use1', 1, NULL);
    if (resIndex <= 0) goto use_once_done;
    recHandle = DmGetResourceIndex(db_ref, resIndex);
    if (recHandle == NULL) goto use_once_done;

    recPtr = MemHandleLock(recHandle);
    if (recPtr == NULL) goto use_once_done;
    MemMove(&useOnce, recPtr, sizeof useOnce);

use_once_done:
    if (recHandle != NULL) {
        MemHandleUnlock(recHandle);
        DmReleaseResource(recHandle);
    }
    if (db_ref != NULL) DmCloseDatabase(db_ref);

    return useOnce;
}

static void deleteApp(UInt cardNo, LocalID dbID) {
    if (DmDeleteDatabase(cardNo, dbID)) {
        Alert("Could not delete Use-Once App");
    }
}

/***********************************************************************
 * FUNCTION:    PilotMain
 * DESCRIPTION: This is the main entry point for the application.
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with
 *              the launch code.
 *              launchFlags -  word value providing extra information
 *              about the launch.
 * RETURNED:    Result of launch
 ***********************************************************************/

DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Err error;
    UInt cardNo;
    LocalID dbID = 0;

    (void)cmdPBP;

    if ((error = RomVersionCompatible (ourMinVersion, launchFlags, 0))) {
        return error;
    }

    switch (cmd) {
       case sysAppLaunchCmdGoTo:
            /* Fall-through */

       case sysAppLaunchCmdNormalLaunch:
            if (cmdPBP != NULL) {
                cardNo = ((GoToParamsPtr)cmdPBP)->matchPos;
                dbID = ((GoToParamsPtr)cmdPBP)->matchCustom;
            }

            if ((error = AppStart())) {
                return error;
            }

            /* Use-Once supercedes Repeat flag */
            if (isUseOnceApp(cardNo, dbID)) {
                deleteApp(cardNo, dbID);
            } else {
                if (cmd == sysAppLaunchCmdGoTo
                       && (prefsFlags & PREFERENCES_REPEAT_FLAG)) {
                    pushTheRunButton();
                }
            }

            AppEventLoop();
            AppStop();
            break;

        default:
            break;

    }

    return 0;
}

/***********************************************************************
 * FUNCTION:    RomVersionCompatible
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h
 *                                for format)
 *              launchFlags     - flags that indicate if the application
 *                                UI is initialized.
 * RETURNED:    error code or zero if rom is compatible
 ***********************************************************************/

static Err
RomVersionCompatible(DWord requiredVersion, Word launchFlags, Word ignore)
{
    DWord romVersion;

    (void)ignore;        /* So we're compatible with KVMCommon.c */

    // See if we're on in minimum required version of the ROM or later.
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion) {

        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) == (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) {

            FrmAlert (RomIncompatibleAlert);
            // Pilot 1.0 will continuously relaunch this app unless we switch
            // to another safe one.
            if (romVersion < sysMakeROMVersion(2,0,0,sysROMStageRelease,0))
                AppLaunchWithCommand(sysFileCDefaultApp,
                     sysAppLaunchCmdNormalLaunch, NULL);
        }

        return (sysErrRomIncompatible);
    }
    return (0);
}

static void
LaunchVM(UInt cardNo, LocalID dbID)
{
    DmSearchStateType searchState;
    GoToParamsPtr theGotoPointer;
    LocalID kvmID;
    UInt    kvmCardNo;
    Word    err;

    err = DmGetNextDatabaseByTypeCreator(true, &searchState,
              sysFileTApplication, 'kJav', true, &kvmCardNo, &kvmID);
    if (err) {
        Alert("Cannot find KVM on this device");
        return;
    }

    /* The following code is stolen from the Palm FAQ. */
    theGotoPointer = MemPtrNew(sizeof(GoToParamsType));
    MemPtrSetOwner(theGotoPointer, 0);
    MemSet(theGotoPointer, sizeof(GoToParamsType), 0); /* can't hurt */

    /* Find the card number and LocalID of this wrapper */
    theGotoPointer->searchStrLen = 1;
    DmOpenDatabaseInfo(DmNextOpenResDatabase(0),
               &theGotoPointer->matchCustom, 0, 0,  // this LocalID
               &theGotoPointer->matchPos, 0);       // this cardNo
    theGotoPointer->dbID = dbID;
    theGotoPointer->dbCardNo = cardNo;

    setStatus("Bon voyage");
    err = SysUIAppSwitch(kvmCardNo, kvmID,
                         sysAppLaunchCmdGoTo, (Ptr)theGotoPointer);
    if (err) {
        Alert("Cannot launch KVM");
    }
}


/***********************************************************************
 * FUNCTION:    GetObjectPtr
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 * PARAMETERS:  formId - id of the form to display
 * RETURNED:    VoidPtr
 ***********************************************************************/

static VoidPtr
GetObjectPtr(Word objectID)
{
    FormPtr frmP = FrmGetActiveForm();
    return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}

/***********************************************************************
 * FUNCTION:    MainFormHandleEvent
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 ***********************************************************************/

static Boolean MainFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;

    switch (eventP->eType) {
        case keyDownEvent:
            // Let the system do the real work.  But we want to delete messages
            if (eventP->data.keyDown.chr == '\n') {
                FieldPtr field = GetObjectPtr(MainURLField);
                Word startPosition, endPosition;
                FldGetSelection(field, &startPosition, &endPosition);
                if (startPosition == endPosition &&
                      startPosition == FldGetTextLength(field)) {
                    pushTheRunButton();
                    handled = true;
                }
            }
            setStatus(" ");
            break;

        case menuEvent:
            if (eventP->data.menu.itemID == MainOptionsAboutStarterApp) {
                MenuEraseStatus (0);
                AbtShowAbout (myCreator);
                handled = true;
            }
            break;

        case frmOpenEvent: {
            char *text;
            Word length;
            FieldPtr field;

            CtlSetValue(GetObjectPtr(MainRepeatCheckbox),
            (prefsFlags & PREFERENCES_REPEAT_FLAG) ? 1 : 0);
            if (INetlib_exists()) {
                CtlSetValue(GetObjectPtr(MainINetLibCheckbox),
                    (prefsFlags & PREFERENCES_INETLIB_FLAG) ?1 : 0);
            } else {
                CtlSetValue(GetObjectPtr(MainINetLibCheckbox), 0);
                CtlSetUsable(GetObjectPtr(MainINetLibCheckbox), 0);
            }
            FrmDrawForm (FrmGetActiveForm());
            handled = true;
            BookmarksUpdate(GetObjectPtr(MainBookmarkList));

            text = BookmarkFromIndex(0);
            
            if (text != NULL) {
                length = strlen(text);
                field = GetObjectPtr(MainURLField);
                FldSetSelection(field, 0, FldGetTextLength(field));
                FldInsert(field, text, length);
                FldSetSelection(field, length, length);
            }

            break;
        }

        case popSelectEvent: {
            UInt16 sel_index = eventP->data.popSelect.selection;
            char *text = BookmarkFromIndex(sel_index);
            Word length = strlen(text);
            FieldPtr field = GetObjectPtr(MainURLField);
            FldSetSelection(field, 0, FldGetTextLength(field));
            FldInsert(field, text, length);
            FldSetSelection(field, length, length);
            BookmarkMakeTop(sel_index);
            handled = true;
            break;
        }

        case ctlSelectEvent:
            switch (eventP->data.ctlSelect.controlID) {
                case MainRemoveButton: {
                    BookmarkRemoveString(FldGetTextPtr(GetObjectPtr(MainURLField)));
                    handled = true;
                }
                break;

                case MainRunButton: {
                    char *text = FldGetTextPtr(GetObjectPtr(MainURLField));
                    if (text != NULL) {
                        bool_t inet =
                            CtlGetValue(GetObjectPtr(MainINetLibCheckbox));
                        LocalID dbID;
                        BookmarkAdd(text);
                        if (JamDownloadURL(text, &dbID, inet)) {
                            LaunchVM(myCardNo, dbID);
                        }

                    }
                    handled = true;
                    break;
                }

                case MainRepeatCheckbox:
                    prefsFlags &= ~PREFERENCES_REPEAT_FLAG;
                    if (CtlGetValue(GetObjectPtr(MainRepeatCheckbox))) {
                        prefsFlags |= PREFERENCES_REPEAT_FLAG;
                    }
                    handled = true;
                    break;

                case MainINetLibCheckbox:
                    prefsFlags &= ~PREFERENCES_INETLIB_FLAG;
                    if (CtlGetValue(GetObjectPtr(MainINetLibCheckbox))) {
                        prefsFlags |= PREFERENCES_INETLIB_FLAG;
                    }
                    handled = true;
                    break;
            }
            break;

        case frmCloseEvent: {
            prefsFlags =
               (CtlGetValue(GetObjectPtr(MainRepeatCheckbox)) ?
                   PREFERENCES_REPEAT_FLAG : 0)
            +  (CtlGetValue(GetObjectPtr(MainINetLibCheckbox)) ?
               PREFERENCES_INETLIB_FLAG : 0);
            handled = false;
        }
        break;

        default:
            break;
    }
    return handled;
}


/***********************************************************************
 * FUNCTION:    AppHandleEvent
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 * PARAMETERS:  event  - a pointer to an EventType structure
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 ***********************************************************************/

static Boolean
AppHandleEvent( EventPtr eventP)
{
    Word formId;
    FormPtr frmP;

    if (eventP->eType == frmLoadEvent) {
        // Load the form resource.
        formId = eventP->data.frmLoad.formID;
        frmP = FrmInitForm(formId);
        FrmSetActiveForm(frmP);

        // Set the event handler for the form.  The handler of the currently
        // active form is called by FrmHandleEvent each time is receives an
        // event.
        switch (formId) {
        case MainForm:
            FrmSetEventHandler(frmP, MainFormHandleEvent);
            break;

        default:
            ErrFatalDisplay("Invalid Form Load Event");
            break;
        }
        return true;
    }
    return false;
}


/***********************************************************************
 * FUNCTION:    AppEventLoop
 * DESCRIPTION: This routine is the event loop for the application.
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/

static void AppEventLoop(void)
{
    Word error;
    EventType event;
    do {
        EvtGetEvent(&event, evtWaitForever);
        if (! SysHandleEvent(&event))
            if (! MenuHandleEvent(0, &event, &error))
                if (! AppHandleEvent(&event))
                    FrmDispatchEvent(&event);
    } while (event.eType != appStopEvent);
}


static void pushTheRunButton() {
    EventType event;
    event.eType = ctlSelectEvent;
    event.data.ctlSelect.controlID = MainRunButton;
    EvtAddEventToQueue (&event);
}


/***********************************************************************
 * FUNCTION:     AppStart
 * DESCRIPTION:  Get the current application's preferences.
 * PARAMETERS:   nothing
 * RETURNED:     Err value 0 if nothing went wrong
 ***********************************************************************/

static Err AppStart(void)
{
    struct KVMprefsStruct KVMprefs;
    JamPreferences *preferences;
    Word    prefsSize = 0;
    SWord   version;
    LocalID databaseID;
    UInt    kvmCardNo;
    DmSearchStateType searchState;
    Err     err;

    DmOpenDatabaseInfo(DmNextOpenDatabase(NULL), &databaseID, NULL, NULL,
               &myCardNo, NULL);
    DmDatabaseInfo(myCardNo, databaseID, NULL, NULL, NULL, NULL, NULL,
           NULL, NULL, NULL, NULL, NULL, &myCreator);

    /* get size of prefs*/
    PrefGetAppPreferences(myCreator, appPrefID,
                    NULL, &prefsSize, FALSE);

    if (prefsSize == 0) {
        prefsSize = sizeof (JamPreferences);
    }

    preferences = MemPtrNew(prefsSize);
    if (preferences == NULL) return memErrNotEnoughSpace;

    version = PrefGetAppPreferences(myCreator, appPrefID,
                    preferences, &prefsSize, FALSE);
    if (version == noPreferenceFound) {
        memset(preferences, 0, prefsSize);
        preferences->flags = 0;
        preferences->count = 0;
        StrCopy(preferences->strings, "");
        PrefSetAppPreferences(myCreator, appPrefID, appPrefVersionNum,
                  preferences, prefsSize, FALSE);
    }

    prefsFlags = preferences->flags;


    // Make sure that the KVM exists on the machine
    err = DmGetNextDatabaseByTypeCreator(
          true, &searchState, sysFileTApplication, KVMCreator,
          true, &kvmCardNo, &databaseID);
    if (err) {
        FrmAlert(NoKVMAlert);
    }

    // Make sure that we have accepted the KVM license

    prefsSize = sizeof(KVMprefs);
    version = PrefGetAppPreferences(KVMCreator, KVMPrefID,
                    &KVMprefs, &prefsSize, true);
    if (version == noPreferenceFound
        || prefsSize != sizeof(KVMprefs)
        || version != KVMPrefVersion
        || !KVMprefs.licenseAccepted) {

    /*************
        KVMprefs.licenseAccepted = 1;
        KVMprefs.saveOutput = 0;
        KVMprefs.displayLines = 0;
        KVMprefs.heapSize = 0;
        KVMprefs.showHeapStats = 0;
        PrefSetAppPreferences(KVMCreator, KVMPrefID, KVMPrefVersion,
                  &KVMprefs, sizeof(KVMprefs), true);
    **************/
        FrmAlert(LicenseAlert);
    }

    FrmGotoForm(MainForm);
    FrmSetActiveForm(FrmGetFormPtr(MainForm));


    BookmarksInit(preferences->strings, preferences->count);
    MemPtrFree(preferences);

    return 0;
}



/***********************************************************************
 * FUNCTION:    AppStop
 * DESCRIPTION: Save the current state of the application.
 * PARAMETERS:  nothing
 * RETURNED:    nothing
 ***********************************************************************/

static void AppStop(void)
{
    char *ptr;
    UInt16 count;
    JamPreferences *preferences;

    FrmCloseAllForms();

    if (!BookmarksCleanup(&ptr, &count)) {
        preferences = MemPtrNew(sizeof(JamPreferences) - 1 + MemPtrSize(ptr));
        preferences->flags = prefsFlags;
        preferences->count = count;
        memmove(preferences->strings, ptr, MemPtrSize(ptr));

        PrefSetAppPreferences(myCreator, appPrefID, appPrefVersionNum,
                  preferences, MemPtrSize(preferences), FALSE);

        MemPtrFree(preferences);
        MemPtrFree(ptr);
    }
}

/***********************************************************************
 * FUNCTION:    lastIndexOf
 * DESCRIPTION: find the last occurence of c in the first len bytes
 *              of str
 * PARAMETERS:  string str, length len, char c
 * RETURNED:    index of last c when found, -1 otherwise
 ***********************************************************************/

static int lastIndexOf(char *str, int len, char c) {
    int i = len - 1;
    for (; i>=0; i--) if (str[i] == c) return i;
    return -1;
}

static bool_t
JamDownloadURL(char *url, LocalID *dbIdP, bool_t useINetLib)
{
    DmOpenRef resourceDB = NULL;
    char host[MAX_URL];
    char path[MAX_URL];
    char dbName[32];
    int port;
    char *content = NULL;
    char *jar = NULL;
    char *mainClass = NULL;
    char *useOnceStr;
    int useOnceLength;
    bool_t useOnce;
    int nameStart;
    bool_t result = FALSE;
    UInt16 dbNameLength;
    UInt16 nameCRC;
    int mainClassLength;
    VoidHand handle;
    char *jarName = NULL;
    int jarNameLength;

    NetworkFetcherType fetchURL = 0;
    NetworkCloserType  closeNetwork = 0;

    setStatus("Getting jam file");
    if (!parseURL(url, StrLen(url), host, &port, path)) {
        Alert("Unable to parse URL");
        goto error;
    }

    if (useINetLib ? !INetlib_open(&fetchURL, &closeNetwork)
               : !Netlib_open(&fetchURL, &closeNetwork)) {
        Alert("%cNetlib_open() failed", (useINetLib ? 'I' : ' '));
        goto error;
    }

    content = fetchURL(host, port, path);
    if (content == NULL) {
        goto error;
    }

    mainClass = JamGetProp(content, MAIN_CLASS_TAG, &mainClassLength);
    if (mainClass == NULL || mainClassLength == 0) {
        Alert("No main class in .jam file");
        goto error;
    }

    nameStart = lastIndexOf(mainClass, mainClassLength, '.') + 1;
    dbNameLength = min(31, mainClassLength - nameStart);
    memmove(dbName, (char *)(&mainClass[nameStart]), dbNameLength);
    dbName[dbNameLength] = '\0';

    /* put app name into raw prc image data */
    StrCopy(prc_image, dbName);

    /* apps need to have unique creator IDs in order
     * to be visible from the Launcher. When two apps
     * have the same creator ID, only the one with the
     * earlier creation date will be shown by the
     * Launcher. To create a unique ID, we calculate the
     * 16-bit CRC of the app name, and append it to
     * the chars 'ja', which lets us distinguish 65536
     * apps.
     */
    nameCRC = Crc16CalcBlock(dbName, dbNameLength, 0);
    prc_image[0x40] = 'j';
    prc_image[0x41] = 'a';
    prc_image[0x42] = (nameCRC >> 8) & 0xff;
    prc_image[0x43] = (nameCRC >> 0) & 0xff;

    setStatus("Creating resource file");

    if (!createResourceDB(&resourceDB, dbIdP, dbName, FALSE)) {
        goto error;
    }

    setStatus("Getting jar file");

    jarName = JamGetProp(content, JAR_FILE_URL_TAG, &jarNameLength);
    if (jarName == NULL || jarNameLength == 0) {
        Alert("No jar file in .jam file");
        goto error;
    }

    useOnceStr = JamGetProp(content, USE_ONCE_TAG, &useOnceLength);
    useOnce = (useOnceStr != NULL && useOnceLength == strlen("true")
                && !StrNCompare("true", useOnceStr, useOnceLength));
    handle = DmNewResource(resourceDB, 'use1', 1, sizeof useOnce);
    if (handle == NULL) {
        Alert("Unable to create use-once resource");
        goto error;
    }
    DmWrite(MemHandleLock(handle), 0, &useOnce, sizeof useOnce);
    MemHandleUnlock(handle);
    DmReleaseResource(handle);

    if (!getJarURL(jarName, jarNameLength, host, &port, path)) {
        Alert("No URL for jar file in .jam file");
        goto error;
    }

    jar = fetchURL(host, port, path);
    if (jar == NULL) {
        goto error;
    }
    // Create the resource indicating the main class
    setStatus("Writing class resources");

    handle = DmNewResource(resourceDB, 'Main', 1, mainClassLength);
    if (handle == NULL) {
        Alert("Unable to create main resource");
        goto error;
    }
    DmWrite(MemHandleLock(handle), 0, mainClass, mainClassLength);
    MemHandleUnlock(handle);
    DmReleaseResource(handle);

    // Set the flag saying we expect networking
    if (!useINetLib) {
        handle = DmNewResource(resourceDB, 'Flag', 1, 1);
        DmReleaseResource(handle);
    }

    // Add all the class resources
    if (!addClassesToResource((unsigned char *)jar, content, resourceDB)) {
        // This should be an Alert, but TCK requires it not to be.
        setStatus("Cannot convert classes to resource file");
        if (prefsFlags & PREFERENCES_REPEAT_FLAG) {
            pushTheRunButton();
        }
        goto error;
    }
    result = TRUE;

error:
    // Clean up everything
    if (content != NULL) {
        MemPtrFree(content);
    }
    if (jar != NULL) {
        MemPtrFree(jar);
    }
    if (resourceDB != NULL) {
        DmCloseDatabase(resourceDB);
    }
    if (closeNetwork != NULL) {
        closeNetwork();
    }
    return result;
}

static bool_t
createResourceDB(DmOpenRef *databaseP, LocalID *databaseIdP, char *fileName, bool_t useOnce)
{
    DmOpenRef resourceDB;
    LocalID databaseID;
    Handle img_handle;
    Handle name_handle;
    int index;
    char *buffer;
    bool_t result = FALSE;

    databaseID = DmFindDatabase(myCardNo, fileName);
    if (databaseID != 0) {
        DmDeleteDatabase(myCardNo, databaseID);
    }


    img_handle = MemHandleNew(prc_image_length);
    if (img_handle == NULL) {
        Alert("cannot create image handle");
        goto create_done;
    }

    buffer = MemHandleLock(img_handle);
    MemMove(buffer, prc_image, prc_image_length);

    if (DmCreateDatabaseFromImage(buffer)) {
        Alert("Could not create app DB");
        goto create_done;
    }

    databaseID = DmFindDatabase(myCardNo, fileName);
    if (databaseID == 0) {
        Alert("Cannot find resource database");
        return FALSE;
    }

    resourceDB = DmOpenDatabase(myCardNo, databaseID, dmModeReadWrite);
    if (resourceDB == NULL) {
        Alert("Cannot open resource database");
        return FALSE;
    }

    /* set correct application name */
    index = DmFindResource(resourceDB, 'tAIN', 1000, NULL);
    name_handle = DmGetResourceIndex(resourceDB, index);
    name_handle = DmResizeResource(name_handle, strlen(fileName) + 1);
    DmWrite(MemHandleLock(name_handle), 0, fileName, strlen(fileName) + 1);
    MemHandleUnlock(name_handle);
    DmReleaseResource(name_handle);

    *databaseP = resourceDB;
    *databaseIdP = databaseID;

    result = TRUE;

create_done:
    if (buffer != NULL) MemPtrFree(buffer);
    return result;
}


static UShort readx86UShort(unsigned char **ptr) {
    UShort val =  ((((unsigned short)(((unsigned char *)*ptr)[0])) << 0)  |  \
                   (((unsigned short)(((unsigned char *)*ptr)[1])) << 8));
    *ptr += 2;
    return val;
}

static Long readx86Long(unsigned char **ptr) {
    Long val =    ((((long)(((unsigned char *)*ptr)[0])) << 0) |  \
                   (((long)(((unsigned char *)*ptr)[1])) << 8) |  \
                   (((long)(((unsigned char *)*ptr)[2])) << 16)|  \
                   (((long)(((unsigned char *)*ptr)[3])) << 24));
    *ptr += 4;
    return val;
}

static void writeUShort(unsigned char **ptr, UShort val) {
    *((*ptr)++) = (val >> 8) & 0xff;
    *((*ptr)++) = (val >> 0) & 0xff;
}

static void writeLong(unsigned char **ptr, Long val) {
    *((*ptr)++) = (val >> 24) & 0xff;
    *((*ptr)++) = (val >> 16) & 0xff;
    *((*ptr)++) = (val >> 8) & 0xff;
    *((*ptr)++) = (val >> 0) & 0xff;
}

static bool_t
getJarURL(char * jarName, int jarLength, char *host, int *port, char *path) {
    if (jarLength >= 6 && StrNCompare(jarName, "http:", 5) == 0) {
        if (jarLength >= MAX_URL) {
            goto invalid_manifest;
        }
        parseURL(jarName, jarLength, host, port, path);
    } else {
        // Find the last slash in the name;
        char *q = StrChr(path, '/');
        if (q == NULL) {
            goto invalid_manifest;
        } else {
            char *nextq;
            while ((nextq = StrChr(q + 1, '/')) != NULL) {
                q = nextq;
            }
        }
        if ((q - path + 1 + jarLength) >= MAX_URL) {
            goto invalid_manifest;
        }

        // Overwrite the end of the path with the new suffix
        MemMove(q + 1, jarName, jarLength);
        q[1 + jarLength] = '\0';
    }
    return TRUE;

invalid_manifest:
    return FALSE;
}


/* Call back functions used by the jar directory reader */

static bool_t
addClassesTestFunction(const char *name, int nameLength,
                      int *extraBytes, void *info);

static void
addClassesRunFunction(const char *name, int nameLength,
                      void *newResource, long length, void *info);

struct jarReaderStruct {
    DmOpenRef  resourceDB;
    char*           iconName;
    int             iconNameLength;
    bool_t          noFailures;
    ULong           resourceType;
} ;

static bool_t
addClassesToResource(unsigned char* JARfile, char *jamContent,
                     DmOpenRef resourceDB)
{
    struct jarReaderStruct jarReader;
    struct jarInfoStruct jarInfo;
    char *iconName;
    int iconNameLength;
    if (!openJARFile(JARfile, MemPtrSize(JARfile), &jarInfo)) {
        return FALSE;
    }
    iconName = JamGetProp(jamContent, APPLICATION_ICON_TAG, &iconNameLength);
    if (iconName == NULL || iconNameLength <= 5 ||
           memcmp(iconName + iconNameLength - 4, ".bmp", 4) != 0) {
        iconNameLength = 0;
    }
    jarReader.iconName = iconName;
    jarReader.iconNameLength = iconNameLength;
    jarReader.resourceDB = resourceDB;
    jarReader.noFailures = TRUE;
    loadJARFileEntries(&jarInfo,
                       addClassesTestFunction, addClassesRunFunction,
                       &jarReader);
    return jarReader.noFailures;
}


static bool_t
addClassesTestFunction(const char *name, int nameLength,
                       int *extraBytes, void *info)
{
    ULong newResourceType;
    struct jarReaderStruct *jarInfo = info;
    if (!jarInfo->noFailures) {
        /* If we've failed to read something, just give up reading anything
         * afterwards
         */
        return FALSE;
    }
    if (nameLength >= 6 && memcmp(name + nameLength - 6, ".class", 6) == 0) {
        newResourceType = 'Clas';
        *extraBytes = (nameLength - 6) + 5;
    } else if (jarInfo->iconNameLength > 0
               && memcmp(name, jarInfo->iconName, jarInfo->iconNameLength)==0){
        newResourceType = 'tAIB';
        /* Leave extra bytes at 0 */
    } else {
        newResourceType = 'Rsrc';
        *extraBytes = nameLength + 1;
    }
    jarInfo->resourceType = newResourceType;
    return TRUE;
}

static void
addClassesRunFunction(const char *name, int nameLength,
                      void *newResource, long length, void *info)
{
    struct jarReaderStruct *jarInfo = info;
    DmOpenRef resourceDB = jarInfo->resourceDB;
    ULong newResourceType = jarInfo->resourceType;
    UInt size;
    UShort resourceID;
    Handle handle;

    if (newResource == NULL) {
        jarInfo->noFailures = FALSE;
        return;
    }

    size = MemPtrSize(newResource);

    if (newResourceType == 'Clas') {
        nameLength -= 6;        /* remove .class */
        memcpy((char *)newResource, name, nameLength);
        memset((char *)newResource + nameLength, 0, 5);
    } else if (newResourceType == 'tAIB') {
        /* do nothing */
    } else if (newResourceType == 'Rsrc') {
        memcpy((char *)newResource, name, nameLength);
        ((char *)newResource)[nameLength] = 0;
    }

    resourceID = Crc16CalcBlock((char *)name, nameLength, 0);

    for ( ; ; resourceID = (resourceID + nameLength) & 0xFFFF) {
        Int index = DmFindResource(resourceDB, newResourceType,
                                   resourceID, NULL);
        if (index < 0) {
            break;
        }
    }

    if (newResourceType == 'tAIB') {
        UInt16 index;

        newResource = convertBitmapToIconFormat(newResource);
        if (newResource == NULL) {
            goto decompress_done;
        }
        size = MemPtrSize(newResource);
        index = DmFindResource(resourceDB, 'tAIB', 1000, NULL);
        handle = DmGetResourceIndex(resourceDB, index);
        DmWrite(MemHandleLock(handle), 0, newResource, size);
        MemHandleUnlock(handle);
        DmReleaseResource(handle);

        goto decompress_done;
    }

    handle = DmNewResource(resourceDB, newResourceType, resourceID, size);
    DmWrite(MemHandleLock(handle), 0, newResource, size);
    MemHandleUnlock(handle);
    DmReleaseResource(handle);

decompress_done:
    MemPtrFree(newResource);
}


unsigned char *
convertBitmapToIconFormat(unsigned char *buf) {
    unsigned char *p = buf + 10;
    unsigned char *icon = NULL;
    UShort iconLen;
    unsigned char *icon_buf;
    ULong srcOffset = (ULong)readx86Long(&p);
    ULong size = (ULong)readx86Long(&p);
    ULong width = (ULong)readx86Long(&p);
    ULong height = (ULong)readx86Long(&p);
    UShort bitCount;
    UShort srcRowSize;
    UShort dstRowSize;
    UShort yy, y;
    UShort dstRowStart;
    ULong compression;

    p += 2;

    bitCount = readx86UShort(&p);
    compression = (ULong)readx86Long(&p);

    if (bitCount != 1) {
        goto convert_done;
    }

    if (compression != 0) {
        goto convert_done;
    }

    if (srcOffset < 62) {
        goto convert_done;
    }

    p +=20;

    p +=8; /* skip color table for now */

    srcRowSize = ((width + 31) & ~31) / 8;
    dstRowSize = ((width + 15) & ~15) / 8;

    iconLen = dstRowSize * height;
    if ((iconLen & 1) == 1) iconLen++;

    icon = MemPtrNew(iconLen + 16);
    if (icon == NULL) {
        goto convert_done;
    }

    /* write header */
    icon_buf = icon;
    writeUShort(&icon_buf, (UShort)width);
    writeUShort(&icon_buf, (UShort)height);
    writeUShort(&icon_buf, (UShort)dstRowSize);
    writeUShort(&icon_buf, (UShort)0); /* flags */
    writeLong(&icon_buf, (Long)0);
    writeLong(&icon_buf, (Long)0);

    if (srcOffset > 62) p += (srcOffset - 62);

    for (yy = 0; yy < height; yy++) {
        y = height - yy - 1;
        dstRowStart = y * dstRowSize;
        memcpy(icon_buf + dstRowStart, p, dstRowSize);
        /* p += (srcRowSize - dstRowSize); */
        p += dstRowSize;
    }

convert_done:
    if (buf != NULL) MemPtrFree(buf);

    return icon;
}


static bool_t
parseURL(char * url, int urlLength, char *host, int *port, char *path) {
    char * start, * p;
    unsigned char c;        /* For single characters */
    char *urlEnd = url + urlLength;
    if (urlLength <= 7 || StrNCompare(url, "http:/", 6) != 0) {
        return FALSE;
    }
    p = url + 6;

    if (*p == '/') {
        // Get host name
        start = ++p;
        while (p < urlEnd && ((c = *p) != 0) &&
               (TxtCharIsAlpha(c) || TxtCharIsDigit(c) ||
               c == '.' || c == '-'|| c == '_')) {
            p++;
        }
        if (p - start >= MAX_URL) {
            return FALSE;
        }
        memcpy(host, start, p - start);
        host[p - start] = '\0';

        // Get port number
        if (*p == ':') {
            char num[10];
            p++;
            start = p;
            while (p < urlEnd && TxtCharIsDigit(*(unsigned char *)p)) {
                p++;
            }
            if ((start - p) > 5) {
                return FALSE;
            }
            memcpy(num, start, p - start);
            num[p - start] = '\0';
            *port = StrAToI(num);
            if (*port <= 0 || *port >= 65535) {
                return FALSE;
            }
        } else {
            *port = 80;
        }
    } else {
        // Single slash is part of the path.
        p--;
        StrCopy(host, "localhost");
    }

    start = p;

    // Find everything up until the first space character
    while (p < urlEnd && !TxtCharIsSpace(*(unsigned char *)p)) {
        p++;
    }

    if (p == start) {
        // First thing after the hostname is space
        StrCopy(path, "/");
    } else if (*start != '/' || p - start >= MAX_URL) {
        // Something other than a slash after hostname, or it's too long
        return FALSE;
    } else {
        // Copy the path.
        MemMove(path, start, p - start);
        path[p - start] = 0;
    }
    // Make sure that all that remains is space;
    while (p < urlEnd && TxtCharIsSpace(*(unsigned char *)p)) {
        p++;
    }

    return (p == urlEnd);
}


char *
JamGetProp(char *buffer, char * name, int *length) {
    int nameLength = StrLen(name);
    char *p = buffer;
    char *endP = buffer + MemPtrSize(p);
    unsigned char c;
    char *retval;

    for(;;) {
        if (// At least nameLength + 1 chars
            endP - p > nameLength
            // The first nameLength  match
            && MemCmp(p, name, nameLength) == 0
            // The next character is either a space or a colon
            && ((c = p[nameLength]) != 0)
            && (TxtCharIsSpace(c) || c == ':')) {

            // We have a match.  Let's skip over all the blank space,
            // including an optional colon.
            p += nameLength;
            while (p < endP && *p != '\n' && TxtCharIsSpace(*(unsigned char*)p)) {
                p++;
            }

            if (*p == ':') {
                p++;
            }

            while (p < endP && *p != '\n' && TxtCharIsSpace(*(unsigned char*)p)) {
                p++;
            }

            retval = p;
            while (p < endP && *p != '\n') {
                p++;
            }

            while (p > retval && TxtCharIsSpace(*(unsigned char *)p)) {
                p--;
            }

            *length = (p - retval + 1);
            return retval;
        } else {
            /* Move forward in the string until just after the next new line */
            while (p < endP && *p != '\n') {
                p++;
            }
            if (++p >= endP) {
                return NULL;
            }
        }
    }
}



/* Read completely from the socket.  Return the result */
char *
readCompletely(void *sock, Err (*reader)(void *, char *, long, ULong *))
{
    Handle handle;
    char *buffer;
    int length;

    /* Try to get as big a read buffer as we can get away with */
    for (length = 65000; length > 10000; length -= 1000) {
        handle = DmNewHandle(DmNextOpenResDatabase(0),length + 1);
        if (handle != NULL) {
            break;
        }
    }
    if (handle == NULL) {
        Alert("Cannot create read handle");
        goto error;
    }

    length = 0;            /* it's now the bytes read already */
    buffer = MemHandleLock(handle);

    if (buffer == NULL) {
        Alert("buff == NULL");
        goto error;
    }

    for (;;) {
        ULong read = MemPtrSize(buffer) - length; /* bytes to read */
        Err errno;
        /* reader must know how to deal with databases */
        errno = reader(sock, buffer, length, &read);
        if (errno) {
            Alert("read got error %ld", (long)errno);
            goto error;
        }
        length += read;
        if (length == MemPtrSize(buffer)) {
            Alert("Buffer overflow in readCompletely");
        } else if (read == 0) {
            /* We're done */
            MemHandleUnlock(handle);
            MemHandleResize(handle, length);
            return MemHandleLock(handle);
        }
    }
 error:
    if (handle != NULL) {
        MemHandleFree(handle);
    }
    return NULL;
}


static void setStatusV(bool_t alert, const char *format, va_list args) {
    static char buffer[1024];
    FieldPtr field = GetObjectPtr(MainStatusField);
    StrVPrintF(buffer, format, args);
    if (alert || field == NULL) {
        FrmCustomAlert(AlertAlert, buffer, NULL, NULL);
    }
    if (field != NULL) {
    	FldSetTextPtr(field, (char *)buffer);
        FldRecalculateField(field, true);
    }
}


void Alert(const char *format, ...) {
    va_list args;
    va_start(args, format);
        setStatusV(true, format, args);
    va_end(args);
}

void setStatus(const char *format, ...) {
    va_list args;
    va_start(args, format);
        setStatusV(false, format, args);
    va_end(args);
}

void
setTitle(const char *format) {
    FrmSetTitle(FrmGetActiveForm(), (char *)format);
}

