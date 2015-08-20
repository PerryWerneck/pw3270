
import pw3270.*;

public class prot
{
    public static void main (String[] args)
    {
        System.out.println("Begin");

        try {

            terminal host = new terminal();

			host.popup_dialog(0,"19,39","Testing","Position 19,39 is " + (host.get_is_protected_at(19,39) == 0 ? "un" : "") + "protected");
			host.popup_dialog(0,"20,39","Testing","Position 19,39 is " + (host.get_is_protected_at(20,39) == 0 ? "un" : "") + "protected");


        } catch( Exception e ) {

            System.out.println("Error: " + e);

        }

        System.out.println("End");
    }
};
