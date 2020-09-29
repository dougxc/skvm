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

abstract class 
Resource extends DatabaseRecord { 
    public int getFlags() { 
        throw new RuntimeException("This shouldn't be called");
    }

    public int getCategory() { 
        throw new RuntimeException("This shouldn't be called");
    }

    // Return a unique ID for this record, or -1 indicating that we don't care.
    public int getUniqueID() { 
        throw new RuntimeException("This shouldn't be called");
    }

    public abstract String getType()  ;
    public abstract int getID() ;

    protected Resource() {}

    String getName()  { 
        int ID = getID() & 0xFFFF;
        String IDx = Integer.toHexString(0x10000 + ID).substring(1);
        return getType().substring(0, 4) + IDx;
    }

    void writeToBin() throws IOException { 
        OutputStream os = new FileOutputStream(getName() + ".bin");
        os.write(getBytes());
        os.flush();
        os.close();
    }

    static int IDfromCRC(String name, BitSet used) { 
        int crc = PalmUtil.CRC(name) & 0xFFFF;
        while (used.get(crc)) { 
            crc = (crc + name.length()) & 0xFFFF;
        }
        used.set(crc);
        return crc;
    }

    void showPretty(PrintStream out, String info, boolean verbose) {
        if (verbose) { 
            String ID = ((getID() & 0xFFFF) + ":      ").substring(0, 7);
            out.print(getType() + " " + ID);
        }
        out.println(info);
    }

    // The simplest resource.  We're told the type, id, and bytes
    static class RawResource extends Resource { 
        byte[] bytes;
        String type;
        int ID;
        int start, end;

        RawResource(byte[] bytes, String type, int ID) {  
            this(bytes, type, ID, 0, bytes.length);
        }

        RawResource(String str, String type, int ID) {  
            this(str.getBytes(), type, ID);
        }

        RawResource(byte[] bytes, String type, int ID, int start, int end) {  
            this.bytes = bytes;
            this.type = type;
            this.ID = ID & 0xFFFF;
            this.start = start;
            this.end = end;
        }

        public String getType()  { return type; }
        public int    getID()    { return ID; }
        public void writeBytes(DataOutput os) throws IOException { 
            os.write(bytes, start, end - start);
        }
    }

    // Resource from a file
    static class FileResource extends Resource { 
        byte[] bytes;
        String type;
        int ID;
        ClassPath.ClassFile file;

        FileResource(String fileName) throws IOException { 
            this(fileName, new ClassPath("."));
        }

        FileResource(String fileName, ClassPath classPath) throws IOException { 
            try { 
                this.type = fileName.substring(0, 4);
                this.ID = (short)Integer.parseInt(fileName.substring(4, 8), 16);
            } catch (RuntimeException e) { 
                throw new IOException("Bad filename for resource: " + fileName);
            }
            file = classPath.getFile(fileName);
            if (file == null) { 
                throw new IOException("Cannot find file " + fileName);
            }
        }

        public void writeBytes(DataOutput os) throws IOException { 
            if (this.bytes == null) { 
                int length = (int)file.length();
                bytes = new byte[length];
                DataInputStream is = new DataInputStream(file.getInputStream());
                is.readFully(bytes);
                is.close();
            }
            os.write(bytes);
        }

        public String getType()  { return type; }
        public int    getID()    { return ID; }
    }

    // A "Java resource", meaning something you get using getResourceAsStream.
    static class JavaResource extends Resource { 
        String fileName;
        byte[] bytes;
        ClassPath.ClassFile file;
        int ID;

        JavaResource(String fileName, ClassPath classPath, BitSet used)
            throws IOException {

            this.fileName = fileName;
            this.file = classPath.getFile(fileName);
            this.ID = Resource.IDfromCRC(fileName, used);
            if (file == null) { 
                throw new IOException("Cannot find file " + fileName);
            }
        }

        public void writeBytes(DataOutput os) throws IOException { 
            os.writeBytes(fileName);
            os.write(0);
            if (bytes == null) { 
                int length = (int)file.length();
                bytes = new byte[length];
                DataInputStream is = new DataInputStream(file.getInputStream());
                is.readFully(bytes);
                is.close();
            }
            os.write(bytes);
        }

        public String getType()  { return "Rsrc"; }
        public int    getID()    { return ID; }
    }


    static class ClassResource extends Resource { 
        ClassInfo ci;
        int ID;
        
        ClassResource(ClassInfo ci, BitSet used) { 
            this.ci = ci;
            String className = ci.getClassName();
            ID = Resource.IDfromCRC(ci.getClassName(), used);
        }

        public String getType() { return "Clas"; }
        public int    getID()   { return ID; }

        public void writeBytes(DataOutput os) throws IOException { 
            os.writeBytes(ci.getClassName());
            os.write(0);
            // 0 = application class (with main function)
            // 1 = Core java class
            // 2 = internal application class (no main function)
            // and then three zeros
            int flag = ci.hasMain() 
                          ? DatabaseRecord.ClassRecord.ApplicationMainFlag
                          : DatabaseRecord.ClassRecord.ApplicationInternalFlag;
            os.writeInt(flag << 24);    // write flag, and three zeros
            os.write(ci.getBytes());
        }
    }

    static class IconResource extends Resource { 
        int ID;
        byte[] bytes;
        Bitmap icon;

        IconResource(Bitmap icon, int ID) { 
            this.icon = icon;
            this.ID = ID & 0xFFFF;
        }
        
        public String getType() { return "tAIB"; }
        public int    getID()   { return ID; }

        public void writeBytes(DataOutput os) throws IOException {
            icon.writeBin(os);
        }
    }

    static class ApplicationNameResource extends RawResource { 
        ApplicationNameResource(String name) { 
            super((name + "\0").getBytes(), "tAIN", 1000);
        }
    }
}


