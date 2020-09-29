/*
 * Copyright 1995-2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */
package javax.microedition.cbs;

/**
 * Thrown by the VM to indicate that a class resource access privilege
 * failed upon verification. If this exception is thrown by the VM under these
 * circumstances, it can only be caught by a handler in a trusted class.
 */
public class IllegalClassResourceAccessError extends CBSError {
    /**
     * Constructs an <code>IllegalClassResourceAccessError</code> with no
     * detail message.
     */
    public IllegalClassResourceAccessError() {
        super();
    }

    /**
     * Constructs an <code>IllegalClassResourceAccessError</code>
     * with the specified detail message.
     *
     * @param   s   the detail message.
     */
    public IllegalClassResourceAccessError(String s) {
        super(s);
    }
}
