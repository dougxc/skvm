package de.fub.bytecode.generic;
import java.io.*;
import de.fub.bytecode.ExceptionConstants;

/** 
 * NEW - Create new object
 * <PRE>Stack: ... -&gt; ..., objectref</PRE>
 *
 * @version $Id: NEW.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class NEW extends CPInstruction
  implements LoadClass, AllocationInstruction, ExceptionThrower, StackProducer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  NEW() {}

  public NEW(int index) {
    super(de.fub.bytecode.Constants.NEW, index);
  }

  public Class[] getExceptions(){
    Class[] cs = new Class[2 + ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length];

    System.arraycopy(ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION, 0,
		     cs, 0, ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length);

    cs[ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length+1] = ExceptionConstants.INSTANTIATION_ERROR;
    cs[ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length]   = ExceptionConstants.ILLEGAL_ACCESS_ERROR;

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
    v.visitNEW(this);
  }
}
