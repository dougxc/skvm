/*
 * Copyright 1995-2001 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

package com.sun.cldc.io.palm.http;

import java.io.IOException;

/**
 * Thrown to indicate that there is an error in the underlying
 * protocol.
 */
public class InetException extends IOException {

    /**
     * Constructs a new <code>ProtocolException</code> with the
     * specified detail message.
     *
     * @param   host   the detail message.
     */
    public InetException(String msg) {
        super(msg);
    }

    /**
     * Constructs a new <code>ProtocolException</code> with no detail message.
     */
    public InetException() {
    }
}


