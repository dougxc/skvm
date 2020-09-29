package de.fub.bytecode.generic;

/**
 * Super class for stack operations like DUP and POP.
 *
 * @version $Id: StackInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public abstract class StackInstruction extends Instruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  StackInstruction() {}

  /**
   * @param tag instruction opcode
   */
  protected StackInstruction(short tag) {
    super(tag, (short)1);
  }

  /** @return Type.UNKNOWN
   */
  public Type getType(ConstantPoolGen cp) {
    return Type.UNKNOWN;
  }  
}

