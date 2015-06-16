
import pw3270.*;

public class efglobe
{
    public static void main (String[] args)
    {
        try
        {
            terminal host = new terminal();

            host.connect("tn3270://zos.efglobe.com:telnet",10);

            host.wait_for_ready(10);

            System.out.println("Read: [" + host.get_string_at(14,19,38)+"]");

            host.disconnect();

        }
        catch( Exception e )
        {
            System.err.println(e);
        }
    }
};
