package br.com.bb.pw3270;

import java.lang.Thread;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import javax.net.SocketFactory;
import java.net.Socket;
import javax.net.ssl.SSLSocketFactory;
import java.io.DataInputStream;
import java.io.DataOutputStream;

public class lib3270
{
	private NetworkThread	mainloop;
	private static final String TAG = "lib3270";

	private boolean 	changed;
	private boolean 	connected	= false;

	DataOutputStream	outData 	= null;
	DataInputStream		inData		= null;

	private String		hostname	= "3270.df.bb";
	private int			port		= 8023;
	private boolean		ssl			= false;

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

	private class byteMessage
	{
    	byte[]	msg;
    	int		sz;

    	byteMessage(byte[] contents, int len)
    	{
    		msg = contents;
    		sz  = len;
    	}

    	byte[] getMessage()
    	{
    		return msg;
    	}

    	int getLength()
    	{
    		return sz;
    	}
	}

	protected int send_data(byte[] data, int len)
	{
		Log.i(TAG,"Bytes a enviar: " + len);

		try
		{
			outData.write(data,0,len);
			outData.flush();
			return len;
		} catch( Exception e )
		{
    		String msg = e.getLocalizedMessage();

    		if(msg == null)
    			msg = e.toString();

    		if(msg == null)
    			msg = "Erro indefinido";

    		Log.i(TAG,"Erro ao enviar dados: " + msg);

    		postPopup(0,"Erro na comunicação","Não foi possível enviar dados",msg);

		}
		return -1;
	}


	// Main Thread
	private class NetworkThread extends Thread
	{
		Handler 			mHandler;
		Socket				sock	= null;

		NetworkThread(Handler h)
		{
            mHandler = h;
        }

		private boolean connect()
		{
            // Connecta no host
			SocketFactory socketFactory;
			if(ssl)
			{
				// Host é SSL
	        	socketFactory = SSLSocketFactory.getDefault();
			}
			else
			{
	        	socketFactory = SocketFactory.getDefault();
			}

			try
        	{
        		sock	= socketFactory.createSocket(hostname,port);
        		outData = new DataOutputStream(sock.getOutputStream());
        		inData	= new DataInputStream(sock.getInputStream());

        	} catch( Exception e )
        	{
        		String msg = e.getLocalizedMessage();

        		if(msg == null)
        			msg = e.toString();

        		if(msg == null)
        			msg = "Erro indefinido";

        		Log.i(TAG,"Erro ao conectar: " + msg);

        		postPopup(0,"Erro na conexão","Não foi possível conectar",msg);

                postMessage(0,0,0);

        		return false;
        	}

        	Log.i(TAG,"Conectado ao host");
        	return true;

		}

        public void run()
        {

        	info(TAG,"Network thread started");
            connected = connect();

            if(connected)
			{
				postMessage(0,0,0);

				while(connected)
				{
					byte[]	in	= new byte[4096];
					int		sz	= -1;

					try
					{
						sz = inData.read(in,0,4096);
					} catch( Exception e ) { sz = -1; }

					if(sz < 0)
					{
						connected = false;
					}
					else if(sz > 0)
					{
						Message msg = mHandler.obtainMessage();
						msg.what = 6;
						msg.obj  = new byteMessage(in,sz);

						mHandler.sendMessage(msg);
					}
				}
			}

			try
			{
				sock.close();
			} catch( Exception e ) { }

			sock = null;
			outData = null;
			inData = null;

			postMessage(0,0,0);

			mainloop = null;
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
        	case 0:	// Connected/Disconnected
        		set_connection_status(connected);
				Log.d(TAG,connected ? "Connected" : "Disconnected");
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
				break;

			case 5: // ctlr_done
        		Log.d(TAG,"ctlr_done");
				if(changed)
				{
					changed = false;
					redraw();
				}
				break;

			case 6: // recv_data
				procRecvdata(((byteMessage) msg.obj).getMessage(),((byteMessage) msg.obj).getLength());
				break;
        	}
        }
    };

    /*---[ Signal methods ]--------------------------------------------------*/

	protected void updateProgramMessage(int id)
	{
	}

	protected void popupMessage(int type, String title, String text, String info)
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

	public void pfkey(int id)
	{
		Log.d(TAG,"PF"+id);
		sendPFkey(id);
	}

	public void xmit()
	{
		Log.d(TAG,"XMIT");
		sendEnter();
	}

    /*---[ External methods ]------------------------------------------------*/

    public int connect()
    {
    	if(mainloop == null)
    	{
        	info(TAG,"Starting comm thread");
    		mainloop = new NetworkThread(handler);
    		mainloop.start();
    		return 0;
    	}
    	error(TAG,"Comm thread already active during connect");
    	return -1;
    }

    public int disconnect()
    {
    	connected = false;
    	return 0;
    }

    /*---[ Native calls ]----------------------------------------------------*/
	static private native int	init();

	private native int			processEvents();
//	private native int		    do_connect();
	private native void			set_connection_status(boolean state);

	// Misc calls
	public native String		getEncoding();
	public native String 		getVersion();
	public native String 		getRevision();

	// Network I/O
	public native void			procRecvdata( byte[] data, int len);

	// Connect/Disconnect status
	public native void 			setHost(String host);
	public native String		getHost();
	public native boolean		isConnected();
	public native boolean		isTerminalReady();

	// Keyboard actions
	public native void			sendEnter();
	public native void			sendPFkey(int id);

	// Get/Set screen contents
	public native byte[]		getHTML();
	public native byte[]		getText();


}
