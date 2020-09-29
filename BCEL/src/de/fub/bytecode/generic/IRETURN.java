package de.fub.bytecode.generic;

/** 
 * IRETURN -  Return int from method
 * <PRE>Stack: ..., value -&gt; &lt;empty&gt;</PRE>
 *
 * @version $Id: IRETURN.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class IRETURN extends ReturnInstruction {
  /** Return int from method
   */
  public IRETURN() {
    super(de.fub.bytecode.Constants.IRETURN);
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
    v.visitIRETURN(this);
  }
}
