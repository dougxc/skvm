//5d
package Q;

//UNTRUSTED CLASS
public class Y {
  public static void catchit() {
    try {
      P.X.throwit();
    } catch (Exception e) {
        System.out.println("Untrusted Class Y");
    }
    return;
  }
}
    
