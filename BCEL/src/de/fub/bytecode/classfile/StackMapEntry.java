package de.fub.bytecode.classfile;

import  de.fub.bytecode.Constants;
import  java.io.*;

/**
 * This class represents a StackMap entry. This is generated by the J2ME
 * preverifier and is used for runtime verification as well as exact
 * garbage collection in KVM, the reference J2ME VM implementation.
 *
 * The types in a stack map are either 1 or 3 byte entries in the classfile.
 * Our internal representation is an integer for each entry. The low order
 * byte will store the tag of the type. If the type is of the 3 byte form
 * (i.e. tag is ITEM_Object or ITEM_NewObject), then the 2 extra bytes of
 * type info are stored in the 2 high order bytes of the integer.
 *
 *
 * <-------- extra bytes -------->                 <---- tag ---->
 *|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|
 *| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 *|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|
 *
 *
 * @author  <A HREF="mailto:Douglas.Simon@Eng.Sun.COM">Doug Simon</A>
 * @see     StackMap
 */
public final class StackMapEntry implements Cloneable {
  private int byte_code_offset;    // Program Counter (PC) corresponds to line
    private int[] types_of_locals;
    private int[] types_of_stack_items;


  /**
   * Construct object from file stream.
   * @param file Input stream
   * @throw IOException
   */
  StackMapEntry(DataInputStream file) throws IOException
  {
        byte_code_offset = file.readUnsignedShort();
        types_of_locals = readTypes(file);
        types_of_stack_items = readTypes(file);
  }

  /**
   * @param byte_code_offset
   * @param number_of_locals
     * @param types_of_locals
   * @param number_of_stack_items
     * @param types_of_stack_items
   */
  public StackMapEntry(int byte_code_offset, int[] types_of_locals,
            int[] types_of_stack_items)
  {
    this.byte_code_offset = byte_code_offset;
        if (types_of_locals == null)
            this.types_of_locals = new int[0];
        else
            this.types_of_locals = types_of_locals;

        if (types_of_stack_items == null)
            this.types_of_stack_items = new int[0];
        else
            this.types_of_stack_items = types_of_stack_items;
  }

    /**
     * Extract the extra 2-byte info from a type encoding.
     */
    public static final int getHighTwoBytes(int type) { return type >> 16; }

    /**
     * Extract just the type tag from the type encoding.
     */
    public static final int getTypeTag(int type) { return type & 0xFF; }

    /**
     * Generate a type encoding given a tag type and two extra bytes.
     */
    public static final int getEncodedType(int tag, int hiBytes)
    {
        return ((byte)tag) | (hiBytes << 16);
    }

    /**
     * Helper to read in an array of types.
     * @param file the inputstream to read from
     * @return the generated array of types
     */
    private int[] readTypes(DataInputStream file) throws IOException
    {
        int length = file.readUnsignedShort();
    int[] types = new int[length];
        for (int i = 0; i < length; i++)
        {
          int tag = file.readUnsignedByte();
            int type;
            // is this one of the extended types?
            if (tag == Constants.ITEM_NewObject || tag == Constants.ITEM_Object)
            {
              int index = file.readUnsignedShort();
                type = getEncodedType(tag,index);
            }
            else
                type = tag;
            types[i] = type;
        }
        return types;
    }

  /**
   * Called by objects that are traversing the nodes of the tree implicitely
   * defined by the contents of a Java class. I.e., the hierarchy of methods,
   * fields, attributes, etc. spawns a tree of objects.
   *
   * @param v Visitor object
   */
  public void accept(Visitor v) {
    v.visitStackMapEntry(this);
  }

    /**
     * Helper to dump an array of types to a data output stream.
     * @param types the array of types
     * @param file the stream for dumping
     */
    private final void dumpTypes(int[] types, DataOutputStream file) throws IOException
    {
        int length = types.length;
    file.writeShort(length);
        for (int i = 0; i < length; i++)
        {
          int tag = getTypeTag(types[i]);
            if (tag == Constants.ITEM_NewObject || tag == Constants.ITEM_Object)
            {
              file.writeByte(tag);
                file.writeShort(getHighTwoBytes(types[i]));
            }
            else
              file.writeByte(tag);
        }
    }

  /**
   * Dump line number/pc pair to file stream in binary format.
   *
   * @param file Output file stream
   * @throw IOException
   */
  public final void dump(DataOutputStream file) throws IOException
  {
    file.writeShort(byte_code_offset);
        dumpTypes(types_of_locals, file);
        dumpTypes(types_of_stack_items, file);
  }

  /**
   * @return Byte code offset
   */
  public final int getByteCodeOffset() { return byte_code_offset; }
  /**
   * @return Number of locals
   */
  public final int getNumberOfLocals() { return types_of_locals.length; }
  /**
   * @return Types of locals
   */
  public final int[] getTypesOfLocals() { return types_of_locals; }
  /**
   * @return Number of stack items
   */
  public final int getNumberOfStackItems() { return types_of_stack_items.length; }
  /**
   * @return Types of stack items
   */
  public final int[] getTypesOfStackItems() { return types_of_stack_items; }

  /**
   * @param byte_code_offset.
   */
  public final void setByteCodeOffset(int byte_code_offset) {
    this.byte_code_offset = byte_code_offset;
  }
  /**
   * @param types_of_locals.
   */
  public final void setTypesOfLocals(int[] types_of_locals) {
        if (types_of_locals == null)
            this.types_of_locals = new int[0];
        else
            this.types_of_locals = types_of_locals;
  }
  /**
   * @param types_of_stack_items.
   */
  public final void setTypesOfStackItems(int[] types_of_stack_items) {
        if (types_of_stack_items == null)
            this.types_of_stack_items = new int[0];
        else
            this.types_of_stack_items = types_of_stack_items;
  }
    /**
     * @return String representation without expanded type names
     */
  public final String toString()
    {
        return toString(null, true);
    }

    /**
     * Helper to print a type array.
     * @param types a type array
     * @param cp an (optional) constant pool that is used to resolve the class
     *        for ITEM_Object types
     * @return the String representation of the array
     */
    public final StringBuffer typesToString(int[] types, ConstantPool cp)
    {
        int length = types.length;
      StringBuffer buf = new StringBuffer("<");
        for (int i = 0; i < length; i++)
        {
            int tag = getTypeTag(types[i]);
            if (tag == Constants.ITEM_NewObject)
            {
                int newInst = getHighTwoBytes(types[i]);
                buf.append(Constants.ITEM_NAMES[tag] + "[" + newInst + "]");
            }
            else if (tag == Constants.ITEM_Object)
            {
                int index = getHighTwoBytes(types[i]);
                buf.append(Constants.ITEM_NAMES[tag] + "[");
                if (cp != null)
                {
                    // show the class name
                    String className = cp.getConstantString(index,
                            Constants.CONSTANT_Class);
                    buf.append(className);
                }
                else
                    // just show the index to the ConstantClass entry
                    buf.append(index);
                buf.append("]");
            }
            else
                buf.append(Constants.ITEM_NAMES[tag]);
            if (i != length - 1)
                buf.append(", ");
        }
        buf.append(">");
        return buf;
    }

  /**
   * @return String representation
   */
  public final String toString(ConstantPool cp, boolean showOffset)
  {
      StringBuffer buf = new StringBuffer("");
                if (showOffset) {
                    buf.append(byte_code_offset+":\t");
                }
        buf.append("locals = " + typesToString(types_of_locals,cp));
        buf.append(" stack = " + typesToString(types_of_stack_items,cp));
        return buf.toString();
  }

  /**
   * @return deep copy of this object
   */
  public StackMapEntry copy() {
    try {
      return (StackMapEntry)clone();
    } catch(CloneNotSupportedException e) {}

    return null;
  }
}
