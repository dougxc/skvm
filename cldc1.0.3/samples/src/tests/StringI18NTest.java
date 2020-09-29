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

public class StringI18NTest {

//------------------------------------------------------------------------
//                              main
//------------------------------------------------------------------------

    public static void main(String[] args) throws IOException {
        String s = "Hello World";
        byte[] bytes = s.getBytes();
        String s2 = new String(bytes, 0, bytes.length);
        System.out.println(s2);
    }

}
