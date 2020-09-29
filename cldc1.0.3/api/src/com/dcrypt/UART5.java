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
 * Provide access to UART5 on the D'Cryptor platform.
 */
public final class UART5 extends UART
{
    /**
     * Construct a UART5 object.
     */
    public UART5() {
        super(5);
        nativeInit();
    }

    /**
     * Assert DSR when assert is true, deassert DSR otherwise.
     */
    public native void putDSR (boolean azzert);

    /**
     * Assert CTS when assert is true, deassert CTS otherwise.
     */
    public native void putCTS (boolean azzert);

    /**
     * Assert DCD when assert is true, deassert DCD otherwise.
     */
    public native void putDCD (boolean azzert);

    /**
     * Assert RI when assert is true, deassert RI otherwise.
     */
    public native void putRI (boolean azzert);

    /**
     * Test if DTR is asserted.
     * @return true when DTR is asserted, false otherwise.
     */
    public native boolean getDTR();

    /**
     * Test if RTS is asserted.
     * @return true when RTS is asserted, false otherwise.
     */
    public native boolean getRTS();

    /**
     * Test whether or not the D'Cryptor card has been slotted into the
     * PCMCIA slot and has been successfully detected by the host.
     * @return true is card is in and detected.
     */
    public native boolean pcmciaOnline();

    private static native void nativeInit () ;
}
