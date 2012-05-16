package br.com.bb.pw3270;

import java.lang.Thread;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class lib3270
{
	private NetworkThread	mainloop;
	private static final String TAG = "lib3270";
	private boolean changed;

	static
	{
		System.loadLibrary("3270");
		init();
	}

	lib3270()
	{
		changed = false;
		mainloop = null;
	}

	private class popupMessageInfo
	{
		public String title;
		public String text;
		public String info;

		popupMessageInfo(String title, String text, String info)
		{
			this.title 	= title;
			this.text 	= text;
			this.info	= info;
		}
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
        	int rc;
        	info(TAG,"Network thread started");
            postMessage(0,1,0);
    		rc = do_connect();
    		info(TAG,"do_connect exits with rc="+rc);
    		while(isConnected())
    			processEvents();
            postMessage(0,0,0);
        	info(TAG,"Network thread stopped");
        }

        public void postMessage(int what, int arg1, int arg2)
        {
            Message msg = mHandler.obtainMessage();
            msg.what = what;
            msg.arg1 = arg1;
            msg.arg2 = arg2;
            mHandler.sendMessage(msg);
        }

    	public void postPopup(int type, String title, String text, String info)
    	{
            Message msg = mHandler.obtainMessage();

            msg.what = 3;
            msg.arg1 = type;
            msg.obj  = new popupMessageInfo(title,text,info);
            mHandler.sendMessage(msg);
    	}

	}

    public void postMessage(int what, int arg1, int arg2)
    {
    	mainloop.postMessage(what, arg1, arg2);
    }

   	public void postPopup(int type, String title, String text, String info)
	{
   		mainloop.postPopup(type, title, text, info);
	}

    // Define the Handler that receives messages from the thread and update the progress
    final Handler handler = new Handler()
    {
        public void handleMessage(Message msg)
        {
        	switch(msg.what)
        	{
        	case 0:	// Start/Stop service thread
        		if(msg.arg1 == 0)
        		{
        			mainloop = null;
        		}
        		break;

        	case 1:	// OIA message has changed
        		updateProgramMessage(msg.arg1);
        		break;

        	case 2: // Screen changed
        		Log.d(TAG,"Screen changed");
				changed = true;
				redraw();
        		break;

        	case 3:	// Popup
        		popupMessageInfo popup = (popupMessageInfo) msg.obj;
        		popupMessage(msg.arg1, popup.title, popup.text, popup.info);
        		break;

			case 4: // erase
				changed = false;
				erase();

			case 5: // ctlr_done
        		Log.d(TAG,"ctlr_done");
				if(changed)
				{
					changed = false;
					redraw();
				}
        	}
        }
    };

    /*---[ Signal methods ]--------------------------------------------------*/

	protected void updateProgramMessage(int id)
	{
	}

	public void popupMessage(int type, String title, String text, String info)
	{
	}

	protected void info(String tag, String msg)
	{
		Log.i(tag,msg);
	}

	protected void error(String tag, String msg)
	{
		Log.e(tag,msg);
	}

	protected void erase()
	{
		Log.i(TAG,"Erase screen");
	}

	protected void redraw()
	{
	}

    /*---[ External methods ]------------------------------------------------*/

    public int connect()
    {
    	if(mainloop == null)
    	{
        	info("jni","Starting comm thread");
    		mainloop = new NetworkThread(handler);
    		mainloop.start();
    		return 0;
    	}
    	error("jni","Comm thread already active during connect");
    	return -1;
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

	// Get/Set screen contents
	public native String		getHTML();


}
