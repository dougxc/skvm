package de.fub.bytecode.util;
import java.io.*;
import de.fub.bytecode.*;
import de.fub.bytecode.classfile.*;
import de.fub.bytecode.generic.*;

/**
 * This class imply tests whether or not the BCEL classes serialize a class
 * in exactly the same form it was loaded.
 */

class InOutException extends RuntimeException {
    int pos;
    InOutException(int p) { pos = p; }
}

public class InOut extends OutputStream {

  byte[] o;
  int pos = 0;
  InOut(byte[] original) {
      o = original;
  }
  public void write(int b) throws IOException {
    if (o[pos++] != (byte)b) {
      throw new InOutException(pos);
    }
  }

  public static void main(String args[]) throws Exception {
    for (int i = 0; i != args.length; ++i) {
      File file = new File(args[i]);
      FileInputStream fis = new FileInputStream(file);
      byte original[] = new byte[fis.available()];
      fis.read(original);
      System.err.println(args[i]+": "+original.length);

      ClassParser parser = new ClassParser(args[i]);

      JavaClass clazz = parser.parse();
      try {
          clazz.dump(new InOut(original));
      } catch (InOutException e) {
            System.err.println("JavaClass.dump output differs at byte " +e.pos);
            e.printStackTrace();
      }

      ClassGen gen = new ClassGen(clazz);
      try {
          gen.getJavaClass().dump(new InOut(original));
      } catch (InOutException e) {
            System.err.println("GenClass.getJavaClass.dump output differs at byte " +e.pos);
            e.printStackTrace();
      }

    }
  }
}
