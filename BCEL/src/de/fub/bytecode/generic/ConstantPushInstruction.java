package de.fub.bytecode.generic;

/**
 * Denotes a push instruction that produces a literal on the stack
 * such as  SIPUSH, BIPUSH, ICONST, etc.
 *
 * @version $Id: ConstantPushInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>

 * @see ICONST
 * @see SIPUSH
 */
public interface ConstantPushInstruction extends PushInstruction, TypedInstruction {
  public Number getValue();
}

