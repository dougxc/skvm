package de.fub.bytecode.generic;

/** 
 * ILOAD - Load int from local variable onto stack
 * <PRE>Stack: ... -&gt; ..., result</PRE>
 *
 * @version $Id: ILOAD.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class ILOAD extends LocalVariableInstruction
  implements PushInstruction, LoadInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ILOAD() {
    super(de.fub.bytecode.Constants.ILOAD, de.fub.bytecode.Constants.ILOAD_0);
  }

  /** Load int from local variable
   * @param n index of local variable
   */
  public ILOAD(int n) {
    super(de.fub.bytecode.Constants.ILOAD, de.fub.bytecode.Constants.ILOAD_0, n);
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
    v.visitStackProducer(this);
    v.visitPushInstruction(this);
    v.visitLoadInstruction(this);
    v.visitTypedInstruction(this);
    v.visitLocalVariableInstruction(this);
    v.visitILOAD(this);
  }
}
