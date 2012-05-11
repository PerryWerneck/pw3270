package br.com.bb.pw3270;

public class lib3270 {

   static 
   {
	   System.loadLibrary("3270");
   }

	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
		// TODO Auto-generated method stub

	}
	
	 public native String getVersion();

}
