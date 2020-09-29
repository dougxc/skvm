/*
 *  Copyright 2000-2001 D'Crypt Pte Ltd
 *  All rights reserved.
 *
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
package com.dcrypt ;

/**
 * Provide access to UART1 on the D'Cryptor platform.
 * All the native methods are static as there is only one UART1
 * at the native level. Note that this poses concurrency issues when
 * multiple instances of this class are used at the same time.
 */
public final class UART1 extends UART
{
    /**
     * Construct a UART1 object.
     * @param baudRate
     * @param flowControl
     * @exception IllegalArgumentException if either baudRate or
     * flowControl are not legal values.
     */
    public UART1(int baudRate, int flowControl)
        throws IllegalArgumentException
    {
        super(1);
        switch (baudRate) {
            case BAUD_230400:
            case BAUD_115200:
            case BAUD_57600:
            case BAUD_38400:
            case BAUD_19200:
            case BAUD_14400:
            case BAUD_9600:
            case BAUD_4800:
            case BAUD_2400:
            case BAUD_1200: break;
            default:
                throw new IllegalArgumentException();
        }
        switch (flowControl) {
            case HW_FLOW:
            case SW_FLOW:
            case NO_FLOW:
                break;
            default:
                throw new IllegalArgumentException();
        }
        nativeInit (baudRate, flowControl) ;
    }

    /**
     * Assert DTR when assert is true, deassert DTR otherwise.
     */
    public native void putDTR (boolean azzert);

    /**
     * Assert RTS when assert is true, deassert RTS otherwise.
     */
    public native void putRTS (boolean azzert);

    /**
     * Test if CTS is asserted.
     * @return true when CTS is asserted, false otherwise.
     */
    public native boolean getCTS();

    /**
     * Test if DCD is asserted.
     * @return true when DCD is asserted, false otherwise.
     */
    public native boolean getDCD();

    /**
     * Test if DSR is asserted.
     * @return true when DSR is asserted, false otherwise.
     */
    public native boolean getDSR();

    /**
     * Test if RI is asserted.
     * @return true when RI is asserted, false otherwise.
     */
    public native boolean getRI();

    private static native void nativeInit (int baudRate, int flowCtrl) ;
}
