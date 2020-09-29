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

import kdp.classparser.*;
import kdp.classparser.attributes.*;
import kdp.classparser.constantpoolclasses.*;

import java.io.*;

/**
 * Encapsulates the LocalVariableTable attribute of a Java class file.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: LocalVariableTableAttribute.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: LocalVariableTableAttribute.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:07  dsimon
 *   initial import of SVM to CVS
 *
 *   Revision 1.2  2000/08/14 19:49:11  zmm3876
 *   *** empty log message ***
 *
 *   Revision 1.1  2000/08/09 15:51:47  zmm3876
 *   *** empty log message ***
 *
 *   Revision 1.1.1.1  2000/07/07 13:34:24  jrv
 *   Initial import of kdp code
 *
 *   Revision 1.1.1.1  2000/05/31 19:14:48  ritsun
 *   Initial import of kvmdt to CVS
 *
 *   Revision 1.1  2000/04/25 00:30:39  ritsun
 *   Initial revision
 *
 */
public class LocalVariableTableAttribute extends Attribute
  {
   /** index into the constant pool table containing the name 
       of this class */
   private int				   	attributeNameIndex;
   /** length of this attribute in bytes */
   private int				   	attributeLength;
   /** the length of the attribute */
   private int 			   	localVariableTableLength;
   /** each entry indicates a range of code array offsets within which a local
       variable has a value */
   private LocalVariableTable		localVariableTable[];
   
   /**
    * Constructor.  Reads the LocalVariableTableAttribute attribute from
    * the class file.
    *
    * @param		iStream			the input stream on which to
    *							read the class file
    * @param		attributeNameIndex	attributeNameIndex member of
    *							attribute structure.
    * @param		attributeLength	attributeLength member of
    *							attribute structure.
    *
    * @exception	IOException		pass IOExceptions up
    */
   public LocalVariableTableAttribute (DataInputStream iStream,
   							 int attributeNameIndex, int attributeLength) throws IOException
     {
      this.attributeNameIndex = attributeNameIndex;
      this.attributeLength = attributeLength;
      
      //read in the length of the LocalVariableTable array
      localVariableTableLength = iStream.readUnsignedShort ();
     
      //allocate space for and read in the LocalVariableTable
      localVariableTable = new LocalVariableTable[localVariableTableLength];
      for (int lcv = 0; lcv < localVariableTableLength; ++lcv)
        {
         localVariableTable[lcv] = new LocalVariableTable ();
         localVariableTable[lcv].startPC = iStream.readUnsignedShort ();
         localVariableTable[lcv].length = iStream.readUnsignedShort ();
         localVariableTable[lcv].nameIndex = iStream.readUnsignedShort ();
         localVariableTable[lcv].descriptorIndex = iStream.readUnsignedShort ();
         localVariableTable[lcv].index = iStream.readUnsignedShort ();
        }
     }

   /**
    * Returns the LocalVariableTableAttribute attribute in a nice easy to
    * read format as a string.
    *
    * @param		constantPool		constant pool of the class file
    *
    * @return 		String			the attribute as a nice easy to
    *							read String
    */
   public String toString (final ConstantPoolInfo[] constantPool)
     {
      ConstantUtf8Info			utf8Info;
      String						s = new String ("");
      
      utf8Info = (ConstantUtf8Info) constantPool[attributeNameIndex];
      s = s + "\t" + utf8Info.toString ();

      for (int lcv = 0; lcv < localVariableTableLength; ++lcv)
        {
         utf8Info = (ConstantUtf8Info)
                              constantPool[localVariableTable[lcv].nameIndex];
         s = s + "\n\t\t\t\tName =\t" + utf8Info.toString ();
         utf8Info = (ConstantUtf8Info)
                         constantPool[localVariableTable[lcv].descriptorIndex];
         s = s + "\n\t\t\t\tClass=\t" + StringParser.parseDataType (utf8Info.toString ());
        }

      return s;
     }
     
      public LocalVariableTable[] getLocalVariableTable(){
        return localVariableTable;
     }
  }
