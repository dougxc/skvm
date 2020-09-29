package de.fub.bytecode.generic;

/** 
 * INSTANCEOF - Determine if object is of given type
 * <PRE>Stack: ..., objectref -&gt; ..., result</PRE>
 *
 * @version $Id: INSTANCEOF.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class INSTANCEOF extends CPInstruction
  implements LoadClass, ExceptionThrower, StackProducer, StackConsumer {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  INSTANCEOF() {}

  public INSTANCEOF(int index) {
    super(de.fub.bytecode.Constants.INSTANCEOF, index);
  }

  public Class[] getExceptions() {
    return de.fub.bytecode.ExceptionConstants.EXCS_CLASS_AND_INTERFACE_RESOLUTION;
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
    v.visitINSTANCEOF(this);
  }
}
