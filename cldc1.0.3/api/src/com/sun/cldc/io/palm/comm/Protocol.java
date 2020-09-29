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

package com.sun.cldc.io.palm.comm;

import com.sun.cldc.io.*;
import java.io.*;
import javax.microedition.io.*;

/**
 * This implements the comm port protocol for the Palm
 *
 * @author  Nik Shaylor
 * @version 2.0 4/11/2000
 * @version 2.1 11/7/2000
 */
public class Protocol extends ConnectionBase implements StreamConnection {

    protected int     opens = 0;
    protected boolean copen = false;
    OutputStream  outputStream;
    InputStream   inputStream;

    // From SerialMgr.h
    private final static
    int serSettingsFlagStopBits1    = 0x00000000; // 1 stop bits

    private final static
    int serSettingsFlagStopBits2    = 0x00000001; // 2 stop bits

    private final static
    int serSettingsFlagParityOnM    = 0x00000002; // mask for parity ON

    private final static
    int serSettingsFlagParityEvenM  = 0x00000004; // mask for parity EVEN

    private final static
    int serSettingsFlagBitsPerChar7 = 0x00000080; // 7 bits/char

    private final static
    int serSettingsFlagBitsPerChar8 = 0x000000C0; // 8 bits/char

    private final static
    int serSettingsFlagRTSAutoM = 0x00000010; // mask for RTS rcv flow control

    private final static
    int serSettingsFlagCTSAutoM = 0x00000020; // mask for CTS xmit flow control

    int bbc    = serSettingsFlagBitsPerChar8;
    int stop   = serSettingsFlagStopBits1;
    int parity = 0;
    int rts    = serSettingsFlagRTSAutoM;
    int cts    = serSettingsFlagCTSAutoM;
    int baud   = 19200;
    int blocking = -1;

    void readParameter(String parm, int start, int end) throws IOException {
        parm = parm.substring(start, end);

        if (parm.equals("baudrate=300"))   { baud = 300;   return; }
        if (parm.equals("baudrate=1200"))  { baud = 1200;  return; }
        if (parm.equals("baudrate=2400"))  { baud = 2400;  return; }
        if (parm.equals("baudrate=4800"))  { baud = 4800;  return; }
        if (parm.equals("baudrate=9600"))  { baud = 9600;  return; }
        if (parm.equals("baudrate=19200")) { baud = 19200; return; }
        if (parm.equals("baudrate=38400")) { baud = 38400; return; }
        if (parm.equals("baudrate=57600")) { baud = 57600; return; }

        if (parm.equals("bitsperchar=7")) {
            bbc = serSettingsFlagBitsPerChar7;
            return;
        }

        if (parm.equals("bitsperchar=8")) {
            bbc = serSettingsFlagBitsPerChar8;
            return;
        }

        if (parm.equals("stopbits=1")) {
            stop = serSettingsFlagStopBits1;
            return;
        }

        if (parm.equals("stopbits=2")) {
            stop = serSettingsFlagStopBits2;
            return;
        }

        if (parm.equals("parity=none")) {
            parity = 0;
            return;
        }

        if (parm.equals("parity=odd")) {
            parity = serSettingsFlagParityOnM;
            return;
        }

        if (parm.equals("parity=even")) {
            parity = serSettingsFlagParityEvenM;
            return;
        }

        if (parm.equals("autorts=off")) {
            rts = 0; return;
        }

        if (parm.equals("autorts=on")) {
            rts = serSettingsFlagRTSAutoM;
            return;
        }

        if (parm.equals("autocts=off")) {
            cts = 0;
            return;
        }

        if (parm.equals("autocts=on")) {
            cts = serSettingsFlagCTSAutoM;
            return;
        }

        if (parm.equals("blocking=off")) {
            blocking = 1;
            return;
        }

        if (parm.equals("blocking=on")) {
            blocking = -1;
            return;
        }

        throw new IllegalArgumentException("Bad parameter");
    }

    /**
     * Open the connection
     *
     * @param name       the target for the connection
     * @param writeable  a flag that is true if the caller expects
     *                   to write to the connection
     * @param timeouts   a flag to indicate that the caller wants
     *                   timeout exceptions
     */
    public void open(String name, int mode, boolean timeouts)
        throws IOException {

        if (name.charAt(0) != '0') {
            throw new IllegalArgumentException("Bad port number");
        }

        int start = 0;
        int pos = 1;
        while (name.length() > pos) {
            if (name.charAt(pos) != ';') {
                throw new IllegalArgumentException("Bad parameter");
            }
            pos++;
            start = pos;
            while (true) {
                if (pos == name.length()) {
                    readParameter(name, start, pos);
                    break;
                }
                if (name.charAt(pos) == ';') {
                    readParameter(name, start, pos);
                    break;
                }
                pos++;
            }
        }

        prim_realOpen(baud, bbc|stop|parity|rts|cts, -1, blocking);
        inputStream  = new PrivateInputStream(this);
        outputStream = new PrivateOutputStream(this);
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
        opens++;
        return inputStream;
    }

    /**
     * Returns an output stream.
     *
     * @return     an output stream for writing bytes to this port.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          output stream.
     */
    synchronized public OutputStream openOutputStream() throws IOException {
        opens++;
        return outputStream;
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
        if (--opens == 0) {
             prim_close();
        }
    }

    /*
     * Real primitive methods
     */
    private native void prim_realOpen(int baud, int flags,
        int ctsTimeout, int receiveTimeout) throws IOException;

    public  native int  prim_readBytes(byte b[], int off, int len)
        throws IOException;

    public  native void prim_writeBytes(byte b[], int off, int len)
        throws IOException;

    private native void prim_close() throws IOException;
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
     * Check if the stream is open
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
        if (read(buf, 0, 1) > 0) {
            return (buf[0] & 0xFF);
        }
        return -1;
    }

    /**
     * Reads up to <code>len</code> bytes of data from the input stream into
     * an array of bytes.
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
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
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

        while (true) {
            count += parent.prim_readBytes(b, off+count, len-count);
            if (count == len) {
                return count;
            }
            if (count > len) {
                throw new IOException("Read overrun in comm:");
            }
            GeneralBase.iowait(); /* Wait a while for I/O to become ready */
        }
    }

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public void close() throws IOException {
        if (parent != null) {
            parent.realClose();
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

    /**
     * Buffer for single char writes
     */
    byte[] buf = new byte[1];

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
        buf[0] = (byte)b;
        write(buf, 0, 1);
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

        parent.prim_writeBytes(b, off, len);
    }

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public void close() throws IOException {
        if (parent != null) {
            parent.realClose();
            parent = null;
        }
    }
}


