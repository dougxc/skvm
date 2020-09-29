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

/*=========================================================================
 * Palm Tool PRC File Generator 
 *=========================================================================
 * FILE:      MakePalmApp.java
 * OVERVIEW:  This program creates a .prc file from an already existing .prc
 *            file and a set of Java class files.
 *            This program adds additional resources of type 'Main' and 'Clas'
 *            to the .prc file. In addition, it can change the already exist-
 *            ing 'tAIB' and 'tAIN' resource. 
 *            When "-JARtoPRC" is specified, it reads a set of Java classes
 *            from the JAR file specified and generates a .prc file. 
 *            The genResourcesFromJAR() method reads the classes from
 *            the JAR file and converts it to a class resource, which is added
 *            as a list of resources.
 *            This class uses the following classes from tools/palm/database:
 *             - Resource for generating class resources
 *             - ClassPath for locating resources from the original .PRC file 
 *             - DatabaseGenerator for getting attributes from the PRC.
 *
 * AUTHOR(s): Frank Yellin, JavaSoft
 *            Initial implementation
 *      
 *            Tasneem Sayeed, JavaSoft 
 *            Enhanced for adding -JARtoPRC for converting JAR to PRC file.
 *=======================================================================*/


public class MakePalmApp extends DatabaseGenerator { 

/*=========================================================================
 * FUNCTION:      main()
 * TYPE:          main method
 * OVERVIEW:      invokes the MakePalmApp() constructor to generate the .PRC
 *                file.
 * INTERFACE:
 *   parameters:  argv (arguments passed)
 *   returns:     nothing
 *=======================================================================*/

    public static void main(String argv[]) throws IOException { 
        new MakePalmApp(argv);
    }

/*=========================================================================
 * Constants and Variables
 *=======================================================================*/

/* 
 * These are values set by parseArguments().  
 * We give some of them their default value, here. 
 */

    /* Name to print in help message */
    String usageName = "java palm.database.MakePalmApp";

    int   verbose = 0;                  /* verbosity */
    String userClassPathString;         /* where to find application classes */
    String userBootClassPathString; 
    String dbCreator;                   /* creator ID for database */
    String dbName;                      /* name for database */
    String shortName;                   /* name for 'tAIN' resource */
    String outputFile;                  /* name of output file */
    String mainClass;                   /* class containing 'main()' method */
    String additionalClasses[];         /* other classes, such as from forName*/
    String iconName;                    /* file containing bitmap */
    String smallIconName;               /* file containing smaller bitmap */
    String versionString;               /* version info */
    String jarFile;                     /* JAR file */
    Vector resourceList;

    boolean emptyApp = false;
    boolean networking = false;         /* do we want networking? */
    boolean JARtoPRC = false;           /* did we want to convert from JAR 
                                         * to PRC?
                                         */
    ClassPath classpath;

    ClassPath systemClassPath = 
            new ClassPath(System.getProperty("java.class.path"));

    /* Original PRC Wrapper file */
    public static final String wrapperName = 
          "palm" + File.separator + "database" + File.separator + "Wrapper.prc";

    /* Tiny Icon */
    public static final String tinyIconName = 
          "palm" + File.separator + "database" + File.separator + "DefaultTiny.bmp";

/*=========================================================================
 * FUNCTION:      MakePalmApp()
 * TYPE:          class constructor
 * OVERVIEW:      generates the .PRC file.
 *                Invokes addClassesFromResources() to add all class
 *                resources from a set of Java classes.
 *                When MakePalmApp is invoked with "-JARtoPRC" option, it
 *                calls genClassesFromJAR() to add all class resources
 *                from the classes read within the JAR file specified.
 * INTERFACE:
 *   parameters:  argv
 *   returns:     nothing
 *=======================================================================*/

    MakePalmApp (String argv[]) throws IOException { 
        if (!parseArguments(argv)) 
        return;
        
        /* Find all the resources from the original .PRC file */
        ClassPath.ClassFile wrapper = systemClassPath.getFile(wrapperName);
        if (wrapper == null) { 
            System.out.println("Cannot find wrapper code \"" + 
                               wrapperName + "\" in classpath");
            return;
        }

        /* Add resources from the original .PRC wrapper file */
        addResourcesFromPRC(wrapper);

        if (JARtoPRC) {
            /* Retrieve the classes from the JAR file */
            String keys[][] = readZipNames(jarFile);
            String[] classKeys = keys[0];
            String[] resourceKeys = keys[1];
            PalmUtil.sort(classKeys);
            classpath = new ClassPath(jarFile);
             /* Add all the resources from the class(es) within the JAR file. */
           genResourcesFromJAR(classKeys, resourceKeys, classpath);
        } else {
            /* add all the resources from the indicated class(es) */
            if (!emptyApp) addResourcesFromClasses();
        }

        if (resourceList != null) { 
            BitSet used = new BitSet();
            ClassPath userClassPath = new ClassPath(userClassPathString);
            for (Enumeration e = resourceList.elements(); e.hasMoreElements();) {
                String resourceName = (String)e.nextElement();
                Resource r = 
                    new Resource.JavaResource(resourceName, userClassPath, used);
                addOneResource(r);
                if (verbose > 0) {
                    r.showPretty(System.out, resourceName, verbose > 1);
                }
            }
        }

        getIcon(iconName, null, 1000, 32, 32);
        getIcon(smallIconName, tinyIconName, 1001, 15, 9);

        /* add a resource for the version string */
        if (versionString != null) { 
            Resource r = new Resource.RawResource(versionString + "\0", 
                                                  "tver", 1000);
            addOneResource(r);
        }

        /* Change the 'tAIN' resource */
        addOneResource(new Resource.ApplicationNameResource(shortName));
        
        if (networking) { 
            /* All that matters, for now, is whether it is exists or not. */
            Resource r = new Resource.RawResource("Hi, Nik\0", "Flag", 1);
            addOneResource(r);
            if (verbose > 0) {
                r.showPretty(System.out, "Networking", verbose > 1);
            }
        }

        /* Dump the database out */
        if (verbose > 0) { 
            System.out.println("Generating output file \"" + outputFile + "\".");
        }
        
        writeDatabase(outputFile, dbName, dbCreator, "appl", getResourceList());
    }


/*=========================================================================
 * FUNCTION:      addResourcesFromPRC()
 * TYPE:          private class file
 * OVERVIEW:      retrieves all the class resources from the .PRC file
 *                and adds each of the classes to the newly created JAR file.
 * INTERFACE:
 *   parameters:  PRC file
 *   returns:     nothing
 *=======================================================================*/

    private void
    addResourcesFromPRC(ClassPath.ClassFile prc) throws IOException { 
        /* Read the file into a buffer, and then we deal with it there. */ 
        byte[] bytes = new byte[(int)prc.length()];
        DataInputStream is = new DataInputStream(prc.getInputStream());
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
            ID[i] = is.readUnsignedShort();
            offset[i] = is.readInt();
        }
        offset[recordCount] = bytes.length; // this simplifies things. . .
        /* Add each of the resources to the database. */
        for (int i = 0; i < recordCount; i++) { 
            Resource r = new Resource.RawResource(bytes, type[i], ID[i],  
                                                  offset[i], offset[i+1]);
            addOneResource(r);
        }
    }


/*=========================================================================
 * FUNCTION:      readZipNames()
 * TYPE:          private class file
 * OVERVIEW:      returns a list of all the .class files from the specified
 *                JAR file. 
 * INTERFACE:
 *   parameters:  JAR file
 *   returns:     string array of classes 
 *=======================================================================*/

    private String[][]
    readZipNames(String jarFileName) throws IOException {
        Vector classes = new Vector();
        Vector resources = new Vector();
        ZipInputStream zip =
            new ZipInputStream(new FileInputStream(jarFileName));
        ZipEntry ent;
        while ((ent = zip.getNextEntry()) != null) {
            if (ent.isDirectory()) {
                // Skip directories
                continue;
            }
            String entName = ent.getName();
            if (entName.endsWith(".class")) {
                /* Get class names, and remove the final .class */
                String className = entName.substring(0, entName.length() - 6);
                classes.addElement(className.replace('/', '.'));
            } else { 
                resources.addElement(entName);
            }
        }
        /* Convert it into an array */
        int classCount = classes.size();
        int resourceCount = resources.size();
        String[] classKeys = new String[classCount];
        String[] resourceKeys = new String[resourceCount];
        classes.copyInto(classKeys);
        resources.copyInto(resourceKeys);
        return new String[][]{ classKeys, resourceKeys };
    }


/*=========================================================================
 * FUNCTION:      addResourcesFromClasses()
 * TYPE:          private class file
 * OVERVIEW:      converts the class containing the main(String[] argv), and
 *                all classes reachable from it in resources to be added to
 *                this wrapper.
 * INTERFACE:
 *   parameters:  none 
 *   returns:     nothing
 *=======================================================================*/

    private void 
    addResourcesFromClasses() throws IOException { 
        BitSet used = new BitSet();
        /* Find all classes reachable from the "top" class */
        ClassPath userClassPath = new ClassPath(userClassPathString);
        ClassPath bootClassPath = null;
        if (userBootClassPathString != null) { 
            bootClassPath = new ClassPath(userBootClassPathString);
        }
        ClassInfo classes[] = 
            ClassInfo.classClosure(mainClass, additionalClasses, 
                                   userClassPath, bootClassPath);
        /* For each of these, create a 'Clas' resource. */
        if (classes == null) { 
            System.exit(1);
        }
        for (int i = 0; i < classes.length; i++) { 
            ClassInfo ci = classes[i];
            Resource r =  new Resource.ClassResource(ci, used);
            addOneResource(r);
            if (verbose > 0) {
                r.showPretty(System.out, ci.getClassName(), verbose > 1);
            }
        }
        /* Add a 'Main' resource giving the name of the main class. */
        String mainResourceName = mainClass.replace('.','/');
        Resource r = new Resource.RawResource(mainResourceName, "Main", 1);
        addOneResource(r);
        if (verbose > 0) {
            r.showPretty(System.out, '"' + mainResourceName + '"', verbose > 1);
        }
    }


/*=========================================================================
 * FUNCTION:      genResourcesFromJAR()
 * TYPE:          private class file
 * OVERVIEW:      converts the class containing the main(String[] argv), and
 *                all classes read from the JAR file and restored into the
 *                classes array into resources to be added to this wrapper.
 * INTERFACE:
 *   parameters:  classes array containing the classes from the JAR file
 *                classpath for locating the "top" class           
 *   returns:     nothing
 *=======================================================================*/

    private void 
    genResourcesFromJAR(String classes[], String resources[], 
                        ClassPath classpath) throws IOException { 

        BitSet used = new BitSet();
        /* Find all classes reachable from the "top" class */
        ClassPath userClassPath = new ClassPath(userClassPathString);
        ClassPath bootClassPath = null;
        if (userBootClassPathString != null) { 
            bootClassPath = new ClassPath(userBootClassPathString);
        }

        /* For each of the classes read from the JAR file, 
         * create a 'Clas' resource. 
         */

        if (classes == null) { 
            System.exit(1);
        }

        /* Make them into resources */
        int size = classes.length;
        for (int i=0; i < classes.length; i++) {
           String className = classes[i];
           ClassInfo ci = new ClassInfo(className, classpath);
           Resource r = new Resource.ClassResource(ci, used);
           addOneResource(r);
           if (verbose > 0) {
               r.showPretty(System.out, ci.getClassName(), verbose > 1);
           }
        }

        used = new BitSet();
        for (int i = 0; i < resources.length; i++) { 
            String resourceName = resources[i];
            Resource r = 
                new Resource.JavaResource(resourceName, classpath, used);
            addOneResource(r);
            if (verbose > 0) {
                r.showPretty(System.out, resourceName, verbose > 1);
            }
        }


        /* Add a 'Main' resource giving the name of the main class. */
        String mainResourceName = mainClass.replace('.','/');
        Resource r = new Resource.RawResource(mainResourceName, "Main", 1);
        addOneResource(r);
        if (verbose > 0) {
            r.showPretty(System.out, '"' + mainResourceName + '"', verbose > 1);
        }
    }


/*=========================================================================
 * FUNCTION:      getIcon()
 * TYPE:          private class file
 * OVERVIEW:      Retrieves the bitmap corresponding to the icon name 
 *                specified. 
 * INTERFACE:
 *   parameters:  filename, default iconName, int ID, int width, int height 
 *   returns:     nothing
 *=======================================================================*/

    private void
    getIcon (String fileName, String defaultIconName, 
             int ID, int width, int height) { 
        Bitmap b = null;
        if (fileName != null) {
            try { 
                b = Bitmap.read(fileName);
            } catch (IOException e) { 
                System.out.println(e);
            } 
        }
        if (b == null && defaultIconName != null) { 
            /* We would have liked to have the small icon be in Wrapper.prc
             * But it appears that there is a bug in the Resource
             * constructor, and it makes the icon be 32x32 instead of 9x15.
             * So we have to attach the default small icon by hand.
             */
            try { 
                b = Bitmap.read(defaultIconName, systemClassPath);
            } catch (IOException e) { 
                System.out.println(e);
            }
        }
        if (b == null) { 
            return;
        }
        if (width != b.width || height != b.height) { 
            if (verbose > 0) { 
                System.out.println("Resizing bitmap from " +
                                   b.width + "x" + b.height + " to " +
                                   width + "x" + height + ".");
            }
            int deltaX = (width - (b.width))/2;
            b = b.resize(width, height);
            if (deltaX != 0) { 
                b = b.offset(deltaX, 0);
            }
        }
        Resource iconResource = new Resource.IconResource(b, ID);
        addOneResource(iconResource);
        if (verbose > 0) { 
            String name = (ID == 1000) ? "Large icon" : "Small icon";
            iconResource.showPretty(System.out, name, verbose > 1);
        }
    }


/*=========================================================================
 * FUNCTION:      parseArguments()
 * TYPE:          private class file
 * OVERVIEW:      parses the arguments to the program 
 * INTERFACE:
 *   parameters:  argv 
 *   returns:     nothing
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
                } else if (arg.equals("-empty")) {
                    emptyApp = true;
                } else if (arg.equals("-classpath")) { 
                    if (userClassPathString == null) { 
                        userClassPathString = argv[++i];
                    } else { 
                        userClassPathString += ":" + argv[++i]; 
                    }
                } else if (arg.equals("-bootclasspath")) { 
                    if (userBootClassPathString == null) { 
                        userBootClassPathString = argv[++i];
                    } else { 
                        userBootClassPathString += ":" + argv[++i]; 
                    }
                } else if (arg.equals("-JARtoPRC")) {
                    jarFile = argv[++i];
                    JARtoPRC = true;
                } else if (arg.equals("-icon")) { 
                    iconName = argv[++i];
                } else if (arg.equals("-smallicon")) { 
                    smallIconName = argv[++i];
                } else if (arg.equals("-longname")) { 
                    dbName = argv[++i];
                } else if (arg.equals("-o") || arg.equals("-outfile")) { 
                    outputFile = argv[++i];
                } else if (arg.equals("-name")) { 
                    shortName = argv[++i];
                } else if (arg.equals("-creator")) { 
                    dbCreator = (argv[++i] + "    ").substring(0, 4);
                } else if (arg.equals("-usagename")) { 
                    usageName = argv[++i];
                } else if (arg.equals("-version")) { 
                    versionString = argv[++i];
                } else if (arg.equals("-networking")) { 
                    networking = true;
                } else if (arg.equals("-nonetworking")) { 
                    networking = false;
                } else if (arg.equals("-help")) { 
                    usage();
                    return false;
                } else if (arg.equals("-skiplist")) { 
                    System.out.println("-skiplist no longer supported");
                    i++;
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
        if (userBootClassPathString == null && !(JARtoPRC || emptyApp)) { 
            System.out.println("**********");
            System.out.println("You have not given a -bootclasspath argument.");
            System.out.println("This is almost certainly a mistake.");
            System.out.println("**********");
        }
        if (userClassPathString == null) { 
            userClassPathString = ".";
        }
        if (!emptyApp && i == argv.length) { 
            System.err.println("No class name given");
            usage();
            return false;
        }
        if (verbose > 2) { 
            System.err.println("I'm don't know how to be >>that<< verbose!");
        }

        if (!emptyApp) {
            mainClass = argv[i++];
            if (i < argv.length) { 
                additionalClasses = new String[argv.length - i];
                System.arraycopy(argv, i, additionalClasses, 0, argv.length - i);
                for (int j = 0; j < additionalClasses.length; j++) { 
                    String arg = additionalClasses[j];
                    if (arg.startsWith("-")) {
                        System.err.println("option \"" + arg 
                                           + "\" must appear before class names");
                        return false;
                    }
                }
            }
        } else {
            mainClass = "Empty";
        }

        /* Now create the default values for those things whose values
         * weren't given.
         */
        String mainClassSansPackage;
        
        if (!emptyApp) {
            mainClassSansPackage = 
                mainClass.substring(mainClass.lastIndexOf('.') + 1);
        } else {
            mainClassSansPackage = mainClass;
        }
        
        if (dbName == null) { 
            /* This maybe needs to be fixed, if the name is longer than
             * 31 characters
             */
            dbName = mainClass;
        }

        if (shortName == null) { 
            shortName = mainClassSansPackage;
        }

        if (outputFile == null) { 
            outputFile = mainClassSansPackage + ".prc";
        }
            
        if (dbCreator == null) { 
            /* this is completely bogus.  We need to create something 
             * vaguely creating a unique creator ID.  We hash the mainClass
             * name, and then create a 4-letter name whose first char is 'J'
             * and the remainder are random.
             */
            int CRC = PalmUtil.longCRC(mainClass) >>> 1;
            /* We use the entire space of Palm characters from 32-255 */
            char buffer[] = new char[4];
            buffer[0] = 'J';
            buffer[1] = (char)(32 + (CRC % 224));  CRC /= 224; 
            buffer[2] = (char)(32 + (CRC % 224));  CRC /= 224; 
            buffer[3] = (char)(32 + (CRC % 224));  CRC /= 224; 
            if (((buffer[1] | buffer[2] | buffer[3]) & 0x80) == 0) { 
                /* We want at least one character to be outside 
                 * "normal space" 
                 */
                buffer[1 +(mainClass.length() % 3)] = 
                    (char)((0x80 | CRC) & 0xFF);
            }
            dbCreator = new String(buffer);
            if (verbose > 1) { 
                System.out.println("Creator id = " + dbCreator);
            }
        }
        return true;
    }

    /* all resources mappings from resource name/type to the actual resource
     * seenRources is a list of name/type Strings in the order we first saw
     * them.  
     */
    private Hashtable allResources = new Hashtable();
    private Vector    seenResources = new Vector();


/*=========================================================================
 * FUNCTION:      addOneResource()
 * TYPE:          private class file
 * OVERVIEW:      Add another resource to the list of things going into the
 *                output file. If this is a resource we've already seen, then
 *                just overwrite the old one.   
 *              
 * INTERFACE:
 *   parameters:  Resource 
 *   returns:     nothing
 *=======================================================================*/

    private void addOneResource(Resource r) { 
        Object old = allResources.put(r.getName(), r);
        if (old == null) { 
            /* This is the first time we've had a resource with this name */
            seenResources.addElement(r.getName());
        } 
        if (allResources.size() != seenResources.size()) { 
            throw new RuntimeException("addOneResource confusion");
        }
    }
    

/*=========================================================================
 * FUNCTION:      getResourceList()
 * TYPE:          private class file
 * OVERVIEW:      retrieves a list of resources 
 *              
 * INTERFACE:
 *   parameters:  none 
 *   returns:     array of resources 
 *=======================================================================*/

    private Resource[] 
    getResourceList() { 
        int size = seenResources.size();
        Resource[] resourceList = new Resource[size];
        for (int recordNo = 0; recordNo < size; recordNo++) { 
            String name = (String)seenResources.elementAt(recordNo);
            resourceList[recordNo] = (Resource)allResources.get(name);
        }
        return resourceList;
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
        System.err.println("     " + usageName + " [<options> . . .] class [class ....]");
        System.err.println(" or  ");
        System.err.println("     " + usageName + " -JARtoPRC <JAR/Zip file> <MainClassName>");
        System.err.println(" or  ");
        System.err.println("     " + usageName + " -empty [<options>. . .]");
        System.err.println();
        System.err.print(
"where options include\n" + 
"-v                 Verbose output (-v -v gives even more information)\n" + 
"-verbose           Same as -v\n" +
"-networking        Application may perform networking\n" +
"-nonetworking      Application does not perform networking (default)\n" +
"-classpath <directories or jar files separated by colons> \n" +
"                   Where to search for application classes\n" +
"-bootclasspath <directories or jar files separated by colons> \n" +
"                   Where to search for system classes\n" +
"-icon <file>       File containing icon for application.\n"+
"                   Must be in bmp, pbm, or bin (Palm Resource) format.\n"+
"-listicon <file>   File containing the \"list\" icon for application\n" +
"-smallicon <file>  Same as -smallicon\n" +      
"-name <name>       Short name of application, seen in the launcher\n" +
"-longname <name>   Long name for the application, seen in beaming, etc\n" +
"-creator <crid>    Creator ID for the application\n" +
"-outfile <outfile> Name of file to create on local disk;\n" +
"                   This is the file that is downloaded to the Palm\n" +
"-resource <file>   Include the specified file as a Java resource\n" + 
"                   This option can be repeated multiple times\n" + 
"-o <outfile>       Same as -outfile\n" + 
"-version <string>  Change version\n" +
"-help              Print this message\n" +
"-usagename         Program name to appear in this help message\n\n" +
"-JARtoPRC <JAR File> <MainClass>\n" +
"                   Converts JAR file containing a list of classes and\n"+
"                   resources to a PRC file\n" +
"-empty             create a resource file with no classes\n" +
""
            );
    }
}

