package de.fub.bytecode.generic;

/**
 * Implement this interface if you're interested in changes to a ClassGen object
 * and register yourself with addObserver().
 *
 * @version $Id: ClassObserver.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface ClassObserver {
  public void notify(ClassGen clazz);
}

