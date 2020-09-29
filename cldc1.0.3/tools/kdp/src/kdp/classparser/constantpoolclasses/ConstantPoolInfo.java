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
 * Base class for representing an item in the 
 * constant pool of a Java class file.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: ConstantPoolInfo.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: ConstantPoolInfo.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:12  dsimon
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
public abstract class ConstantPoolInfo
  {
   /** All the possible types of constant pool entries as specified by
       the JVM Specification. */
   public static final byte	CONSTANT_Class				= 7;
   public static final byte	CONSTANT_Fieldref			= 9;
   public static final byte	CONSTANT_Methodref			= 10;
   public static final byte	CONSTANT_InterfaceMethodref 	= 11;
   public static final byte	CONSTANT_String			= 8;
   public static final byte	CONSTANT_Integer			= 3;
   public static final byte	CONSTANT_Float				= 4;
   public static final byte	CONSTANT_Long				= 5;
   public static final byte	CONSTANT_Double			= 6;
   public static final byte 	CONSTANT_NameAndType		= 12;
   public static final byte	CONSTANT_Utf8				= 1;

   /** Stores the type of constant pool entry this is */
   protected byte			tag;

   public byte getTag() {
       return tag;
   }

   /** All subclasses must implement the toString class to display their
       individual attributes */
   public abstract String toString ();
  }
  
