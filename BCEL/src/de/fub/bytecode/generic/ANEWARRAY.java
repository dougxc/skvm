package de.fub.bytecode.generic;
import de.fub.bytecode.ExceptionConstants;

/** 
 * ANEWARRAY -  Create new array of references
 * <PRE>Stack: ..., count -&gt; ..., arrayref</PRE>
 *
 * @version $Id: ANEWARRAY.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class ANEWARRAY extends CPInstruction
  implements LoadClass, AllocationInstruction, ExceptionThrower, StackProducer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ANEWARRAY() {}

  public ANEWARRAY(int index) {
    super(de.fub.bytecode.Constants.ANEWARRAY, index);
  }

  public Class[] getExceptions(){
    Class[] cs = new Class[1 + ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length];

    System.arraycopy(ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION, 0,
		     cs, 0, ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length);
    cs[ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length] =
      ExceptionConstants.NEGATIVE_ARRAY_SIZE_EXCEPTION;
    return cs;
  }

  /**
   * Call corresponding visitor method(s). The order is:
   * Call visitor methods of implemented interfaces first, then
   * call methods according to the class hierarchy in descending order,
   * i.e., the most specific visitXXX() call comes last.
   *
   * @param v Visitor object
   */
  public void accept(Visitor v) {
    v.visitLoadClass(this);
    v.visitAllocationInstruction(this);
    v.visitExceptionThrower(this);
    v.visitStackProducer(this);
    v.visitTypedInstruction(this);
    v.visitCPInstruction(this);
    v.visitANEWARRAY(this);
  }
}
