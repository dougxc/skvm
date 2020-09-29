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
 * RawPRC
 *
 * This tool creates a Palm resource database from a set of "bin" files.
 */

package palm.database;

import java.io.*;
import java.util.*;

public class RawPRC extends DatabaseGenerator { 
    public static void main(String argv[]) throws IOException { 
        if (argv.length <= 3) { 
            System.out.println("RawPRC <prcName> <longName> <appID> <bin>...");
                               System.exit(1);
        } 
        new RawPRC().run(argv);
    }

    // This does the actual work.
    private void run(String argv[]) throws IOException { 

        Vector resourceVector = new Vector();
        // Look at each filename specified as an argument
        for (int i = 3; i < argv.length; i++) { 
            String fileName = argv[i];
            try { 
                Resource resource = new Resource.FileResource(fileName);
                resourceVector.addElement(resource);
            } catch (IOException e) { 
                System.err.println(e);
            }
        }
        // Convert the vector into an array;
        int resourceCount = resourceVector.size();
        Resource[] resources = new Resource[resourceCount];
        resourceVector.copyInto(resources);
        // Write the file
        long fileSize = 
            writeDatabase(argv[0], argv[1], argv[2], "appl", resources);
        System.out.println(resourceCount + " resources; " 
                           + fileSize + " bytes");
    }
}

