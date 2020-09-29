package de.fub.bytecode.generic;

/** 
 * DLOAD - Load double from local variable
 * <PRE>Stack ... -&gt; ..., result.word1, result.word2</PRE>
 *
 * @version $Id: DLOAD.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class DLOAD extends LocalVariableInstruction
  implements PushInstruction, LoadInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  DLOAD() {
    super(de.fub.bytecode.Constants.DLOAD, de.fub.bytecode.Constants.DLOAD_0);
  }

  /** Load double from local variable
   * @param n index of local variable
   */
  public DLOAD(int n) {
    super(de.fub.bytecode.Constants.DLOAD, de.fub.bytecode.Constants.DLOAD_0, n);
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
    v.visitDLOAD(this);
  }
}
