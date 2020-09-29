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
 * Represents an individual entry in the InnerClasses table of
 * a Java class file.
 *
 * This class has never been thoroughly tested.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: InnerClassesTable.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: InnerClassesTable.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:05  dsimon
 *   initial import of SVM to CVS
 *
 *   Revision 1.1.1.1  2000/07/07 13:34:24  jrv
 *   Initial import of kdp code
 *
 *   Revision 1.1.1.1  2000/05/31 19:14:47  ritsun
 *   Initial import of kvmdt to CVS
 *
 *   Revision 1.1  2000/04/25 00:30:39  ritsun
 *   Initial revision
 *
 */
public class InnerClassesTable
  {
   /** Access flags for inner classes */
   private static final int		ACC_PUBLIC		= 0x0001;
   private static final int		ACC_PRIVATE		= 0x0002;
   private static final int		ACC_PROTECTED		= 0x0004;
   private static final int		ACC_STATIC		= 0x0008;
   private static final int		ACC_FINAL			= 0x0010;
   private static final int		ACC_INTERFACE		= 0x0200;
   private static final int		ACC_ABSTRACT		= 0x0400;

   /** constant pool index identifying the inner class */
   private int		               innerClassInfoIndex;
   /** constant pool index identifying the outer class */
   private int		               outerClassInfoIndex;
   /** the index of the name of the inner class */
   private int		               innerNameIndex;
   /** access flags of the inner class */
   private int		               innerClassAccessFlags;

   /**
    * Constructor.  Just read in all the fields.
    *
    * @param        iStream             input stream to read from
    *
    * @exception    IOException         just pass io exceptions up
    */
   public InnerClassesTable (DataInputStream iStream) throws IOException
     {
      innerClassInfoIndex = iStream.readUnsignedShort ();
      outerClassInfoIndex = iStream.readUnsignedShort ();
      innerNameIndex = iStream.readUnsignedShort ();
      innerClassAccessFlags = iStream.readUnsignedShort ();
     }

   /**
    * Convert the information we have into an easy to read format and return
    * as a string.
    *
    * @return       String              the info we have as a string
    */
   public String toString ()
     {
      String				s = new String ("");

      s = s + "\n\tinnerClassInfoIndex=" + innerClassInfoIndex;
      s = s + "\n\touterClassInfoIndex=" + outerClassInfoIndex;
      s = s + "\n\tinnerNameIndex=" + innerNameIndex;
      s = s + "\n\tinnerClassAccessFlags=";

      if ((innerClassAccessFlags & ACC_PUBLIC) > 0)
        s = s + "public ";

      if ((innerClassAccessFlags & ACC_PRIVATE) > 0)
        s = s + "private ";

      if ((innerClassAccessFlags & ACC_PROTECTED) > 0)
        s = s + "protected ";

      if ((innerClassAccessFlags & ACC_STATIC) > 0)
        s = s + "static ";

      if ((innerClassAccessFlags & ACC_FINAL) > 0)
        s = s + "final ";

      if ((innerClassAccessFlags & ACC_INTERFACE) > 0)
        s = s + "interface ";

      if ((innerClassAccessFlags & ACC_ABSTRACT) > 0)
        s = s + "abstract";

      return s;
     }
  }
