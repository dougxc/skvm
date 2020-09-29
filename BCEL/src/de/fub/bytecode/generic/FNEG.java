package de.fub.bytecode.generic;

/** 
 * FNEG - Negate float
 * <PRE>Stack: ..., value -&gt; ..., result</PRE>
 *
 * @version $Id: FNEG.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class FNEG extends ArithmeticInstruction {
  public FNEG() {
    super(de.fub.bytecode.Constants.FNEG);
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
    v.visitFNEG(this);
  }
}
