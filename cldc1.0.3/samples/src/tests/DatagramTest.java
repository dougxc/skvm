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
 * datagram API.
 *
 * Note:
 *
 * 1, The casting from the Connector.open() call to the expected connection type.
 *
 * 2, The way datagram objects are derived from the connection object.
 *
 * 3, The way datagram buffers are automatically allocated (this is an option,
 *    buffers can also be manually allocated.
 *
 * 4, That the datagram interface included DataInput and DataOutput, permitting
 *    the direct use of binary data formatting functions.
 */


package tests;

import java.io.*;
import javax.microedition.io.*;

public class DatagramTest extends BaseTest {

   public String testName() {
      return "DatagramTest";
   }

    public boolean runTest(int n) throws IOException {

        /* Start the server thread */
        new DatagramTestThread().start();

        /* Get our receive port */
        DatagramConnection dgc = (DatagramConnection) Connector.open("datagram://localhost:6222");

        /* Get a new datagram 100 byte going to port 6222 */
        Datagram dg = dgc.newDatagram(100);

        /* loop 3 times */
        for(int i = 0 ; i < 3 ; i++) {

            /* Reset the write pointer */
            dg.reset();

            /* Write the variable 'i' to the datagram in binary */
            dg.writeInt(i);

            /* Send the datagram */
            dgc.send(dg);

            /* Reset the length */
            dg.setLength(100);

            /* Wait for a reply */
            dgc.receive(dg);

            /* And print it */
            String s = dg.readUTF();

            if(!s.equals("Hello World "+i)) {
                throw new RuntimeException(s+"!="+"Hello World "+i);
            }
        }

        /* close the connection */
        dgc.close();

        return true;
    }
}




class DatagramTestThread extends Thread {

   /**
    * The server thread
    */
    public void run() {
        try {
            /* Get our endpoint */
            DatagramConnection dgc = (DatagramConnection) Connector.open("datagram://:6222");

            /* Get a new datagram */
            Datagram dg = dgc.newDatagram(100);

            /* Loop until we read a binary 3 */
            for(int i = 0 ; i < 2;) {

                /* Reset the length */
                dg.setLength(100);

                /* Receive a datagram */
                dgc.receive(dg);

                /* Read the number */
                i = dg.readInt();

                /* Reset the datagram */
                dg.reset();

                /* Write our message */
                dg.writeUTF("Hello World "+i);

                /* And send it */
                dgc.send(dg);
            }

            /* close the connection */
            dgc.close();

        } catch(IOException x) {
            System.out.println(x);
        }
    }

}

