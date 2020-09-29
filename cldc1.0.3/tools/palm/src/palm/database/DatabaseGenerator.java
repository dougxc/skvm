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

import java.util.*;
import java.io.*;

public class DatabaseGenerator { 

    static final int dmHdrAttrResDB    = 0x0001;
    static final int dmHdrAttrReadOnly = 0x0002;    
    static final int dmHdrAttrAppInfoDirty = 0x0004;  
    static final int dmHdrAttrBackup = 0x0008;
    static final int dmHdrAttrOKToInstallNewer = 0x0010;
    static final int dmHdrAttrResetAfterInstall = 0x0020;
    static final int dmHdrAttrCopyPrevention = 0x0040;
    static final int dmHdrAttrStream = 0x0080;
    // These next two aren't official.  I don't know their real names
    static final int dmHdrAttrHideMe = 0x0100;
    static final int dmHdrAttrShowMe = 0x0200;
    static final int dmHdrAttrOpen = 0x8000;    // Database not closed properly

    // It's not really documented what the maximum size is.  But this
    // should give a sufficient warning.
    static final int MAXIMUM_RECORD_SIZE = 65500;

    private static long epoch;  // January 1, 1904, 00:00 GMT

    static { 
        GregorianCalendar c = 
            new GregorianCalendar(1904, Calendar.JANUARY,1,0,0,0);
        c.setTimeZone(TimeZone.getTimeZone("GMT"));
        epoch = c.getTime().getTime();
    }

    // Return the seconds since January 1, 1904.
    public static int getPalmOSDate(Date d) {
        return (int)((d.getTime() - epoch)/1000);
    }

    private int now = getPalmOSDate(new Date());

    // Basic values used by most databases.  Some databases may want
    // to override these.
    public   int  getVersion()              { return 1; } 
    public   int  getCreationDate()         { return now; }
    public   int  getModificationDate()     { return now; }
    public   int  getLastBackupDate()       { return now; }
    public   int  getModificationNumber()   { return 1; }
    public   int  getDBAttributes()         { return 0; }
    public   void dumpAppInfo(DataOutput o) { }
    public   void dumpSortInfo(DataOutput o){ }

    public long
    writeDatabase(String fileName, String dbName, 
                  String dbCreator, String dbType, 
                  DatabaseRecord[] records) 
             throws IOException {
        // Open the new database file.
        File file = new File(fileName);
        if (file.exists()) 
            file.delete();
        RandomAccessFile os = new RandomAccessFile(file, "rw");

        // How many records are there?
        int recordCount = records.length;
        // Is this a resource database, or normal database?
        boolean isResource = 
            (recordCount > 0 && records[0] instanceof Resource);

        // keep track of the offset of each record.
        long recordOffset[] = new long[recordCount + 1];
        
        // Let's skip over the header.  We'll come back later. 
        os.seek(80 + (recordCount * (isResource ? 10 : 8)));

        // Dump the application information; remember where we put it
        long appInfoOffset = os.getFilePointer();
        dumpAppInfo(os);
        if ((os.getFilePointer() - appInfoOffset) > MAXIMUM_RECORD_SIZE) { 
            warnTooBig("Application Info");
        }

        // Dump the sort information; remember where we put it
        long sortInfoOffset = os.getFilePointer();
        dumpSortInfo(os);
        if ((os.getFilePointer() - sortInfoOffset) > MAXIMUM_RECORD_SIZE) { 
            warnTooBig("Sort Info");
        }

        // Dump each of the records; remember where we put it
        for (int i = 0; i < recordCount; i++) {
            recordOffset[i] = os.getFilePointer();
            records[i].writeBytes(os);
            if ((os.getFilePointer() - recordOffset[i]) > MAXIMUM_RECORD_SIZE) {
                if (isResource) { 
                    Resource r = (Resource)records[i];
                    warnTooBig("Resource \"" + r.getType() + " " + r.getID()
                               + "\"");
                } else { 
                    warnTooBig("Database entry #" + i);
                }
            }
        }
        // Now, back and write the header.
        os.seek(0);             

        // The dbName is supposed to be 32bytes, but its really 31 bytes and
        // and must have a null at the end.
        writeString(os, dbName, 31, '\0'); os.writeByte(0);
        os.writeShort(getDBAttributes() | (isResource ? dmHdrAttrResDB : 0));
        os.writeShort(getVersion());                                 
        os.writeInt(getCreationDate()); 
        os.writeInt(getModificationDate()); 
        os.writeInt(getLastBackupDate()); 
        os.writeInt(getModificationNumber()); 
        os.writeInt(appInfoOffset == sortInfoOffset ? 0 : (int)appInfoOffset);
        os.writeInt(sortInfoOffset == recordOffset[0] ? 0 : (int)sortInfoOffset);
        writeString(os, dbType, 4, ' ');
        writeString(os, dbCreator, 4, ' ');
        os.writeInt(0);         // unique ID seed ???
        os.writeInt(0);         // unused
        os.writeShort(recordCount); 
        checkFileOffset(os, 78);
        if (isResource) { 
            for (int i = 0; i < recordCount; i++) {
                Resource r = (Resource)records[i];
                writeString(os, r.getType(), 4, ' ');
                os.writeShort(r.getID());
                os.writeInt((int)recordOffset[i]);
            }
        } else { 
            for (int i = 0; i < recordCount; i++) {
                DatabaseRecord de = records[i];
                int uniqueID = de.getUniqueID();
                if (uniqueID < 0) { 
                    // flag indicating that we don't really care
                    uniqueID = i + 1;
                }
                os.writeInt((int)recordOffset[i]);
                os.writeByte(de.getFlags() | de.getCategory());
                os.writeByte((byte)(uniqueID >> 16));
                os.writeShort((short)(uniqueID));
            }
        }
        os.writeShort(0);
        checkFileOffset(os, appInfoOffset);

        // Close everything
        os.close();

        // return the file length
        return recordOffset[recordCount];
    }

    // Write a string using exactly the specified number of bytes.
    // Pad with the padding character, if necessary.
    protected static void 
    writeString(DataOutput os, String s, int bytes, char padding) 
               throws IOException { 
        int length = s.length();
        if (length <= bytes) { 
            os.writeBytes(s);
            for (int i = length; i < bytes; i++) 
                os.write((int)padding);
        } else { 
            os.writeBytes(s.substring(0, bytes));
        }
    }

    private static void 
    checkFileOffset(RandomAccessFile os, long expectedOffset) 
        throws IOException {

        long currentOffset = os.getFilePointer();
        if (currentOffset != expectedOffset) { 
            System.err.println("Bad file offset.  " + 
                               "Expected: " + expectedOffset + "; " + 
                               "Actual: " + currentOffset);
            throw new IOException("Bad offset");
        }
    }

    private static void 
    warnTooBig(String name) { 
        System.err.println("WARNING: " + name + " may be too large.  Hotsync may fail.");
    }
}

