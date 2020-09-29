package de.fub.bytecode.generic;

/**
 * Get the type associated with an instruction, int for ILOAD, or the type
 * of the field of a PUTFIELD instruction, e.g..
 *
 * @version $Id: TypedInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface TypedInstruction {
  public Type getType(ConstantPoolGen cpg);
}

