package de.fub.bytecode.generic;

/** 
 * I2C - Convert int to char
 * <PRE>Stack: ..., value -&gt; ..., result</PRE>
 *
 * @version $Id: I2C.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class I2C extends ConversionInstruction {
  /** Convert int to char
   */
  public I2C() {
    super(de.fub.bytecode.Constants.I2C);
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
    v.visitConversionInstruction(this);
    v.visitI2C(this);
  }
}
