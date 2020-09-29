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
/**
 * Represents an individual item in the LocalVariableTable of a
 * Java class file.
 *
 * @author 			Aaron Dietrich
 * @version			$Id: LocalVariableTable.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: LocalVariableTable.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:07  dsimon
 *   initial import of SVM to CVS
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
package kdp.classparser.attributes;

import kdp.classparser.attributes.*;

public class LocalVariableTable
  {
   /** index into code array that begins the range where
       a local variable has a value */
   public int		startPC;
   /** index into code array, startPC + length specifies
       the position where the local variable ceases to
       have a value */
   public int		length;
   /** index into constant pool table containing the name
       of the local variable as a simple name */
   public int		nameIndex;
   /** index into constant pool table containing the
       encoded data type of the local variable */
   public int		descriptorIndex;
   /** local variable must be at index in the local 
       variable array of the current frame */
   public int		index;
  }
