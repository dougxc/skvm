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
package kdp.classparser.constantpoolclasses;

import java.io.*;

/**
 * Encapsulates a Constant_Class item in the Java class
 * file constant pool. 
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ConstantClassInfo.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ConstantClassInfo.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:09  dsimon
 *   initial import of SVM to CVS
 *
 *   Revision 1.1.1.1  2000/07/07 13:34:24  jrv
 *   Initial import of kdp code
 *
 *   Revision 1.1.1.1  2000/05/31 19:14:48  ritsun
 *   Initial import of kvmdt to CVS
 *
 *   Revision 1.1  2000/04/25 00:34:06  ritsun
 *   Initial revision
 *
 */
public class ConstantClassInfo extends ConstantPoolInfo
  {
   /** index into constant pool table containing a UTF8Info structure
       that specifies a class name */
   private int		nameIndex;

   /**
    * Constructor.  Reads appropriate fields from the specified data
    * input stream.
    *
    * @param             iStream        input stream to read from
    *
    * @exception         IOException    just pass exceptions up
    */
   public ConstantClassInfo (DataInputStream iStream) throws IOException
     {
      tag = ConstantPoolInfo.CONSTANT_Class;

      nameIndex = iStream.readUnsignedShort ();
     }

   /**
    * Retrieve the constant pool index of the class name this
    * ConstantClassInfo structure represents.
    *
    * @return            int            constant pool index containing
    *                                   UTF8Info structure with class's name
    */
   public int getNameIndex ()
     {
      return nameIndex;
     }
     
   /**
    * Return this ConstantClassInfo structure's fields as a string.
    */ 
   public String toString ()
     {
      return ("CONSTANT_Class" + "\t\tnameIndex=" + nameIndex);
     }
  }
