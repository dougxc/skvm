package bubble;

import java.io.*;
import java.util.*;

public class BubbleClient extends BubbleApplet {

    int downx;
    int downy;

    String location = "/";

    String nodeid;
    String nodeParent;
    String nodeType;
    String nodeName = "";
    String nodeText = null;

    Vector things;

    BBubble b0;
    BButton b1;
    BButton b2;
    BButton b3;
    BButton b4;
    BButton bdone;

    int num, idle;

    BBubble dialogBubble;

    boolean newBubble  = false;
    boolean newDoc     = false;
    boolean edited     = false;
    boolean newNode    = false;

    boolean busy       = false;

    int xxxClicks = 0;

    public void bubbleInit() {
        b0 = new BBubble(this, null, "n", " ", 0, 0, 4) {
            void click(int x, int y) {
                if(!location.equals("/")) {
                    setlocation(nodeParent);
                }
            }
            void writeOn(PrintStream out) {
            }
        };

        int basey = 0-(height/2 - boxHeight/2);
        b1 = new BButton(this, "Rfsh", 0-(boxWidth+boxWidth/2), basey) {
            void click() {
                if(edited) {
                    commit();
                } else {
                    readData(nodeid);
                }
            }
            void paint() {
                if(busy) {
                    text = "Busy";
                } else {
                    text = (edited) ? "Cmit" : "Rfsh";
                }
                super.paint();
            }
        };


        b2 = new BButton(this, "Doc",  0-(         boxWidth/2), basey) {
            void click() {
                newDoc  = true;
                edited  = true;
            }
        };

        b3 = new BButton(this, "Bub",  0+(         boxWidth/2), basey) {
            void click() {
                newBubble = true;
                edited  = true;
            }
        };

        b4 = new BButton(this, "xxx",  0+(boxWidth+boxWidth/2), basey) {
            void click() {
                xxxClicks++;
            }
        };

        bdone = new BButton(this, "Done", 0+(boxWidth+boxWidth/2), basey) {
            void click() {
                setlocation(nodeParent);
            }
        };

        addButtons();
        readData("0");
System.out.println("**bubbleInit done**");

/*******************************************
        new Thread() {
             public void run() {
                 while(true) {
                     try {
                         --idle;
                         b4.text = ""+idle;
                         if(idle <=0) {
                             if(!busy) {
                                 b1.click();
                             }
                             idle = 120;
                         }
                         repaint();
                         try {
                             sleep(1000);
                         } catch(InterruptedException ex) {
                         }
                     } catch(Exception ex) {
                     }
                 }
             }
        }.start();
************************************************/
    }


    void addButtons() {
        things = new Vector();
        things.addElement(b0);
        things.addElement(b1);
        things.addElement(b2);
        things.addElement(b3);
        things.addElement(b4);
    }


    void addDocButtons() {
        things = new Vector();
        things.addElement(b1);
        things.addElement(bdone);
    }

    void sayBusy(boolean b) {
        busy = b;
        b1.paint();
    }


    public void paint() {
        for(Enumeration e = things.elements() ; e.hasMoreElements() ; ) {
            GraphicalThing thing = (GraphicalThing)e.nextElement();
            thing.paint();
        }
        text(0-(width/2), height/2-10, nodeName);
    }

    void down(int x, int y) {
        idle = 120;
        downx = x;
        downy = y;
    }

    void up(int x, int y) {
        if(newBubble || newDoc) {
            newClick(x, y, newDoc);
            newBubble = newDoc = false;
        } else {
            if(Math.abs(x-downx) <= 1 && Math.abs(y-downy) <=1) {
                click(downx, downy);
            } else {
                drag(downx, downy, x, y);
            }
        }
    }

    void newClick(int x, int y, boolean doc) {
        newNode = edited = true;
        BBubble     b = doc ?
                           new BDocument(this, "X", "d", "new", x, y, 5)
                           :
                           new   BBubble(this, "X", "n", "new", x, y, 5)
                           ;

        b.moved = b.nameChanged = true;
        things.addElement(b);
        b.editName();
    }

    void drag(int dnx, int dny, int upx, int upy) {
        for(Enumeration e = things.elements() ; e.hasMoreElements() ; ) {
            GraphicalThing thing = (GraphicalThing)e.nextElement();
            if(thing != b0 && thing.intersects(dnx, dny)) {
                thing.drag(upx, upy);
                edited = true;
                return;
            }
        }
    }


    void click(int x, int y) {
        if(newNode) {
            commit();
        }
        for(Enumeration e = things.elements() ; e.hasMoreElements() ; ) {
            GraphicalThing thing = (GraphicalThing)e.nextElement();
            if(thing.intersects(x, y)) {
                thing.click(x, y);
                return;
            }
        }
    }

    synchronized void setlocation(String nid) {
        commit();
        readData(nid);
    }


    synchronized void commit() {
        if(edited) {
            edited = false;
            newNode = false;
            writeData();
        }
    }


    synchronized int readData(String nid) {
        sayBusy(true);
        PrintStream out = openWriteStream();
        out.println("*"); // passwords
        out.println(nid);
        out.println("read");

        int res = readWriteData();
        sayBusy(false);
        return res;
    }


    synchronized int readWriteData() {
        try {
            InputStream in = openReadStream();

            if(in == null) {
                return 400;
            }

            String  result  = readLine(in);
            int res = 400;

            try {
                res = Integer.parseInt(result);
            } catch(NumberFormatException ex) {
                return 400;
            }

            if(res != 200) {
                return res;
            }

            nodeid             = readLine(in);
            nodeParent         = readLine(in);
            nodeType           = readLine(in);
            nodeName           = readLine(in);
            location           = readLine(in);
            nodeText           = null;
            removeTextArea();

System.out.println("id ="+nodeid);
System.out.println("parent ="+nodeParent);
System.out.println("type ="+nodeType);

            try {
                if(nodeType.equals("d")) {
                    addDocButtons();
                    readText(in);
                } else {
                    addButtons();
                    readNodes(in);
                }
            } catch(NumberFormatException ex) {
                System.out.println(ex);
            }

            closeStreams();
            return 200;
        } catch(IOException x) {
            System.out.println("Write failure "+x);
        }
        return 400;
    }

    void readNodes(InputStream in) throws NumberFormatException, IOException {
        while(true) {
            String id = readLine(in);
            if(id == null) {
                break;
            }

            String type = readLine(in);
            String name = readLine(in);
            String x    = readLine(in);
            String y    = readLine(in);


System.out.println("id ="+id);
System.out.println("type ="+type);
System.out.println("name ="+name);
System.out.println("x ="+x);
System.out.println("y ="+y);

            int xx = Integer.parseInt(x);
            int yy = Integer.parseInt(y);

            GraphicalThing g;

            if(type.equals("d")) {
                g = new BDocument(this, id, type, name, xx, yy, 5);
            } else {
                g = new BBubble(this, id, type, name, xx, yy, 5);
            }
            things.addElement(g);
        }
    }

    void readText(InputStream in) throws NumberFormatException, IOException {
        StringBuffer sb = new StringBuffer();
        while(true) {
            int ch = readChar(in);
            if(ch == -1) {
                break;
            }
            sb.append((char)ch);
        }
        nodeText = sb.toString();
        addTextArea(nodeText);
        edited = true;
    }


    synchronized int writeData() {
        sayBusy(true);
        PrintStream out = openWriteStream();
        out.println("*"); // passwords
        out.println(nodeid);
        out.println("write");

        if(nodeText != null) {
            nodeText = removeTextArea();
            out.print(nodeText);
            nodeText = null;
        } else {
            for(Enumeration e = things.elements() ; e.hasMoreElements() ; ) {
                GraphicalThing thing = (GraphicalThing)e.nextElement();
                thing.writeOn(out);
            }
        }
        int res = readWriteData();
        sayBusy(false);
        return res;
    }


    void endDialog(String name) {
        if(dialogBubble != null && name != null && name.length() > 0) {
            dialogBubble.name = name;
        }
        repaint();
    }


    public static void main(String[] argv) throws Exception {
        new BubbleClient().runApplication();
    }

}

/*
 * GraphicalThing
 */
class GraphicalThing {
    void paint() {
    }

    boolean intersects(int x, int y) {
        return false;
    }

    void click(int x, int y) {
        click();
    }

    void click() {
    }

    void drag(int x, int y) {
    }

    void writeOn(PrintStream out) {
    }
}


/*
 * Bubble
 */
class BBubble extends GraphicalThing {
    BubbleClient parent;
    String id;
    String type;
    String name;
    int x = -1;
    int y = -1;
    int radius;
    boolean ro;

    boolean nameChanged = false;
    boolean moved       = false;

    public BBubble() {}

    public BBubble(BubbleClient parent, String id, String type, String name, int x, int y, int radius) {

        this.parent = parent;
        this.id     = id;
        this.type   = type;
        this.name   = name;
        this.radius = radius;
        drag(x, y);
        this.ro     = name.endsWith("*");
        nameChanged = moved = false;
    }

    String displayName() {
        if(name.length() == 0) {
            name = "<none>";
        }
        int pos = name.indexOf('|');
        if(pos == -1) {
            return name;
        } else {
            return name.substring(0, pos);
        }
    }

    void paint() {
        parent.circle(x, y, radius, true, displayName());
    }

    boolean intersects(int x, int y) {
        return intersectsBubble(x, y) || intersectsLabel(x, y);
    }

    boolean intersectsBubble(int x, int y) {
        return (Math.abs(this.x - x) * Math.abs(this.y - y)) < (radius*2);
    }


    boolean intersectsLabel(int x, int y) {

        if(name.length() == 0) {
            return false;
        }

        int left  = this.x+radius+2;
        int width = displayName().length()*parent.CHARWIDTH;
        int mid   = left + (width/2);

        return (Math.abs(mid - x) < (width/2) &&
                Math.abs(this.y - y) < parent.CHARWIDTH/2);
    }


    void drag(int x, int y) {
        int hw = parent.width/2;
        int hh = parent.height/2;
        int maxx = hw - 10;
        int minx = 0 - maxx;
        int maxy = hh - 10;
        int miny = (0 - maxy) + parent.boxHeight;

        x = Math.min(maxx, x);
        x = Math.max(minx, x);
        y = Math.min(maxy, y);
        y = Math.max(miny, y);

        if(ro && parent.xxxClicks !=5) {
            parent.dialog("Read Only", null);
        } else {
            moved = true;
            this.x = x;
            this.y = y;
        }
    }


    void click(int x, int y) {
        if(intersectsLabel(x, y)) {
            if(ro && parent.xxxClicks !=5) {
                parent.dialog("Read Only", null);
            } else {
                editName();
            }
        } else {
            parent.setlocation(id);
        }
    }

    void editName() {
        nameChanged = true;
        parent.dialogBubble = this;
        parent.edited = true;
        parent.dialog("Name", name);
    }

    void writeOn(PrintStream out) {
        out.println(id);
        out.println(type);
        if(nameChanged) {
            out.println(name);
        } else {
            out.println("-");
        }
        if(moved) {
            out.println(x);
            out.println(y);
        } else {
            out.println("-");
            out.println("-");
        }
        nameChanged = moved = false;
    }
}


/*
 * Document
 */
class BDocument extends BBubble {
    public BDocument(BubbleClient parent, String id, String type, String name, int x, int y, int radius) {
        super(parent, id, type, name, x, y, radius);
    }

    void paint() {
        parent.circle(x, y, radius, false, displayName());
    }
}


/*
 * Button
 */
class BButton extends GraphicalThing {
    BubbleClient parent;
    String text;
    int x;
    int y;

    public BButton(BubbleClient parent, String text, int x, int y) {
        this.parent = parent;
        this.text = text;
        this.x = x;
        this.y = y;
    }

    void paint() {
        parent.box(x, y, parent.boxWidth, parent.boxHeight, text);
    }

    boolean intersects(int x, int y) {
        return (Math.abs(this.x - x) < parent.boxWidth/2 &&
                Math.abs(this.y - y) < parent.boxHeight/2);
    }

    void drag(int x, int y) {
        click();
    }

    void click() {
        System.out.println(text);
    }
}
