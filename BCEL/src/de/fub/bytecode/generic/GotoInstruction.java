package de.fub.bytecode.generic;

/** 
 * Super class for GOTO
 *
 * @version $Id: GotoInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public abstract class GotoInstruction extends BranchInstruction
  implements UnconditionalBranch
{
  GotoInstruction(short tag, InstructionHandle target) {
    super(tag, target);
  }

  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  GotoInstruction(){}
}
