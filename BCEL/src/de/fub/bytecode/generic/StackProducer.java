package de.fub.bytecode.generic;

/**
 * Denote an instruction that may produce a value on top of the stack
 * (this excludes DUP_X1, e.g.)
 *
 * @version $Id: StackProducer.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface StackProducer {
  /** @return how many words are produced on stack
   */
  public int produceStack(ConstantPoolGen cpg);
}

