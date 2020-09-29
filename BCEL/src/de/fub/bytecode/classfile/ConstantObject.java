package de.fub.bytecode.classfile;

/** 
 * This interface denotes those constants that have a "natural" value,
 * such as ConstantLong, ConstantString, etc..
 *
 * @version $Id: ConstantObject.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 * @see     Constant
 */
public interface ConstantObject {
  /** @return object representing the constant, e.g., Long for ConstantLong
   */
  public abstract Object getConstantValue(ConstantPool cp);
}
