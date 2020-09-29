/*
 * Copyright 1994-2001 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package javax.microedition.cbs;

/**
 * <code>TrustedLoader</code> is a singleton class that implements a
 * mechanism for converting a classfile (as an array of bytes) to a Class.
 */
public final class TrustedLoader {
    /**
     * Hide the constructor.
     */
    private TrustedLoader() {}

    /**
     * Convert an array of bytes into a trusted instance of class
     * <code>Class</code>.
     * @param name The name of the class.
     * @param b The bytes that make up the class data.
     * @param off The start offset of the class data.
     * @param len The length of the class data.
     * @exception IllegalSubclassError if the named class already
     * exists in the system (either as a loaded class or a classfile known
     * to the classfile manager) or it is not a trusted class or the subclass
     * privilege verification failed or the data did not contain a valid
     * class. This instance of IllegalSubclassError can only be caught
     * by a trusted class.
     * @exception IndexOutOfBoundsException if either <code>off</code> or
     * <code>len</code> is negative or if <code>off+len &gt; b.length</code>.
     * @exception NullPointerException if <code>name</code> is null.
     * @exception ExceptionInInitializerException if the initialization
     * provoked by this method fails.
     */
    public static Class loadTrustedClass(String name, byte[] b, int off,
        int len) throws IllegalSubclassError, IndexOutOfBoundsException,
        NullPointerException
    {
        if (name == null)
            throw new NullPointerException();
        if (off < 0 || len < 0 || (off+len) > b.length)
            throw new IndexOutOfBoundsException();
        
        return nativeLoadTrustedClass(name,b,off,len);
    }

    private static native Class nativeLoadTrustedClass(String name, byte[] b,
        int off,int len);
};
 
