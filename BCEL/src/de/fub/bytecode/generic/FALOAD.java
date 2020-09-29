package de.fub.bytecode.generic;

/** 
 * FALOAD - Load float from array
 * <PRE>Stack: ..., arrayref, index -&gt; ..., value</PRE>
 *
 * @version $Id: FALOAD.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class FALOAD extends ArrayInstruction implements StackProducer {
  /** Load float from array
   */
  public FALOAD() {
    super(de.fub.bytecode.Constants.FALOAD);
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
    v.visitStackProducer(this);
    v.visitExceptionThrower(this);
    v.visitTypedInstruction(this);
    v.visitArrayInstruction(this);
    v.visitFALOAD(this);
  }
}
