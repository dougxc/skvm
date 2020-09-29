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


/**
 * This class is used to represent a class path, which can contain both
 * directories and zip files.
 */
public
class ClassPath {

    // Array of entries.  Each entry in this list represents either 
    // a zip file or a directory. 

    private ClassPathEntry[] path;
    private String pathString;


    /**
     * Build a class path from the specified path string
     */
    public ClassPath(String pathString) {
        this.pathString = pathString;
        Vector pathVector = new Vector();
        int length = pathString.length();

        // Separate the pathString into tokens, separated by the path separator

        int i, j;
        for (i =  0; i < length; i = j + 1) {
            j = pathString.indexOf(File.pathSeparator, i);
            String pathEntry;
            if (j == -1) { 
                j = length;
            }
            if (i == j) {
                File here = new File(".");
                pathVector.addElement(new DirectoryClassPathEntry(here));
            } else { 
                String fileName = pathString.substring(i, j);
                File file = new File(fileName);
                if (file.isFile()) {
                    try {
                        ZipFile zf = new ZipFile(file);
                        pathVector.addElement(new ZipClassPathEntry(zf));
                    } catch (ZipException e) {
                        // ignore 
                    } catch (IOException e) {
                        // Ignore exceptions, at least for now...
                    }
                } else {
                    pathVector.addElement(new DirectoryClassPathEntry(file));
                }
            }
        }
        // Make an array containing exactly the items in the vector.
        path = new ClassPathEntry[pathVector.size()];
        pathVector.copyInto(path);
    }

    // Return a ClassFile corresponding to the specified file.
    public ClassFile getFile(String name) {
        for (int i = 0; i < path.length; i++) {
            ClassFile cf = path[i].getFile(name);
            if (cf != null) {
                return cf;
            }
        }
        return null;
    }

    public void close() throws IOException {
        for (int i = path.length; --i >= 0; ) {
            path[i].close();
        }
    }

    // These are the two operations we need to do on a file found somewhere
    // in a classpath
    abstract static public class ClassFile {
        abstract public long length();
        abstract public InputStream getInputStream() throws IOException;
    }
    

    // Either a zip file or a directory.
    abstract static class ClassPathEntry {
        abstract ClassFile getFile(String name);
        public void close() throws IOException {}
    }

    static class ZipClassPathEntry extends ClassPathEntry { 
        final ZipFile zipFile;

        ZipClassPathEntry (ZipFile zipFile) { 
            this.zipFile = zipFile; 
        }

        ClassFile getFile(String name) { 
            // Put the name into zip form
            String zipname = name.replace(File.separatorChar, '/');
            final ZipEntry zipEntry = zipFile.getEntry(zipname);
            if (zipEntry != null) { 
                // We have the entry.  Create a subclass of ClassFile.
                return new ClassFile () { 
                   public long length() { return zipEntry.getSize(); }
                   public InputStream getInputStream() throws IOException {
                       try {
                           return zipFile.getInputStream(zipEntry);
                       } catch (ZipException e) {
                           throw new IOException(e.getMessage());
                       }
                   }
                };
             }
             return null;
        }

        public void close() throws IOException { 
            zipFile.close();
        }
    }

    static class DirectoryClassPathEntry extends ClassPathEntry { 
        final File dir;

        DirectoryClassPathEntry (File dir) { this.dir = dir; }
        ClassFile getFile(String name) { 
            File file = null;
            int lastSlash = name.lastIndexOf(File.separatorChar);
            String subdir = name.substring(0, lastSlash + 1);
            String basename = name.substring(lastSlash + 1);
            String list[] = getFiles(subdir);
            for (int j = 0; j < list.length; j++) {
                if (basename.equals(list[j])) {
                    file = new File(dir.getPath(), name);
                    break;
                }
            }
            if (file == null) {
                return null;
            } else { 
                final File xfile = file;
                return new ClassFile() { 
                    public long length() { return xfile.length(); }
                    public InputStream getInputStream() throws IOException {
                        return new FileInputStream(xfile);
                    }
                };
            }
        }

        private Hashtable subdirs = new Hashtable(29); // cache of dir listings
        private String[] getFiles(String subdir) { 
            String files[] = (String[]) subdirs.get(subdir);
            if (files == null) {
                // search the directory, exactly once
                File sd = new File(dir.getPath(), subdir);
                if (sd.isDirectory()) {
                    files = sd.list();
                    if (files == null) {
                        // should not happen, but just in case, fail silently
                        files = new String[0];
                    }
                } else {
                    files = new String[0];
                }
                subdirs.put(subdir, files);
            }
            return files;
        }
    }

    public String toString() { 
        return pathString;
    }

}

