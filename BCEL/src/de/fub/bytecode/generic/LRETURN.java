package de.fub.bytecode.generic;

/** 
 * LRETURN -  Return long from method
 * <PRE>Stack: ..., value.word1, value.word2 -&gt; &lt;empty&gt;</PRE>
 *
 * @version $Id: LRETURN.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class LRETURN extends ReturnInstruction {
  public LRETURN() {
    super(de.fub.bytecode.Constants.LRETURN);
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
    v.visitExceptionThrower(this);
    v.visitTypedInstruction(this);
    v.visitStackConsumer(this);
    v.visitReturnInstruction(this);
    v.visitLRETURN(this);
  }
}
