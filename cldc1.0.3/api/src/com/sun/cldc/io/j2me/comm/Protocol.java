/*
 *  Copyright (c) 2000-2001 Sun Microsystems, Inc., 901 San Antonio Road,
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

package com.sun.cldc.io.j2me.comm;

import com.sun.cldc.io.*;
import java.io.*;
import javax.microedition.io.*;

/**
 * This implements the comm port protocol
 *
 * @author  Nik Shaylor
 * @author  Stephen Flores
 * @author  Efren Serra
 * @version 3.1 5/11/2001
 */
public class Protocol extends ConnectionBase
                      implements StreamConnection {

    /** Size of the read ahead / write behind buffers */
    public static int bufferSize = 0; /* Default is no buffering */

    /**
     * Class initializer
     */
    static {
        /* See if a read ahead / write behind buffer size has been specified */
        String size = System.getProperty("com.sun.cldc.io.j2me.comm.buffersize");
        if(size != null) {
            try {
                bufferSize = Integer.parseInt(size);
            } catch(NumberFormatException ex) {}
        }
    }

    protected int     opens = 0;
    protected boolean copen = false;

    OutputStream      outputStream;
    InputStream       inputStream;

    int hPort;  // a native handle to the serial port

    // From SerialMgr.h of the Palm api
    private final static int serSettingsFlagStopBits1     = 0x00000000;
              // 1 stop bits
    private final static int serSettingsFlagStopBits2     = 0x00000001;
              // 2 stop bits
    private final static int serSettingsFlagParityOddM    = 0x00000002;
              // mask for parity on
    private final static int serSettingsFlagParityEvenM   = 0x00000004;
              // mask for parity even
    private final static int serSettingsFlagRTSAutoM      = 0x00000010;
              // mask for RTS rcv flow control
    private final static int serSettingsFlagCTSAutoM      = 0x00000020;
              // mask for CTS xmit flow control
    private final static int serSettingsFlagBitsPerChar7  = 0x00000080;
              // 7 bits/char
    private final static int serSettingsFlagBitsPerChar8  = 0x000000C0;
              // 8 bits/char

    int bbc      = serSettingsFlagBitsPerChar8;
    int stop     = serSettingsFlagStopBits1;
    int parity   = 0;
    int rts      = serSettingsFlagRTSAutoM;
    int cts      = serSettingsFlagCTSAutoM;
    int baud     = 19200;
    int blocking = 1;

    private void parseParameter(String parm, int start, int end)
             throws IOException {

        parm = parm.substring(start, end);
        //System.out.println("Testing "+parm);

        if (parm.equals("baudrate=110")) {
            baud = 110;
        } else if (parm.equals("baudrate=300")) {
            baud = 300;
        } else if (parm.equals("baudrate=600")) {
            baud = 600;
        } else if (parm.equals("baudrate=1200")) {
            baud = 1200;
        } else if (parm.equals("baudrate=2400")) {
            baud = 2400;
        } else if (parm.equals("baudrate=4800")) {
            baud = 4800;
        } else if (parm.equals("baudrate=9600")) {
            baud = 9600;
        } else if (parm.equals("baudrate=14400")) {
            baud = 14400;
        } else if (parm.equals("baudrate=19200")) {
            baud = 19200;
        } else if (parm.equals("baudrate=38400")) {
            baud = 38400;
        } else if (parm.equals("baudrate=56000")) {
            baud = 56000;
        } else if (parm.equals("baudrate=57600")) {
            baud = 57600;
        } else if (parm.equals("baudrate=115200")) {
            baud = 115200;
        } else if (parm.equals("baudrate=128000")) {
            baud = 128000;
        } else if (parm.equals("baudrate=256000")) {
            baud = 256000;
        } else if (parm.equals("bitsperchar=7")) {
            bbc   = serSettingsFlagBitsPerChar7;
        } else if (parm.equals("bitsperchar=8")) {
            bbc   = serSettingsFlagBitsPerChar8;
        } else if (parm.equals("stopbits=1")) {
            stop   = serSettingsFlagStopBits1;
        } else if (parm.equals("stopbits=2")) {
            stop   = serSettingsFlagStopBits2;
        } else if (parm.equals("parity=none")) {
            parity = 0;
        } else if (parm.equals("parity=odd")) {
            parity = serSettingsFlagParityOddM;
        } else if (parm.equals("parity=even")) {
            parity = serSettingsFlagParityEvenM;
        } else if (parm.equals("autorts=off")) {
            rts = 0;
        } else if (parm.equals("autorts=on")) {
            rts = serSettingsFlagRTSAutoM;
        } else if (parm.equals("autocts=off")) {
            cts = 0;
        } else if (parm.equals("autocts=on")) {
            cts = serSettingsFlagCTSAutoM;
        } else if (parm.equals("blocking=off")) {
            blocking = 0;
        } else if (parm.equals("blocking=on")) {
            blocking = 1;
        } else {
            throw new IllegalArgumentException("Bad parameter");
        }
    }

    /**
     * Open a serial port connection.
     * Note: DTR line is always on.
     * Hint: On Solaris opening by port number or /dev/term/* will block
     *       until the Data Set Ready line is On. To work around this open
     *       by device name using /dev/cua/* as root.
     * @param name A URI with the type and parameters for the connection.
     *             The scheme must be: comm
     *
     *             The first parameter must be a port ID: A device name or
     *             a logical port number from 0 to 9.
     *
     *             Any additional parameters must be separated by a ";" and
     *             spaces are not allowed.
     *
     *             The optional parameters are:
     *
     *             baudrate:    The speed of the port, defaults to 19200.
     *             bitsperchar: The number bits that character is. 7 or 8.
     *                          Defaults to 8.
     *             stopbits:    The number of stop bits per char. 1 or 2.
     *                          Defaults to 1.
     *             parity:      The parity can be "odd", "even", or "none".
     *                          Defaults to "none".
     *             blocking:    If "on" wait for a full buffer when reading.
     *                          Defaults to "on".
     *             autocts:     If "on", wait for the CTS line to be on
     *                          before writing. Defaults to "on".
     *             autorts:     If "on", turn on the RTS line when the
     *                          input buffer is not full. If "off",
     *                          the RTS line is always on.
     *                          Defaults to "on".
     * @param writeable  A flag that is true if the caller expects
     *                   to write to the connection. This is ignored
     *                   in all connections that are read-write.
     * @param timeouts   A flag to indicate that the called wants
     *                   timeout exceptions. This is ignored.
     *
     * @exception  IOException  if an I/O error occurs, or
     *             IllegalArgumentException
     *                          if the name string is has an error.
     */
    public void open(String name, int mode, boolean timeouts)
            throws IOException {

        int portNumber = 0;
        String deviceName = null;
        int start = 0;
        int pos = 0;

        if (name.length() == 0) {
             throw new IllegalArgumentException("Missing port ID");
        }

        if (Character.isDigit(name.charAt(0))) {
            portNumber = Integer.parseInt(name.substring(0, 1));
            pos++;
        } else {
            pos = name.indexOf(";");
            if (pos < 0) {
                deviceName = name;
                pos = name.length();
            } else {
                deviceName = name.substring(0, pos);
            }
        }

        while(name.length() > pos) {
            if (name.charAt(pos) != ';') {
                throw new IllegalArgumentException(
                    "missing parameter delimiter");
            }

            pos++;
            start = pos;
            while(true) {
                if (pos == name.length()) {
                    parseParameter(name, start, pos);
                    break;
                }

                if (name.charAt(pos) == ';') {
                    parseParameter(name, start, pos);
                    break;
                }
                pos++;
            }
        }

        if (deviceName != null) {
            hPort = native_openByName(deviceName, baud,
                        bbc|stop|parity|rts|cts, blocking);
        } else {
            hPort = native_openByNumber(portNumber, baud,
                        bbc|stop|parity|rts|cts, blocking);
        }

        if (bufferSize == 0) {
            inputStream = new PrivateInputStream(this);
        } else {
            inputStream = new PrivateInputStreamWithBuffer(this, bufferSize);
        }
        outputStream = new PrivateOutputStream(this);
        opens++;
        copen = true;
    }

    /**
     * Returns an input stream.
     *
     * @return     an input stream for writing bytes to this port.
     * @exception  IOException  if an I/O error occurs when creating
     *                          the output stream.
     */
    synchronized public InputStream openInputStream()
        throws IOException {

        opens++;
        return inputStream;
    }

    /**
     * Returns an output stream.
     *
     * @return     an output stream for writing bytes to this port.
     * @exception  IOException  if an I/O error occurs when creating
     *                          the output stream.
     */
    synchronized public OutputStream openOutputStream()
        throws IOException {

        opens++;
        return outputStream;
    }

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs when closing
     *                          the connection.
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
             native_close(hPort);
        }
    }

    /*
     * Real primitive methods
     */
    private static native int native_openByNumber(int port, int baud,
        int flags, int blocking) throws IOException;
    private static native int native_openByName(String name, int baud,
        int flags, int blocking) throws IOException;
    static native void native_close(int hPort) throws IOException;
    static native int native_readBytes(int hPort, byte b[], int off,
        int len) throws IOException;
    static native int native_available(int hPort) throws IOException;
    static native void native_writeBytes(int hPort, byte b[], int off,
        int len) throws IOException;
}

/**
 * Input stream for the connection
 */
class PrivateInputStream extends InputStream {

    /**
     * Buffer for single char reads
     */
    byte[] buf = new byte[1];

    /**
     * Pointer to the connection
     */
    private Protocol parent;

    /**
     * Constructor
     * @param pointer to the connection object
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateInputStream(Protocol parent)
        throws IOException {
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
        } else if ((off < 0) || (off > b.length) || (len < 0) ||
                   ((off + len) > b.length) || ((off + len) < 0)) {
            throw new IndexOutOfBoundsException();
        }

        for(;;) {
            int count = read1(parent.hPort, b, off, len);
            if (count != 0 || parent.blocking == 0) {
                if (count > len) { // Is this really needed?
                    throw new IOException("Read overrun in comm:");
                }
                return count;
            }
            GeneralBase.iowait();
        }
    }

   /*
    * read1
    */
    protected int read1(int port, byte b[], int off, int len) throws IOException {
        return Protocol.native_readBytes(port, b, off, len);
    }

    /**
     * Returns the number of bytes that can be read (or skipped over)
     * from this input stream without blocking the next caller of
     * a method for this input stream.
     *
     * @return     the number of bytes that can be read from this input stream.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized int available() throws IOException {
        ensureOpen();
        return Protocol.native_available(parent.hPort);
    }

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void close() throws IOException {
        if (parent != null) {
            parent.realClose();
            parent = null;
        }
    }
}


/**
 * Input stream for the connection
 */
class PrivateInputStreamWithBuffer extends PrivateInputStream {

    /**
     * The internal buffer array where the data is stored.
     * When necessary, it may be replaced by another array
     * of a different size.
     */
    protected byte buf[];

    /**
     * The index one greater than the index of the last valid
     * byte in the buffer.
     * This value is always in the range
     * <code>0</code> through <code>buf.length</code>;
     * elements <code>buf[0]</code> through <code>buf[count-1]
     * </code>contain buffered input data obtained
     * from the underlying input stream.
     */
    protected int count;

    /**
     * The current position in the buffer. This is the index
     * of the next character to be read from the
     * <code>buf</code> array.
     * <p>
     * This value is always in the range <code>0</code>
     * through <code>count</code>. If it is less
     * than <code>count</code>, then <code>buf[pos]</code>
     * is the next byte to be supplied as input;
     * if it is equal to <code>count</code>, then
     * the  next <code>read</code> or <code>skip</code>
     * operation will require more bytes to be
     * read from the contained input stream.
     */
    protected int pos;

    /**
     * Constructor
     * @param pointer to the connection object
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateInputStreamWithBuffer(Protocol parent, int bufferSize) throws IOException {
        super(parent);
        buf = new byte[bufferSize];
    }

   /*
    * read1
    */
    protected int read1(int port, byte b[], int off, int len) throws IOException {
        if (count == 0) {
            if (len >= buf.length) {
                return super.read1(port, b, off, len);
            } else {
                pos = 0;
                int res = super.read1(port, buf, pos, buf.length);
                if (res <= 0) {
                    return res;
                } else {
                    count = res;
                }
            }
        }
        if (len > count) {
            len = count;
        }
        System.arraycopy(buf, pos, b, off, len);
        count -= len;
        pos   += len;
        return len;
    }


    /**
     * Returns the number of bytes that can be read (or skipped over) from
     * this input stream without blocking by the next caller of a method for
     * this input stream.
     *
     * @return     the number of bytes that can be read from this
     *             input stream.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized int available() throws IOException {
        ensureOpen();
        return count;
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

        int test;

        ensureOpen();

        if (len <= 0) {
            return;
        }

        // test the parameters so we don't crash in the native code
        test = b[off] + b[off + len - 1];

        Protocol.native_writeBytes(parent.hPort, b, off, len);
    }


    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void close() throws IOException {
        if (parent != null) {
            parent.realClose();
            parent = null;
        }
    }
}


