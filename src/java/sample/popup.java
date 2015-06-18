
import pw3270.*;

public class popup
{
    public static void main (String[] args)
    {
        System.out.println("Begin");

        try {

            terminal host = new terminal();
             System.out.println("Output: " + host.popup_dialog(0,"Title","This is a popup message", "And this is a secondary and more detailed text"));

        } catch( Exception e ) {

            System.out.println("Error: " + e);

        }

        System.out.println("End");
    }
};
