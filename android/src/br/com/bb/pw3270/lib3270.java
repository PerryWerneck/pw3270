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
		do_connect();
		while(isConnected())
			processEvents();
	}

	static private native int	init();
	
	private native int			processEvents();
	private native int		    do_connect();

	// Misc calls
	public native String		getEncoding();

	public native String 		getVersion();
	public native String 		getRevision();

	// Connect/Disconnect status
	public native void 			setHost(String host);
	public native String		getHost();
	public native boolean		isConnected();
	public native boolean		isTerminalReady();
	
	public void connect(String hostname)
	{
		setHost(hostname);
		start();
	}
	




}
