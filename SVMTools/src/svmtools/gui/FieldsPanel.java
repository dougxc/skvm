package svmtools.gui;

import java.util.Set;
import java.util.TreeSet;
import de.fub.bytecode.classfile.JavaClass;
import de.fub.bytecode.classfile.Trusted;
import de.fub.bytecode.classfile.FieldOrMethod;
import de.fub.bytecode.classfile.Field;
import de.fub.bytecode.classfile.Utility;
import svmtools.*;

public class FieldsPanel extends MembersPanel {

    static public String fieldToString(Field f) {
        return f.getName() + " : " +
                       Utility.signatureToString(f.getSignature());
    }
    protected String memberToName(FieldOrMethod f) {
        return fieldToString((Field)f);
    }

    public FieldsPanel(LoadedClass clazz) {
        super(clazz,"field");
    }

    protected FieldOrMethod[] getMembers() {
        return model.getOriginalClass().getFields();
    }
    protected boolean getDefaultSetting(Trusted t) {
        return t.default_field_accessibility;
    }
    protected int[] getNonDefaultMembers(Trusted t) {
        return t.non_default_fields;
    }
    protected void setDefaultMemberAccessibility(boolean b) {
        model.setDefaultFieldAccessibility(b);
    }
    protected void clearNonDefaultMembers() {
        model.clearNonDefaultMembers(true);
    }
    protected void addNonDefaultMember(int m) {
        model.addNonDefaultMember(m,true);
    }
}