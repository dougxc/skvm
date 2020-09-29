package de.fub.bytecode.generic;

/** 
 * FSTORE - Store float into local variable
 * <PRE>Stack: ..., value -&gt; ... </PRE>
 *
 * @version $Id: FSTORE.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class FSTORE extends LocalVariableInstruction
  implements PopInstruction, StoreInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  FSTORE() {
    super(de.fub.bytecode.Constants.FSTORE, de.fub.bytecode.Constants.FSTORE_0);
  }

  /** Store float into local variable
   * @param n index of local variable
   */
  public FSTORE(int n) {
    super(de.fub.bytecode.Constants.FSTORE, de.fub.bytecode.Constants.FSTORE_0, n);
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
    v.visitStackConsumer(this);
    v.visitPopInstruction(this);
    v.visitStoreInstruction(this);
    v.visitTypedInstruction(this);
    v.visitLocalVariableInstruction(this);
    v.visitFSTORE(this);
  }
}
