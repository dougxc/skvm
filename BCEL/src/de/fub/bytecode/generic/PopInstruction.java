package de.fub.bytecode.generic;

/**
 * Denotes an unparameterized instruction to pop a value on top from the stack,
 * such as ISTORE, POP, PUTSTATIC.
 *
 * @version $Id: PopInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 * @see ISTORE
 * @see POP
 */
public interface PopInstruction extends StackConsumer {
}

