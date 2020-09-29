/*
 * Copyright 2000-2001 D'Crypt Pte Ltd
 * All rights reserved.
 *
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
 * SUBSYSTEM: DCryptor-specific functions needed classes in com.dcrypt
 *            packages.
 * FILE:      nativeDCryptor.c
 * AUTHOR:    K. H. Ong
 *            Doug Simon
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <uart1.h>
#include <uart5.h>
#include <audio.h>
#include <melp.h>

#include <global.h>

/*
 * Pop and discard the 'this' pointer from the call stack.
 */
#define DISCARD_THIS oneLess

/*
 * Field indexes.
 */
#define UART_id           0
#define UART_blockOnRead  1
#define UART_blockOnWrite 2


/*=========================================================================
 * FUNCTION:      Java_com_dcrypt_UART_enable
 *                Java_com_dcrypt_UART_disable
 *                Java_com_dcrypt_UART_writeReady
 *                Java_com_dcrypt_UART_write
 *                Java_com_dcrypt_UART_readReady
 *                Java_com_dcrypt_UART_read
 * TYPE:          Native method implementation.
 * OVERVIEW:      Implementation of native methods in the com.dcrypt.UART
 *                class. Note that all of the functions assume that the UART
 *                object was initialised properly. That is, it 'id' field
 *                has the value 1 or 5.
 *=======================================================================*/

/*
 * Access and test the 'UART.id' instance field.
 */
#define IS_UART1(this) (this->data[UART_id].cell == 1)

void    Java_com_dcrypt_UART_enable()
{
    INSTANCE this = popStackAsType(INSTANCE);
    if (IS_UART1(this))
        uart1_enable();
    else
        uart5_enable();
}

void    Java_com_dcrypt_UART_disable()
{
    INSTANCE this = popStackAsType(INSTANCE);
    if (IS_UART1(this))
        uart1_disable();
    else
        uart5_disable();
}

void    Java_com_dcrypt_UART_writeReady()
{
    INSTANCE this = popStackAsType(INSTANCE);
    
    if (IS_UART1(this))
        pushStack (uart1_putc_rdy());
    else
        pushStack (uart5_putc_rdy());
}

void    Java_com_dcrypt_UART_nativeWriteByte()
{
    char     ch = popStack();
    INSTANCE this = popStackAsType(INSTANCE);
    
    if (IS_UART1(this)) {
        uart1_putc (ch);
    }
    else {
        uart5_putc (ch);
    }
}
typedef void (*PutcFunctionPtr)(char);  
void    Java_com_dcrypt_UART_nativeWriteString()
{
    STRING_INSTANCE string = popStackAsType(STRING_INSTANCE);
    INSTANCE this = popStackAsType(INSTANCE);
    
    if (string == NULL)
        raiseException(NullPointerException);
    else {
        short* s = string->array->sdata;
        int len = string->length;
        PutcFunctionPtr putcPtr;
        if (IS_UART1(this)) {
            putcPtr = uart1_putc;
        }
        else {
            putcPtr = (PutcFunctionPtr)uart5_putc;
        }
        while (len-- != 0) {
            (*putcPtr)((char)*s);
            s++;
        }
    }
}

void    Java_com_dcrypt_UART_readReady()
{
    INSTANCE this = popStackAsType(INSTANCE);
    
    if (IS_UART1(this))
        pushStack (uart1_getc_rdy()) ;
    else
        pushStack (uart5_getc_rdy()) ;
}

void    Java_com_dcrypt_UART_nativeReadByte()
{
    INSTANCE this = popStackAsType(INSTANCE);
    
    if (IS_UART1(this)) {
        pushStack (uart1_getc());
    }
    else {
        pushStack (uart5_getc()) ;
    }
}

/*=========================================================================
 * FUNCTION:      Java_com_dcrypt_UART1_putDTR
 *                Java_com_dcrypt_UART1_putRTS
 *                Java_com_dcrypt_UART1_getCTS
 *                Java_com_dcrypt_UART1_getDCD
 *                Java_com_dcrypt_UART1_getDSR
 *                Java_com_dcrypt_UART1_getRI
 *                Java_com_dcrypt_UART1_nativeInit
 * TYPE:          Native method implementation.
 * OVERVIEW:      Implementation of native methods in the com.dcrypt.UART1
 *                class.
 *=======================================================================*/

void    Java_com_dcrypt_UART1_nativeInit()
{
    int flow_ctrl = popStack();
    int baud_rate = popStack();
    
    uart1_init(baud_rate,flow_ctrl);
}

void    Java_com_dcrypt_UART1_putDTR()
{
    uart1_put_dtr (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART1_putRTS()
{
    uart1_put_rts (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART1_getCTS()
{
    DISCARD_THIS;
    pushStack (uart1_get_cts()) ;
}

void    Java_com_dcrypt_UART1_getDCD()
{
    DISCARD_THIS;
    pushStack (uart1_get_dcd()) ;
}

void    Java_com_dcrypt_UART1_getDSR()
{
    DISCARD_THIS;
    pushStack (uart1_get_dsr()) ;
}

void    Java_com_dcrypt_UART1_getRI()
{
    DISCARD_THIS;
    pushStack (uart1_get_ri()) ;
}

/*=========================================================================
 * FUNCTION:      Java_com_dcrypt_UART5_getDTR
 *                Java_com_dcrypt_UART5_getRTS
 *                Java_com_dcrypt_UART5_putCTS
 *                Java_com_dcrypt_UART5_putDCD
 *                Java_com_dcrypt_UART5_putDSR
 *                Java_com_dcrypt_UART5_putRI
 *                Java_com_dcrypt_UART5_nativeInit
 *                Java_com_dcrypt_UART5_pcmciaOnline
 * TYPE:          Native method implementation.
 * OVERVIEW:      Implementation of native methods in the com.dcrypt.UART5
 *                class.
 *=======================================================================*/

void    Java_com_dcrypt_UART5_nativeInit()
{
    uart5_init();
}

void    Java_com_dcrypt_UART5_pcmciaOnline()
{
    DISCARD_THIS;
    pushStack(pcmcia_online());
}

void    Java_com_dcrypt_UART5_putDSR()
{
    uart5_put_dsr (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART5_putCTS()
{
    uart5_put_cts (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART5_putDCD()
{
    uart5_put_dcd (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART5_putRI()
{
    uart5_put_ri (popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_UART5_getDTR()
{
    DISCARD_THIS;
    pushStack (uart5_get_dtr()) ;
}

void    Java_com_dcrypt_UART5_getRTS()
{
    DISCARD_THIS;
    pushStack (uart5_get_rts()) ;
}

/*=========================================================================
 * FUNCTION:      Java_com_dcrypt_Audio_enable
 *                Java_com_dcrypt_Audio_disable
 *                Java_com_dcrypt_Audio_nativeInit
 *                Java_com_dcrypt_Audio_nativeTransmit
 *                Java_com_dcrypt_Audio_nativeReceive
 *                Java_com_dcrypt_Audio_nativeSetSpeakerGain
 * TYPE:          Native method implementation.
 * OVERVIEW:      Implementation of native methods in the com.dcrypt.Audio
 *                class.
 *=======================================================================*/

void    Java_com_dcrypt_Audio_nativeSetSampleRate()
{
    audio_init(popStack()) ;
    DISCARD_THIS;
}

void    Java_com_dcrypt_Audio_enable()
{
    DISCARD_THIS;
    audio_enable() ;
}

void    Java_com_dcrypt_Audio_disable()
{
    DISCARD_THIS;
    audio_disable() ;
}

void    Java_com_dcrypt_Audio_nativeTransmit()
{
    SHORTARRAY    audio_frame = popStackAsType (SHORTARRAY);
    DISCARD_THIS;

    memcpy (audio_shared, audio_frame->sdata, sizeof(AUDIO_FRAME)) ;
    pushStack (audio_xmit()) ;
}

void    Java_com_dcrypt_Audio_nativeReceive()
{
    SHORTARRAY    audio_frame = popStackAsType (SHORTARRAY);
        
    DISCARD_THIS;
    pushStack (audio_recv()) ;
    memcpy (audio_frame->sdata, audio_shared, sizeof(AUDIO_FRAME)) ;
}

void    Java_com_dcrypt_Audio_nativeSetSpeakerGain()
{
    int    spk_gain = popStack();

    DISCARD_THIS;
    audio_set_spk_gain (spk_gain) ;
}

/*=========================================================================
 * FUNCTION:      Java_com_dcrypt_Melp_initCompression
 *                Java_com_dcrypt_Melp_initDecompression
 *                Java_com_dcrypt_Melp_nativeCompress
 *                Java_com_dcrypt_Melp_nativeDecompress
 * TYPE:          Native method implementation.
 * OVERVIEW:      Implementation of native methods in the com.dcrypt.Melp
 *                class.
 *=======================================================================*/

void    Java_com_dcrypt_Melp_initCompression()
{
    melp_ana_init (popStack());
    DISCARD_THIS;
}

void    Java_com_dcrypt_Melp_initDecompression()
{
    melp_syn_init (popStack());
    DISCARD_THIS;
}

void    Java_com_dcrypt_Melp_nativeCompress()
{
    BYTEARRAY melp_frame = popStackAsType (BYTEARRAY) ;
    SHORTARRAY audio_frame = popStackAsType (SHORTARRAY) ;
    
    DISCARD_THIS;
    melp_ana_mod (audio_frame->sdata, melp_frame->bdata) ;
}

void    Java_com_dcrypt_Melp_nativeDecompress()
{
    SHORTARRAY    audio_frame = popStackAsType (SHORTARRAY);
    BYTEARRAY    melp_frame = popStackAsType (BYTEARRAY);

    DISCARD_THIS;
    melp_syn_mod (melp_frame->bdata, audio_frame->sdata) ;
}
