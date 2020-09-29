package de.fub.bytecode.generic;

/**
 * Denote an instruction that may consume a value from the stack.
 *
 * @version $Id: StackConsumer.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface StackConsumer {
  /** @return how many words are consumed from stack
   */
  public int consumeStack(ConstantPoolGen cpg);
}

