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

/*
 * MakePalmDB
 *  
 * This program takes a set of Java classes and converts them into
 * a Palm database (PDB) file.
 *
 * This tool is used rarely these days now that all the system 
 * classes are typically prelinked (romized) into the KVM executable
 * using the JavaCodeCompact tool.
*/

package palm.database;

import java.io.*;
import java.util.*;

public class MakePalmDB extends DatabaseGenerator { 

    public static void main(String argv[]) throws IOException {
        new MakePalmDB(argv);
    }

    /* 
     * These are values set by parseArguments().  
     * We give some of them their default value, here. 
     */

    // Verbosity
    int   verbose = 0;
    // Where to find application classes
    String userClassPathString = ".";
    // Name to print in help message
    String usageName = "java palm.database.MakePalmDB";
    // Classes that are part of the system
    String systemPackageList = "java:com.sun.kjava";            
    String dbCreator;           // creator ID for database
    String dbType;              // type for database
    String dbName;              // name for database
    String outputFile;          // name of output file

    String classNames[];    // other classes, such as from forName

    MakePalmDB (String argv[]) throws IOException { 
        if (!parseArguments(argv)) 
            return;
        String systemPackages[] = mungeSkipList(systemPackageList);

        boolean error = false;
        // Add all the resources from the indicated class(es).
        ClassPath userClassPath = new ClassPath(userClassPathString);
        DatabaseRecord[] records = new DatabaseRecord[classNames.length];

        for (int i = 0; i < classNames.length; i++) { 
            String name = classNames[i];
            boolean isFile = name.endsWith(".class");
            ClassInfo ci = isFile ? new ClassInfo(name)
                                  : new ClassInfo(name, userClassPath);
            if (ci != null) { 
                boolean isSystemClass = false;
                String className = ci.getClassName().replace('/', '.');
                for (int j = 0; j < systemPackages.length; j++) { 
                    if (className.startsWith(systemPackages[j])) {
                      isSystemClass = true;
                      break;
                    }
                }
                DatabaseRecord.ClassRecord r = 
                       new DatabaseRecord.ClassRecord(ci, isSystemClass);
                records[i] = r;
                if (verbose > 0) { 
                    System.out.println(ci.getClassName() + " " + r.type);
                }
            } else { 
                error = true;
                System.out.println("Cannot " + 
                                   (isFile ? "read file " : "find class ") +
                                   name);
            }
        }
        // Dump the database out
        if (error) { 
            return;
        }
        if (verbose > 0) { 
            System.out.println("Generating output file \"" + outputFile + "\".");
        }
        writeDatabase(outputFile, dbName, dbCreator, dbType, records);
    }

    /*
     * Parse the arguments
     */
    private boolean 
    parseArguments(String argv[]) { 
        int i;

        for (i = 0 ; i < argv.length ; i++) {
            String arg = argv[i];
            if (!arg.startsWith("-")) {
                // No more options.
                break;
            }
            try { 
                if (arg.equals("-v")  || arg.equals("-verbose")) {
                    verbose++;
                } else if (arg.equals("-classpath")) { 
                    userClassPathString = argv[++i];
                } else if (arg.equals("-longname") || arg.equals("-name")) { 
                    dbName = argv[++i];
                } else if (arg.equals("-name")) { 
                    dbName = argv[++i];
                } else if (arg.equals("-o") || arg.equals("-outfile")) { 
                    outputFile = argv[++i];
                } else if (arg.equals("-creator")) { 
                    dbCreator = (argv[++i] + "    ").substring(0, 4);
                } else if (arg.equals("-type")) { 
                    dbType = (argv[++i] + "    ").substring(0, 4);
                } else if (arg.equals("-systemlist")) { 
                    systemPackageList = argv[++i];
                } else if (arg.equals("-help")) { 
                    usage();
                    return false;
                } else {
                    System.err.println("invalid flag: " + arg);
                    usage();
                    return false;
                }
            } catch (ArrayIndexOutOfBoundsException e) { 
                System.err.println(argv[argv.length - 1] + 
                                   " requires an argument");
                return false;
            }
        }
        if (i == argv.length) { 
            System.err.println("No class names given");
            usage();
            return false;
        }
        if (verbose > 2) { 
            System.err.println("I don't know how to be >>that<< verbose!");
        }
        Vector classNameVect = new Vector();
        for (int j = i; j < argv.length; j++) { 
            String arg = argv[j];
            if (arg.startsWith("-")) {
                System.err.println("option \"" + arg 
                                   + "\" must appear before class names");
                return false;
            }
            if (!(new File(arg)).isDirectory()) classNameVect.addElement(arg);
        }
        classNames = new String[classNameVect.size()];
        classNameVect.copyInto(classNames);
        
        if (dbName == null) { 
          dbName = "KauaiClassesPatchDB";
        }
        if (outputFile == null) { 
            outputFile = "ClassesDBPatch.pdb";
        }
        if (dbCreator == null) { 
            dbCreator = "kJax";
        }
        if (dbType == null) { 
            dbType = "Data";
        }
        return true;
    }

    private static String[]
    mungeSkipList(String skipList) { 
        Vector v = new Vector();
        StringTokenizer t = new StringTokenizer(skipList, ":;");
        while (t.hasMoreTokens()) { 
            String str = t.nextToken();
            if (!str.endsWith(".")) 
                str += ".";
            v.addElement(str);
        }
        String[] result = new String[v.size()];
        v.copyInto(result);
        return result;
    }

    /**
     * Tell users how to use this function
     */

    private void usage() { 
        System.err.print(usageName);
        System.err.print(
" [<options> . . .] <classname|filename> [<classname|filename>] . . .\n" +
"where options include\n" + 
"-v                 Verbose output (-v -v gives even more information)\n" + 
"-verbose           Same as -v\n" +
"-classpath <directories separated by colons> \n" +
"                   Directories in which to search for application classes\n" +
"-name <name>       Name of the database.\n"+
"-longname <name>   Same as -name.  [Databases only have one name]\n" + 
"-creator <crid>    Creator ID for the database (4 characters)\n" +
"-type <type>       Type ID for the database (4 characters)\n" +
"-outfile <outfile> Name of file to create on local disk;\n" +
"                   This is the file that is downloaded to the Palm\n" +
"-o <outfile>       Same as -outfile\n" + 
"-systemlist <packagelist>\n" + 
"                   Colon-separated list of packages in the core VM\n" + 
"-help              Print this message\n" +
"-usagename         Program name to appear in this help message\n" +
""
            );
    }
}
