package de.fub.bytecode.generic;
import java.io.*;

/** 
 * JSR - Jump to subroutine
 *
 * @version $Id: JSR.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class JSR extends JsrInstruction implements VariableLengthInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  JSR() {}

  public JSR(InstructionHandle target) {
    super(de.fub.bytecode.Constants.JSR, target);
  }

  /**
   * Dump instruction as byte code to stream out.
   * @param out Output stream
   */
  public void dump(DataOutputStream out) throws IOException {
    index = getTargetOffset();
    if(tag == de.fub.bytecode.Constants.JSR)
      super.dump(out);
    else { // JSR_W
      index = getTargetOffset();
      out.writeByte(tag);
      out.writeInt(index);
    }
  }

  protected int updatePosition(int offset, int max_offset) {
    int i = getTargetOffset(); // Depending on old position value

    position += offset; // Position may be shifted by preceding expansions

    if(Math.abs(i) >= (32767 - max_offset)) { // to large for short (estimate)
      tag    = de.fub.bytecode.Constants.JSR_W;
      length = 5;
      return 2; // 5 - 3
    }

    return 0;
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
    v.visitVariableLengthInstruction(this);
    v.visitInstructionTargeter(this);
    v.visitBranchInstruction(this);
    v.visitJsrInstruction(this);
    v.visitJSR(this);
  }
}
