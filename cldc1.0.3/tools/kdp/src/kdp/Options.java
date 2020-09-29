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
import java.net.*;
import java.io.*;

class Options {

    int localport,
        remoteport;
    String remotehost;

    String classpath;
    int verbosity;
    boolean useClassParser;

    public void setLocalPort( int port ) { localport = port; }
    public int  getLocalPort() { return localport; }

    public void setRemoteHost( String host ) { remotehost = host; }
    public String getRemoteHost() { return remotehost; }

    public void setRemotePort( int port ) { remoteport = port; }
    public int  getRemotePort() { return remoteport; }

    public void setClassPath( String path ) { classpath = path; }
    public String getClassPath() { return classpath; }

    public void setVerbosity( int lvl ) { verbosity = lvl; }
    public int getVerbosity() { return verbosity; }

    public void setUseClassParser( boolean on ) { useClassParser = on; }
    public boolean getUseClassParser() { return useClassParser; };
} // Options
