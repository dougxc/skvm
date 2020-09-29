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
 * SUBSYSTEM: event handling
 * FILE:      nativeSpotlet.c
 * OVERVIEW:  This file defines the routines to intercept events
 *            from the host operating system (such as Palm OS),
 *            and invoke the registered handler methods for events
 *            that are made visible at the Java level.  This file
 *            also implements a fake database system for emulating
 *            Palm databases on desktop platforms.
 * AUTHOR:    Doug Simon (original version), Antero Taivalsaari
 *            Various authors for KVM
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Event handling functions
 *=======================================================================*/

/* The option flags when registering for events. */
enum eventOptions {
    NOEVENTOPTIONS = 0x00,
    WANTSYSTEMKEYS = 0x01   /* User wants system "hard" key events */
};

int wantPalmSysKeys;

void Java_com_sun_kjava_Spotlet_setPalmEventOptions(void)
{
    wantPalmSysKeys = popStack() & WANTSYSTEMKEYS;
}

/*=========================================================================
 * Fake definitions of beamSend/getFlashID functions for desktop platforms
 *=======================================================================*/

#ifndef PILOT

void Java_com_sun_kjava_Spotlet_getFlashID(void) {
    fatalError(KVM_MSG_NOT_IMPLEMENTED);
}

void Java_com_sun_kjava_Spotlet_beamSend(void) {
    fatalError(KVM_MSG_NOT_IMPLEMENTED);
}

#endif /* NOT PILOT */

/*=========================================================================
 * Fake database system to emulate Palm databases on desktop platforms
 *=======================================================================*/

/* 
 * This code implements fake Palm OS database support
 * for the old com.sun.kjava "legacy" classes.  It allows
 * us to emulate Palm databases on Windows and Unix.
 */

#ifndef PILOT

#ifdef UNIX
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef _S_IREAD
#define _S_IREAD 0444
#endif

#ifndef _S_IWRITE
#define _S_IWRITE 0220
#endif

/*
 * Open a file but create if missing
 */
static int real_openFile(INSTANCE instance, char *name, bool_t writing) {
    if(writing) {
        return open(name, O_CREAT | O_RDWR   | O_BINARY, _S_IREAD | _S_IWRITE);
    } else {
        return open(name, O_RDONLY | O_BINARY);
    }
}

static char *prefix = "Database";
static char name[100];

int Fake_DmCreateDatabase(int cardNumber, char *rawName, int creatorID, int typeID, int resourceDB) {
    return TRUE;
}

long Fake_DmOpenDatabaseByTypeCreator(int typeID, int creatorID, int mode) {
    return 1;
}

int Fake_DmCloseDatabase(long dbRef) {
    return TRUE;
}

int Fake_DmNumRecords(long dbRef) {
   return -1;
}

int Fake_DmGetRecord(long dbRef, int recordNumber, char *buffer) {
    int length;
    int fd;
    sprintf(name, "%s_%d", prefix, recordNumber);

    fd = real_openFile(0, name, FALSE);
    if(fd < 0) {
        return -1;
    }
    length = read(fd, buffer, 65536);
    close(fd);
    return length;
}

int Fake_DmWrite(long dbRef, int recordNumber, char *data, int length) {
    int fd;
    sprintf(name, "%s_%d", prefix, recordNumber);
    fd = real_openFile(0, name, TRUE);
    if(fd < 0) {
        return FALSE;
    }
    write(fd, data, length);
    close(fd);
    return TRUE;
}

int Fake_DmRemoveRecord(long dbRef, int recordNumber) {
    sprintf(name, "%s_%d", prefix, recordNumber);
    return (remove(name) == 0);
}

#ifdef sp
#undef sp
#undef ip
#undef fp
#endif

void Java_com_sun_kjava_Database_create(void) {
    int resourceDB = popStack();
    int typeID     = popStack();
    int creatorID  = popStack();
    STRING_INSTANCE name  = (STRING_INSTANCE)popStack();
    int cardNumber = topStack;

    char* rawName = getStringContents(name);
    int err;

    err = Fake_DmCreateDatabase(cardNumber, rawName, creatorID, 
                                typeID, resourceDB);

    if (err == 0)
         topStack = TRUE;
    else topStack = FALSE;
}

void Java_com_sun_kjava_Database_open(void) {
    long mode      = popStack();
    long creatorID = popStack();
    long typeID    = topStack;

    long dbRef = (long)Fake_DmOpenDatabaseByTypeCreator(typeID, 
                       creatorID, mode);

    topStack = dbRef;
}

void Java_com_sun_kjava_Database_close(void) {
    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)popStack();

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Close the database */
    if (dbRef) Fake_DmCloseDatabase(dbRef);
}

void Java_com_sun_kjava_Database_getNumberOfRecords(void) {
    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Read the number of records */
    if (dbRef) {
        topStack = Fake_DmNumRecords(dbRef);
    } else {
        topStack = 0;
    }
}

static char dbuffer[65536];

void Java_com_sun_kjava_Database_getRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    int length = Fake_DmGetRecord(dbRef, recordNumber, dbuffer);

    /* Skip deleted/non-existing records */
    if (length >= 0) {
        /* Instantiate new bytearray */
        ARRAY_CLASS bac = (ARRAY_CLASS)getClass("[B");
        BYTEARRAY byteArray = (BYTEARRAY)instantiateArray(bac, length);

        /* Copy data from record to bytearray */
        int i;
        for (i = 0; i < length; i++) {
            byteArray->bdata[i] = dbuffer[i];
        }

        /* Unlock the record handle and release the record */
        topStack = (cell)byteArray;
    } else {
        topStack = NIL;
    }
}

void Java_com_sun_kjava_Database_setRecord(void) {

    BYTEARRAY byteArray = (BYTEARRAY)popStack();

    unsigned short recordNumber = (unsigned short)popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    topStack = Fake_DmWrite(dbRef, recordNumber,
                   (char*)byteArray->bdata, byteArray->length);
}

void Java_com_sun_kjava_Database_deleteRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Assume that operation may fail */
    topStack = FALSE;

    /* Ensure that database is open */
    if (!dbRef) return;

    /* Remove the record from the database */
    topStack = Fake_DmRemoveRecord(dbRef, recordNumber);
}

void Java_com_sun_kjava_Database_readRecordToBuffer(void) {
    fatalError(KVM_MSG_NOT_IMPLEMENTED);
}

void Java_com_sun_kjava_Database_writeRecordFromBuffer(void) {
    fatalError(KVM_MSG_NOT_IMPLEMENTED);
}

#endif /* NOT PILOT */

/* TEMP - To allow old classes to work */
void Java_com_sun_kjava_DebugIO_putchar(void)
{
    Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar();
}


