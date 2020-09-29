/* 
 * Copyright © 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 * Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 * 
 * Sun Microsystems, Inc. has intellectual property rights relating
 * to the technology embodied in this software.  In particular, and
 * without limitation, these intellectual property rights may include
 * one or more U.S. patents, foreign patents, or pending
 * applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 * and all Sun-based and Java-based marks are trademarks or
 * registered trademarks of Sun Microsystems, Inc.  in the United
 * States and other countries.
 * 
 * This software is distributed under licenses restricting its use,
 * copying, distribution, and decompilation.  No part of this
 * software may be reproduced in any form by any means without prior
 * written authorization of Sun and its licensors, if any.
 * 
 * FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 * Subject to Standard License Terms and Conditions
 * 
 */


package palm.database;

import java.io.*;
import java.util.*;

abstract class DatabaseRecord { 
    static final int dmRecAttrSecret = 0x10; 
    static final int dmRecAttrBusy = 0x20;
    static final int dmRecAttrDirty = 0x40;
    static final int dmRecAttrDelete = 0x80;

    abstract int getFlags() throws IOException ;
    abstract int getCategory() throws IOException;

    // Subclasses must implement one or the other of the following
    // two methods, or else we'll have an infinite loop.

    protected byte[] bytes;

    public byte[] getBytes() throws IOException 
    { 
        if (bytes == null) { 
            ByteArrayOutputStream  b = new ByteArrayOutputStream();
            DataOutputStream os = new DataOutputStream(b);
            writeBytes(os);
            os.flush();
            os.close();
            bytes = b.toByteArray();
        } 
        return bytes;
    }

    public void writeBytes(DataOutput os) throws IOException { 
        os.write(getBytes());
    }

    // Return a unique ID for this record, or -1 indicating that we don't care.
    public int getUniqueID() { 
        return -1;
    }


    static class ClassRecord extends DatabaseRecord { 
        ClassInfo ci;
        int type;

        public final static int ApplicationMainFlag = 0;
        public final static int CoreJavaFlag = 1;
        public final static int ApplicationInternalFlag = 2;

        ClassRecord(ClassInfo ci, boolean isCore) { 
            this.ci = ci;
            this.type = isCore ? CoreJavaFlag : 
                ci.hasMain() ? ApplicationMainFlag : ApplicationInternalFlag;
        }

        public void writeBytes(DataOutput os) throws IOException { 
            os.writeBytes(ci.getClassName());
            os.write(0);
            os.writeInt(type << 24);    // the type, followed by three 0's
            os.write(ci.getBytes());
        }

        public int getCategory() { 
            return ci.hasMain() ? 1 : 2;
        }

        public int getFlags() { return 0; } 
    }

}   
