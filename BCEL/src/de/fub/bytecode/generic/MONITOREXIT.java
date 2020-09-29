package de.fub.bytecode.generic;

/** 
 * MONITOREXIT - Exit monitor for object
 * <PRE>Stack: ..., objectref -&gt; ...</PRE>
 *
 * @version $Id: MONITOREXIT.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class MONITOREXIT extends Instruction
  implements ExceptionThrower, StackConsumer {
  public MONITOREXIT() {
    super(de.fub.bytecode.Constants.MONITOREXIT, (short)1);
  }

  public Class[] getExceptions() {
    return new Class[] { de.fub.bytecode.ExceptionConstants.NULL_POINTER_EXCEPTION };
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
    v.visitExceptionThrower(this);
    v.visitStackConsumer(this);
    v.visitMONITOREXIT(this);
  }
}
