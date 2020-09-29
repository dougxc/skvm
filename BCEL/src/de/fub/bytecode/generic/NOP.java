package de.fub.bytecode.generic;

/**
 * NOP - Do nothing
 *
 * @version $Id: NOP.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class NOP extends Instruction {
  public NOP() {
    super(de.fub.bytecode.Constants.NOP, (short)1);
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
    v.visitNOP(this);
  }
}
