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

import java.util.*;
import java.io.*;
import kdp.Log;

/**
 * Main program to run my Java class file parser. 
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ClassParser.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ClassParser.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:00  dsimon
 *   initial import of SVM to CVS
 *
 *   Revision 1.2  2000/08/14 19:49:10  zmm3876
 *   *** empty log message ***
 *
 *   Revision 1.1  2000/08/09 16:11:49  zmm3876
 *   *** empty log message ***
 *
 *   Revision 1.1.1.1  2000/07/07 13:34:23  jrv
 *   Initial import of kdp code
 *
 *   Revision 1.1.1.1  2000/05/31 19:14:47  ritsun
 *   Initial import of kvmdt to CVS
 *
 *   Revision 1.2  2000/05/15 16:52:51  jpc1870
 *   added locationsOfLine() to refType()
 *
 *   Revision 1.1  2000/04/25 00:36:43  ritsun
 *   Initial revision
 *
 */
public class ClassParser {
    /** Name of the Java class file to read */
    private String		className;
    private String		baseName;
    /** line number to look up in the class file, -1 if we don't want to
	look up a specific line just print all available class information */
    private int                lineNumber = -1;
    /** stores the class file contents */
    public ClassFile          classFile; // changed to public -- jpc
    /* the class ID of this class as told to us by the KVM itself */
    private int classID;
    /* the type of class,  (class, interface, array) */
    private byte typeTag;
    
    
    /**
     * Constructor.
     *
     * @param        className      classname
     */
    public ClassParser (String className, byte typeTag) {
	//make sure they passed at least 1 arg
	int index;

	
	//store the class file name
	this.className = className;
	if ((index = className.lastIndexOf('/')) != -1)
		this.baseName = className.substring(index+1);
	else
		this.baseName = className;
	this.typeTag = typeTag;
    }

    public boolean equals( String newclassName ) {
	    return className.equals( newclassName );
    }

    public String getBaseName() {
	return baseName;
    }

    public String getClassName() {
	return className;
    }

    /**
     * Returns the class filename
     */
    
    public String getClassFileName() {
	return (className + ".class");
    }

    public String getFullClassFileName() {
	return (className + ".class");
    }
    
    /**
     * Returns the classFile object 
     */
    
    public ClassFile getClassFile() {
	return classFile;
    }

    public byte getJDWPTypeTag() {
	return typeTag;
    }

    public void setClassID(int classID) {
	this.classID = classID;
    }
    public int getClassID() {
	return (classID);
    }

    /**
     * Main method.  Where it all begins... and ends too.
     *
     * @param		args[]			array of command line arguments
     */

    
      public static void main (String args[]) {
	int lineNumber = 0;
	ClassFile classfile;
        FileReference file = FileReference.create(args[0], args[1]);

      //declare a ClassParser object
      
      //create a new ClassFile object passing it the name of the
      //class file
      classfile = new ClassFile (file, args[1], (byte)'L');
      //read the class file
		try {
			classfile.readClassFile ();
		} catch (IOException e) {
			Log.LOGN(3, "Main: caught IOException " + e);
		}

      Log.LOGN(3, "\n");

      //if a line number was not specified then just output everything
      if (args.length < 3)
        classfile.print (System.out);
      else {
        //just output the information for the method the specified by line number
	 lineNumber = Integer.parseInt (args[2]);
         MethodInfo    method = classfile.lookUpMethodByLineNumber (lineNumber);
         if (method != null)
           {
            Log.LOGN(3, "Associated the method \"" + method.getName () +
                                "\" with line number " + lineNumber + ".");
            Log.LOGN(3, "The following information if available for " +
                                 method.getName () + ".");
            Log.LOGN(3, method.toString());

            //output the code array index for this method at which a breakpoint
            //should be placed in order to stop at the specified line in the
            //source file.
            Log.LOGN(3, "");
            Log.LOGN(3, "To put a breakpoint at line " + lineNumber +
                                " break at code index " +
                                method.getCodeIndex (lineNumber) + ".");
            
            Log.LOGN(3, "");
            Log.LOGN(3, "Breakpoints can be set at the following lines in " +
                                 method.getName () + ":");
            Log.LOGN(3, "tSource Line Number\tBytecode Offset");
            Log.LOGN(3, "t------------------\t---------------");
            
            int     lineNumbers[][] = method.getBreakableLineNumbers ();
            if (lineNumbers == null)
              Log.LOGN(3, "Method contains no breakable lines.");
            else
              for (int lcv = 0; lcv < lineNumbers.length; ++lcv)
                Log.LOGN(3, "t\t" + lineNumbers[lcv][0] + "\t\t\t" + lineNumbers[lcv][1]);
           }
         else
           //the specified line was not an executable line of code within a method
           Log.LOGN(3, "Line number " + lineNumber + " does not contain" +
                               " code within a method.");
        }
        
       // classfile.getVariableTableForMethodName("main");
     }
} // ClassParser
