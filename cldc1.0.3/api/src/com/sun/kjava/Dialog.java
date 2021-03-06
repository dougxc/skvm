/*
 * Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
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
 */

package com.sun.kjava;

/**
 * A pop-up modal dialog that displays a title string, text box full of text,
 * and a dismiss button.
 */
public class Dialog extends Spotlet {

    private final int MAXWIDTH = 160; 
    private final int MAXHEIGHT = 140;
    private final int BUTTONX = 80;
    private final int BUTTONY = 145;

    protected int x;
    protected int y;
    protected int width;
    protected int height;

    protected Button button;
    protected TextBox tb;
    protected String text;
    protected String title;
    protected Graphics g = Graphics.getGraphics();
    protected DialogOwner owner;
    protected boolean haveScroll = false;

    /**
     * Create a new Dialog with a arbitrary size.  Note: MAXSIZE is 160 X 140
     * The contents of the box is passed in the str parameter.
     * A button is created which allows for dismissal of
     * the Dialog. The text for the button is passed in buttonText.
     *
     * If the text overflows the text box, a ScrollTextBox is used to 
     * display it.
     *
     * The owner of the Dialog gets called through the DialogOwner interface
     * dialogDismissed() method when the dialog is dismissed. The owner 
     * must then re-register the Spotlet that was running when the Dialog 
     * was created. It must also re-paint the screen as appropriate. 
     *
     * @param x the x location of this Dialog
     * @param y the y location of this Dialog
     * @param width the width of this Dialog
     * @param height the height of this Dialog
     * @param t the title of this Dialog - used when the Dialog is dismissed
     * @param str the contents of the TextBox
     * @param buttonText the label of the button
     */
    public Dialog( int x, int y, int width, int height,
        DialogOwner o, String t, String str, String buttonText) {

        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;

        if ( this.width > MAXWIDTH ) {
            this.width = MAXWIDTH;
        }
        if ( this.height > MAXHEIGHT ) {
            this.height = MAXHEIGHT;
        }

        owner = o;
        title = t;
        text = str;
        button = new Button(buttonText, BUTTONX, BUTTONY);
        tb = new TextBox(text, x, y, width, height);
        int numLines = tb.getNumLines();
        int textHeight = g.getHeight(text);
        if((numLines * textHeight) > width) {
            tb = null;
            tb = new ScrollTextBox(text, x, y, width, height);
            haveScroll = true;
        }
    }


    /**
     * Create a new Dialog of a fixed size. Creates a TextBox 140x120
     * at position 10,10. The contents of the box is passed in the
     * str parameter. A button is created which allows for dismissal of
     * the Dialog. The text for the button is passed in buttonText.
     *
     * If the text overflows the text box, a ScrollTextBox is used to 
     * display it.
     *
     * The owner of the Dialog gets called through the DialogOwner interface
     * dialogDismissed() method when the dialog is dismissed. The owner 
     * must then re-register the Spotlet that was running when the Dialog 
     * was created. It must also re-paint the screen as appropriate. 
     *
     * @param t the title of this Dialog - used when the Dialog is dismissed
     * @param str the contents of the TextBox
     * @param buttonText the label of the button
     */
    public Dialog(DialogOwner o, String t, String str, String buttonText) {
        this( 10, 10, 140, 120, o, t, str, buttonText );
    }

    /**
     * Paint the Dialog.
     */
    public void paint() {
        g.clearScreen();
        tb.paint();
        button.paint();
    }


    /**
     * Show the Dialog: register it and paint it.
     */
    public void showDialog() {
        register(NO_EVENT_OPTIONS);
        paint();
    }

    /**
     * Dismiss the Dialog.  Unregister it and alert the owner.
     */
    public void dismissDialog() {
        unregister();
        owner.dialogDismissed(title);
    }

    /**
     * If the user pressed the dismiss button, dismiss the Dialog.
     * If we have a ScrollTextBox, then allow scrolling.
     *
     * @param x the X coordinate of the user's press.
     * @param y the Y coordinate of the user's press.
     */
    public void penDown(int x, int y) {

        if(button.pressed(x,y)) {
            dismissDialog();
            return;
        }

        if(haveScroll) {
            ScrollTextBox stb = (ScrollTextBox)tb;
            if(stb.contains(x,y)) {
                stb.handlePenDown(x,y);
            }
        }
    }

    /**
     * If we have a ScrollTextBox, then allow scrolling.
     *
     * @param x the X coordinate of the user's press.
     * @param y the Y coordinate of the user's press.
     */
    public void penMove(int x, int y) {
        if(haveScroll) {
            ScrollTextBox stb = (ScrollTextBox)tb;
            if(stb.contains(x,y)) {
                stb.handlePenMove(x,y);
            }
        }
    }

    /**
     * If we have a ScrollTextBox, then allow scrolling.
     *
     * @param key the key pressed/entered by the user
     */
    public void keyDown(int key) {
        if(haveScroll) {
            ScrollTextBox stb = (ScrollTextBox)tb;
            stb.handleKeyDown(key);
        }
    }

}
