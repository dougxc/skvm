/*
 *  Copyright (c) 1999 Sun Microsystems,Inc.,901 San Antonio Road,
 *  Palo Alto,CA 94303,U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems,Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular,and
 *  without limitation,these intellectual property rights may include
 *  one or more U.S. patents,foreign patents,or pending
 *  applications.  Sun,Sun Microsystems,the Sun logo,Java,KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems,Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying,distribution,and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors,if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

package midi;

import com.sun.kjava.*;


public class PalmPiano extends Spotlet {
    Graphics g;
    Button exitButton;
    Button[] oct1 = new Button[12];
    Button[] oct2 = new Button[12];
    Button[] oct3 = new Button[12];
    
    String octaves[] = {"A","#","B","C","#","D","#","E","F","#","G","#"};
    
    int octave1[] = {220,238,256,274,293,311,329,348,366,384,403,421};
    int octave2[] = {440,476,513,550,686,623,660,696,733,770,806,843};
    int octave3[] = {880,953,1026,1100,1173,1246,1320,1393,1466,1540,1613,
                      1686};

    public static void main(String[] args) {
        (new PalmPiano()).register(NO_EVENT_OPTIONS);
    }

    Button createNoteButton(int octave,int note,int x0,int y0) {
        return new Button(octaves[note],x0 + note * 12 + 5,
                y0 + octave * 30 + 10);
    }

    void paint() {
        exitButton.paint();

        for (int i = 0; i < octaves.length; i++) {
            oct1[i].paint();
        }

        for (int i = 0; i < octaves.length; i++) {
            oct2[i].paint();
        }

        for (int i = 0; i < octaves.length; i++) {
            oct3[i].paint();
        }
    }

    public PalmPiano() {
        g = Graphics.getGraphics();
        exitButton = new Button("Exit",139,145);

        for (int i = 0; i < octaves.length; i++) {
            oct1[i] = createNoteButton(0,i,1,1);
        }

        for (int i = 0; i < octaves.length; i++) {
            oct2[i] = createNoteButton(1,i,1,1);
        }

        for (int i = 0; i < octaves.length; i++) {
            oct3[i] = createNoteButton(2,i,1,1);
        }
        paint();
    }

    public void penDown(int x,int y) {
        for (int i = 0; i < octaves.length; i++) {
            if (oct1[i].pressed(x,y)) {
                g.playSoundHz(octave1[i],200,127);
            }
        }

        for (int i = 0; i < octaves.length; i++) {
            if (oct2[i].pressed(x,y)) {
                g.playSoundHz(octave2[i],200,127);
            }
        }

        for (int i = 0; i < octaves.length; i++) {
            if (oct3[i].pressed(x,y)) {
                g.playSoundHz(octave3[i],200,127);
            }
        }

        if (exitButton.pressed(x,y)) {
            System.exit(0);
        }
    }
}
