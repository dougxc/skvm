/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */

/*
 * The following program demonstrates the Generic Connection interface to the
 * socket and file APIs.
 *
 * Note:
 *
 * 1, The use of Connector.openOutputStream() and Connector.openInputStream()
 *    to directly get the general streams without having to open the connections
 *    manually in the program.
 *
 * 2, That the stream interfaces included DataInput and DataOutput, permitting
 *    the direct use of binary data formatting functions.
 *
 * 3, That the output stream interface implements the print() and println() functions
 *    permitting textual output.
 *
 * 4, That the input stream interface implements the scanLine() function (amongst others)
 *    permitting textual input.
 *
 * 5, The way the StreamConnectionNotifier connection is used to wait for a socket connection
 *    to be made.
 */

package tests;

import java.io.*;
import javax.microedition.io.*;

public class SocketTest extends BaseTest {

    public String testName() {
       return "SocketTest";
    }

    static String result = "foo";

   /**
    * main
    */
    public boolean runTest(int n) throws IOException {

        /* Start the server side of port 1234 */
        new SocketTestThread().start();

        while(result != null) {
            Thread.yield(); // Wait for server to start
        }

        /* Run test 1 */
        test1();

        while(result == null) {
            Thread.yield(); // Wait for server to complete
        }

        if(!result.equals("Hello World")) {
            throw new RuntimeException(result+"!=Hello World");
        }

        return true;
    }

   /**
    * test1
    */
    public static void test1() throws IOException {
        /* Open the client side of a connection to port 1234 of the local machine */
        DataOutputStream os =
            Connector.openDataOutputStream("socket://localhost:1234");

        /* Write the string out in textual form */
        os.writeUTF("Hello World");

        /* Close the output stream */
        os.close();
    }

}


class SocketTestThread extends Thread {

   /**
    * Server thread for test2
    */
    public void run() {

        try {
            /* Create the server listening socket for port 1234 */
            StreamConnectionNotifier scn = (StreamConnectionNotifier)Connector.open("socket://:1234");

            /* Say we are going */
            SocketTest.result = null;

            /* Wait for a connection */
            StreamConnection sc = scn.acceptAndOpen();

            /* Get the input stream of the connection */
            DataInputStream is = sc.openDataInputStream();

            /* Save the result */
            SocketTest.result = is.readUTF();

            /* Close everything */
            is.close();
            sc.close();
            scn.close();

        } catch(IOException x) {
            System.out.println(x);
            x.printStackTrace();
        }
    }

}
