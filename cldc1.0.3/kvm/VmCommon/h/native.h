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
 * FILE:      native.h
 * OVERVIEW:  This file contains the definitions of native functions
 *            needed by the Java Virtual Machine. The implementation
 *            is _not_ based on JNI (Java Native Interface),
 *            because JNI seems too expensive for small devices.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Type definitions and global variables
 *=======================================================================*/

typedef  void NativeFunctionType(void);
typedef  NativeFunctionType *NativeFunctionPtr;

typedef struct {
    const char *const name;
    const char *const signature;
    const NativeFunctionPtr implementation;
} NativeImplementationType;

typedef struct {
    const char *const packageName;
    const char *const baseName;
    const NativeImplementationType *const implementation;
} ClassNativeImplementationType;

#define NATIVE_END_OF_LIST { NULL, NULL }

extern const ClassNativeImplementationType nativeImplementations[];

/*=========================================================================
 * Operations on native functions
 *=======================================================================*/

/* Set to the currently executing native method, or NULL */
extern METHOD CurrentNativeMethod;

NativeFunctionPtr getNativeFunction(INSTANCE_CLASS clazz, 
                                    const char* methodName, 
                                    const char* methodSignature);


void  invokeNativeFunction(METHOD thisMethod);
void  nativeInitialization(int *argc, char **argv);

#if INCLUDEDEBUGCODE
void printNativeFunctions(void);
#endif

/*=========================================================================
 * Native function prototypes
 *=======================================================================*/

/*
 * Function prototypes of those native functions 
 * that are commonly linked into KVM.
 */
void Java_java_lang_Object_getClass(void);
void Java_java_lang_Object_hashCode(void);
void Java_java_lang_System_identityHashCode(void);
void Java_java_lang_Object_notify(void);
void Java_java_lang_Object_notifyAll(void);
void Java_java_lang_Object_wait(void);
void Java_java_lang_Math_randomInt(void);
void Java_java_lang_Class_isInterface(void);
void Java_java_lang_Class_isPrimitive(void);
void Java_java_lang_Class_forName(void);
void Java_java_lang_Class_newInstance(void);
void Java_java_lang_Class_getName(void);
void Java_java_lang_Class_isInstance(void);
void Java_java_lang_Class_isArray(void);
void Java_java_lang_Class_isAssignableFrom(void);
void Java_java_lang_Class_getPrimitiveClass(void) ;
void Java_java_lang_Class_exists(void);
void Java_java_lang_Class_getResourceAsStream0(void);
void Java_java_lang_Thread_activeCount(void);
void Java_java_lang_Thread_currentThread(void);
void Java_java_lang_Thread_yield(void);
void Java_java_lang_Thread_sleep(void);
void Java_java_lang_Thread_start(void);
void Java_java_lang_Thread_isAlive(void);
void Java_java_lang_Thread_setPriority0(void);
void Java_java_lang_Thread_setPriority(void);
void Java_java_lang_Throwable_fillInStackTrace(void);
void Java_java_lang_Throwable_printStackTrace0(void);
void Java_java_lang_Runtime_exitInternal(void);
void Java_java_lang_Runtime_freeMemory(void);
void Java_java_lang_Runtime_totalMemory(void);
void Java_java_lang_Runtime_gc(void);
void Java_java_lang_System_arraycopy (void);
void Java_java_lang_System_currentTimeMillis(void);
void Java_java_lang_System_getProperty0(void);
void Java_java_lang_System_getProperty(void);
void Java_java_lang_String_charAt(void);
void Java_java_lang_String_equals(void);
void Java_java_lang_String_indexOf__I(void);
void Java_java_lang_String_indexOf__II(void);
void Java_java_lang_String_intern(void);
void Java_java_lang_StringBuffer_append__I(void);
void Java_java_lang_StringBuffer_append__Ljava_lang_String_2(void);
void Java_java_lang_StringBuffer_toString(void);
void Java_java_util_Calendar_init(void);
void Java_java_io_PrintStream_putchar(void);

void printString(INSTANCE string);

void Java_com_sun_cldc_io_j2me_datagram_Protocol_open0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_send0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_receive0(void);
void Java_com_sun_cldc_io_j2me_datagram_Protocol_close0(void);

void Java_com_sun_cldc_io_j2me_socket_Protocol_open0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_read0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_write0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_available0(void);
void Java_com_sun_cldc_io_j2me_socket_Protocol_close0(void);

void Java_com_sun_cldc_io_j2me_serversocket_Protocol_open(void);
void Java_com_sun_cldc_io_j2me_serversocket_Protocol_accept(void);
void Java_com_sun_cldc_io_j2me_serversocket_Protocol_close(void);

void Java_com_sun_cldc_io_GeneralBase_iowait(void);

void Java_com_sun_cldc_io_NetworkConnectionBase_initializeInternal(void);

void Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar(void);

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_open(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_close(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readInt(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readByteArray(void);
void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readUTF(void);
