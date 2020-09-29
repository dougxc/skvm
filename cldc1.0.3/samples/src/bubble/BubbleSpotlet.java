package bubble;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;
import com.sun.kjava.*;

public abstract class BubbleSpotlet extends Spotlet {

    final static int CHARHEIGHT = 10;
    final static int CHARWIDTH  = 8;
    final static int BUFSIZE = 64;

    int boxHeight = 10;
    int boxWidth  = 40;

    int width   = 160;
    int height  = 160;
    int xoffset = width/2;
    int yoffset = height/2;

    private StreamConnection uc;
    private OutputStream os;
    private PrintStream out;
    private InputStream is;
    private byte[] buf = new byte[BUFSIZE];
    private int bufCount = 0;
    private int bufIndex = 0;

    boolean seenDown = false;

    boolean inet = true;

    private Graphics g = Graphics.getGraphics();

    public void init() {
        //com.sun.cldc.io.palm.http.Inet.DEBUG = true;
        bubbleInit();
    }
    abstract void bubbleInit();

    public void start() {
        repaint();
    }

    void clear() {
        g.clearScreen();
    }

    void circle(int x, int y, int radius, boolean white, String text) {
        circle(x, y, radius, white);
        text(x+radius+2, (y-(radius/2))+6, text);
    }

    void circle(int x, int y, int radius, boolean white) {
        int xx = (xoffset + x) - radius;
        int yy = (yoffset - y) - radius;

        int ballSize = radius*2;
        g.drawRectangle(xx,yy,ballSize,ballSize,g.PLAIN,radius);
        if(white) {
            g.drawRectangle(xx+1,yy+1,ballSize-2,ballSize-2,g.ERASE,radius-1);
        }

   //   box(x,y, 2, 2, "");
    }

    void box(int x, int y, int w, int h, String msg) {
        int xx = (xoffset + x) - w/2;
        int yy = (yoffset - y) - h/2;
   //     g.drawRectangle(xx, yy, w, h, Graphics.PLAIN, 0);
        g.drawRectangle(xx+1, yy+1, w-1, h-1, Graphics.ERASE, 0);
        g.drawString(msg, xx+(boxWidth/4), yy);

        g.drawLine(xx,   yy,   xx+w, yy,      Graphics.PLAIN);
        g.drawLine(xx,   yy,   xx,   yy+h,    Graphics.PLAIN);
        g.drawLine(xx+w, yy,   xx+w, yy+h,    Graphics.PLAIN);
        g.drawLine(xx+h, yy+h, xx,   yy+h,    Graphics.PLAIN);
    }

    void text(int x, int y, String msg) {
        int xx = (xoffset + x);
        int yy = (yoffset - y);
        g.drawString(msg, xx, yy+1);
    }

    public void repaint() {
        if(bd == null) {
            clear();
            if(ta != null) {
                ta.paint();
            }
            paint();
        }
    }
    abstract void paint();

    public void penDown(int x, int y) {
        seenDown = true;
        down(x-xoffset, yoffset-y);
    }
    abstract void down(int x, int y);

    public void penUp(int x, int y) {
        if(seenDown) {
            seenDown = false;
            up(x-xoffset, yoffset-y);
            repaint();
        }
    }
    abstract void up(int x, int y);

    public void keyDown(int ch) {
        if(ta != null) {
            ta.handleKeyDown(ch);
        }
    }



    ScrollTextArea ta;

    public void addTextArea(String data) {
        ta = new ScrollTextArea(data, 20, 20, 120, 120);
    }

    public String removeTextArea() {
        if(ta == null) {
            return null;
        }
        String data = ta.getText();
        ta.kill();
        ta = null;
        g.setDrawRegion(0, 0, 160, 160);
        return data;
    }

    BubbleDialog bd = null;

    public void dialog(String title, String initalValue) {
        bd = new BubbleDialog(title, initalValue, "Ok", this);
        bd.showDialog();
    }
    void dialogDone(String name) {
        bd = null;
        endDialog(name);
    }
    abstract void endDialog(String name);

    PrintStream openWriteStream() {
        try {
            try {
                uc = (StreamConnection)Connector.open("http://www.shaylor.com/servlet/BubbleServer/?cache=no");
                } catch(ConnectionNotFoundException ex) {
                     uc = (StreamConnection)Connector.open("http://129.144.176.97/servlet/BubbleServer/?cache=no");
                     ((com.sun.cldc.io.connections.HttpConnection)uc).setRequestMethod("POST");
                     inet = false;
                }
            os = uc.openOutputStream();
            out = new PrintStream(os);
            out.print("data=");
            return out;
        } catch(IOException x) {
            System.out.println("Open failure "+x);
            return null;
        }
    }

    InputStream openReadStream() {
        try {
            if(inet) {
                os.close();
            }
            is = uc.openInputStream();
            bufCount = 0;
            return is;
        } catch(IOException x) {
            System.out.println("Close / open input  failure "+x);
            return null;
        }
    }

    void closeStreams() {
        try {
            out.close();
            if(!inet) {
                os.close();
            }
            is.close();
            uc.close();
        } catch(IOException x) {
            System.out.println("Close input failure "+x);
        }
    }

    int readChar(InputStream in) throws IOException {
        if (bufCount == 0) {
            if ((bufCount = in.read(buf, 0, BUFSIZE)) == 0) {
                return -1;
            }
            if(bufCount == -1) {
                return -1; // Some funny bug
            }
            bufIndex = 0;
        }
        bufCount--;
        return buf[bufIndex++];
    }

    String readLine(InputStream in) throws IOException {
        int ch = readChar(in);
        if(ch == -1) {
            return null;
        }
        StringBuffer sb = new StringBuffer();

        do {
            if(ch != '\r') {
                if(ch == '\n') {
                    break;
                }
                sb.append((char)ch);
            }
        } while ((ch = readChar(in)) != -1);

        return sb.toString();
    }


    public void runApplication() throws Exception {
        init();
        start();
        register(NO_EVENT_OPTIONS);
    }
}




class BubbleDialog extends Spotlet {

    protected com.sun.kjava.Button button;
    protected TextField tf;
    protected String text;
    protected String title;
    protected Graphics g = Graphics.getGraphics();
    BubbleSpotlet spot;

    public BubbleDialog(String t, String str, String buttonText, BubbleSpotlet s) {
        title = t;
        text = str;
        spot = s;
        button = new com.sun.kjava.Button(buttonText, 115, 145);
        tf = new TextField(title, 10, 10, 100, 10);
    }

    /**
     * Paint the Dialog.
     */
    public void paint() {
        g.clearScreen();
        tf.paint();
        button.paint();
    }


    /**
     * Show the Dialog: register it and paint it.
     */
    public void showDialog() {
        spot.unregister();
        register(NO_EVENT_OPTIONS);
        paint();
    }

    /**
     * Dismiss the Dialog.  Unregister it and alert the owner.
     */
    public void dismissDialog() {
        unregister();
        spot.register(NO_EVENT_OPTIONS);
        spot.dialogDone(tf.getText());
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
        }
    }

    public void keyDown(int key) {
        tf.handleKeyDown(key);
    }

}
