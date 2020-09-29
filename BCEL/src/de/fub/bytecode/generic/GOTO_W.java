package de.fub.bytecode.generic;
import java.io.*;
import de.fub.bytecode.util.ByteSequence;

/** 
 * GOTO_W - Branch always (to relative offset, not absolute address)
 *
 * @version $Id: GOTO_W.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author  <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A>
 */
public class GOTO_W extends GotoInstruction implements UnconditionalBranch {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  GOTO_W() {}

  public GOTO_W(InstructionHandle target) {
    super(de.fub.bytecode.Constants.GOTO_W, target);
    length = 5;
  }

  /**
   * Dump instruction as byte code to stream out.
   * @param out Output stream
   */
  public void dump(DataOutputStream out) throws IOException {
    index = getTargetOffset();
    out.writeByte(tag);
    out.writeInt(index);
  }

  /**
   * Read needed data (e.g. index) from file.
   */
  protected void initFromFile(ByteSequence bytes, boolean wide) throws IOException
  {
    index  = bytes.readInt();
    length = 5;
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
    v.visitUnconditionalBranch(this);
    v.visitInstructionTargeter(this);
    v.visitBranchInstruction(this);
    v.visitGotoInstruction(this);
    v.visitGOTO_W(this);
  }
}
