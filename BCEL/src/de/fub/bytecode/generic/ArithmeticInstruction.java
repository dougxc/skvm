package de.fub.bytecode.generic;
import de.fub.bytecode.Constants;
/**
 * Super class for the family of arithmetic instructions.
 *
 * @version $Id: ArithmeticInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public abstract class ArithmeticInstruction extends Instruction
  implements TypedInstruction, StackProducer, StackConsumer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ArithmeticInstruction() {}

  /**
   * @param tag opcode of instruction
   */
  protected ArithmeticInstruction(short tag) {
    super(tag, (short)1);
  }

  /** @return type associated with the instruction
   */
  public Type getType(ConstantPoolGen cp) {
    switch(tag) {
    case Constants.DADD: case Constants.DDIV: case Constants.DMUL:
    case Constants.DNEG: case Constants.DREM: case Constants.DSUB:
      return Type.DOUBLE;

    case Constants.FADD: case Constants.FDIV: case Constants.FMUL:
    case Constants.FNEG: case Constants.FREM: case Constants.FSUB:
      return Type.FLOAT;

    case Constants.IADD: case Constants.IAND: case Constants.IDIV:
    case Constants.IMUL: case Constants.INEG: case Constants.IOR: case Constants.IREM:
    case Constants.ISHL: case Constants.ISHR: case Constants.ISUB:
    case Constants.IUSHR: case Constants.IXOR:
      return Type.INT;

    case Constants.LADD: case Constants.LAND: case Constants.LDIV:
    case Constants.LMUL: case Constants.LNEG: case Constants.LOR: case Constants.LREM:
    case Constants.LSHL: case Constants.LSHR: case Constants.LSUB:
    case Constants.LUSHR: case Constants.LXOR:
      return Type.LONG;

    default: // Never reached
      throw new ClassGenException("Unknown type " + tag);
    }
  }
}
