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

public class Inet {

    public static boolean DEBUG = false;

    private static int InetHandle = -1;

    int InetSockH = -1;
    private boolean selected, inited;

    public Inet() {
        if (!inited) {
            if ((InetHandle = initProto()) < 0 && InetHandle > -10) {
                int lasterr = getlasterror();
                String msg = "INET: Couldn't init InetLib " + 
                             InetHandle + ", " + lasterr;
                System.out.println(msg);

                throw new RuntimeException("Inet not initialized");
            }

            if (DEBUG) {
                System.out.println("INIT: initialized");
            }
            inited = true;
        }
    }

    public void InetShowSignal(int x, int y) {
        showSignal(x, y);
    }

    public void InetOpenSock(int scheme) throws InetException {
        if ((InetSockH = openHTTPsocket(scheme, InetHandle)) == -1)
            throw new InetException("Couldn't open inet socket");

        if (DEBUG) {
            System.out.println("INET:OpenSock ----------- "+InetSockH);
        }
    }

    public void InetHTTPReqCreateAndSend(String verb, 
        byte[] data, String resource, String referer)
        throws InetException  {

        selected = false;

        if (DEBUG) {
            System.out.println("INET: CreateAndSendReq " +
                verb + " res=" + resource + " ref=" + referer);

            if (data == null) {
                System.out.println();
            } else {
                System.out.println(" with " + data.length + 
                                   " bytes of post data");
            }
        }
        HTTPCreateAndSendReq(InetSockH, verb, data, resource, referer);
    }

    public int InetSockRead(byte b[], int offset, int count)
        throws InetException{

        int nread;
        int retval;
        if (!selected) {
            while ((retval = select()) == 0) {
                Thread.yield();
            }

            if (retval < 0) {
                if (DEBUG) {
                    System.out.println("INET:select returning -1");
                }

                return -1;
            }
            selected = true;
        }

        nread = readHTTPsocket(InetSockH, b, offset, count);

        if (DEBUG) {
             System.out.println("INET: read " + nread + " bytes");
        }

        return (nread);
    }

    public int InetSockRead(byte b[], int count) throws InetException{
        return InetSockRead(b, 0, count);
    }

    public void InetSockClose() throws InetException {
        if (DEBUG) {
            System.out.println("INET: SockClose ----------- "+InetSockH);
        }
        closeHTTPsocket(InetSockH);
    }

    public static native int getlasterror();

    private static native int initProto();
    private static native int select();

    private native void showSignal(int x, int y);
    private native int openHTTPsocket(int scheme, int handle);

    private native void HTTPCreateAndSendReq(
        int sockhandle, String verb, byte[] data,
        String resource, String referer) throws InetException;

    private native int readHTTPsocket(
        int sockhandle, byte b[], int offset, int count);

    private native void closeHTTPsocket(int sockhandle) throws InetException;

}


