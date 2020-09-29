package de.fub.bytecode.generic;

/**
 * IMPDEP2 - Implementation dependent
 *
 * @version $Id: IMPDEP2.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class IMPDEP2 extends Instruction {
  public IMPDEP2() {
    super(de.fub.bytecode.Constants.IMPDEP2, (short)1);
  }


  /**
   * Call corresponding visitor method(s). The order is:
   * Call visitor methods of implemented interfaces first, then
   * call methods according to the class hierarchy in descending order,
   * i.e., the most specific visitXXX() call comes last.
   *
   * @param v Visitor object
   */
  public void accept(Visitor v) {
    v.visitIMPDEP2(this);
  }
}
