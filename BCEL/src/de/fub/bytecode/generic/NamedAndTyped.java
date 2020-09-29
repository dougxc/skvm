package de.fub.bytecode.generic;

/**
 * Denote entity that has both name and type. This is true for local variables,
 * methods and fields.
 *
 * @version $Id: NamedAndTyped.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public interface NamedAndTyped {
  public String getName();
  public Type   getType();
  public void   setName(String name);
  public void   setType(Type type);

}

