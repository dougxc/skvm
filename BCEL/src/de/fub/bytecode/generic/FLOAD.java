package de.fub.bytecode.generic;

/** 
 * FLOAD - Load float from local variable
 * <PRE>Stack ... -&gt; ..., result</PRE>
 *
 * @version $Id: FLOAD.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class FLOAD extends LocalVariableInstruction
  implements PushInstruction, LoadInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  FLOAD() {
    super(de.fub.bytecode.Constants.FLOAD, de.fub.bytecode.Constants.FLOAD_0);
  }

  /** Load float from local variable
   * @param n index of local variable
   */
  public FLOAD(int n) {
    super(de.fub.bytecode.Constants.FLOAD, de.fub.bytecode.Constants.FLOAD_0, n);
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
    v.visitFLOAD(this);
  }
}
