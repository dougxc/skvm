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
 **************************IMPORTANT*************************************
 *
 * Run this test with between 62KB and 74KB of heap,
 * use KVMutil to set the heap size.
 *
 * This test shows the use of the wireless http/https protocol on a
 * Palm VII handheld device.  It also shows how to use threads to 
 * take advantage of the non-blocking nature of the underlying protocol.
 *
 **************************IMPORTANT*************************************
 */

package tests;

import java.io.*;
import javax.microedition.io.*;
import com.sun.kjava.*;

class KvmHttpTest extends Spotlet {

    static Graphics g = Graphics.getGraphics();
    Button startButton, exitButton;
    public boolean startTests = false;

    public KvmHttpTest() {

        startButton = new Button("Start Tests", 1, 146);
        exitButton = new Button("Exit", 138, 146);
        g.clearScreen();
        startButton.paint();
        exitButton.paint();
        new runTests().start();

    }

    public void penDown(int x, int y) {

        System.out.println("pendown in httptest");

        if (exitButton.pressed(x, y)) {
            System.exit(0);
        }

        if (startButton.pressed(x, y)) {
            /*
             * We don't want the event thread to block so we just send a
             * 'signal' to the runTests thread to start the tests.  The
             * event thread is then free to process other events
             */
            startTests = true;
        }
    }


    // Inner class runTests
    public class runTests extends Thread {

        runTests() {
            super();
        }

        public void run() {
            for (;;) {
                while (startTests == false)
                    Thread.yield();
                execTests();
                startTests = false;
            }
        }

        public void execTests() {

            InetTest t;
            char [] c = {'/', '-', '\\', '|'};
            int i;

            g.drawRectangle(0, 0, 160, 140, Graphics.ERASE, 0);
            g.drawString("Starting tests", 1, 1, g.PLAIN);

            g.drawString("Starting HTTP test", 1, 10, g.PLAIN);
            t = new InetTest(1);
            t.start();
            i = 0;
            while (t.done() == 0) {
                g.drawString("Test 1... Waiting " + c[i++], 1, 20, g.PLAIN);
                Thread.yield();
                i &= 3;
            }
            g.drawString("HTTP test got " + t.totalRead() + " chars", 1, 20, g.PLAIN);

            g.drawString("Starting HTTPS test", 1, 30, g.PLAIN);
            t = new InetTest(2);
            t.start();
            while (t.done() == 0) {
                g.drawString("Test 2... Waiting " + c[i++], 1, 40, g.PLAIN);
                Thread.yield();
                i &= 3;
            }

            g.drawString("HTTPS test got " + t.totalRead() + " chars", 1, 40, g.PLAIN);

            g.drawString("Starting HTTP POST test", 1, 50, g.PLAIN);
            t = new InetTest(3);
            t.start();
            while (t.done() == 0) {
                g.drawString("Test 3... Waiting " + c[i++], 1, 60, g.PLAIN);
                Thread.yield();
                i &= 3;
            }

            g.drawString("HTTP POST test got " + t.totalRead() + " chars", 1, 60, g.PLAIN);

            g.drawString("Tests done", 1, 70, g.PLAIN);
        }

    }


    // Inner class InetTest
    public class InetTest extends Thread {

        int currentTest;

        int testdone = 0;
        int total = 0;
        int count;
        int c;
        byte [] b = new byte[32];
        InputStream in = null;
        StreamConnection con = null;

        public InetTest(int testNumber) {
            super();
            currentTest = testNumber;
        }

        public int done() {
            return testdone;
        }

        public int totalRead() {
            return total;
        }

        public void InetTest1() {
            try {
                in = Connector.openInputStream("http://javaweb/index.html");

                while ((count = in.read(b, 0, 32)) > 0) {
                    total += count;
                }
            } catch(IOException e) {
                System.out.println("HTTP read caught exception " + e);
                try {
                    in.close();
                } catch (IOException x) {
                    System.out.println("HTTP close exception " + x);
                    System.exit(1);
                }
                System.exit(1);
            }

            try {
                in.close();
            } catch (IOException e) {
                System.out.println("HTTP close exception " + e);
                System.exit(1);
            }

            testdone = 1;
        }

        public void InetTest2() {
            try {
                in = Connector.openInputStream("https://east.sun.net");

                total = 0;
                while ((count = in.read(b, 0, 32)) > 0 && total < 128) {
                    total += count;
                }
            } catch(IOException e) {
                System.out.println("HTTPS read caught exception " + e);
                try {
                    in.close();
                } catch (IOException x) {
                    System.out.println("HTTPS close exception " + x);
                    System.exit(1);
                }
                System.exit(1);
            }

            try {
                in.close();
            } catch (IOException e) {
                System.out.println("HTTPS close exception " + e);
                System.exit(1);
            }

            testdone = 1;
        }

        public void InetTest3() {
            try {
                con = (StreamConnection)Connector.open("http://java.sun.com/cgi-bin/test2.cgi");

                OutputStream os = con.openOutputStream();

                os.close(); // POST w/no data
                in = con.openInputStream();
            } catch (IOException e) {
                System.out.println("HTTP POST exception " + e);
                try {
                    in.close();
                } catch (IOException x) {
                    System.out.println("HTTP POST close exception " + x);
                    System.exit(1);
                }
                System.exit(1);
            }

            total = 0;

            try {
                while ((count = in.read(b, 0, 32)) > 0) {
                    total += count;
                }
            } catch(IOException e) {
                System.out.println("HTTP POST caught exception " + e);
                try {
                    in.close();
                } catch (IOException x) {
                    System.out.println("HTTP POST close exception " + x);
                    System.exit(1);
                }
                System.exit(1);
            }

            try {
                in.close();
            } catch (IOException e) {
                System.out.println("HTTP POST close exception " + e);
                System.exit(1);
            }

            try {
                con.close();
            } catch (IOException e) {
                System.out.println("HTTP POST connection close exception " + e);
                System.exit(1);
            }

            testdone = 1;
        }

        public void run() {

            switch (currentTest) {
                case 1:
                    InetTest1();
                    return;
                case 2:
                    InetTest2();
                    return;
                case 3:
                    InetTest3();
                    return;
            }
        }
    }

    public static void main(String[] args) throws Throwable {
        (new KvmHttpTest()).register(NO_EVENT_OPTIONS);
    }

}


