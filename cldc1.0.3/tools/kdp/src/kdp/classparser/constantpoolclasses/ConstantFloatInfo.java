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
 * Encapsulates a CONSTANT_Float in a Java class file
 * constant pool. 
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ConstantFloatInfo.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ConstantFloatInfo.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:10  dsimon
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
public class ConstantFloatInfo extends ConstantPoolInfo
  {
   /** the bytes that make up this field */
   private int			bytes;

   /**
    * Constructor.  Creates the constant float info object.
    *
    * @param             iStream        input stream to read from
    *
    * @exception         IOException    just pass IOExceptions up.
    */
   public ConstantFloatInfo (DataInputStream iStream) throws IOException
     {
      tag = ConstantPoolInfo.CONSTANT_Float;

      bytes = iStream.readInt ();
     }

   /**
    * Returns this ConstantFloatInfo as a string for displaying.
    * Converted to a float as specified in the JVM Specification
    *
    * @return            String         the float as a string.
    */
   public String toString ()
     {
      if (bytes == 0x7f800000)
        return ("CONSTANT_Float=\t" + "Positive Infinity");

      if (bytes == 0xff800000)
        return ("CONSTANT_Float=\t" + "Negative Infinity");

      if (((bytes >= 0x7f800001) && (bytes <= 0x7fffffff)) ||
          ((bytes >= 0xff800001) && (bytes <= 0xffffffff)))
        return ("CONSTANT_Float=\t" + "NaN");

      int s = ((bytes >> 31) == 0) ? 1 : -1;
      int e = ((bytes >> 23) & 0xff);
      int m = (e == 0) ? (bytes & 0x7fffff) << 1 :
                         (bytes & 0x7fffff) | 0x800000;

      float    value = s * m * (2^(e - 150));

      return ("CONSTANT_Float=\t" + value);
     }
  }
