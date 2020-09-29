//5c

package P;

//TRUSTED CLASS
class X {
  public static void main(String[] args) {
    try {
      Q.Y.Throw();
    } catch (Exception e) {
        System.out.println(e.getMessage());
    }
    return;
  }
} 
