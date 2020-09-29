package de.fub.bytecode.generic;

/** 
 * IUSHR - Logical shift right int
 * <PRE>Stack: ..., value1, value2 -&gt; ..., result</PRE>
 *
 * @version $Id: IUSHR.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class IUSHR extends ArithmeticInstruction {
  public IUSHR() {
    super(de.fub.bytecode.Constants.IUSHR);
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
    v.visitIUSHR(this);
  }
}
