package de.fub.bytecode.generic;

import de.fub.bytecode.Constants;
import de.fub.bytecode.classfile.*;

/** 
 * This class represents an entry in a StackMap for a method. The entry
 * encapsulates the runtime form (size and types) of local variable and
 * operand stacks at a given instruction.
 *
 * @author  <A HREF="mailtp:Douglas.Simon@Eng.Sun.COM">Doug Simon</A>
 * @see     StackMapItem
 * @see     MethodGen
 * @see     StackMapEntry
 */
public class StackMapEntryGen implements InstructionTargeter, Cloneable {
  private StackMapItem[] locals;
  private StackMapItem[] stack_items;
  private InstructionHandle inst;

	 
  /**
   * Generate a stack map entry for instruction as `inst'.
   *
   * @param inst
   * @param locals
   * @param stack_items
   */
  public StackMapEntryGen(InstructionHandle inst, StackMapItem[] locals,
		StackMapItem[] stack_items)
	{  
    this.inst  = inst;
		if (locals == null)
			locals = new StackMapItem[0];
		if (stack_items == null)
			stack_items = new StackMapItem[0];
		setLocals(locals);
		setStackItems(stack_items);
  }

	/**
	 * Helper to generate an array of types using the encoding described in
	 * StackMap.
	 * @param types
	 * @param cp
	 */
	private final int[] getEncodedTypes(StackMapItem[] types, ConstantPoolGen cp)
	{
		int[] enc_types = new int[types.length];
		for (int i = 0; i < enc_types.length; i++)
		{
			StackMapItem type = types[i];
			byte tag = type.getTag();
			if (tag == Constants.ITEM_Object)
			{
				// need to distinguish between ArrayType and ObjectType as the class
				// name is retrieved differently
				ReferenceType refType = type.getReferenceType();
				String className;
				if (refType instanceof ArrayType)
					className = refType.getSignature();
				else
					className = ((ObjectType)refType).getClassName();
				int index = cp.addClass(className);
				enc_types[i] = StackMapEntry.getEncodedType(tag,index);
			}
			else if (tag == Constants.ITEM_NewObject)
			{
				int index = type.getNewInstruction().getPosition();
				enc_types[i] = StackMapEntry.getEncodedType(tag,index);
			}
			else
				enc_types[i] = tag;
		}
		return enc_types;
	}
	 
  /**
   * Get StackMapEntry object.
   *
   * This relies on that the instruction list has already been dumped
	 * to byte code or or that the `setPositions' methods has been called
	 * for the instruction list.
   *
   * @param cp constant pool
   */
  public StackMapEntry getStackMapEntry(ConstantPoolGen cp) {
	
    int byte_code_offset       = inst.getPosition();
		int[] types_of_locals      = getEncodedTypes(locals,cp);
		int[] types_of_stack_items = getEncodedTypes(stack_items,cp);

    return new StackMapEntry(byte_code_offset, types_of_locals,
			types_of_stack_items);
  }

	/**
	 * Set the instruction handle for this stack map entry.
	 * @param inst
	 */
  public void setInstruction(InstructionHandle inst) {
    BranchInstruction.notifyTarget(this.inst, inst, this);
		this.inst = inst;
	}
	
	/**
	 * Get the instruction handle for this stack map entry.
	 */
  public InstructionHandle getInstruction() { return inst; }
	
  public void setLocals(StackMapItem[] locals)
	{
		notifyTargets(this.locals,locals);
		this.locals = locals;
	}
  public StackMapItem[]    getLocals()  { return locals; }
  public void setStackItems(StackMapItem[] stack_items)
	{
		notifyTargets(this.stack_items,stack_items);
		this.stack_items = stack_items;
	}
  public StackMapItem[]    getStackItems() { return stack_items; }
	
  /**
   * @param old_ih old target, either start or end
   * @param new_ih new target
   */
  public void updateTarget(InstructionHandle old_ih, InstructionHandle new_ih)
	{
    boolean targeted = false;

    if(inst == old_ih) {
      targeted = true;
      setInstruction(new_ih);
    }

		targeted |= findAndUpdateTargets(locals,old_ih,new_ih,true);
		targeted |= findAndUpdateTargets(stack_items,old_ih,new_ih,true);
		
    if(!targeted)
      throw new ClassGenException("Not targeting " + old_ih);
  }

  /**
   * @return true, if ih is target of this variable
   */
  public boolean containsTarget(InstructionHandle ih) {
    return (inst == ih) || findAndUpdateTargets(locals,ih,null,false) ||
			findAndUpdateTargets(stack_items,ih,null,false);
  }
	
	/**
	 * Search for an instances of the <em>old_ih</em> in <em>types</em>. If
	 * instances are found and <em>doUpdate</em> is true, then replace them
	 * with <em>new_ih</em>.
	 * @param types
	 * @param old_ih
	 * @param new_ih
	 * @param doUpdate
	 * @return true if <em>old_ih</em> was within <em>types</em>
	 */
	private final boolean findAndUpdateTargets(StackMapItem[] types,
		InstructionHandle old_ih, InstructionHandle new_ih, boolean doUpdate)
	{
		boolean result = false;
		for (int i = 0; i < types.length; i++)
		{
			StackMapItem type = types[i];
			InstructionHandle newInstruction = type.getNewInstruction();
			if (newInstruction != null && newInstruction == old_ih)
			{
				result = true;
				if (doUpdate)
				{
					type.setNewInstruction(new_ih);
					old_ih.removeTargeter(this);
					new_ih.addTargeter(this);
				}
			}
		}
		return result;
	}
	
	/**
	 * Helper to register this object (an InstructionTargeter) with any
	 * instructions inside the type maps of this stack map entry. This object
	 * is also deregistered from any existing instructions.
	 *
	 * @param oldTypes
	 * @param newTypes
	 */
	private void notifyTargets(StackMapItem[] oldTypes, StackMapItem[] newTypes)
	{
		// first remove old targeted instructions
		if (oldTypes != null)
		{
			for (int i = 0; i < oldTypes.length; i++)
			{
				StackMapItem type = oldTypes[i];
				InstructionHandle newInstruction = type.getNewInstruction();
				if (newInstruction != null)
					newInstruction.removeTargeter(this);
			}
		}
		// now notify the new targeted instructions
		for (int i = 0; i < newTypes.length; i++)
		{
			StackMapItem type = newTypes[i];
			InstructionHandle newInstruction = type.getNewInstruction();
			if (newInstruction != null)
				newInstruction.addTargeter(this);
		}
	}

  public String toString() {
    return "StackMapEntryGen(" + inst +  ", locals(" + locals.length +
			"), stack_items(" + stack_items.length + ")";
  }

  public Object clone() {
    try {
      return super.clone();
    } catch(CloneNotSupportedException e) {
      System.err.println(e);
      return null;
    }
  }

	/**
	 * Helper to calculate the number of bytes an array of stack types would
	 * occupy in a classfile.
	 * @param types
	 * @return the number of bytes
	 */
	private final int getSizeInBytes(StackMapItem[] types)
	{
		int size = 0;
		for (int i = 0; i < types.length; i++)
		{
			if (types[i].getTag() == Constants.ITEM_NewObject ||
				  types[i].getTag() == Constants.ITEM_Object)
				size += 3;
			else
				size += 1;
		}
		return size;
	}
	/**
	 * @return the size (in bytes) that this stack map entry would occupy in
	 *         a class file
	 */
	public int getSize()
	{
		return 6 + getSizeInBytes(locals) + getSizeInBytes(stack_items);
	}

	/**
	 * Equality relationship is determined by matching byte code offsets.
	 */
	public boolean equals(Object o)
	{
		if (!(o instanceof StackMapEntryGen))
			return false;
		InstructionHandle inst = ((StackMapEntryGen)o).inst;
		return (inst != null) && (inst.getPosition() == this.inst.getPosition());
	}
}
