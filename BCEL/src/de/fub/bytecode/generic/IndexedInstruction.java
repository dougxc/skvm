package de.fub.bytecode.generic;

/**
 * Denote entity that refers to an index, e.g. local variable instructions,
 * RET, CPInstruction, etc.
 *
 * @version $Id: IndexedInstruction.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface IndexedInstruction {
  public int getIndex();
  public void setIndex(int index);
}

