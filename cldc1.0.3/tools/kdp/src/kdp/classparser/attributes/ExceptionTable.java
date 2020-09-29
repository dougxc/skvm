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
package kdp.classparser.attributes;

import kdp.classparser.attributes.*;

import java.io.*;

/**
 * Represents a single entry in the ExceptionsAttribute's 
 * exception table.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ExceptionTable.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ExceptionTable.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:04  dsimon
 *   initial import of SVM to CVS
 *
 *   Revision 1.1.1.1  2000/07/07 13:34:23  jrv
 *   Initial import of kdp code
 *
 *   Revision 1.1.1.1  2000/05/31 19:14:48  ritsun
 *   Initial import of kvmdt to CVS
 *
 *   Revision 1.1  2000/04/25 00:30:39  ritsun
 *   Initial revision
 *
 */
public class ExceptionTable
  {
   /** index into the code array which denotes the
       location the exception handler becomes active */    
   private int			startPC;
   /** index into the code array which denotes the
       location the exception handler ceases to be active */
   private int			endPC;
   /** index into the code array denoting the start of
       the exception handler */
   private int			handlerPC;
   /** index into the constant pool table identifying
       the class of exceptions this exception handler
       is designed to catch */
   private int			catchType;
   
   /** 
    * Constructor.  Reads an ExceptionTable from the specified
    * data source.
    *
    * @param		iStream			source of the ExceptionTable
    *
    * @exception	IOException		pass IOExceptions up
    */
   public ExceptionTable (DataInputStream iStream) throws IOException
     {
      //read in all the fields from the DataInputStream
      startPC = iStream.readUnsignedShort ();
      endPC = iStream.readUnsignedShort ();
      handlerPC = iStream.readUnsignedShort ();
      catchType = iStream.readUnsignedShort ();
     }
   
   /**
    * Returns the contents of the ExceptionTable as a
    * String in an easy to read format.
    *
    * @return		String			the contents of the exception
    *							table as a String
    */  
   public String toString ()
     {
      return ("Exception Table:\n\tstartPC=" + startPC +
      		  "\n\tendPC=" + endPC + "\n\thandlerPC=" + 
      		  handlerPC + "\n\tcatchType=" + catchType);
     }
  }
