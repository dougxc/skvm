package de.fub.bytecode.generic;

/** 
 * FCMPL - Compare floats: value1 < value2
 * <PRE>Stack: ..., value1, value2 -&gt; ..., result</PRE>
 *
 * @version $Id: FCMPL.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class FCMPL extends Instruction
  implements TypedInstruction, StackProducer, StackConsumer {
  public FCMPL() {
    super(de.fub.bytecode.Constants.FCMPL, (short)1);
  }

  /** @return Type.FLOAT
   */
  public Type getType(ConstantPoolGen cp) {
    return Type.FLOAT;
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
    v.visitTypedInstruction(this);
    v.visitStackProducer(this);
    v.visitStackConsumer(this);
    v.visitFCMPL(this);
  }
}
