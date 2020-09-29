/*
 *  Copyright (c) 1999-2001 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */
package com.dcrypt;

/**
 * Base class for the UART1 and UART5 classes which provide an interface to
 * the UART ports of a dCryptor device. Note that this is a closed base
 * class. That is, it can only be subclassed by the known UART classes
 * (i.e. UART1 and UART5).
 */
public abstract class UART
{
    /**
     * Baud rate constants.
     */
    public static final int BAUD_230400 = 0x00 ;
    public static final int BAUD_115200 = 0x01 ;
    public static final int BAUD_57600  = 0x03 ;
    public static final int BAUD_38400  = 0x05 ;
    public static final int BAUD_19200  = 0x0b ;
    public static final int BAUD_14400  = 0x0f ;
    public static final int BAUD_9600   = 0x17 ;
    public static final int BAUD_4800   = 0x2f ;
    public static final int BAUD_2400   = 0x5f ;
    public static final int BAUD_1200   = 0xbf ;

    /**
     * Flow control constants.
     */
    public static final int HW_FLOW = 0x00 ;
    public static final int SW_FLOW = 0x01 ;
    public static final int NO_FLOW = 0x02 ;

    /**
     * Which UART port are we, 1 or 5.
     */
    private int id;

    /**
     * Blocking modes.
     */
    private boolean blockOnRead  = true;
    private boolean blockOnWrite = true;

    /**
     * Contruct an instance representing to a specific port.
     */
    protected UART(int id) {
        if (id != 1 && id != 5)
            throw new IllegalArgumentException();
        this.id = id;
    }

    /**
     * Toggle the blocking mode of writing to this port.
     */
    public void setBlockOnWrite(boolean flag) {
        blockOnWrite = flag;
    }

    /**
     * Toggle the blocking mode of reading from this port.
     */
    public void setBlockOnRead(boolean flag) {
        blockOnRead = flag;
    }

    /**
     * Return the blocking mode of reading from this port.
     */
    public boolean getBlockOnRead() {
        return blockOnRead;
    }

    /**
     * Return the blocking mode of writing to this port.
     */
    public boolean getBlockOnWrite() {
        return blockOnWrite;
    }

    /**
     * Enable UART port.
     */
    public native void enable();

    /**
     * Disable UART port.
     */
    public native void disable();

    /**
     * Test whether or not UART port is ready to write a byte.
     * @return true if UART is ready to write a byte, false otherwise.
     */
    public native boolean writeReady();

    /**
     * Write a byte to the UART port. Call writeReady() to check whether
     * UART port is ready before calling this function.
     */
    public void write (byte ch) {
        if (blockOnWrite)
            while (!writeReady())
                Thread.yield();
        nativeWriteByte(ch);
    }

    /**
     * Write a string to the UART port. Call writeReady() to check whether
     * UART port is ready before calling this function.
     */
    public void write (String s) {
        if (blockOnWrite) {
            char[] chars = s.toCharArray();
            for (int i = 0; i != chars.length; i++) {
                while (!writeReady())
                    Thread.yield();
                nativeWriteByte((byte)chars[i]);
            }
        }
        else
            nativeWriteString(s);
    }

    /**
     * Test whether or not UART port is ready to read a byte.
     * @return true if UART is ready to write a byte, false otherwise.
     */
    public native boolean readReady();

    /**
     * Receive a byte from UART port. Call readReady() to check whether
     * UART port is ready before calling this function.
     */
    public byte read() {
        if (blockOnRead)
            while(!readReady())
                Thread.yield();
        return nativeReadByte();
    }

    private native byte nativeReadByte();
    private native void nativeWriteByte(byte ch);
    private native void nativeWriteString(String s);
}
