package de.fub.bytecode.generic;

/** 
 * D2I - Convert double to int
 * <PRE>Stack: ..., value.word1, value.word2 -&gt; ..., result</PRE>
 *
 * @version $Id: D2I.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class D2I extends ConversionInstruction {
  /** Convert double to int
   */
  public D2I() {
    super(de.fub.bytecode.Constants.D2I);
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
    v.visitConversionInstruction(this);
    v.visitD2I(this);
  }
}
