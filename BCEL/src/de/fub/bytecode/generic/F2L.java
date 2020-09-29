package de.fub.bytecode.generic;

/** 
 * F2L - Convert float to long
 * <PRE>Stack: ..., value -&gt; ..., result.word1, result.word2</PRE>
 *
 * @version $Id: F2L.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class F2L extends ConversionInstruction {
  /** Convert float to long
   */
  public F2L() {
    super(de.fub.bytecode.Constants.F2L);
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
    v.visitF2L(this);
  }
}
