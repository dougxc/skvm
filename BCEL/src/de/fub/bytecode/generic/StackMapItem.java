package de.fub.bytecode.generic;
import de.fub.bytecode.Constants;
import de.fub.bytecode.Repository;
import de.fub.bytecode.classfile.JavaClass;

/** 
 * This class encapsulates an item found in the stack map of an
 * instruction that models the local variable and operand stack's
 * form (size and types) at runtime for the instruction.
 *
 * @author  <A HREF="mailto:Douglas.Simon@Eng.Sun.COM">Doug Simon</A>
 * @see StackMapEntry
 * @see StackMapGen
 */
public final class StackMapItem {

	private byte tag;
	ReferenceType  refType;
	InstructionHandle newInst;
	
	/**
	 * This is a convenience constructor to simply initialising a StackMapItem
	 * from a stack map in a parsed classfile.
	 * @param ty the tag as found in a class file
	 * @param method the method in which this stack map item will belong
	 * @param il instruction list
	 */
	public StackMapItem(int ty, MethodGen method)
	{
		ConstantPoolGen cpGen = method.getConstantPool();
		InstructionList il = method.getInstructionList();
		
		if (ty > Constants.ITEM_InitObject)
		{
			tag = (byte)(ty >> 16);
			if (tag == Constants.ITEM_Object)
			{
				int index = ty & 0xFFFF;
				refType =  (ReferenceType)Type.getType(cpGen.getConstantPool().
					      getConstantString(index, Constants.CONSTANT_Class));
			}
			else if (tag == Constants.ITEM_NewObject)
			{
				int offset = ty & 0xFFFF;
				newInst = il.findHandle(offset);
			}
			else
				throw new ClassFormatError("Unknown stack tag `"+tag+"'");
		}
		else
			tag = (byte)ty;
	}
	 
  /**
   * @param tag the ITEM_* tag in Constants.java
	 * @param refType the reference type of the object on the stack when
	 *        `tag' == ITEM_Object
	 * @param newInst the offset to the new instruction when 
	 *        `tag' == ITEM_NewObject
   */ 
  public StackMapItem(byte tag, ReferenceType refType, InstructionHandle newInst) {
	  this.tag = tag;
		this.refType = refType;
		this.newInst = newInst;
		if (tag == Constants.ITEM_Object && refType == null)
			throw new ClassFormatError("Missing class tag for `Object' stack tag");
		if (tag == Constants.ITEM_NewObject && newInst == null)
			throw new ClassFormatError("Missing `new' instruction handle for `NewObject' stack tag");
  }

  /**
	 * @return the `new' instruction that created this ITEM_NewObject or null
   */
  public InstructionHandle getNewInstruction() { return newInst; }

  /**
   * Set the `new' instruction for this ITEM_NewObject stack tag.
   * @param ih
   */
  public void setNewInstruction(InstructionHandle ih)
  {
    if (tag != Constants.ITEM_NewObject)
      throw new ClassFormatError("Cannot set `new' instruction handle for stack tag `"+
        Constants.ITEM_NAMES[tag]+"'");
    newInst = ih;
  }
  
  /**
	 * @return name of referenced class or null if this is not an ITEM_Object tag
   */
  public ReferenceType getReferenceType() {	return refType; }

	/** @return stack tag
	 */
	public byte getTag() { return tag; }
}
