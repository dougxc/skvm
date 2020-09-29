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
import java.util.zip.*;
import java.util.jar.*;

/*=========================================================================
 * Palm Tool Converter 
 *=========================================================================
 * FILE:      convPRCtoJAR.java
 * OVERVIEW:  This program takes a .PRC file and converts it to a jar file. 
 *            The class resources within the PRC file are retrieved, and a
 *            a jar file entry is created for each of the classes and written 
 *            out to the jar file along with the class data.
 *            This class uses the following classes from tools/palm/database:
 *             - Resource for reading class resources
 *             - ClassPath for locating the PRC file
 *             - DatabaseGenerator for getting attributes from the PRC.
 *
 * AUTHOR:    Tasneem Sayeed, JavaSoft 
 *=======================================================================*/

public class ConvPRCtoJAR extends DatabaseGenerator { 

/*=========================================================================
 * FUNCTION:      main()
 * TYPE:          main method
 * OVERVIEW:      invokes the ConvPRCtoJAR() constructor to convert the PRC 
 *                to a jar file. 
 * INTERFACE:
 *   parameters:  argv (arguments passed)
 *   returns:     nothing 
 *=======================================================================*/

    public static void main(String argv[]) throws IOException { 
        new ConvPRCtoJAR (argv);
    }

/*=========================================================================
 * Constants and Variables 
 *=======================================================================*/

 /* 
  * These are values set by parseArguments().  
  * We give some of them their default value, here. 
  */

    int   verbose = 0;                  /* verbosity */
    String userClassPathString = ".";   /* where to find application classes */
                                        /* Name to print in help message */
    String usageName = "java palm.database.MakePalmApp";
    String mainClass;                   /* class containing 'main()' method */
    String PRCFile = "";                /* PRC file to be converted */
    String jarFile;                     /* JAR file to output */

    ClassPath classpath;

                                        /* system class path for searching 
                                         * the PRC file specified.
                                         */
    ClassPath systemClassPath = 
            new ClassPath(System.getProperty("java.class.path"));


/*=========================================================================
 * FUNCTION:      ConvPRCtoJAR()
 * TYPE:          class constructor 
 * OVERVIEW:      converts the PRC file to a jar file. 
 *                Invokes GetClassesFromResources() to retrieve all class
 *                resources and outputs them to a jar file. 
 * INTERFACE:
 *   parameters:  argv 
 *   returns:     nothing 
 *=======================================================================*/

    ConvPRCtoJAR(String argv[]) throws IOException { 
        if (!parseArguments(argv)) 
            return;
        
         String PRCFileName =
          "tools" + File.separator + "palm" + File.separator + PRCFile;

        // Find all the resources from the original .PRC file
        ClassPath.ClassFile PRC = systemClassPath.getFile(PRCFileName);

        if (verbose > 0) {
            System.out.println("System ClassPath : "+systemClassPath);
        }

        if (PRC == null) { 
            System.out.println("Cannot find PRC code \"" + 
                PRCFileName + "\" in classpath");
            return;
        }

        GetResourcesFromPRC(PRC);

        if (verbose > 0) { 
            dumpJarFile(jarFile);
        } 
    }


/*=========================================================================
 * FUNCTION:      GetResourcesFromPRC()
 * TYPE:          private class file 
 * OVERVIEW:      retrieves all the class resources from the .PRC file
 *                and adds each of the classes to the newly created jar file. 
 * INTERFACE:
 *   parameters:  PRC file 
 *   returns:     nothing 
 *=======================================================================*/

    private void
    GetResourcesFromPRC(ClassPath.ClassFile prc) throws IOException { 
        /* Read the file into a buffer, and then we deal with it there. */ 
        byte[] bytes = new byte[(int)prc.length()];
        DataInputStream is = new DataInputStream(prc.getInputStream());
        String dirName = "";
        String className = "";
        is.readFully(bytes);
        is.close();

        /* Now turn the array of bytes into a DataInput Stream */
        is = new DataInputStream(new ByteArrayInputStream(bytes));
        is.skip(32);
        int attributes = is.readShort();
        if ((attributes & DatabaseGenerator.dmHdrAttrResDB) == 0) { 
            System.err.println(prc + " is not a PalmOS resource file");
            throw new RuntimeException("Bad .prc file");
        }
        is.skip(76 - 34);
        int recordCount = is.readShort();

        /* Read the types, ID's and offsets into arrays */
        String type[]   = new String[recordCount]; 
        int    ID[]     = new int[recordCount];
        int    offset[] = new int[recordCount + 1];
        for (int i = 0; i < recordCount; i++) { 
            type[i] = "" + (char)is.readUnsignedByte() + 
                           (char)is.readUnsignedByte() + 
                           (char)is.readUnsignedByte() + 
                           (char)is.readUnsignedByte();
            if (verbose > 1) {
                System.out.println("Resource type: "+type[i].toString());
            }
            ID[i] = is.readUnsignedShort();
            offset[i] = is.readInt();
            if (verbose > 1) {
                System.out.println("Resource ID: " + ID[i]);
                System.out.println("Resource offset: "+ offset[i]);
            }
        }

        /* Construct a JAR outputstream to create a JAR file */
        JarOutputStream jout = new JarOutputStream(new FileOutputStream(jarFile));
        if (verbose > 1) {
            System.out.println("Created JarOutputStream for JAR file : "+jarFile);
        }

        /* initialize for the Manifest file */
        Manifest man = new Manifest();
        man.getMainAttributes().put(Attributes.Name.MANIFEST_VERSION, "1.0");

        offset[recordCount] = bytes.length; // this simplifies things. . .
        /* Add each of the resources to the database. */
        for (int i = 0; i < recordCount; i++) { 
            Resource r = new Resource.RawResource(bytes, type[i], ID[i],  
                                                  offset[i], offset[i+1]);
            /* Check if this is a CLASS resource */
            if ((type[i].toString()).equals("Clas")) {
                /* set up the stream */
                ByteArrayOutputStream b = new ByteArrayOutputStream();
                DataOutputStream dos = new DataOutputStream(b);
                /* get the bytes */
                r.writeBytes(dos);
                dos.close();
                /* convert to a byte array */
                byte[] resBytes = b.toByteArray();
                /* search for the classname until the first null character */
                /*
                   int j = 0;
                   while (resBytes[j] != '\0')
                       j++;
                   className = new String(resBytes, 0, j);
                */

                int j = 0;
                int k = 0;
                Vector dirList = new Vector();
                String classFname = "";
                while (resBytes[j] != '\0') { 
                    if (resBytes[j] == '/') {
                        /* got a directory */ 
                        dirName = new String(resBytes, k, j-k);
                        k = j + 1;
                        dirList.addElement(dirName);

                        if (verbose > 1) {
                            if (dirName != "")
                                System.out.println("Directory Name : "+dirName);
                            dirName = ""; /* get the next dir or class */
                        }
                    } 
                    j++;
                } 
                if (dirName == "") {
                    /* there are no directories, only class files */
                    className = new String(resBytes, 0, j);
                }
                if ((j>0) && (resBytes[j-1] != '/')) {
                    /* the last entry was a classname, so restore it */
                    /* don't append the 4 bytes at the end */
                    classFname = new String (resBytes, k, j-k);
                }

                int size = dirList.size();

                dirName = "";
                if (size != 0) {
                    /* get the entire directory */
                    for (int l=0; l < size; l++) {
                        dirName = dirName + dirList.elementAt(l) + '/';
                    }
                    if (verbose > 1) {

                        System.out.println("The number of directory entries: "+size);
                        System.out.println("DirName :"+dirName);
                        System.out.println("className :"+classFname);
                    }
                    /* get the fully qualified class name */
                    className = dirName + classFname; 
                } 

                if (resBytes[j] == '\0') {
                    if (resBytes[j-1] == '/') {
                        /* we just have to write out the 
                         * dir entry and then continue 
                         */

                        /* create a directory entry */
                        File dir = new File (dirName);
                        JarEntry je = new JarEntry(dirName);
                        /* write out the JAR entry */
                        jout.putNextEntry(je);
                        jout.closeEntry();
                        continue;
                    }
                }

                /* we have a className so append the .class suffix */
                className = className + ".class"; 
                if (verbose > 1) {
                    System.out.println("ClassName : "+className);
                }

                /* create the Manifest */
                File f = new File(className);
                if (verbose > 0) { 
                    System.out.println("Full path of ClassName : "+f.getPath());
                }
                Attributes attr = new Attributes();
                attr.put(new Attributes.Name("Date"),
                         new Date(f.lastModified()).toString());
                man.getEntries().put(f.getPath(),attr);

                /* create the JAR entry for this class */
                JarEntry je = new JarEntry(f.getPath());
                /* write out the JAR entry */
                jout.putNextEntry(je);
                /* skip the null after the classname, then the 4 bytes */
                /* then write out the class data */ 
                jout.write(resBytes, j+5, resBytes.length-(j+5));
                jout.closeEntry();
            } 
        }
        /* write out the Manifest file entries */
        /* create the META-INF directory to hold the manifest file */
        File dir = new File("META-INF/");
        JarEntry je = new JarEntry(dir.getName() + "/");
        jout.putNextEntry(je);
        /* create the JAR entry for the Manifest file */
        String manifestFile = dir.getName() + "/" + "MANIFEST.MF";
        File mf = new File(manifestFile);
        je = new JarEntry(manifestFile);
        /* write out the JAR entry */
        jout.putNextEntry(je);
        /* dump the manifest file */
        man.write(jout);

        /* dump the Manifest */
        if (verbose > 0) {
            System.out.println("Manifest Entries: ");
            man.write(System.out); 
        }

        jout.closeEntry();

        /* close the JAR file */
        jout.close();
    }


/*=========================================================================
 * FUNCTION:      dumpJarFile()
 * TYPE:          private class file 
 * OVERVIEW:      dumps the JAR entries from the JAR file specified. 
 * INTERFACE:
 *   parameters:  JAR file 
 *   returns:     nothing 
 *=======================================================================*/
     
    private void
    dumpJarFile(String jarFile) throws IOException { 
        JarInputStream jis = new JarInputStream(new FileInputStream(jarFile));

        /* Print the manifest entries */
        Manifest man = jis.getManifest();
        if (man != null) {
            printManifest(man);
        }
        System.out.println("\n");
        
        /* Dump the attributes for each of the JAR entries */
        JarEntry je;
        while ((je = jis.getNextJarEntry()) != null) {
            System.out.println("++++++++ " + je);
            if (je.getAttributes() != null) {
                printAttributes(je.getAttributes());
            }
        }
        jis.close();
    }
    
/*=========================================================================
 * FUNCTION:      printManifest()
 * TYPE:          private class file 
 * OVERVIEW:      dumps the Manifest file entries from the JAR file specified. 
 * INTERFACE:
 *   parameters:  Manifest 
 *   returns:     nothing 
 *=======================================================================*/

    static void 
    printManifest(Manifest man) {
        System.out.println("---------- Main Attributes");
        printAttributes(man.getMainAttributes());
        for (Iterator it=man.getEntries().keySet().iterator(); it.hasNext(); )
        {
            String ename = (String) it.next();
            System.out.println("---------- " + ename);
            printAttributes(man.getAttributes(ename));
        }
   }


/*=========================================================================
 * FUNCTION:      printAttributes()
 * TYPE:          private class file 
 * OVERVIEW:      dumps the attributes of the JAR entries within the JAR file. 
 * INTERFACE:
 *   parameters:  Attributes
 *   returns:     nothing 
 *=======================================================================*/

    static void 
    printAttributes(Attributes attr) {
        for (Iterator it=attr.keySet().iterator(); it.hasNext(); ) {
            Object key = it.next();
            System.out.println(key+": "+attr.get(key));
        }
    }


/*=========================================================================
 * FUNCTION:      parseArguments()
 * TYPE:          private class file 
 * OVERVIEW:      parses the arguments to the program 
 *                The synopsis of the command is as follows:
 *                    convPRCtoJAR -verbose
 *                                 -classpath <dir or JAR files>
 *                                 -PRCfile <PRCFile>
 *                                 -outfile <JAR file>
 * INTERFACE:
 *   parameters:  argv
 *   returns:     true if arguments were passed properly
 *                otherwise, returns false after returning the usage() error.
 *=======================================================================*/

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
                } else if (arg.equals("-PRCfile")) {
                    PRCFile = argv[++i];
                } else if (arg.equals("-o") || arg.equals("-outfile")) { 
                    jarFile = argv[++i];
                } else if (arg.equals("-usagename")) { 
                    usageName = argv[++i];
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
            System.err.println("No 'main()' class name given");
            usage();
            return false;
        }
        if (verbose > 2) { 
            System.err.println("I'm don't know how to be >>that<< verbose!");
        }

        mainClass = argv[i++];

        /* Now create the default values for those things whose values
         * weren't given
         */
        String mainClassSansPackage = 
            mainClass.substring(mainClass.lastIndexOf('.') + 1);

        if (jarFile == null) { 
            jarFile = mainClassSansPackage + ".jar";
        }
            
        return true;
    }

    
/*=========================================================================
 * FUNCTION:      usage()
 * TYPE:          private class file 
 * OVERVIEW:      tells users how to use this function. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     nothing 
 *=======================================================================*/

    private void usage() { 
        System.err.print(usageName);
        System.err.print(
" [<options> . . .] class [class ....]      \n\n" +
"where options include\n" + 
"-v                 Verbose output (-v -v gives even more information)\n" + 
"-verbose           Same as -v\n" +
"-classpath <directories or jar files separated by colons> \n" +
"                   Where to search for application classes\n" +
"-PRCfile <PRCFile> Application PRC file that is to be converted " +
"                   to a jar file\n" +
"-outfile <JARfile> Name of JAR file to create on local disk;\n" +
"                   This is the JAR file that consists of all classes," +
"                   bitmaps, etc.\n" +
"-o <outfile>       Same as -outfile\n" + 
"-help              Print this message\n" +
"-usagename         Program name to appear in this help message\n" +
""
        );
    }
}

