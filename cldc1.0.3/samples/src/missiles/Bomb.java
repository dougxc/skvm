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

package missiles;

import com.sun.kjava.*;

public class Bomb extends GamePiece {

    private String bomb =
          "__iiii__"
        + "___ii___"
        + "__iiii__"
        + "__iiii__"
        + "__iiii__"
        + "__iiii__"
        + "__iiii__"
        + "___ii___";

    private String splat =
          "_i____iii___i___"
        + "i_i__i_____i____"
        + "___i__i__i_____i"
        + "_i__i___i__ii___"
        + "i_i__i_i__i__i__"
        + "___i_iii_i______"
        + "__ii_iiiiiiii___"
        + "iiiiiiiiiiiiiiii";

    public static int interval = 0;

    public static GameBitmap staticBitmap;
    public static int firstPiece;
    public static int count;
    public static int leftOffset;

    public static GameBitmap hitBitmap;
    public static int hitLeftOffset;
    public static int hitTopOffset;

    public boolean isaHit;

    public Bomb() {
        waitTicks = MAXTICKS;
        if (staticBitmap != null) {
            bitmap = staticBitmap;
            return;
        }

        staticBitmap = bitmap =
            new GameBitmap(3, 8, Missiles.x, Missiles.y,
                GameBitmap.VERT, 4,
                new Bitmap( bomb, 8 ));

        leftOffset = bitmap.width >> 1;

        hitBitmap =
            new GameBitmap(16, 9, Missiles.x+10, Missiles.y,
                GameBitmap.HORZ, 1,
                new Bitmap( splat, 16 ));

        hitLeftOffset = (bitmap.width - hitBitmap.width) >> 1;
        hitTopOffset = (bitmap.height - hitBitmap.height) >> 1;

        GamePiece[] pieces = new GamePiece[count = 5];
        pieces[0] = this;
        for (int i = 1; i < count; i++) {
            pieces[i] = new Bomb();
        }
        firstPiece = Missiles.addPieces(Missiles.BOMB, pieces);
    }

    public static boolean drop(int center, int top) {
        for (int i = firstPiece; i < firstPiece+count; i++) {
            GamePiece piece = Missiles.gamePieces[i];
            if (piece.waitTicks > 100) {
                piece.start(center, top);
                return true;
            }
        }
        return false;
    }

    public void start(int center, int top) {
        bitmap.draw(center - leftOffset, top, this);
        waitTicks = interval;
        isaHit = false;
    }

    public void tick() {
        if (isaHit) {
            hitBitmap.draw(left, top, this);
            recycle();
        } else {
            bitmap.move(+1, this);
            waitTicks = interval;
            if (Missiles.launcher != null && Missiles.launcher.overlaps(this)) {
                if (Missiles.doSound) {
                    //SOUND_INFO,SOUND_WARNING,SOUND_ERROR,SOUND_ALARM
                    Graphics.playSound(Graphics.SOUND_CONFIRMATION);
                }
                Missiles.decrementLives();
                erase();
                hitBitmap.draw(left + hitLeftOffset,
                        top + hitTopOffset, this);
                isaHit = true;
                waitTicks = 10;
            } else {
                checkOffScreen();
            }
        }
    }
}


