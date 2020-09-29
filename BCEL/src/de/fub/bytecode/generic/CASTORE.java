package de.fub.bytecode.generic;

/** 
 * CASTORE -  Store into char array
 * <PRE>Stack: ..., arrayref, index, value -&gt; ...</PRE>
 *
 * @version $Id: CASTORE.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class CASTORE extends ArrayInstruction implements StackConsumer {
  /** Store char into array
   */
  public CASTORE() {
    super(de.fub.bytecode.Constants.CASTORE);
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
    v.visitStackConsumer(this);
    v.visitExceptionThrower(this);
    v.visitTypedInstruction(this);
    v.visitArrayInstruction(this);
    v.visitCASTORE(this);
  }
}
