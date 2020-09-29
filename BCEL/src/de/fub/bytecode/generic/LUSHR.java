package de.fub.bytecode.generic;

/** 
 * LUSHR - Logical shift right long
 * <PRE>Stack: ..., value1, value2 -&gt; ..., result</PRE>
 *
 * @version $Id: LUSHR.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class LUSHR extends ArithmeticInstruction {
  public LUSHR() {
    super(de.fub.bytecode.Constants.LUSHR);
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
    v.visitTypedInstruction(this);
    v.visitStackProducer(this);
    v.visitStackConsumer(this);
    v.visitArithmeticInstruction(this);
    v.visitLUSHR(this);
  }
}
