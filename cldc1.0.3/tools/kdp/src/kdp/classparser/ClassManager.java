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

package kdp.classparser;

import java.io.*;
import java.util.*;
import kdp.Log;
import kdp.VMConstants;

public class ClassManager {

    List classes;
    SearchPath path;
    public static Map classMap = new HashMap(128, 0.75f);

    public ClassManager() {
        classes = new Vector();
        path = null;
    }

    public ClassManager( SearchPath path ) {
        this();
        this.path = path;
    }

    private ClassFile loadClass(String className, 
                                FileReference file, 
                                byte typetag) {

        Log.LOGN(3, "loadclass: " + file );

        ClassFile cf = new ClassFile(file, className, typetag);

        try { 
            cf.readClassFile();
            classes.add( cf );
        }
        catch ( Exception e ) { 
            Log.LOGN(2,  "Error loading: " + file );
            cf = null;
        }

        return cf; 
    }

    public List getAllClasses() {
        return Collections.unmodifiableList( classes );
    }

    public ClassFile findClass( byte typetag, String className ) {
        ClassFile cf=null;
        Iterator iter = classes.iterator();


        while ( iter.hasNext() ) {
            cf = ( ClassFile )iter.next();
            if ( cf.equals( className ) )
                return cf;
        }

	if (typetag == VMConstants.TYPE_TAG_ARRAY) {
            Log.LOGN(4,  "findclass: Array class " + className );
            cf = new ClassFile(null, className, typetag);
            classes.add( cf );
	    return cf;
	}
        if ( path != null ) {
            FileReference file;

            Log.LOGN(4,  "findclass: finding " + className );
            if ( ( file = path.resolve( className ) ) != null ) {
                return loadClass(className, file, typetag);
            } 
        }
        return null;
    }
} // ClassManager
