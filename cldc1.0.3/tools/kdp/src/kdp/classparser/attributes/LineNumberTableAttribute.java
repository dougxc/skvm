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

import kdp.Log;
import kdp.classparser.attributes.*;
import kdp.classparser.constantpoolclasses.*;

import java.io.*;

/**
 * Encapsulates a LineNumberTable attribute of a Java class file.
 *
 * @author        Aaron Dietrich
 * @version       $Id: LineNumberTableAttribute.java,v 1.1.1.1 2002/05/21 20:38:48 dougxc Exp $
 *
 * Revision History
 *   $Log: LineNumberTableAttribute.java,v $
 *   Revision 1.1.1.1  2002/05/21 20:38:48  dougxc
 *   All the Secure KVM project stuff under one top level CVS module
 *
 *   Revision 1.1.1.1  2002/02/26 00:20:06  dsimon
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
public class LineNumberTableAttribute extends Attribute
  {
    /** index into the constant pool table containing the name 
         of this class */
    private int                attributeNameIndex;
    /** length of this attribute in bytes */
    private int                attributeLength;
    /** number of elements in the line number table array */
    private int                lineNumberTableLength;
    /** each entry indicates that the line number in the original 
         source file changes at a given point in the code array. */
    private LineNumberTable    lineNumberTable[];
    
    /**
     * Constructor.  Reads the LineNumberTableAttribute attribute from
     * the class file.
     *
     * @param    iStream           the input stream on which to
     *                             read the class file
     * @param    attributeNameIndex     attributeNameIndex member of
     *                                  attribute structure.
     * @param    attributeLength     attributeLength member of
     *                               attribute structure.
     *
     * @exception    IOException         pass IOExceptions up
     */
    public LineNumberTableAttribute (DataInputStream iStream,
        int attributeNameIndex, int attributeLength) throws IOException
      {
        this.attributeNameIndex = attributeNameIndex;
        this.attributeLength = attributeLength;

        //read in the length of the LineNumberTable array
        lineNumberTableLength = iStream.readUnsignedShort ();

        //allocate space for the LineNumberTable array and read in the data
        lineNumberTable = new LineNumberTable[lineNumberTableLength];
        for (int lcv = 0; lcv < lineNumberTableLength; ++lcv)
          {
           lineNumberTable[lcv] = new LineNumberTable ();
           lineNumberTable[lcv].startPC = iStream.readUnsignedShort ();
           lineNumberTable[lcv].lineNumber = iStream.readUnsignedShort ();
          }
      }

    public int[][] getLineNumbersAndIndicesAsArray ()
      {
        int      lineArray[][] = new int[lineNumberTableLength][2];

        for (int lcv = 0; lcv < lineNumberTableLength; ++lcv)
          {
           lineArray[lcv][0] = lineNumberTable[lcv].lineNumber;
           lineArray[lcv][1] = lineNumberTable[lcv].startPC;
          }

        return lineArray;
      }

    /**
     * Returns the first code index associated with the specified source
     * file line number.
     *
     * @param        lineNumber          source file line number
     *
     * @return       int                 index into code array of
     *                                   first bytecode of this instruction
     *                                   If the line number does not map to
     *                                   a code array index then return -1
     */
    public int getCodeIndexBySourceLineNumber (int lineNumber)
      {
        boolean       found = false;
        int           index = 0;

        //loop through line number table until we find an entry with the
        //specified source file line number or run out of entries
        while ((!found) && (index < lineNumberTableLength))
          {
           if (lineNumber == lineNumberTable[index].lineNumber)
             found = true;
           else
             ++index;
          }

        //if an entry was found return the code array index associated with it
        if (found)
          return (lineNumberTable[index].startPC);
        else
          return -1; //no entry found so return -1
      }

    /**
     * Determine if this lineNumber table contains the specified source file
     * line number.
     *
     * @param        lineNumber          source file line number
     *
     * @return       boolean             true if line number was found, false
     *                                   otherwise
     */
    public boolean containsLine (int lineNumber)
      {
        boolean       found = false;
        int           index = 0;

        //loop through each entry in the line number table until
        //the specified line number is found or there are no
        //entries left
        while ((!found) && (index < lineNumberTableLength))
          {
           if (lineNumber == lineNumberTable[index].lineNumber)
             found = true;
           else
             ++index;
          }
        return found;
      }

    /**
     * Returns the line that this opcode offset lies on
     *
     */
    private int getIndexThatContainsOpcode(long offset) {

         for (int i=0; i < lineNumberTableLength - 1; i++) {
             if (offset >= lineNumberTable[ i ].startPC && 
                  offset < lineNumberTable[ i + 1 ].startPC) {
                 return i;
             }
         }
         if (offset >= lineNumberTable[ lineNumberTableLength - 1 ].startPC)
             return lineNumberTableLength - 1;

         return -1; // lineNumberTable[ 0 ].lineNumber;

    }

    public int getDupCurrentExecutableLineCodeIndex(long offset) {

    int firstIndex = getNextExecutableLineCodeIndex(offset, false);
    if (firstIndex == -1)
        return -1;
    firstIndex--;
    return getOtherLineIndex(firstIndex);
    }

    public long getOffsetofDupNextLine(int index) {
    if (index == -1 || index == lineNumberTableLength - 1)
        return -1;
    return lineNumberTable[index + 1].startPC;
    }

    private int getOtherLineIndex(int index) {
         for (int i=0; i < lineNumberTableLength; i++) {
             if (i == index) continue;
             if (lineNumberTable[ i ].lineNumber == 
                  lineNumberTable[ index ].lineNumber) {
                 return i;
             }
         }
         return -1;

    }

    public int getCurrentLineCodeIndex(long offset) {

        int currentIndex = getIndexThatContainsOpcode(offset);
        Log.LOGN(3,  "getCurrent: index=" + currentIndex);
        return currentIndex;
    }

    public int getNextExecutableLineCodeIndex(long offset) {
    return getNextExecutableLineCodeIndex(offset, true);
    }

    private int getNextExecutableLineCodeIndex(long offset, boolean logging) {

         int firstIndex = getIndexThatContainsOpcode(offset);
         if (logging)
             Log.LOGN(3,  "getNext: firstIndex=" + firstIndex);
         int otherIndex; 
         if ((otherIndex = getOtherLineIndex(firstIndex)) != -1) {
             if (logging)
                 Log.LOGN(3,  "getNext: otherIndex=" + otherIndex);
             if (otherIndex < firstIndex) {
                 if (logging)
            Log.LOGN(3,  "getNext: otherIndex < firstIndex");
        return otherIndex + 1;
             } else {
                 if (logging)
            Log.LOGN(3,  "getNext: otherIndex > firstIndex");
        return firstIndex + 1;
             }
         }
 
         if (firstIndex+1 < lineNumberTableLength)
             return firstIndex + 1;
         else
             return -1;
    }

    public long getStartPCFromIndex(int index) {
        if (index == -1)
            return -1;
        return lineNumberTable[index].startPC;
    }

    public int getLineNumberFromIndex(int index) {
        if (index == -1 || index == lineNumberTableLength - 1)
            return (-1);
        return lineNumberTable[index].lineNumber;
    }

    /**
     * Returns the LineNumberTableAttribute attribute in a nice easy to
     * read format as a string.
     *
     * @param        constantPool        constant pool of the class file
     *
     * @return         String            the attribute as a nice easy to
     *                            read String
     */
    public String toString (final ConstantPoolInfo[] constantPool)
      {
        ConstantUtf8Info            utf8Info;
        String                        s = new String ("");

        utf8Info = (ConstantUtf8Info) constantPool[attributeNameIndex];
        s = s + "\t" + utf8Info.toString ();
 /*
        for (int lcv = 0; lcv < lineNumberTableLength; ++lcv)
          {
           s = s + "\n\t\t\t\tStart PC=  " + lineNumberTable[lcv].startPC;
           s = s + "\tLine Number=  " + lineNumberTable[lcv].lineNumber;
          }
 */       
        return s;
      }
  }
