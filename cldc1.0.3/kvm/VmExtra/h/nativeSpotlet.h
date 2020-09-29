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
 * FILE:      nativeSpotlet.h
 * OVERVIEW:  This file defines the low-level operations for event
 *            handling.  Originally, these functions were closely
 *            coupled with the "Spotlet" application model that was
 *            inherited from the Spotless JVM (precursor of KVM).
 *            The current version is more decoupled from a specific
 *            application model, and should be relatively easy to 
 *            generalize for use with other graphics libraries.
 * AUTHOR:    Doug Simon (original version), Antero Taivalsaari
 *            Various authors for KVM
 *=======================================================================*/

#ifndef _NATIVE_SPOTLET_H_
#define _NATIVE_SPOTLET_H_

/*=========================================================================
 * Definitions and declarations
 *=======================================================================*/

/* The type of events for which handlers can be registered. Corresponds
 * exactly with the handler methods defined in com.sun.kjava.Spotlet.
 * The "pen*" constants must be the first three elements in the list
 * and in the order DOWN,UP,MOVE to correspond with the definition
 * in the events enumerated type in Events.h.
 */
enum KVMEventTypes {
    /* appStopKVMEvent = -1 */
    beamReceiveKVMEvent = 0,
    penDownKVMEvent = 1,
    penUpKVMEvent   = 2,
    penMoveKVMEvent = 3,
    keyDownKVMEvent = 4
};

/*=========================================================================
 * Function prototypes for the native event handling & graphics operations
 *=======================================================================*/

void Java_com_sun_kjava_DebugIO_putchar(void);
void Java_com_sun_kjava_Spotlet_register(void);
void Java_com_sun_kjava_Spotlet_unregister(void);
void Java_com_sun_kjava_Spotlet_register0(void);
void Java_com_sun_kjava_Spotlet_setPalmEventOptions(void);

void Java_com_sun_kjava_Spotlet_unregister0(void);
void Java_com_sun_kjava_Spotlet_beamSend(void);
void Java_com_sun_kjava_Spotlet_getFlashID(void);
void Java_com_sun_kjava_Graphics_copyOffScreenRegion(void);
void Java_com_sun_kjava_Graphics_copyRegion(void);
void Java_com_sun_kjava_Graphics_drawBitmap(void);
void Java_com_sun_kjava_Graphics_drawBorder(void);
void Java_com_sun_kjava_Graphics_drawLine(void);
void Java_com_sun_kjava_Graphics_drawRectangle(void);
void Java_com_sun_kjava_Graphics_drawString(void);
void Java_com_sun_kjava_Graphics_getHeight(void);
void Java_com_sun_kjava_Graphics_getWidth(void);
void Java_com_sun_kjava_Graphics_playSound(void);
void Java_com_sun_kjava_Graphics_playSoundHz(void);
void Java_com_sun_kjava_Graphics_playSMF(void);
void Java_com_sun_kjava_Graphics_resetDrawRegion(void);
void Java_com_sun_kjava_Graphics_setDrawRegion(void);
void Java_com_sun_kjava_Database_createDatabase(void);
void Java_com_sun_kjava_Database_openDatabase(void);
void Java_com_sun_kjava_Database_closeDatabase(void);
void Java_com_sun_kjava_Database_getNumberOfRecords(void);
void Java_com_sun_kjava_Database_getRecord(void);
void Java_com_sun_kjava_Database_setRecord(void);
void Java_com_sun_kjava_Database_deleteRecord(void);

#ifndef PILOT

int  Fake_DmCreateDatabase(int cardNumber, char *rawName, 
            int creatorID, int typeID, int resourceDB);
long Fake_DmOpenDatabaseByTypeCreator(int typeID, int creatorID, int mode);
int  Fake_DmCloseDatabase(long dbRef);
int  Fake_DmNumRecords(long dbRef);
int  Fake_DmGetRecord(long dbRef, int recordNumber, char *buffer);
int  Fake_DmWrite(long dbRef, int recordNumber, char *data, int length);
int  Fake_DmRemoveRecord(long dbRef, int recordNumber);

#endif /* !PILOT */

#endif /* _NATIVE_SPOTLET_H_ */


