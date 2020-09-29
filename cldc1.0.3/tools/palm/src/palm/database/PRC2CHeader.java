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

public class PRC2CHeader {
    public PRC2CHeader(String filename) throws IOException {
        FileInputStream fis = new FileInputStream(filename);
        DataInputStream in = new DataInputStream(fis);
        int b, i;

        System.out.println("/* GENERATED AUTOMATICALLY BY PRC2CHeader, DO NOT EDIT MANUALLY! */");
        System.out.println();
        System.out.println("#ifndef PRC_H__");
        System.out.println("#define PRC_H__");
        System.out.println();
        System.out.print("static char prc_image[] = {");

        for (i=0;;i++) {
            b = in.read();
            if (b < 0) break;

            System.out.print((i % 16 == 0 ? (i == 0 ? "" : ",") + "\r\n    " : ", ") + format(b));
        }

        System.out.println("\r\n};");
        System.out.println();
        System.out.println("static int prc_image_length = " + i + ";");
        System.out.println();
        System.out.println("#endif /* PRC_H__ */");
    }
        
    private String format(int b) {
        return "0x" + (b < 16 ? "0" : "") + Integer.toHexString(b);
    }

    public static void main(String[] arg) throws IOException {
        if (arg.length != 1) {
            System.out.println("usage: PRC2CHeader <prc_file>");
            System.exit(-1);
        }

        new PRC2CHeader(arg[0]);
    }
}
