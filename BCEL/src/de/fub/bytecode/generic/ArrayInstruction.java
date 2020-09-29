package de.fub.bytecode.generic;

/**
 * Super class for instructions dealing with array access such as IALOAD.
 *
 * @version $Id: ArrayInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public abstract class ArrayInstruction extends Instruction
  implements ExceptionThrower, TypedInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ArrayInstruction() {}

  /**
   * @param tag opcode of instruction
   */
  protected ArrayInstruction(short tag) {
    super(tag, (short)1);
  }

  public Class[] getExceptions() {
    return de.fub.bytecode.ExceptionConstants.EXCS_ARRAY_EXCEPTION;
  }

  /** @return type associated with the instruction
   */
  public Type getType(ConstantPoolGen cp) {
    switch(tag) {
    case de.fub.bytecode.Constants.IALOAD: case de.fub.bytecode.Constants.IASTORE: 
      return Type.INT;
    case de.fub.bytecode.Constants.CALOAD: case de.fub.bytecode.Constants.CASTORE: 
      return Type.CHAR;
    case de.fub.bytecode.Constants.BALOAD: case de.fub.bytecode.Constants.BASTORE: 
      return Type.BYTE;
    case de.fub.bytecode.Constants.LALOAD: case de.fub.bytecode.Constants.LASTORE: 
      return Type.LONG;
    case de.fub.bytecode.Constants.DALOAD: case de.fub.bytecode.Constants.DASTORE: 
      return Type.DOUBLE;
    case de.fub.bytecode.Constants.FALOAD: case de.fub.bytecode.Constants.FASTORE: 
      return Type.FLOAT;
    case de.fub.bytecode.Constants.AALOAD: case de.fub.bytecode.Constants.AASTORE:
      return Type.OBJECT;

    default: throw new ClassGenException("Oops: unknown case in switch" + tag);
    }
  }
}
