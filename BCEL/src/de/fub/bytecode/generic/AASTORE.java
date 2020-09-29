package de.fub.bytecode.generic;

/** 
 * AASTORE -  Store into reference array
 * <PRE>Stack: ..., arrayref, index, value -&gt; ...</PRE>
 *
 * @version $Id: AASTORE.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class AASTORE extends ArrayInstruction implements StackConsumer {
  /** Store into reference array
   */
  public AASTORE() {
    super(de.fub.bytecode.Constants.AASTORE);
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
    v.visitAASTORE(this);
  }
}
