//5d
package P;

//TRUSTED CLASS
public class X { 
  public static void main(String[] args) {
    try {
      Q.Y.catchit();
    } catch (Exception e) {
        System.out.println("Trusted Class X");
    }
    return;
  }

  public static void throwit() {
    throw new P.TestException("Some Exception");
  }
}
    
