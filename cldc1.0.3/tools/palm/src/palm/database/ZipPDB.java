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

/**
 * ZipPDB
 *
 * This class reads all Java classes in a .zip/jar database and 
 * converts them into a Palm database (PDB) pdb file.
 */

package palm.database;

import java.util.*;
import java.io.*;
import java.util.zip.*;

public class ZipPDB extends DatabaseGenerator { 

    public static void main(String argv[]) throws IOException { 
        if (argv.length != 5) { 
            System.out.println("ZipPDB <pdbName> <longName> <creator> <type> <zipfile>");
                               System.exit(1);
        } 
        new ZipPDB().run(argv);
    }

    // Does the actual work of parsing the arguments and dumping the file
    private void 
    run(String argv[]) throws IOException {
        String dbName = argv[1];
        String dbCreator = argv[2];
        String dbType = argv[3];
        // Get all the .class files in the zip 
        String keys[] = readZipNames(argv[4]);
        PalmUtil.sort(keys);
        ClassPath classpath = new ClassPath(argv[4]);

        // Create a list of Database Records.
        int size = keys.length;
        DatabaseRecord records[] = new DatabaseRecord[size];
        for (int recordNo = 0; recordNo < size; recordNo++) { 
            String className = keys[recordNo];
            ClassInfo ci = new ClassInfo(className, classpath);
            records[recordNo] = new DatabaseRecord.ClassRecord(ci, true); 
        }

        // Write out the file
        writeDatabase(argv[0], dbName, dbCreator, dbType, records);
    }

    // Return a list of all the .class files in the specified zip file
    private String[] 
    readZipNames(String zipFileName) throws IOException { 
        Vector names = new Vector();
        ZipInputStream zip = 
            new ZipInputStream(new FileInputStream(zipFileName));
        ZipEntry ent;
        while ((ent = zip.getNextEntry()) != null) { 
            if (ent.isDirectory()) { 
                // Skip directories
                continue;
            }
            String entName = ent.getName();
            if (entName.endsWith(".class")) {
                // Get class names, and remove the final .class
                String className = entName.substring(0, entName.length() - 6);
                names.addElement(className.replace('/', '.'));
            }
        }
        // Convert it into an array
        int classCount = names.size();
        String[] keys = new String[classCount];
        names.copyInto(keys);
        return keys;
    }
}





