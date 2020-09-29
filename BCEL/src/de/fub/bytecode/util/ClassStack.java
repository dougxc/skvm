package de.fub.bytecode.util;
import java.util.Stack;
import de.fub.bytecode.classfile.JavaClass;

/** 
 * Utility class implementing a (typesafe) stack of JavaClass objects.
 *
 * @version $Id: ClassStack.java,v 1.1.1.1 2002/05/21 20:38:49 dougxc Exp $
 * @author <A HREF="http://www.inf.fu-berlin.de/~dahm">M. Dahm</A> 
 * @see Stack
*/
public class ClassStack {
  private Stack stack = new Stack();

  public void      push(JavaClass clazz) { stack.push(clazz); }
  public JavaClass pop()                 { return (JavaClass)stack.pop(); }
  public JavaClass top()                 { return (JavaClass)stack.peek(); }
  public boolean   empty()               { return stack.empty(); }
}  
