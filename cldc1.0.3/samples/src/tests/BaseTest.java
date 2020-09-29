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

package tests;

import java.io.*;
import javax.microedition.io.*;

public class BaseTest {

    public String testName() {
       return "BaseTest";
    }

   /**
    * run
    */
    public void run() {
        String name = testName()+":               ";
        System.out.print(name.substring(0, 20));
        int i = 0 ;
        try {
            while(true) {
                if(runTest(++i)) {
                    break;
                }
            }
            System.out.println("PASSED");
        } catch(Throwable t) {
            System.out.println("FAILED running test "+i+" "+t);
            t.printStackTrace();
        }
    }


   public boolean runTest(int n) throws Throwable {
        return true;
    }

}
