package de.fub.bytecode.generic;

/** 
 * DCONST - Push 0.0 or 1.0, other values cause an exception
 *
 * <PRE>Stack: ... -&gt; ..., <i></PRE>
 *
 * @version $Id: DCONST.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class DCONST extends Instruction
  implements ConstantPushInstruction, TypedInstruction {
  private double value;

  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  DCONST() {}

  public DCONST(double f) {
    super(de.fub.bytecode.Constants.DCONST_0, (short)1);

    if(f == 0.0)
      tag = de.fub.bytecode.Constants.DCONST_0;
    else if(f == 1.0)
      tag = de.fub.bytecode.Constants.DCONST_1;
    else
      throw new ClassGenException("DCONST can be used only for 0.0 and 1.0: " + f);

    value = f;
  }

  public Number getValue() { return new Double(value); }

  /** @return Type.DOUBLE
   */
  public Type getType(ConstantPoolGen cp) {
    return Type.DOUBLE;
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
    v.visitPushInstruction(this);
    v.visitStackProducer(this);
    v.visitTypedInstruction(this);
    v.visitConstantPushInstruction(this);
    v.visitDCONST(this);
  }
}
