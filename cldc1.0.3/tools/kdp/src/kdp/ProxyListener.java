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
import java.util.*;
import java.io.*;

abstract class ProxyListener extends Thread {

    int verbose=0;


    Map waitingQueue = new HashMap(8, 0.75f);
    private List packetQueue;
    boolean timeToQuit = false;

    String VMcmds[][] = 
                      { { "" },   // make the list 1-based
                        { "Virtual Machine", 
                              "Version", "ClassBySignature", "AllClasses",
                              "AllThreads", "TopLevelThreadGroups",
                              "Dispose", "IDSizes", "Suspend", "Resume",
                              "Exit", "CreateString", "Capabilities",
                              "ClassPaths", "DisposeObjects",
                              "HoldEvents", "ReleaseEvents" },
                        { "ReferenceType", 
                              "Signature", "ClassLoader", "Modifiers",
                              "Fields", "Methods", "GetValues", 
                              "SourceFile", "NestedTypes", "Status",
                              "Interfaces", "ClassObject" },
                        { "ClassType",
                              "Superclass", "SetValues", "InvokeMethod",
                              "NewInstance" },
                        { "ArrayType",
                              "NewInstance" },
                        { "InterfaceType",
                              "" },
                        { "Method",
                              "LineTable", "VariableTable", "Bytecodes" },
                        { "UNUSED", "UNUSED" },
                        { "Field",
                              "" },
                        { "ObjectReference",
                              "ReferenceType", "GetValues", "SetValues",
                              "UNUSED", "MonitorInfo", "InvokeMethod", 
                              "DisableCollection", "EnableCollection",
                              "IsCollected" },
                        { "StringReference",
                              "Value" },
                        { "ThreadReference",
                              "Name", "Suspend", "Resume", "Status",
                              "ThreadGroup", "Frames", "FrameCount",
                              "OwnedMonitors", "CurrentCountendedMonitor",
                              "Stop", "Interrupt", "SuspendCount" },
                        { "ThreadGroupReference", 
                              "Name", "Parent", "Children" },
                        { "ArrayReference",
                              "Length", "GetValues", "SetValues" },
                        { "ClassLoaderReference",
                              "VisibleClasses" },
                        { "EventRequest", 
                              "Set", "Clear", "ClearAllBreakpoints" },
                        { "StackFrame",
                              "GetValues", "SetValues", "ThisObject" },
                        { "ClassObjectReference", 
                              "ReflectedType" },
                      };

    String DBGcmds[][] = 
                      { { "Event",
                              "Composite" },
                   
                      };

    String VENcmds[][] = 
                      { { "Vender-Specific",
                              "UNKNOWN" }
                      };

    public ProxyListener() {
		packetQueue = Collections.synchronizedList(new LinkedList());
    }

    public void newPacket(Packet p) {

	if (p == null) {
		quit();
		synchronized(packetQueue) {
			packetQueue.notify();
		}
		return;
	}
	synchronized(packetQueue) {
		packetQueue.add(p);
		packetQueue.notify();
	}
    }

    public Packet waitForPacket() {

	synchronized(packetQueue) {
		while (!timeToQuit && packetQueue.size() == 0) {
			try {
				packetQueue.wait();
			} catch (InterruptedException e) {
			}
		}
	}
	if (timeToQuit)
		return null;
	return ((Packet)packetQueue.remove(0));
    }

	void replyReceived(Packet p) {
		Packet p2;
		if (p == null) {
			quit();
			synchronized(waitingQueue) {
			    Iterator iter = waitingQueue.values().iterator();
			    while (iter.hasNext()) {
				p2 = (Packet)iter.next();
				p2.notify();
			    }
			}
			return;
		}
			
		String idString = String.valueOf(p.id);
		synchronized(waitingQueue) {
			p2 = (Packet)waitingQueue.get(idString);
			if (p2 != null)
				waitingQueue.remove(idString);
		}
		if (p2 == null) {
			System.err.println("Received reply with no sender!");
			return;
		}
		p2.errorCode = p.errorCode;
		p2.data = p.data;
		p2.replied = true;
		synchronized(p2) {
			p2.notify();
		}

	}
    
    public void waitForReply(Packet p) {

		synchronized(p) {
			while (!timeToQuit && !p.replied) {
				try { p.wait();
				} catch (InterruptedException e) {
				}
			}
			if (!p.replied)
				throw new RuntimeException();
		}
    }

    public void verbose( int lvl ) {
        verbose = lvl;
    }
    abstract public void send(Packet p) throws IOException;

    abstract public void quit();

    protected void vp( int vlevel, String str ) {
        if ( verbose >= vlevel )
            System.out.print( str );
    }

    protected void vpe( int vlevel, String str ) {
        if ( verbose == vlevel )
            vp( vlevel, str );
    }

    protected void disp(Packet p ) {
        String[][] cmds;
	int cmdSet = p.cmdSet;
	int cmd = p.cmd;
	int cmdSetIndex = cmdSet;
	int cmdIndex = cmd;

        if ( cmdSet < 64 ) {
            cmds = VMcmds;
        } else if ( cmdSet < 128 ) {
            cmds = DBGcmds;
            cmdSetIndex -= 64;
/*
 * THIS IS LIKELY TO CAUSE PROBLEMS IN THE FUTURE
 */
            cmdIndex -= 99;
        } else {
            cmds = VENcmds;
            cmdSetIndex = cmdIndex = 0;
        }

        try {
            Log.LOGN(3, "Sending through: " +
                    cmds[ cmdSetIndex ][ 0  ] + "(" + cmdSet + ")/" +
                    cmds[ cmdSetIndex ][ cmdIndex ] + "(" + cmd    + ")" );
	    Log.LOGN(8, "Packet: " + p);
        } catch ( ArrayIndexOutOfBoundsException e ) {
            System.out.println( "UNKNOWN COMMAND: " + cmdSet + "/" + cmd );
        }
    }

} // ProxyListener
