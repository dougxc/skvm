package de.fub.bytecode.generic;

/** 
 * IFEQ - Branch if int comparison with zero succeeds
 *
 * <PRE>Stack: ..., value -&gt; ...</PRE>
 *
 * @version $Id: IFEQ.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class IFEQ extends IfInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  IFEQ() {}

  public IFEQ(InstructionHandle target) {
    super(de.fub.bytecode.Constants.IFEQ, target);
  }

  /**
   * @return negation of instruction, e.g. IFEQ.negate() == IFNE
   */
  public IfInstruction negate() {
    return new IFNE(target);
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
    v.visitInstructionTargeter(this);
    v.visitBranchInstruction(this);
    v.visitIfInstruction(this);
    v.visitIFEQ(this);
  }
}
