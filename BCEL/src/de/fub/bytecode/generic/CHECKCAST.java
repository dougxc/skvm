package de.fub.bytecode.generic;
import de.fub.bytecode.ExceptionConstants;
/** 
 * CHECKCAST - Check whether object is of given type
 * <PRE>Stack: ..., objectref -&gt; ..., objectref</PRE>
 *
 * @version $Id: CHECKCAST.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class CHECKCAST extends CPInstruction
  implements LoadClass, ExceptionThrower, StackProducer, StackConsumer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  CHECKCAST() {}

  /** Check whether object is of given type
   * @param n index to class in constant pool
   */
  public CHECKCAST(int index) {
    super(de.fub.bytecode.Constants.CHECKCAST, index);
  }

  /** @return exceptions this instruction may cause
   */
  public Class[] getExceptions() {
    Class[] cs = new Class[1 + ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length];

    System.arraycopy(ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION, 0,
		     cs, 0, ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length);
    cs[ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION.length] =
      ExceptionConstants.CLASS_CAST_EXCEPTION;
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
    v.visitExceptionThrower(this);
    v.visitStackProducer(this);
    v.visitStackConsumer(this);
    v.visitTypedInstruction(this);
    v.visitCPInstruction(this);
    v.visitCHECKCAST(this);
  }
}
