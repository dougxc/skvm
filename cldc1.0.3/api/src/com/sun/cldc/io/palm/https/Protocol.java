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

package com.sun.cldc.io.palm.https;

import com.sun.cldc.io.palm.http.*;
import com.sun.cldc.io.*;
import java.io.*;
import javax.microedition.io.*;

/**
 * This implements the https protocol for the Palm
 *
 * @author  Nik Shaylor
 * @version 2.0 4/11/2000
 */
public class Protocol extends ConnectionBase implements StreamConnection {

    protected boolean sopen = false;
    protected int     opens = 0;
    protected boolean copen = false;
    protected static final int SCHEME_HTTPS = 2;

    Inet   soc = new Inet();
    String url;

    /**
     * Open the connection
     * @param name       the target for the connection
     * @param writeable  a flag that is true if the caller expects
     *                   to write to the connection
     * @param timeouts   a flag to indicate that the caller wants
     *                   timeout exceptions
     */
    public void open(String name, int mode, boolean timeouts)
        throws IOException {

        if (name.charAt(0) != '/' || name.charAt(1) != '/') {
            throw new IllegalArgumentException(
                      "Protocol must start with \"//\" "+name);
        }

        url = name.substring(2);
        soc.InetShowSignal(130, 1);
        soc.InetOpenSock(SCHEME_HTTPS);
        opens++;
        copen = true;
    }

    /**
     * Returns an input stream.
     *
     * @return     an input stream for writing bytes to this port.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          input stream.
     */
    synchronized public InputStream openInputStream() throws IOException {

        if (sopen) {
            throw new IOException("Stream already opened");
        }

        sopen = true;
        soc.InetHTTPReqCreateAndSend("GET", null, url, null);
        opens++;

        return new PrivateInputStream(this);
    }

    /**
     * Returns an output stream.
     *
     * @return     an output stream for writing bytes to this port.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          output stream.
     */
    synchronized public OutputStream openOutputStream() throws IOException {

        if (sopen) {
            throw new IOException("Stream already opened");
        }

        sopen = true;
        opens++;

        return new PrivateOutputStream(this);
    }

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs when closing the
     *                          connection.
     */
    synchronized public void close() throws IOException {
        if (copen) {
            copen = false;
            realClose();
        }
    }

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized void realClose() throws IOException {
        if (--opens == 0) {;
            soc.InetSockClose();
        }
    }

    /*
     * Real primitive methods
     */
    public int readBytes(byte buf[], int off, int len) throws IOException {
        return soc.InetSockRead(buf, off, len);
    }

    public void writeAll(byte buf[]) throws IOException {
        soc.InetHTTPReqCreateAndSend("POST", buf, url, null);
    }
}

/**
 * Input stream for the connection
 */
class PrivateInputStream extends InputStream {

    /**
     * Pointer to the connection
     */
    private Protocol parent;

    /**
     * Buffer for single char reads
     */
    byte[] buf = new byte[1];

    /**
     * Constructor
     * @param pointer to the connection object
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateInputStream(Protocol parent) throws IOException {
        this.parent = parent;
    }

    /**
     * Check the stream is open
     *
     * @exception  IOException  if the stream is not open.
     */
    void ensureOpen() throws IOException {
        if (parent == null) {
            throw new IOException("Stream closed");
        }
    }

    /**
     * Reads the next byte of data from the input stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @return     the next byte of data, or <code>-1</code>
     *             if the end of the stream is reached.
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public int read() throws IOException {
        read(buf, 0, 1);
        return buf[0];
    }

    /**
     * Reads up to <code>len</code> bytes of data from the
     * input stream into an array of bytes.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset in array <code>b</code>
     *                   at which the data is written.
     * @param      len   the maximum number of bytes to read.
     *
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because
     *             the end of the stream has been reached.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public int read(byte b[], int off, int len)
        throws IOException {

        ensureOpen();

        if (b == null) {
            throw new NullPointerException();
        }

        if (len == 0) {
            return 0;
        }

        int count = 0;

        while (count < len) {
            int n = parent.readBytes(b, off+count, len-count);
            if (n == -1) {
                break; // EOF
            }

            if (n < 0) {
                throw new IOException();
            }
            count += n;
        }
        return count;
    }

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public void close() throws IOException {
        if (parent != null) {
            parent.realClose();
            parent.sopen = false;
            parent = null;
        }
    }
}

/**
 * Output stream for the connection
 */
class PrivateOutputStream extends OutputStream {

    /**
     * Pointer to the connection
     */
    Protocol parent;
    ByteArrayOutputStream os = new ByteArrayOutputStream();

    /**
     * Constructor
     *
     * @param pointer to the parent connection
     */
    public PrivateOutputStream(Protocol p) {
        parent = p;
    }

    /**
     * Check the stream is open
     *
     * @exception  IOException  if the stream is not open.
     */
    void ensureOpen() throws IOException {
        if (parent == null) {
            throw new IOException("Stream closed");
        }
    }

    /**
     * Writes the specified byte to this output stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b   the <code>byte</code>.
     * @exception  IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> may be thrown if the
     *             output stream has been closed.
     */
    synchronized public void write(int b) throws IOException {
        ensureOpen();
        os.write(b);
    }

    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this output stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> is thrown if the output
     *             stream is closed.
     */
    synchronized public void write(byte b[], int off, int len)
        throws IOException {

        ensureOpen();

        if (b == null) {
            throw new NullPointerException();
        }

        if (len == 0) {
            return;
        }

        os.write(b, off, len);
    }

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public void close() throws IOException {
        if (parent != null) {
            parent.writeAll(os.toByteArray());
            parent.realClose();
            parent.sopen = false;
            parent = null;
        }
    }
}


