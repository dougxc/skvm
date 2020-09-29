//5c

package Q;

//UNTRUSTED CLASS
public class Y {
  public static void Throw() {
    try {
      throw new P.TestException("Untrusted Class Y");
    } catch (P.TestException e) {
        System.out.println(e.getMessage());
    }
    return;
  }   
}
