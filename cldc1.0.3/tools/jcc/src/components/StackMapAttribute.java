/*
 *	%M%	%I%	%E%
 *
 * Copyright (c) 2000 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 */

package components;
import java.io.DataOutput;
import java.io.DataInput;
import java.io.IOException;
import util.DataFormatException;
import main.Const;

/*
 * A class to represent the StackMap attribute of a method's code.
 */

public
class StackMapAttribute extends Attribute
{
    public StackMapFrame	data[];

    public
    StackMapAttribute( UnicodeConstant name, int l, StackMapFrame d[] ){
	super( name, l );
	this.data = d;
    }

    public void
    externalize( ConstantPool p ){
	super.externalize( p );
 	for (int i = 0; i < data.length; i++) {
	    data[i].externalize(p);
	}
    }

    protected int
    writeData( DataOutput o ) throws IOException{
	int length = 2;
	o.writeShort(data.length);
	for ( int i = 0; i < data.length; i++ ){
	    length += data[i].writeData(o);
	}
	return length;
    }

    public void
    countConstantReferences( boolean isRelocatable ){
	super.countConstantReferences( isRelocatable );
	for (int i = 0; i < data.length; i++) {
	    data[i].countConstantReferences(isRelocatable);
	}
    }

    public static Attribute
    readAttribute( DataInput i, ConstantObject globals[] ) throws IOException{
	UnicodeConstant name;
	name = (UnicodeConstant)globals[i.readUnsignedShort()];
	return finishReadAttribute(i, name, globals );
    }

    //
    // for those cases where we already read the name index
    // and know that its not something requiring special handling.
    //
    public static Attribute
    finishReadAttribute(DataInput in,
			UnicodeConstant name,
			ConstantObject globals[] ) throws IOException {
	int length = in.readInt();
	// Read the number of frames
	int n = in.readUnsignedShort();
	StackMapFrame d[] = new StackMapFrame[n];
	// Read each frame
	for (int i = 0; i < n; i++) {
	    d[i] = new StackMapFrame(in, globals);
	}
	return new StackMapAttribute(name, length, d);
    }


}
