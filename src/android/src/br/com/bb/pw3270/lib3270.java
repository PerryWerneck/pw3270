package br.com.bb.pw3270;

import java.lang.Thread;

public class lib3270 extends Thread
{

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
	
	/**
	 * Connect to host, keep event loop running until disconnected.
	 */
	public void run()
	{
		while(isConnected())
			processEvents();
	}

	static private native int	init();
	private native int			processEvents();

	// Misc calls
	public native String		getEncoding();

	public native String 		getVersion();
	public native String 		getRevision();
	
	// Connection status
	public native boolean		isConnected();
	public native boolean		isTerminalReady();



}
