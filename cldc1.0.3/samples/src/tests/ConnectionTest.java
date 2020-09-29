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

public class ConnectionTest extends BaseTest {

   public String testName() {
      return "ConnectionTest";
   }

   public boolean runTest(int n) throws Throwable {
       Connection c;
       try {
           switch(n) {
                case 1:  Connector.open(null);           break;
                case 2:  Connector.open("");             break;
                case 3:  Connector.open("0");            break;
                case 4:  Connector.open("x");            break;
                case 5:  Connector.open(".");            break;
                case 6:  Connector.open("/");            break;
                case 7:  Connector.open("\\");           break;
                case 8:  Connector.open("\n");           break;
                case 9:  Connector.open("foo/");         break;
                case 10: Connector.open(":");            break;
                case 11: Connector.open("foo:");         break;
                case 12: Connector.open("foo//:");       break;
                case 13: Connector.open("foo://:");      break;
                case 14: Connector.open("foododod:odododododododo:dodododododdo//");    break;

                case 15: Connector.openInputStream("foo://");                   break;
                case 16: Connector.openDataInputStream("foo://");               break;
                case 17: Connector.openOutputStream("foo://");                  break;
                case 18: Connector.openDataOutputStream("foo://");              break;

                default: return true;
            }
            throw new RuntimeException("Did not trap");
        } catch(ConnectionNotFoundException x) {
            if(n <= 10) {
                throw x;
            }
        } catch(IllegalArgumentException x) {
            if(n > 10) {
                throw x;
            }
        }
        return false;
    }

}
