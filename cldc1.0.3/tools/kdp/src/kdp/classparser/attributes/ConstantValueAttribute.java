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
import kdp.classparser.constantpoolclasses.*;

import java.io.*;

/**
 * Encapsulates the ConstantValue attribute of a Java class file.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ConstantValueAttribute.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ConstantValueAttribute.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:03  dsimon
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
public class ConstantValueAttribute extends Attribute
  {
   /** index into the constant pool table containing the name 
       of this class */
   private int				attributeNameIndex;
   /** length of this attribute in bytes */
   private int				attributeLength;
   /** the constant pool entry at that index gives the constant 
       value represented by this attribute */
   private int				constantValueIndex;
   
   /**
    * Constructor.  Reads the ConstantValue attribute from
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
   public ConstantValueAttribute (DataInputStream iStream,
                                  int attributeNameIndex, int attributeLength)
                                                              throws IOException
     {
      this.attributeNameIndex = attributeNameIndex;
      this.attributeLength = attributeLength;

      //read the constantValueIndex from the class file
      constantValueIndex = iStream.readUnsignedShort ();
     }

   /**
    * Returns the ConstantValue attribute in a nice easy to
    * read format as a string.
    *
    * @param		constantPool		constant pool of the class file
    *
    * @return 		String		    	the attribute as a nice easy to
    *						    	read String
    */
   public String toString (final ConstantPoolInfo[] constantPool)
     {
      String				s = new String ("");

      ConstantUtf8Info	utf8Info = (ConstantUtf8Info)
                                             constantPool[attributeNameIndex];
      s = s + utf8Info.toString () + "\n\t\t\t";

      ConstantPoolInfo   cpInfo = (ConstantPoolInfo)
                                             constantPool[constantValueIndex];

      switch (cpInfo.getTag())
        {
         case ConstantPoolInfo.CONSTANT_Long :
           {
            ConstantLongInfo cpInfo2 = (ConstantLongInfo) cpInfo;
            s = s + cpInfo2.toString ();
            break;
           }

         case ConstantPoolInfo.CONSTANT_Float :
           {
            ConstantFloatInfo cpInfo2 = (ConstantFloatInfo) cpInfo;
            s = s + cpInfo2.toString ();
            break;
           }

         case ConstantPoolInfo.CONSTANT_Double :
           {
            ConstantDoubleInfo cpInfo2 = (ConstantDoubleInfo) cpInfo;
            s = s + cpInfo2.toString ();
            break;
           }

         case ConstantPoolInfo.CONSTANT_Integer :
           {
            ConstantIntegerInfo cpInfo2 = (ConstantIntegerInfo) cpInfo;
            s = s + cpInfo2.toString ();
            break;
           }

         case ConstantPoolInfo.CONSTANT_String :
           {
            ConstantStringInfo      cpInfo2 = (ConstantStringInfo) cpInfo;
            s = s + cpInfo2.toString (constantPool);
            break;
           }
        }
      return s;
     }
  }
