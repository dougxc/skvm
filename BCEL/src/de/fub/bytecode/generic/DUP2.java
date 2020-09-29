package de.fub.bytecode.generic;

/** 
 * DUP2 - Duplicate two top operand stack words
 * <PRE>Stack: ..., word2, word1 -&gt; ..., word2, word1, word2, word1</PRE>
 *
 * @version $Id: DUP2.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class DUP2 extends StackInstruction implements PushInstruction {
  public DUP2() {
    super(de.fub.bytecode.Constants.DUP2);
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
    v.visitPushInstruction(this);
    v.visitStackInstruction(this);
    v.visitDUP2(this);
  }
}
