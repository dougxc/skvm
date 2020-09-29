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

public class ClassInfo { 
    public static final int CONSTANT_Utf8 = 1;
    public static final int CONSTANT_Unicode = 2;
    public static final int CONSTANT_Integer = 3;
    public static final int CONSTANT_Float = 4;
    public static final int CONSTANT_Long = 5;
    public static final int CONSTANT_Double = 6;
    public static final int CONSTANT_Class = 7;
    public static final int CONSTANT_String = 8;
    public static final int CONSTANT_Field = 9;
    public static final int CONSTANT_Method = 10;
    public static final int CONSTANT_InterfaceMethod = 11;
    public static final int CONSTANT_NameAndType = 12;

    public static final int ACC_PUBLIC = 1;
    public static final int ACC_PRIVATE = 2;
    public static final int ACC_PROTECTED = 4;
    public static final int ACC_STATIC = 8;
    public static final int ACC_FINAL = 16;
    public static final int ACC_SYNCHRONIZED = 32;
    public static final int ACC_VOLATILE = 64;
    public static final int ACC_TRANSIENT = 128;
    public static final int ACC_NATIVE = 256;
    public static final int ACC_INTERFACE = 512;
    public static final int ACC_ABSTRACT = 1024;
    public static final int ACC_SUPER = 32;

    private Object[] pool;
    private Vector classReferences = new Vector();
    private String className;
    private boolean hasMain;
    private Vector natives;
    private byte bytes[];

    public String  getClassName(){  return className; }
    public boolean hasMain()     {  return hasMain; }
    public byte[]  getBytes()    {  return bytes; }

    ClassInfo (String fileName) throws IOException { 
        File file = new File(fileName);
        int length = (int)file.length();
        InputStream is = new FileInputStream(fileName);
        this.bytes = new byte[length];
        if (is.read(bytes) != length) { 
            throw new IOException("Failed to read " + fileName);
        } 
        is.close();
        readClassFile();
    }

    ClassInfo (byte[] bytes) throws IOException { 
        this.bytes = bytes;
        readClassFile();
    }

    ClassInfo (String className, ClassPath classpath) throws IOException { 
        if ((className.indexOf('/') != -1) || 
               (className.indexOf('\\') != -1)) { 
            throw new IOException("Bad classname " + className);
        }
        String fileName = 
             className.replace('.', File.separatorChar)  + ".class";
        ClassPath.ClassFile file = classpath.getFile(fileName);
        if (file == null) { 
            throw new IOException("Cannot find file " + fileName);
        }
        DataInputStream is = new DataInputStream(file.getInputStream());
        bytes = new byte[(int)file.length()];
        is.readFully(bytes);
        is.close();
        readClassFile();
    }

    private void 
    readClassFile() throws IOException {
        DataInput in = new DataInputStream (new ByteArrayInputStream(bytes));
        int count;
        in.skipBytes(8);        // magic, minorID, majorID
        readConstantPool(in);
        int accessFlags =  in.readUnsignedShort();
        int thisClass   = in.readUnsignedShort();
        int superClass  = in.readUnsignedShort();

        this.className = className(thisClass);

        // Ignore Interfaces
        count = in.readUnsignedShort();
        for (int i = 0; i < count; i++) { 
            int intf = in.readUnsignedShort();
            classReferences.addElement(className(intf));
        }

        // Read the fields 
        count = in.readUnsignedShort();
        for (int i = 0; i < count; i++) { 
            // int fieldAccess = in.readUnsignedShort();
            // int nameIndex = in.readUnsignedShort();
            // int typeIndex = in.readUnsignedShort();
            in.skipBytes(6);
            skipAttributes(in);
        }
        
        // Read the methods
        count = in.readUnsignedShort();
        for (int i = 0; i < count; i++) {
            int methodAccess = in.readUnsignedShort();
            int nameIndex = in.readUnsignedShort();
            int typeIndex = in.readUnsignedShort();
            String name = (String)pool[nameIndex];
            String type = (String)pool[typeIndex];
            skipAttributes(in);
            if (   name.equals("main") 
                && type.equals("([Ljava/lang/String;)V") 
                   && ((methodAccess & ACC_PUBLIC) != 0)) { 
                hasMain = true;
            }
            if ((methodAccess & ACC_NATIVE) != 0) { 
                if (natives == null) 
                    natives = new Vector();
                natives.addElement(name);
            }
        }
        // Skip the attributes
        skipAttributes(in);
    }

    void skipAttributes(DataInput in) throws IOException { 
        int count = in.readUnsignedShort();
        for (int i = 0; i < count; i++) { 
            int name = in.readUnsignedShort();
            int length = in.readInt();
            in.skipBytes(length);
        }
    }
        
    void 
    readConstantPool(DataInput in) throws IOException { 
        int count = in.readUnsignedShort();
        Object[] pool = new Object[count];
        int[] tags = new int[count];
        pool[0] = tags;
        for (int i = 1; i < count; i++) { 
            int tag = in.readUnsignedByte();
            tags[i] = tag;
            switch (tag) { 
                case CONSTANT_Class: 
                case CONSTANT_String: 
                    pool[i] = new Integer(in.readUnsignedShort());
                    break;

                case CONSTANT_Utf8: 
                    pool[i] = in.readUTF();
                    break;

                case CONSTANT_Field: 
                case CONSTANT_Method: 
                case CONSTANT_InterfaceMethod: 
                case CONSTANT_NameAndType: 
                case CONSTANT_Integer: 
                case CONSTANT_Float: 
                    // We don't need these
                    in.skipBytes(4);
                    break;
                
                case CONSTANT_Long: 
                case CONSTANT_Double:
                    // We don't need these
                    in.skipBytes(8);
                    i++;
                    break;

                default:
                    System.err.println("Unknown tag " + tag);
                    break;

            }
        }
        this.pool = pool;

        for (int i = 1; i < count; i++) { 
            if (tags[i] == CONSTANT_Class) { 
                String className = className(i);
                if (className.charAt(0) == '[') { 
                    int length = className.length();
                    if (className.charAt(length - 1) == ';') { 
                        int L = className.indexOf('L');
                        className = className.substring(L + 1, length - 1);
                    } else { 
                        continue;
                    }
                }
                classReferences.addElement(className);
            }
        }

    }

    String className(int index) { 
        int info = ((Integer)(pool[index])).intValue();
        return (String)pool[info];
    }


    static ClassInfo[] 
    classClosure(String className, String[] additionalClasses, 
                 ClassPath classPath, ClassPath bootClassPath) { 
        // the classes in "queue" are identical to the keys in "seen".  The
        // separate hashtable just makes it faster for us to determine if
        // we've already seen an item or not.  Every item on these are
        // in "canonical form".
        Hashtable seen = new Hashtable();
        Vector queue = new Vector();
        Vector result = new Vector();
        boolean error = false;

        // We purposely don't put className or additionalClasses into
        // canonical form.  This should be an error.
        seen.put(className, className);
        queue.addElement(className);

        if (additionalClasses != null) { 
            for (int i = 0; i < additionalClasses.length; i++) { 
                String clazz = additionalClasses[i];
                if (seen.get(clazz) == null) { 
                    seen.put(clazz, clazz);
                    queue.addElement(clazz);
                }
            }
        }
        // All classes with index before this are in, whether or not
        // they are system classes
        int requiredClasses = queue.size();

    classLoop:
        for (int i = 0; i < queue.size(); i++) { 
            String thisClass = (String)queue.elementAt(i);
            ClassInfo ci;
            if (bootClassPath != null) { 
                try { 
                    ci = new ClassInfo(thisClass, bootClassPath);
                    // If we didn't get an error, this is a boot class
                    if (i < requiredClasses) { 
                        result.addElement(ci);
                    }
                    continue classLoop;
                }
                catch (IOException e) { }
            } 
            try { 
                ci = new ClassInfo(thisClass, classPath);
            } catch (IOException e) { 
                System.err.println("ERROR: " + e);
                error = true;
                continue classLoop;
            }
            result.addElement(ci);
            Vector references = ci.classReferences;
            for (int j = 0; j < references.size(); j++) { 
                String reference = (String)references.elementAt(j);
                // Put the reference into "canonical" form. 
                // They contain a "/" in the class file
                reference = reference.replace('/', '.');
                if (seen.get(reference) == null) { 
                    seen.put(reference, reference);
                    queue.addElement(reference);
                }
            }
        }
        if (error) { 
            return null;
        }
        ClassInfo infos[] = new ClassInfo[result.size()];
        result.copyInto(infos);
        return infos;
    }

    // For testing
    public static void main(String argv[]) { 
        String className = 
            argv.length > 0 ? argv[0] : "palm.database.ClassInfo";
        String classPath = 
            argv.length > 1 ? argv[1] : "classes.zip";
        String bootClassPath = 
            argv.length > 2 ? argv[2] : 
                "/usr/local/java/jdk1.2/solaris/jre/lib/rt.jar";
        ClassInfo result[] = 
            classClosure(className, 
                         new String[] { "java.lang.System" }, 
                         new ClassPath(classPath), 
                         new ClassPath(bootClassPath));

        if (result != null) { 
            for (int i = 0; i < result.length; i++) { 
                System.out.println(result[i].className);
            }
        }
    }
}

