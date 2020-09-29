/*
 * Copyright 1995-2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */
package javax.microedition.cbs;

/**
 * Signals that a Capabilities Based Security exception of some sort has
 * occurred.
 */
public class CBSError extends Error {
    /**
     * Constructs an <code>CBSError</code> with no
     * detail message.
     */
    public CBSError() {
        super();
    }

    /**
     * Constructs an <code>CBSError</code> with the specified
     * detail message.
     *
     * @param   s   the detail message.
     */
    public CBSError(String s) {
        super(s);
    }
}
