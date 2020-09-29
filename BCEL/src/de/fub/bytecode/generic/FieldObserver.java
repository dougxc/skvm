package de.fub.bytecode.generic;

/**
 * Imnplement this interface if you're interested in changes to a FieldGen object
 * and register yourself with addObserver().
 *
 * @version $Id: FieldObserver.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface FieldObserver {
  public void notify(FieldGen field);
}

