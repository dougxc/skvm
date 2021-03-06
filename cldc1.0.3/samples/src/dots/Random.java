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

package dots;

import com.sun.kjava.*;

public class Random extends DotGame {
    
    /**
     * 
     */
    public short myTurn() {
        if (emptyBoxes == 0) return -1;
        int n = (random.nextInt() >>> 1) % (emptyBoxes << 2);
        int startSide = n & 0x3;
        n >>= 2;
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int boxCode = box[row][col];
                if (boxCode >= 0 && (n-- == 0)) {
                    for (int side = TOP; side <= RIGHT; side++) {
                        short playCode = getPlayCode(row, col, (side+startSide) & 0x3);
                        if (playCode != -1) {
                            return playCode;
                        }
                    }
                }
            }
        }
        Object obj = null;
        obj.hashCode();
        return -1;
    }

}


