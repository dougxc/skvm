package de.fub.bytecode.generic;
import de.fub.bytecode.Constants;
import de.fub.bytecode.ExceptionConstants;

/**
 * Super class for the xRETURN family of instructions.
 *
 * @version $Id: ReturnInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public abstract class ReturnInstruction extends Instruction
  implements ExceptionThrower, TypedInstruction, StackConsumer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ReturnInstruction() {}

  /**
   * @param tag opcode of instruction
   */
  protected ReturnInstruction(short tag) {
    super(tag, (short)1);
  }

  public Type getType() {
    switch(tag) {
      case Constants.IRETURN: return Type.INT;
      case Constants.LRETURN: return Type.LONG;
      case Constants.FRETURN: return Type.FLOAT;
      case Constants.DRETURN: return Type.DOUBLE;
      case Constants.ARETURN: return Type.OBJECT;
      case Constants.RETURN:  return Type.VOID;
 
    default: // Never reached
      throw new ClassGenException("Unknown type " + tag);
    }
  }

  public Class[] getExceptions() {
    return new Class[] { ExceptionConstants.ILLEGAL_MONITOR_STATE };
  }

  /** @return type associated with the instruction
   */
  public Type getType(ConstantPoolGen cp) {
    return getType();
  }
}

