/*
 *  Copyright (c) 2000 Sun Microsystems, Inc., 901 San Antonio Road,
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

package kdp;

import kdp.classparser.*;
import kdp.classparser.attributes.*;
import java.io.*;
import java.net.*;

class KVMListener extends ProxyListener implements VMConstants {

    SocketConnection connKvm;
    ProxyListener debuggerListener=null;
    ClassManager manager;
    Options options;
    boolean Ready = false;
    boolean useClassParser = false;
    Socket remoteSocket = null;


    public KVMListener(Options options) {
	super();
	this.options = options;
	useClassParser = options.getUseClassParser();
    }

    public void set( ProxyListener debuggerListener, ClassManager manager ) {
        this.debuggerListener = debuggerListener;
        this.manager = manager;
    }
	/*
	 * Sends packet to our output socket
	 */
	public synchronized void send(Packet p) throws IOException{
		synchronized(this) {
			while (!Ready) {
				try {
					this.wait();
				} catch (InterruptedException e) {
				}
			}
		}
		String id = String.valueOf(p.id);
		synchronized(waitingQueue) {
			if ((p.flags & Packet.Reply) == 0 && p.id < 0)
				waitingQueue.put(id, p);
			}
		connKvm.send(p);
	}

    public void quit() {
	boolean oldtimeToQuit = timeToQuit;
	timeToQuit = true;
	if (oldtimeToQuit != timeToQuit) {
	    try {
		if (remoteSocket != null) {
//			remoteSocket.shutdownInput();
//			remoteSocket.shutdownOutput();
	    		remoteSocket.close();
		}
	    } catch (IOException e) {}
	    debuggerListener.quit();
	}
    }

    public void run() {

        boolean handled;
	PacketStream ps;
	byte typeTag;
	int classID;
	String className;
	int classStatus;
	ClassFile cf;

        try {
            /* Attempt to reconnect to KVM by polling every 2000ms until 
             * connection is established.
             */
            while (remoteSocket == null) {
                try { 
	            remoteSocket = new Socket(options.getRemoteHost(), 
                                              options.getRemotePort());
                } catch (ConnectException e) {
                    System.err.println("KVM not ready");
                    try {
                        Thread.sleep(2000);
                    } catch (InterruptedException ie) {}
                }
             }
             connKvm = new SocketConnection(this, remoteSocket);
         } catch (IOException e) {
             System.out.println("KVMListener: " + e.getMessage());
         } catch (SecurityException e) {
             System.out.println("KVMListener: " + e.getMessage());
         }

	synchronized(this) {
		Ready = true;
		this.notify();
	}
	if (!useClassParser) {
	    byte [] handshake = new String("JDWP-Handshake").getBytes();
	    try {
		    // debugger -> vm
		    for ( int i=0; i < handshake.length; i++ )
			    connKvm.sendByte(handshake[i]);

		    // debugger <- vm
		    for ( int i=0; i < handshake.length; i++ )
			    connKvm.receiveByte();
	    } catch (IOException e) {
	    }
	}

	new Thread(connKvm).start();

	if (useClassParser) {

	    ps = new PacketStream(this, KVM_CMDSET, KVM_HANDSHAKE_CMD);
	    ps.writeString("KVM Reference Debugger Agent");
	    ps.writeByte((byte)1);	// Major version
	    ps.writeByte((byte)1);	// minor version
	    ps.send();
	    try {
	        ps.waitForReply();
	    } catch (Exception e) {
	        System.out.println("Exception during handshake: " + e +
	            " exiting...");
	        Runtime.getRuntime().exit(1);
	    }
	    String s = ps.readString();
	    int option_bits = ps.readInt();
	    Log.LOGN(1, "KVM Handshake return string: " + s);
	    Log.LOGN(1, "KVM Handshake return options: " + option_bits);

	    ps = new PacketStream(this, VIRTUALMACHINE_CMDSET,
		ALL_CLASSES_CMD);
	    ps.send();
	    try {
		ps.waitForReply();
	    } catch (Exception e) {
		System.out.println("Couldn't get list of classes from KVM");
	    }
	    int numClasses = ps.readInt();
	    Log.LOGN(2, numClasses + " classes");
	    for (int i = 0; i < numClasses; i++) {
    	        typeTag = ps.readByte();
    	        classID = ps.readInt();
    	        className = ps.readString();
		if (typeTag != TYPE_TAG_ARRAY) {
    	            // strip off leading 'L' and trailing ';'
    	            className = new String(className.substring(1,
			className.length() - 1));
		}
    	        classStatus = ps.readInt();
    		if ((cf = (ClassFile)ClassManager.classMap.get(new
    		    Integer(classID))) == null) {
    		    Log.LOGN(3, "allclasses: new class: " +
    		        className + " " + Integer.toHexString(classID));
    		    cf = manager.findClass(typeTag, className);
    		    if (cf != null) {
			cf.setClassID(classID);
			cf.setClassStatus(classStatus);
    			ClassManager.classMap.put(new
    		  	    Integer(classID), cf);
    		    } else {
    		        Log.LOGN(3, "allclasses: couldn't find class "
    		            + className);
    		    }
    	        }
            }
	}
        try {
            while (!timeToQuit) {
		PacketStream in;
     
                handled = false;
		Packet p = waitForPacket();
		if (p == null){
			break;		// must be time to quit
		}
		if (useClassParser && (p.flags & Packet.Reply) == 0) {
		switch (p.cmdSet) {
		case EVENT_CMDSET:
		    switch (p.cmd) {
		    case COMPOSITE_CMD:
			in = new PacketStream(this, p);
		    	byte suspendPolicy = in.readByte();
		    	int numEvents = in.readInt();
		    	// we KNOW that KVM only sends one event
		    	byte eventKind = in.readByte();
		    	if (eventKind != JDWP_EventKind_CLASS_PREPARE)
		    		break;
		    	int requestID = in.readInt();
		    	int threadID = in.readInt();
		    	typeTag = in.readByte();
		    	classID = in.readInt();
		    	className = in.readString();
			classStatus = in.readInt();
			if (typeTag != TYPE_TAG_ARRAY) {
			    // strip off leading 'L' and trailing ';'
			    className =
		    	        new String(className.substring(1,
		    	        className.length() - 1));
			}
		    	Log.LOGN(3, "ClassPrepare:  " + className + ", ID = "
			    + Integer.toHexString(classID));
		    	// see if we have a class parser reference
		    	if ((cf = (ClassFile)ClassManager.classMap.get(new Integer(classID))) == null) {
		    	    cf = manager.findClass(typeTag, className);
		    	    if (cf != null) {
		    		ClassManager.classMap.put(new Integer(classID),
				    cf);
			        cf.setClassID(classID);
				cf.setClassStatus(classStatus);
			    } else {
				Log.LOGN(3, "ClassPrepare: null cf!");
			    }
		    	} else {
				Log.LOGN(3, "ClassPrepare: got classfile " + cf.getClassName());
				cf.setClassStatus(classStatus);
			}
		    	break;
		    }
		    break;
		case KVM_CMDSET:
                    switch (p.cmd) {
                    case KVM_STEPPING_EVENT_CMD: /* need stepping info */
			in = new PacketStream(this, p);
                        handleSteppingInfo(in);
                        handled = true;
                        break;
		    }
		    break;
                }
		}

                if ( !handled ) {
                    Log.LOG(3, "KVMListener:: ");
                    disp(p);
		    if (p.cmdSet == 64 && p.cmd == 100)
			Log.LOGN(3, "Eventkind = " + p.data[5]);
                    debuggerListener.send(p);
                }
            }
        } catch (IOException e) {
        }
    }

    private void handleSteppingInfo(PacketStream in) {
	ClassFile cf;
	MethodInfo mi;
	int index, index2, currentIndex;
        long offs, offs2, offs3;

	int cep = in.readInt();	// opaque buffer pointer we just pass back
        int cid = in.readInt();
        int mid = in.readInt();
        long offset = in.readLong();

	PacketStream ps = new PacketStream(this, KVM_CMDSET,
	    KVM_GET_STEPPINGINFO_CMD);

	ps.writeInt(cep);
	cf = (ClassFile)ClassManager.classMap.get(new Integer(cid));
	if (cf == null || (mi = cf.getMethodInfoByIndex(mid)) == null) {
		ps.writeLong(0);
		ps.writeLong(0);
            	ps.send();
        	try {
			ps.waitForReply();
        	} catch ( Exception e ) {}
		return;
	}

        LineNumberTableAttribute table = mi.getCodeAttribute().
	    getLineNumberTable();

/*
        int line = table.getLineThatContainsOpcode( offset );
        System.out.println( "handle..." + table.getLineThatContainsOpcode( 271 ) );
*/

        //line = table.getNextExecutableLineIndex( line );
        if (table == null) {
            offs = offs2 = offs3 = -1;
            index = index2 = currentIndex = -1;
        } else {
	    // where are we now.
	    currentIndex = table.getCurrentLineCodeIndex(offset);
	    // what is the index of the next line we should execute
            index = table.getNextExecutableLineCodeIndex( offset );
	    // does the current offset have a duplicate line?
            index2 = table.getDupCurrentExecutableLineCodeIndex(offset);
	    // offset of current line
            offs = table.getStartPCFromIndex(index);
	    // offset of possible duplicate of current line
            offs2 = table.getStartPCFromIndex(index2);
	    // offset of next line after duplicate
	    offs3 = table.getOffsetofDupNextLine(index2);
        }
        Log.LOGN(3, "handleSteppingInfo  current offset = " + offset );
        Log.LOGN(3, "handleSteppingInfo  target offset = " + offs );
        Log.LOGN(3, "handleSteppingInfo  dup current line offset = " + offs2 );
        Log.LOGN(3, "handleSteppingInfo  offset after current dup = " + offs3 );
	Log.LOGN(3, "handleSteppingInfo current line number = " + table.getLineNumberFromIndex(currentIndex));
/*
        long offs = table.getCodeIndexBySourceLineNumber( line );
*/

        ps.writeLong( offs );
        ps.writeLong( offs2 );
        ps.writeLong( offs3 );
        ps.send();
   
        try {
	    ps.waitForReply();
        } catch ( Exception e ) {}
    }

    public String toString() {
	return (new String("KVMListener: "));
    }
} // KVMListener
