package de.fub.bytecode.generic;

/**
 * POP - Pop top operand stack word
 *
 * <PRE>Stack: ..., word -&gt; ...</PRE>
 *
 * @version $Id: POP.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class POP extends StackInstruction implements PopInstruction {
  public POP() {
    super(de.fub.bytecode.Constants.POP);
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
    v.visitStackConsumer(this);
    v.visitPopInstruction(this);
    v.visitStackInstruction(this);
    v.visitPOP(this);
  }
}
