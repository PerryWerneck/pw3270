package br.com.bb.pw3270;

import java.lang.Thread;
// import java.util.Observer;
// import java.util.Observable;
import android.os.Handler;
import android.os.Message;

public class lib3270
{
	NetworkThread	terminal;
	
	static
	{
		System.loadLibrary("3270");
		init();
	}

	lib3270()
	{
		terminal = new NetworkThread(handler);
	}
	
	// Main Thread
	private class NetworkThread extends Thread 
	{
		Handler mHandler;
		
		NetworkThread(Handler h) 
		{
            mHandler = h;
        }
       
        public void run()
        {
        	postMessage(0,99,0);
/*        	
    		do_connect();
    		while(isConnected())
    			processEvents();
*/    			
        }
	
        public void postMessage(int what, int arg1, int arg2)
        {
            Message msg = mHandler.obtainMessage();
            msg.what = what;
            msg.arg1 = arg1;
            msg.arg2 = arg2;
            mHandler.sendMessage(msg);
        }
        
	}
	
    // Define the Handler that receives messages from the thread and update the progress
    final Handler handler = new Handler() 
    {
        public void handleMessage(Message msg)
        {
        	switch(msg.what)
        	{
        	case 0:
        		update_message(msg.arg1);
        		break;
        	
        	}
        }
    };
    
    /*---[ Signal methods ]--------------------------------------------------*/
    
	protected void update_message(int id)
	{
	}
    

    /*---[ External methods ]------------------------------------------------*/

    public int connect()
    {
    	terminal.start();
    	return 0;
    }
    
    /*---[ Native calls ]----------------------------------------------------*/
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

	
}
