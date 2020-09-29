/*
 * @(#)ClassFormatError.java	1.17 00/02/02
 *
 * Copyright 1994-2000 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package javax.microedition.cbs;

/**
 * Thrown when the Java Virtual Machine attempts to read a trusted class 
 * file and determines that the Trusted attribute in the file is malformed
 * or otherwise cannot be interpreted correctly. 
 */
public class TrustedClassFormatError extends CBSError {
    /**
     * Constructs a <code>TrustedClassFormatError</code> with no detail message.
     */
    public TrustedClassFormatError() {
      	super();
    }

    /**
     * Constructs a <code>TrustedClassFormatError</code> with the specified 
     * detail message. 
     *
     * @param   s   the detail message.
     */
    public TrustedClassFormatError(String s) {
      	super(s);
    }
}
