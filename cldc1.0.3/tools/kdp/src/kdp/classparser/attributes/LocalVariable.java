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

public class LocalVariable
{
    private String name;
    private String type ;
    private int length;
    private int slot;
    private long codeIndex;
    
    public LocalVariable(String name, String type, int codeIndex, 
                         int length, int slot ){
        this.name = name;
        this.type = type;
        this.codeIndex = codeIndex;
        this.length = length;
        this.slot = slot;
    }
    
    
    public String getName(){
        return name;
    }
    
    public String getType(){
        return type;   
    }
    
    public int getLength(){
        return length;
    }
    
    public int getSlot() { 
        return slot;
    }
    
    public long getCodeIndex(){
        return codeIndex;
    }
    
    public void print(){
        Log.LOGN(5, "Name: " + name + "\nClass: " + type);  
        Log.LOGN(5, "CodeIndex: " + codeIndex + "\nLength: " + length);
         Log.LOGN(5, "Slot: " + slot);
    }

}
