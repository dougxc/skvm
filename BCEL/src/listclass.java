import java.io.*;
import de.fub.bytecode.classfile.*;
import de.fub.bytecode.*;
import de.fub.bytecode.ClassPath;

/**
 * A classfile disassembler.
 */
public class listclass
{

    public static void usage(String errMsg) {
        PrintStream out = System.out;
        if (errMsg != null) {
            out.println(errMsg);
        }
        out.println("Usage: listclass [-options] [ class | classfile ]* ");
        out.println("where options include:");
        out.println("    -code            show code disassembly");
        out.println("    -constants       show constant pool disassembly");
        out.println("    -cp <path>       use path to search for classes");
        out.println("    -h               show this help message and exit");
    }

    public static void main(String argv[])
    {
        String file_name[] = new String[argv.length];
        int files          = 0;
        boolean code       = false;
        boolean constants  = false;
        String name        = null;
        ClassPath path     = null;
        for (int i = 0; i < argv.length; i++) {
            if (argv[i].charAt(0) == '-') {
                if (argv[i].equals("-constants")) {
                    constants = true;
                    continue;
                }
                if (argv[i].equals("-code")) {
                    code = true;
                    continue;
                }
                if (argv[i].equals("-cp")) {
                    path = new ClassPath(argv[++i].replace('/', File.separatorChar).replace(':', File.pathSeparatorChar));
                    continue;
                }
                if (argv[i].startsWith("-h")) {
                    usage(null);
                    return;
                } else {
                    usage("Unknown switch ".concat(String.valueOf(String.valueOf(argv[i]))));
                    return;
                }
            }
            file_name[files++] = argv[i];
        }

        try {
            if (files == 0) {
                usage("list: No input files specified");
                return;
            }
            for (int i = 0; i < files; i++) {
                name = file_name[i];
                JavaClass java_class;
                if(name.endsWith(".class")) {
                    if((java_class = Repository.lookupClass(name)) == null)
                        java_class = (new ClassParser(name)).parse();
                } else {
                    de.fub.bytecode.ClassPath.ClassFile classFile = path.getClassFile(name);
                    java_class = (new ClassParser(classFile.getInputStream(), classFile.getPath())).parse();
                }
                ConstantPool cp = java_class.getConstantPool();
                Trusted trusted = null;
                Attribute attrs[] = java_class.getAttributes();
                for (int ii = 0; ii != attrs.length; ii++) {
                    Attribute attr = attrs[ii];
                    String attrName = ((ConstantUtf8)cp.getConstant(attr.getNameIndex())).getBytes();
                    if(attrName.equals("Trusted"))
                        trusted = (Trusted)attr;
                }

                if (trusted != null) {
                    int insert = 0;
                    Attribute newAttrs[] = new Attribute[attrs.length - 1];
                    for(int ii = 0; ii != attrs.length; ii++)
                        if(attrs[ii] != trusted)
                            newAttrs[insert++] = attrs[ii];

                    java_class.setAttributes(newAttrs);
                }
                System.out.println(java_class);
                if (trusted != null) {
                    System.out.println("Trusted attribute:\n".concat(String.valueOf(String.valueOf(trusted.toString(java_class)))));
                }
                if (constants) {
                    System.out.println(cp);
                }
                if (code) {
                    printCode(java_class.getMethods());
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println("Couldn't find class ".concat(String.valueOf(String.valueOf(name))));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void printCode(Method methods[]) {
        try {
            for (int i = 0; i < methods.length; i++) {
                Method method = methods[i];
                System.out.println(method);
                Code code = method.getCode();
                if (code != null) {
                    System.out.println(code.toString());
                }
            }
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}