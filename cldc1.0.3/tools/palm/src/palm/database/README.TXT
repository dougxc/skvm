PALM TOOLS FOR KVM

This directory contains a number of tools 
and conversion utilities that were originally 
written for the Palm version of the K Virtual 
Machine:

- Bitmap         Converts Windows bitmaps (.bmp files)
                 into icons that can be shown in the Palm
                 application launcher.

- ConvPRCtoJAR   Converts a Palm executable (PRC) file
                 into a JAR file, extracting all the Java
                 class data from the executable file.
                 This is the opposite of MakePalmApp.

- MakePalmApp    Takes a set of Java classfiles and converts
                 them into a Palm executable (PRC) file that
                 can be run in the Palm version of the KVM.

- MakePalmDB     This is similar to MakePalmApp, except that
                 it generates a Palm database (PDB) file rather
                 than a Palm executable (PRC).  This tool is 
                 very rarely used now that all the system 
                 classes are prelinked ("romized") into the 
                 KVM executable using the JavaCodeCompact tool.

- RawPRC         Creates a Palm resource database from a set
                 of "bin" files.

- ZipPDB         This is similar to MakePalmDB, except that
                 input format can be a zip/jar file containing
                 Java classfiles.
      
The rest of the files in this directory are used internally
by the tools above.

