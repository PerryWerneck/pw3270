package br.com.bb.pw3270;

public class lib3270 {

	static
	{
		System.loadLibrary("3270");
		init();
	}

	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		// TODO Auto-generated method stub

	}

	// Misc calls
	static private native int		init();
	public native String 			getVersion();
	public native String 			getRevision();



}
