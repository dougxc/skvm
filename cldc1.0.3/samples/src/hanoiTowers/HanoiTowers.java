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
package hanoiTowers;

import java.util.*;
import com.sun.kjava.*;

/**
 * This class implements the Towers of Hanoi game.
 * You can only move one disk at a time and you
 * must follow size order (a bigger disk can't go
 * on a smaller disk).  You win once you have trans-
 * ferred the disks from one tower to another.
 *
 * @author Efren Serra
 * @since KVM 1.0
 *
 */
public class HanoiTowers extends Spotlet {
    /**
     *
     */
    class Rect {
        public int _X0;
        public int _Y0;
        public int _X1;
        public int _Y1;

        public Rect() {
            _X0 = _Y0 = _X1 = _Y1 = 0;
        }
    }

    /**
     *
     */
    class Point {
        public int _X;
        public int _Y;

        public Point() {
            _X = _Y = 0;
        }

        public String toString() {
            return ("(" + _X + ", " + _Y + ")");
        }
    }

    /**
     *
     */
    abstract class Glyph {
        /**
         * Opposite corners of the smallest rectangle
         * that contains this glyph.
         */
        protected int _X0;
        protected int _Y0;
        protected int _X1;
        protected int _Y1;

        /**
         * The width and height of this glyph.
         */
        protected int _Width;
        protected int _Height;

        /**
         * Constructs an object of this class.
         */
        public Glyph(int width, int height) {
            _Width = width;
            _Height = height;
        }

        // Appearance
        /**
         * Draws the glyph on the graphics object.
         */
        public abstract void draw(Graphics g);

        // Hit detection
        /**
         * Returns the rectangular area that the glyph occupies.
         * It returns the opposite corners of the smallest rectangle
         * area that contains the glyph.
         */
        public void getBounds(Rect r) {
            r._X0 = _X0;
            r._Y0 = _Y0;
            r._X1 = _X1;
            r._Y1 = _Y1;
        }

        /**
         * Returns whether a specified point intersects the glyph.
         */
        public boolean intersects(Point p) {
            return (_X0 <= p._X && p._X <= _X1 && _Y0 <= p._Y && p._Y <= _Y1);
        }

        // Structure
        /**
         * Inserts a glyph into the group of children.
         */
        public abstract void insert(Glyph g);

        /**
         * Removes a specified glyph if it is indeed a child.
         */
        public abstract void remove(Glyph g);

        /**
         * Returns the child at the specified index.
         */
        public abstract Glyph getChild(int i);

        /**
         * Returns the child count.
         */
        public int getChildCount() {
            return (0);
        }

        /**
         * Provides a standard interface to the glyph's parent,
         * if any.
         */
        public abstract Glyph getParent();

        /**
         * Set upper left corner of this glyph.
         */
        public void setULCorner(int x0, int y0) {
            _X0 = x0;
            _Y0 = y0;
            _X1 = x0 + _Width;
            _Y1 = y0 + _Height;
        }
        /**
         *
         */
        public int width() {
            return (_Width);
        }
        /**
         *
         */
        public int height() {
            return (_Height);
        }
        /**
         *
         */
        public String toString() {
            String string0 = "(" + _X0 + ", " + _Y0 + ")";
            String string1 = "-------|\n";
            String string2 = "\t\t\t|            |\n";
            String string3 = "\t\t\t----------(" + _X1 + ", " + _Y1 + ")";
            return (string0 + string1 + string2 + string3);
        }
    }

    /**
     *
     */
    class Tower extends Glyph {
        /**
         * The children of this glyph.
         */
        private Vector _Children = new Vector();

        /**
         * Constructs an object of this class.
         */
        public Tower(int x0, int y0, int width, int height) {
            super(width, height);
            setULCorner(x0, y0);
        }
        /**
         * Constructs an object of this class.
         */
        public Tower() {
            super(0, 0);
            setULCorner(0, 0);
        }
        /**
         *
         */
        public void draw(Graphics g) {
            // draw yourself
            g.drawRectangle(_X0, _Y0, _Width, _Height, g.GRAY, 0);

            // draw your children
            int size = _Children.size();
            for (int i = 0; i < size; i++) {
                ((Glyph)_Children.elementAt(i)).draw(g);
            }
        }
        /**
         *
         */
        public void insert(Glyph g) {
            int count = _Children.size();
            int x0 = _X0 - (g.width() - _Width)/2;
            int y0 = _Y0 + (_Height - (count + 1) * g.height());

            g.setULCorner(x0, y0);
            _Children.addElement(g);
        }
        /**
         *
         */
        public void remove(Glyph g) {
            _Children.removeElement(g);
        }
        /**
         *
         */
        public Glyph getChild(int i) {
            return ((Glyph)_Children.elementAt(i));
        }
        /**
         *
         */
        public int getChildCount() {
            return (_Children.size());
        }
        /**
         *
         */
        public Glyph getParent() {
            return (null);
        }
    }

    /**
     *
     */
    class Ring extends Glyph {
        /**
         * The parent of this glyph.
         */
        private Glyph _Parent;

        /**
         *
         */
        public Ring(int x0, int y0, int width, int height) {
            super(width, height);
            setULCorner(x0, y0);
        }
        /**
         *
         */
        public Ring() {
            super(0, 0);
            setULCorner(0, 0);
        }
        /**
         *
         */
        public void draw(Graphics g) {
            // draw yourself
            g.drawRectangle(_X0, _Y0, _Width, _Height, g.GRAY, 0);
            g.drawBorder(_X0, _Y0, _Width-1, _Height-1, g.GRAY, g.RAISED);
        }
        /**
         *
         */
        public void insert(Glyph g) {
        }
        /**
         *
         */
        public void remove(Glyph g) {
        }
        /**
         *
         */
        public Glyph getChild(int i) {
            return (null);
        }
        /**
         *
         */
        public Glyph getParent() {
            return (_Parent);
        }
        /**
         *
         */
        public void setParent(Glyph parent) {
            _Parent = parent;
        }
    }

    /**
     * Index of picked glyph.
     */
    private int _Index;

    /**
     * Whether a ring has been picked.
     */
    private boolean _Rpickd;

    /**
     * Whether the game is over.
     */
    private boolean _Gover;

    /**
     * The intersection point.
     */
    private Point _Point = new Point();

    /**
     * Glyph selected with pen down event.
     */
    private Glyph _Glyph;

    /**
     * The Towers + Rings.
     */
    private Tower[] _Towers = new Tower[3];
    private Ring[] _Rings = new Ring[5];

    /**
     * The GUI controls
     */
    Button exitButton, resetButton;

    /**
     * The Singleton graphics object.
     */
    static Graphics _Graphics;

    static {
        _Graphics = Graphics.getGraphics();
    }

    /**
     *
     */
    private void initialize() {
        // Create the towers
        _Towers[0] = new Tower( 30, 60, 4, 80);
        _Towers[1] = new Tower( 80, 60, 4, 80);
        _Towers[2] = new Tower(130, 60, 4, 80);

        // Create the Rings
        _Rings[0] = new Ring( 7, 135, 50, 5);
        _Rings[1] = new Ring(12, 130, 40, 5);
        _Rings[2] = new Ring(17, 125, 30, 5);
        _Rings[3] = new Ring(22, 120, 20, 5);
        _Rings[4] = new Ring(27, 115, 10, 5);

        // Create the GUI controls
        resetButton = new Button("Reset",108,145);
        exitButton = new Button("Exit",136,145);

        // Set glyph's parent
        for (int i = 0; i < _Rings.length; i++) {
            _Rings[i].setParent(_Towers[0]);
            _Towers[0].insert(_Rings[i]);
        }
    }
    /**
     *
     */
    private void paint() {
        // Clear the screen
        _Graphics.clearScreen();

        // Draw the frame
        _Graphics.drawBorder(0, 0, 159, 139, _Graphics.PLAIN, _Graphics.SIMPLE);

        // Clear space below frame
        _Graphics.drawRectangle(0, 140, 160, 20, _Graphics.ERASE, 0);

        // Draw the title
        _Graphics.drawString("Towers of Hanoi", 50, 20);

        // Draw the glyphs
        for (int i = 0; i < _Towers.length; i++) {
            _Towers[i].draw(_Graphics);
        }

        // Draw the GUI controls
        resetButton.paint();
        exitButton.paint();
    }
    /**
     *
     */
    private void reset() {
        // Remove Rings from Towers
        for (int i = 0; i < _Towers.length; i++) {
            int count = _Towers[i].getChildCount();
            for (int j = 0; j < count; j++) {
                Glyph g = _Towers[i].getChild(0);
                _Towers[i].remove(g);
            }
        }

        // Set glyph's parent
        for (int i = 0; i < _Rings.length; i++) {
            _Rings[i].setParent(_Towers[0]);
            _Towers[0].insert(_Rings[i]);
        }

        // Re-draw all
        paint();
    }

    /**
     *
     */
    public HanoiTowers() {
        initialize();
        paint();
    }
    /**
     * Handle a pen down event.
     */
    public void penDown(int x, int y) {
        // Save coordinates
        _Point._X = x;
        _Point._Y = y;

        // Exit button pressed?
        if (exitButton.pressed(x, y)) {
            System.exit(0);
        }

        // Reset button pressed?
        if (resetButton.pressed(x, y)) {
            reset();
            return;
        }

        // Determine whether the point (x, y) coincides with the
        // top Ring of the current Tower (the one underneath the
        // point where the pen went down).
        for (int i = 0; i < _Towers.length; i++) {
            int count = _Towers[i].getChildCount();
            _Glyph = count > 0 ? _Towers[i].getChild(count - 1) : null;
            if (_Glyph != null && _Glyph.intersects(_Point)) {
                //System.out.println(_Point + " intersects " + _Glyph);
                _Rpickd = true;
                _Index = i;
                return;
            }
        }
    }
    /**
     * Handle a pen move event.
     */
    public void penMove(int x, int y) {
        //System.out.println("(" + x + ", " + y + ")");
        if (!_Gover && _Rpickd) {
            // Update glyph's state
            int deltaX = _Glyph.width()/2;
            int deltaY = _Glyph.height()/2;
            _Glyph.setULCorner(x - deltaX, y - deltaY);

            // Draw the glyphs
            paint();
        }
    }
    /**
     * Handle a pen up event.
     */
    public void penUp(int x, int y) {
        boolean inserted = false;
        // Save coordinates
        _Point._X = x;
        _Point._Y = y;

        //System.out.println("(" + x + ", " + y + ")");
        if (!_Gover && _Rpickd) {
            _Towers[_Index].remove(_Glyph);
            // Determine whether the point (x, y) coincides with
            // any one of the Towers.  If so, determine whether
            // its top Ring is bigger than the picked Ring and put
            // the latter in the picked Tower.  If the picked
            // Tower does not have any Rings, just drop it there.
            for (int i = 0; i < _Towers.length; i++) {
                if (_Towers[i].intersects(_Point)) {
                    int count = _Towers[i].getChildCount();
                    Glyph top = count > 0 ? _Towers[i].getChild(count - 1) : null;
                    if (count == 0 || top.width() > _Glyph.width()) {
                        inserted = true;
                        _Towers[i].insert(_Glyph);
                        ((Ring)_Glyph).setParent(_Towers[i]);
                        break;
                    }
                }
            }
            // Re-insert if a Tower didn't coincide
            if (!inserted) {
                _Towers[_Index].insert(_Glyph);
            }

            // Draw the glyphs
            paint();
        }
        _Rpickd = false;

        // See if player won
        for (int i = 1; i < _Towers.length; i++) {
            int count = _Towers[i].getChildCount();
            if (count == _Rings.length) {
                _Gover = true;
                _Graphics.drawString("Congratulations!!!", 46, 40);
                break;
            }
        }
    }

    /**
     *
     */
    public static void main(String[] args) {
        new HanoiTowers().register(NO_EVENT_OPTIONS);
    }
}
