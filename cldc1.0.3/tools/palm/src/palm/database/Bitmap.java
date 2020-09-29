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
 * Bitmap
 *
 * This class converts Windows bitmaps (.bmp) files into
 * icons that can be shown in the Palm application launcher.
 */

package palm.database;

import java.io.*;

public class Bitmap { 
    int width;
    int height;
    int rowSize;
    int flags;
    byte bytes[];
    byte compressedBytes[];

    static final int BITMAP_COMPRESSED_FLAG = 0x8000;

    // For testing
    public static void main(String argv[]) throws IOException { 
        int i = 0;
        Bitmap b;
        String lastCommand = null;
        if (argv.length == 0) { 
            usage();
            return;
        }
        try { 
             b = read(argv[i++]);
        } catch (IOException e) { 
            System.err.println("Cannot read bitmap file " + argv[0]);
            e.printStackTrace();
            return;
        }
        if (argv.length == 1) { 
            // default command, if the user doesn't give any other commands
            b.show();
            return;
        }
        try { 
            while (i < argv.length) { 
                lastCommand = argv[i++];
                if (lastCommand.equals("-help")) { 
                    usage();
                    return;
                } else if (lastCommand.equals("-dump")) {
                    String varName = argv[i++];
                    String fileName = argv[i++];
                    b.dumpJava(argv[0], varName, fileName);
                } else if (lastCommand.equals("-Dump")) {
                    String varName = argv[i++];
                    String fileName = argv[i++];
                    b.dumpJavaNew(argv[0], varName, fileName);
                } else if (lastCommand.equals("-resize")) { 
                    int width = Integer.parseInt(argv[i++]);
                    int height = Integer.parseInt(argv[i++]);
                    b = b.resize(width, height);
                } else if (lastCommand.equals("-invert")) { 
                    b.invert();
                } else if (lastCommand.equals("-expand")) { 
                    int width = Integer.parseInt(argv[i++]);
                    int height = Integer.parseInt(argv[i++]);
                    int deltaX = (width - (b.width))/2;
                    b = b.resize(width, height);
                    if (deltaX != 0) {
                        b = b.offset(deltaX, 0);
                    }
                } else if (lastCommand.equals("-compress")) { 
                    b.compress(false);
                } else if (lastCommand.equals("-Compress")) { 
                    b.compress(true);
                } else if (lastCommand.equals("-uncompress")) { 
                    b.compressedBytes = null;
                    b.flags &= ~BITMAP_COMPRESSED_FLAG;
                } else if (lastCommand.equals("-write")) { 
                    String file = argv[i++];
                    System.out.println("Writing to " + file);
                    DataOutputStream dos = 
                        new DataOutputStream(new FileOutputStream(file));
                    b.writeBin(dos);
                    dos.flush(); dos.close();
                } else if (lastCommand.equals("-show")) { 
                    b.show();
                } else { 
                    System.err.println("Unknown command " + lastCommand);
                    usage();
                    return;
                }
            }
        } catch (ArrayIndexOutOfBoundsException e) { 
            System.out.println("Insufficient arguments to " + lastCommand);
            usage();
        }
    }

    Bitmap (int width, int height, int rowSize, byte bytes[], int flags) { 
        this.width = width;
        this.height = height;
        this.rowSize = rowSize;
        this.bytes = bytes;
        this.flags = flags;
    }

    Bitmap (int width, int height) { 
        this.width = width;
        this.height = height;
        this.rowSize = ((width + 15) & ~15)/8;
        this.bytes = new byte[this.rowSize * height];
    }



    public static Bitmap
    read(String fileName) throws IOException { 
        return read(fileName, new FileInputStream(fileName));
    }

    public static Bitmap
    read(String fileName, ClassPath classPath) throws IOException {
        return read(fileName, classPath.getFile(fileName).getInputStream());
    }

    private static Bitmap
    read(String fileName, InputStream data) throws IOException { 
        DataInputStream is = new DataInputStream(data);
        try { 
            if (fileName.endsWith(".bmp")) { 
                return readBMP(is);
            } else if (fileName.endsWith(".pbm")) { 
                return readPBM(is);
            } else if (fileName.endsWith(".bin")) { 
                return readBIN(is);
            } else { 
                throw new IOException("Unknown file extension");
            }
        } finally { 
            is.close();
        }
    }

    private static Bitmap 
    readBMP(DataInput is) throws IOException { 
        is.skipBytes(10);
        int srcOffset = x86Long(is); // offset 10-13
        int size = x86Long(is);      // offset 14-17
        int width = x86Long(is);     // offset 18-21
        int height = x86Long(is);    // offset 22-25
        is.skipBytes(2);             // offset 26-27, planes
        int bitCount = x86Word(is);  // offset 28-29
        int compression = x86Long(is); // offset 30-33

        if (bitCount != 1) {
            throw new IOException("Cannot read " + bitCount + "-bit bitmaps");
        }
        if (compression != 0) { 
            throw new IOException("Can't handle compressed bitmap");
        }
        is.skipBytes(20);       // offset 34-53
        byte colorTable[] = new byte[8];
        // The first 4 items are "0", the next four are "1"
        is.readFully(colorTable); // offset 54-61

        int srcRowSize = ((width + 31) & ~31) / 8;
        int dstRowSize = ((width + 15) & ~15) / 8;
        byte[] bytes = new byte[dstRowSize * height];
        
        if (srcOffset != 62) { 
            System.out.println("SrcOffset <> 62???");
            if (srcOffset > 62) { 
                is.skipBytes(srcOffset - 62);
            } else { 
                throw new IOException("Malformed bitmap file");
            }
        }
        for (int yy = 0; yy < height; yy++) { 
            int y = height - yy - 1;
            int dstRowStart = y * dstRowSize;
            is.readFully(bytes, dstRowStart, dstRowSize);
            is.skipBytes(srcRowSize - dstRowSize);
        }

        Bitmap b = new Bitmap(width, height, dstRowSize, bytes, 0);
        if ((colorTable[0] & 0xFF) < (colorTable[4] & 0xFF)) { 
            b.invert();
        } else { 
            // I'm not sure if this is necessary.  But it can't hurt.
            b.cleanupBits();
        }
        return b;
    }

    // Invert all the bits in the bitmap.
    private void invert() { 
        int length = bytes.length;
        for (int i = 0; i < length; i++) { 
            bytes[i] = (byte)~bytes[i];
        }
        cleanupBits();
        compressedBytes = null;
        flags &= ~BITMAP_COMPRESSED_FLAG;
    }

    // Some bits off on the right hand side may be "1", even though they
    // shouldn't bit.  Fix them up.
    private void cleanupBits() { 
        int width = this.width;
        int rowSize = this.rowSize;
        byte bytes[] = this.bytes;
        int length = bytes.length;
        
        int mask =  0xFFFF0000 >> (((width - 1) & 15) + 1);
        /**
         * System.out.println("width = " + width + " mask=" +
         *                 Integer.toHexString(mask & 0xFFFF));
         */
        int leftMask = (mask >> 8) & 0xFF;
        int rightMask = mask & 0xFF;
        for (int row = 0; row < bytes.length; row += rowSize) { 
            bytes[row + rowSize - 2] &= leftMask;
            bytes[row + rowSize - 1] &= rightMask;
        }
    }

    private static Bitmap 
    readPBM (DataInput is) throws IOException { 
        int start, end;

        // The first line is P1 or P4 followed by stuff we don't care about
        String header = is.readLine();
        if (header.charAt(0) != 'P') 
            throw new IOException("Not a PBM file");
        char type = header.charAt(1);
        if (type != '1' && type != '4') 
            throw new IOException("Not a monochrome file");
        
        // The next line contains two numbers, giving the width and height
        String sizeLine = is.readLine() + " ";
        for (start = 0; Character.isSpace(sizeLine.charAt(start)); start++);
        for (end = start; Character.isDigit(sizeLine.charAt(end)); end++);
        int width = Integer.parseInt(sizeLine.substring(start, end));
        for (start = end; Character.isSpace(sizeLine.charAt(start)); start++);
        for (end = start; Character.isDigit(sizeLine.charAt(end)); end++);
        int height = Integer.parseInt(sizeLine.substring(start, end));

        Bitmap result = new Bitmap(width, height);
        if (type=='1') {
            result.readAsciiPBM(is);
        } else { 
            result.readRawPBM(is);
        }
        return result;
    }

    static Bitmap 
    readBIN(DataInput is) throws IOException { 
        int width = is.readShort();
        int height = is.readShort();
        int rowSize = is.readShort();
        int flags = is.readShort();
        byte bytes[] = new byte[rowSize * height];
        Bitmap result = new Bitmap(width, height, rowSize, bytes, flags);
            
        is.skipBytes(8);                // unused
        if ((flags & BITMAP_COMPRESSED_FLAG) == 0) { 
            // Normal bitmap
            is.readFully(bytes);
        } else { 
            // Compressed bitmap
            int byte0 = is.readUnsignedByte();
            int byte1 = is.readUnsignedByte();
            int size =  (byte0 << 8) + byte1; // big endian
            byte compressedBytes[] = new byte[size];
            compressedBytes[0] = (byte)byte0;
            compressedBytes[1] = (byte)byte1;
            is.readFully(compressedBytes, 2, size - 2);
            int dupFlag = 0;
            int arrayOffset = 2;
            // Look at each row
            for (int row = 0; row < bytes.length; row += rowSize) { 
                int prevRow = row - rowSize;
                // Look at each byte in that row
                for (int offset = 0; offset < rowSize; offset++) {
                    // We need a new dupFlag every 8 bytes.
                    if ((offset & 7) == 0) { 
                        dupFlag = compressedBytes[arrayOffset++];
                    }
                    // Is this byte a duplicate of the one above it?
                    boolean isDup = ((dupFlag & (0x80 >> (offset & 7))) == 0);
                    // If yes, copy the element above it.  
                    // Otherwise, read a value from the input stream.
                    bytes[row + offset] = isDup ? bytes[prevRow + offset] 
                                               : compressedBytes[arrayOffset++];
                }
            }
            result.compressedBytes = compressedBytes;
        }
        return result;
    }

    // Change the size of a bitmap.  Rows are added/deleted on the bottom.
    // Columns are added/deleted on the right.
    public Bitmap 
    resize(int newWidth, int newHeight) { 
        Bitmap result = new Bitmap(newWidth, newHeight);
        int minHeight = Math.min(height, newHeight);
        int minRowSize = Math.min(rowSize, result.rowSize);
        for (int y = 0; y < minHeight; y++) {
            System.arraycopy(bytes, y * rowSize, 
                             result.bytes, y * result.rowSize, minRowSize);
        }
        return result;
    }

    // Offset a bitmap to the right and down by the given amount.  Negative
    // values are okay for the offsets.
    public Bitmap
    offset(int xOffset, int yOffset) { 
        // This is utterly slow, but it's not worth fixing.
        int width = this.width;
        int height = this.height;
        Bitmap result = new Bitmap(width, height);
        for (int y = 0; y < height; y++) {
            int newY = y + yOffset;
            if (newY >= 0 && newY < height) { 
                for (int x = 0; x < width; x++) {
                    int newX = x + xOffset;
                    if (newX >= 0 && newX < width) { 
                        if (getBit(x, y)) { 
                            result.setBit(newX, newY);
                        }
                    }
                }
            }
        }
        return result;
    }

    // Get the bit at the specified position
    private boolean getBit(int x, int y) { 
        int offset = y * rowSize + (x >> 3);
        int mask = 0x80 >> (x & 7);
        return ((bytes[offset] & mask) != 0);
    }

    // Set the bit at the specified position.
    private void setBit(int x, int y) { 
        if (x < 0 || x >= width  || y < 0 || y >= height) 
            throw new ArrayIndexOutOfBoundsException();
        int offset = y * rowSize + (x >> 3);
        int mask = 0x80 >> (x & 7);
        bytes[offset] |= mask;
        compressedBytes = null;
        flags &= ~BITMAP_COMPRESSED_FLAG;
    }


    private void
    readAsciiPBM(DataInput is) throws IOException { 
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int c;
                // Read the next character that's a '0' or '1'
                do { 
                    c = (is.readByte() & 0xFF);
                } while ( c != '0' && c != '1');
                if (c =='1') {
                    bytes[(y * rowSize) + (x >> 3)] |= (1 << (7 - (x & 7)));
                }
            }
        }
    }

    private void
    readRawPBM(DataInput is) throws IOException { 
        int rowBytes = ((width + 7) & ~7)/8; // may be one less that rowSize
        for (int y = 0, offset = 0; y < height; y++, offset += rowSize) {
            is.readFully(bytes, offset, rowBytes);
        }
    }

    private void error(String msg) throws IOException { 
        System.err.println(msg);
        throw new IOException();
    }

    private static int 
    x86Word(DataInput is) throws IOException { 
        return is.readUnsignedByte() + (is.readUnsignedByte() << 8);
    }

    private static int 
    x86Long(DataInput is) throws IOException { 
        return x86Word(is) + (x86Word(is) << 16);
    }

    public void 
    writeBin(DataOutput os) throws IOException { 
        os.writeShort(width);
        os.writeShort(height);
        os.writeShort(rowSize);
        os.writeShort(flags);
        os.writeLong(0);        // eight unused bytes
        byte[] outBytes = 
            ((flags & BITMAP_COMPRESSED_FLAG) != 0) ? compressedBytes : bytes;
        os.write(outBytes);
        if ((outBytes.length & 1) == 1) {
            os.write(0);
        }
    }


    // Print something not very readable to the screen
    private void show() { 
        System.out.println("width=" + width + "  height=" + height + 
                           " rowSize=" + rowSize);
        for (int y = 0; y < height; y++) { 
            for (int x = 0; x < width; x++) { 
                System.out.print(getBit(x, y) ? "#" : ".");
            }
            System.out.print("  ");
            for (int i = 0; i < rowSize; i++) { 
                int b = bytes[y * rowSize + i] & 0xFF;
                String bb = Integer.toHexString(b);
                if (bb.length() == 1) bb = "0" + bb;
                System.out.print(bb + " ");
            }
            System.out.println();
        }
    }

    private void dumpJava(String bitmapFile, String name, String fileName) 
                    throws IOException { 
        final String indent = "    ";
        PrintStream out = System.out;
        boolean haveFile = false;
        if (!fileName.equals("-")) { 
            out = new PrintStream(new FileOutputStream(fileName));
            haveFile = true;
        }
        out.println("Bitmap " + name + " = new Bitmap((short)" + 
                         rowSize + ", " + "new byte[] {");
        out.println(indent + "// File " + bitmapFile + ";  size: " + 
                           width + "x" + height);

        for (int i = 0; i < bytes.length; i++) { 
            if ((i % 8) == 0) { 
                out.print(i == 0 ? indent : (",\n" + indent));
            } else { 
                out.print(", ");
            }
            out.print(formatNumber(bytes[i], 4));
        }
        out.println("\n});");
        if (haveFile) { 
            out.flush(); out.close();
        }
    }

    private void dumpJavaNew(String bitmapFile, String name, String fileName) 
             throws IOException { 
        final String indent = "    ";
        PrintStream out = System.out;
        boolean haveFile = false;
        if (!fileName.equals("-")) { 
            out = new PrintStream(new FileOutputStream(fileName));
            haveFile = true;
        }

        boolean compressed = ((flags & BITMAP_COMPRESSED_FLAG) != 0);
        out.println("Bitmap " + name + " = new Bitmap(new short[] {");
        out.println(indent + "// File " + bitmapFile + ";  size: " + 
                           width + "x" + height + 
                          (compressed ? " (compressed) " : ""));
        out.println(indent + "// header");
        out.println(indent + formatNumber((short)width,   6) + ", " +
                             formatNumber((short)height,  6) + ", " +
                             formatNumber((short)rowSize, 6) + ", " +
                             formatNumber((short)flags,   6) + ", " + 
                             formatNumber(0,              6) + ", " + 
                             formatNumber(0,              6) + ", " + 
                             formatNumber(0,              6) + ", " +
                             formatNumber(0,              6) + ", ");

        // Print out the bytes
        byte[] bytes = compressed ? compressedBytes : this.bytes;
        out.println(indent + "// bits");
        for (int i = 0; i < bytes.length; i += 2) { 
            if ((i % 16) == 0) { 
                out.print(i == 0 ? indent : ",\n" + indent);
            } else { 
                out.print(", ");
            }
            int value = (bytes[i] << 8);
            try { 
                value +=  (bytes[i+1] & 0xFF); 
            } catch (ArrayIndexOutOfBoundsException e) {}
            out.print(formatNumber(value, 6));
        }
        out.println("\n});");
        if (haveFile) { 
            out.flush(); out.close();
        }
    }

    // Return the number, as a string, using the specified width
    private String
    formatNumber (int number, int width) { 
        StringBuffer sb = new StringBuffer().append(number).reverse();
        for (int i = sb.length(); i < width; i++) 
            sb.append(' ');
        return sb.reverse().toString();
    }


    // Compress the bitmap
    public boolean
    compress(boolean force) { 
        int rowSize = this.rowSize;
        int height = this.height;
        byte bytes[] = this.bytes;
        
        byte newBytes[] = new byte[2 + 2 * rowSize * height]; // way too big

        // The first two bytes are reserved for holding the size
        int newBytesIndex = 2;
        int dupFlagIndex = -1;

        // Look at each row
        for (int row = 0; row < bytes.length; row += rowSize) { 
            int prevRow = row - rowSize;
            // Look at each byte in this row
            for (int offset = 0; offset < rowSize; offset++) {
                byte thisByte = bytes[row + offset];
                if ((offset & 7) == 0) { 
                    // This is the first of a group of 8.  Set aside a byte
                    // for indicating which bits are duplications of row above.
                    // Initialize it to zero, though that may change.
                    dupFlagIndex = newBytesIndex++;
                    newBytes[dupFlagIndex] = 0;
                }
                if (prevRow < 0 || thisByte != bytes[prevRow + offset]) {
                    // This byte is not a duplicate of the byte above it.
                    // Change the flag, and write out the byte.
                    newBytes[dupFlagIndex] |= (0x80 >> (offset & 7));
                    newBytes[newBytesIndex++] = thisByte;
                }
            }
        }
        
        int compressedSize = newBytesIndex + 16;
        int uncompressedSize = bytes.length + 16;
        double compression = (double)compressedSize / uncompressedSize;
        java.text.DecimalFormat df = new java.text.DecimalFormat("(#.##%)");
        System.out.println("Uncompressed size: " + uncompressedSize + " bytes; "
                           + "compressed size: " + compressedSize + " bytes; "+
                           df.format(compression));
        if (force || newBytesIndex < bytes.length) { 
            // Insert the length into the space reserved
            newBytes[0] = (byte)(newBytesIndex >> 8);
            newBytes[1] = (byte)(newBytesIndex & 0xFF);
            // Move it to an array of exactly the right size
            compressedBytes = new byte[newBytesIndex];
            System.arraycopy(newBytes, 0, compressedBytes, 0, newBytesIndex);
            // Set the flag, indicating we have a compressed bitmap
            flags |= BITMAP_COMPRESSED_FLAG;
            return true;
        } else { 
            System.out.println("Bitmap not compressed");
            return false;
        }
    }


    static private void 
    usage() { 
        System.err.print(
"Usage:\n" + 
"    java palm.database.Bitmap <bitmapFile> commands . . . .\n\n" +
"where commands include\n\n" +
"-help                    Print this help command\n" + 
"-resize <width> <height> Resize the bitmap\n" + 
"-expand <width> <height> Like resize, but centers the old bitmap\n" + 
"-compress                Creates a \"compressed\" bitmap, if this saves space\n"+ 
"-Compress                Like -compress, but always performs compression\n"+
"-uncompress              Undo any compression\n" +
"-invert                  Invert the bitmap (exchange black and white)\n" +
"-show                    Print the bitmap on the screen as ascii art\n" +
"-dump <varName> <file>   Output a Java statement to create this bitmap\n" + 
"                         Use \"-\" as file to write to standard output\n" +
"-Dump <varName> <file>   Use an alternative Java bitmap constructor\n" + 
"                         arguments are the same as -dump\n" + 
"-write <file>            Write the bitmap to a file.  (In Palm format)\n" + 
"\n" + 
""
);
    }

}
