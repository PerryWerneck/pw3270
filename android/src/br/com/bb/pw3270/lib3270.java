/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como lib3270.java e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

package br.com.bb.pw3270;

import java.lang.Thread;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Handler;
import android.os.Message;
import android.os.CountDownTimer;
import android.preference.PreferenceManager;
import android.util.Log;
import android.webkit.WebChromeClient;
// import android.webkit.WebResourceResponse;
import android.webkit.WebView;
// import android.webkit.WebViewClient;
import javax.net.SocketFactory;
import java.net.Socket;
import javax.net.ssl.SSLSocketFactory;
import java.io.DataInputStream;
import java.io.DataOutputStream;

// import java.util.Vector;

public abstract class lib3270
{
	private static final String			TAG				= "lib3270";

	protected int						screenState 	= -1;

	private lib3270						hSession		= this;
	
	public ProgressDialog				dlgSysMessage	= null;
	public WebView						view			= null;
	public Resources					res				= null;
	public Activity 					mainact 		= null;
	
	private static boolean				initialized		= false;
	private static NetworkThread 		mainloop 		= null;
	private static boolean				connected		= false;
	private static boolean 				reconnect		= false;
	private static boolean				refresh			= true;
	private static Socket 				sock			= null;

	private static DataOutputStream		outData			= null;
	private	static DataInputStream		inData			= null;

	protected SharedPreferences 		settings;

	// Define the Handler that receives messages from the thread
	final Handler mHandler = new Handler()
	{
		public void handleMessage(Message msg)
		{
			switch (msg.what)
			{
			case 0: // Reconnect
				if(!hSession.isConnected() && settings.getString("hostname","") != "" && settings.getBoolean("reconnect",false))
				{
					Log.d(TAG,"Connection lost, reconnecting");
					connect();
				}
				break;

			case 1: // OIA message has changed
				showProgramMessage(msg.arg1);
				break;

			case 2: // Screen changed
				Log.d(TAG, "Screen changed");
				screenState = 1;
				break;

			case 3: // Popup
				popupMessageInfo popup = (popupMessageInfo) msg.obj;
				showPopupMessage(msg.arg1, popup.title, popup.text, popup.info);
				break;

			case 4: // erase
				if(screenState != 2)
				{
					screenState = 2;
					erase();
				}
				break;

			case 5: // ctlr_done
				Log.d(TAG, "ctlr_done");
				break;

			case 7: // ready
				if(initialized)
				{
					dlgSysMessage.hide();
				}
				else
				{
					initialized = true;
					if(settings.getString("hostname","") != "" && settings.getBoolean("autoconnect",false))
						connect();
					else
						dlgSysMessage.hide();
				}
				break;

			case 8: // busy
				showProgressDialog("Aguarde...");
				break;

			case 9: // Create timer
				new timer(((Long) msg.obj).longValue(), msg.arg1);
				break;

			}
		}
	};


	static
	{
		System.loadLibrary("3270");
	}

	lib3270(Activity act)
	{
		String toggle[] = { "dstrace", "screentrace", "eventtrace", "reconnect" };

		setActivity(act);
		this.screenState	= -1;

		for(int f = 0; f < toggle.length; f++)
			setToggle(toggle[f],settings.getBoolean(toggle[f],false));

	}

	private class timer extends CountDownTimer
	{
		private long id;
		private lib3270 terminal;

		timer(long timer_id, int msec)
		{
			super(msec, msec);

			terminal	= hSession;
			id			= timer_id;

			Log.d(TAG, "Timer " + id + " set to " + msec + " ms");

			this.start();
		}

		public void onTick(long millisUntilFinished)
		{
		}

		public void onFinish()
		{
			Log.d(TAG, "Timer " + id + " finished");
			terminal.timerFinish(id);
		}

	}

	private class popupMessageInfo
	{
		public String title;
		public String text;
		public String info;

		popupMessageInfo(String title, String text, String info)
		{
			this.title = title;
			this.text = text;
			this.info = info;
		}
	}

	protected int send_data(byte[] data, int len)
	{
		Log.i(TAG, "Bytes a enviar: " + len);

		try
		{
			outData.write(data, 0, len);
			outData.flush();
			return len;
		}
		catch (Exception e)
		{
			String msg = e.getLocalizedMessage();

			if (msg == null)
				msg = e.toString();

			if (msg == null)
				msg = "";

			Log.i(TAG, "Erro ao enviar dados: " + msg);

			postPopup(1, "Desconectado", "Erro de comunicação ao enviar dados", msg);

			connected = false;

		}
		return -1;
	}

	// Main Thread
	private class NetworkThread extends Thread
	{
		private boolean connect()
		{
			// Connecta no host
			SocketFactory	socketFactory;
			String 			hostname = settings.getString("hostname","");
			Integer			port = Integer.valueOf(settings.getString("port", "23"));

			if (hostname == "" || port == 0)
				return false;

			postMessage(1, 14, 0);

			if(settings.getBoolean("ssl",false))
			{
				// Host é SSL
				socketFactory = SSLSocketFactory.getDefault();
				Log.v(TAG,"Conecting with SSLSocketFactory");
			}
			else
			{
				Log.v(TAG,"Conecting with SocketFactory");
				socketFactory = SocketFactory.getDefault();
			}

			try
			{
				Log.v(TAG,"Getting socket for " + hostname + ":" + port.toString());
				sock = socketFactory.createSocket(hostname, port);
				outData = new DataOutputStream(sock.getOutputStream());
				inData = new DataInputStream(sock.getInputStream());
			}
			catch (Exception e)
			{
				String msg = e.getLocalizedMessage();

				if (msg == null)
					msg = e.toString();

				if (msg == null)
					msg = "";

				Log.i(TAG, "Erro ao conectar: " + msg);

				postPopup(0, "Erro na conexão", "Não foi possível conectar", msg);

				return false;
			}

			Log.i(TAG, "Conectado ao host");
			return true;

		}

		public void run()
		{
			info(TAG, "Network thread started");
			reconnect = connected = connect();

			if (connected)
			{
				set_connection_status(true);

				while (connected)
				{
					byte[] in = new byte[4096];
					int sz = -1;

					try
					{
						sz = inData.read(in, 0, 4096);

					} catch (Exception e)
					{
						String msg = e.getLocalizedMessage();
						Log.i(TAG, "Erro ao receber dados do host: " + msg);
						postPopup(1, "Desconectado", "Erro de comunicação ao receber dados", msg);
						connected = false;
						sz = -1;
					}

					if (sz > 0)
						procRecvdata(in,sz);

				}

			}

			Log.v(TAG, "Exiting communication thread");

			try
			{
				sock.close();
			}
			catch (Exception e) { }

			sock = null;
			outData = null;
			inData = null;

			set_connection_status(false);

			mainloop = null;
			info(TAG, "Network thread stopped");

			if(reconnect)
				postMessage(0, 0, 0); // Pede por reconexão

		}

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
		msg.obj  = new popupMessageInfo(title, text, info);
		mHandler.sendMessage(msg);
	}
	
	void setActivity(Activity act)
	{
    	this.mainact	= act;
        this.settings 	= PreferenceManager.getDefaultSharedPreferences(act);
    	this.res 		= act.getResources();
	}
	
	WebView setView(WebView v)
	{
		this.view = v;
		
		view.addJavascriptInterface(this, "pw3270");

		view.setWebChromeClient(new WebChromeClient());

		view.getSettings().setBuiltInZoomControls(true);
		view.getSettings().setSupportZoom(true);
		view.getSettings().setUseWideViewPort(true);
		view.getSettings().setLoadWithOverviewMode(true);
		view.getSettings().setJavaScriptEnabled(true);

		return view;
		
	}
	
	public abstract String getProgramMessageText(int id);
	/*
	{
		return "Message " + Integer.toString(id);
	}
	*/
	
	/*---[ Signal methods ]--------------------------------------------------*/

	protected void showProgramMessage(int id)
	{
		switch(id)
		{
		case 0:	// LIB3270_MESSAGE_NONE
			if(screenState != 0)
			{
				screenState = 0;
				Log.v(TAG, "Status changed to NONE");
				view.reload();
			}
			break;

		case 4: // LIB3270_MESSAGE_DISCONNECTED
			Log.v(TAG, "Status changed to disconnected");
			connected = false;
			erase();
			break;

		case 3: // LIB3270_MESSAGE_CONNECTED
			Log.v(TAG, "Status changed to connected");
			break;

	        // 01 LIB3270_MESSAGE_SYSWAIT
			// 02 LIB3270_MESSAGE_TWAIT
			// 03 LIB3270_MESSAGE_CONNECTED
			// 05 LIB3270_MESSAGE_AWAITING_FIRST
			// 06 LIB3270_MESSAGE_MINUS
			// 07 LIB3270_MESSAGE_PROTECTED
			// 08 LIB3270_MESSAGE_NUMERIC
			// 09 LIB3270_MESSAGE_OVERFLOW
			// 10 LIB3270_MESSAGE_INHIBIT
			// 11 LIB3270_MESSAGE_KYBDLOCK
			// 12 LIB3270_MESSAGE_X
			// 13 LIB3270_MESSAGE_RESOLVING
			// 14 LIB3270_MESSAGE_CONNECTING

		default:
			dlgSysMessage.setMessage(getProgramMessageText(id));
			dlgSysMessage.show();
		}
	}

	protected void showPopupMessage(int type, String title, String text, String info)
	{
		Log.v(TAG,"Popup Message:");
		Log.v(TAG,title);
		Log.v(TAG,text);
		Log.v(TAG,info);

		AlertDialog d = new AlertDialog.Builder(mainact).create();

		if(title != "")
			d.setTitle(title);

		if(text != "")
			d.setMessage(text);

		d.setCancelable(true);
		dlgSysMessage.hide();
		d.show();
	}

	protected void info(String tag, String msg)
	{
		Log.i(tag, msg);
	}

	protected void error(String tag, String msg)
	{
		Log.e(tag, msg);
	}

	protected void erase()
	{
	}

	public void pfkey(int id)
	{
		sendPFkey(id);
	}

	public void xmit()
	{
		sendEnter();
	}

	public void ready()
	{
		postMessage(7, 0, 0);
	}

	public void busy()
	{
		postMessage(8, 0, 0);
	}

	public void showProgressDialog(String msg)
	{
	}

	/*---[ External methods ]------------------------------------------------*/

	public int connect()
	{
		if (mainloop == null)
		{
			info(TAG, "Starting comm thread");
			mainloop = new NetworkThread();
			mainloop.start();
			return 0;
		}
		error(TAG, "Comm thread already active during connect");
		return -1;
	}

	public int disconnect()
	{
		Log.v(TAG, "Disconnecting");
		connected = reconnect = false;

		if(sock != null)
		{
			try
			{
				sock.shutdownInput();
				sock.shutdownOutput();
			} catch(Exception e) { }
		}

		return 0;
	}

	public void setStringAt(int offset, String str)
	{
		refresh = false;
		try {
			setTextAt(offset, str.getBytes(getEncoding()), str.length());
		} catch (Exception e) {
		}
		refresh = true;
	}

//	@SuppressWarnings("unused")
	public String getscreencontents()
	{
		String text;

		try
		{
			text = new String(getHTML(),getEncoding());
		}
		catch(Exception e)
		{
			Log.e(TAG,e.getLocalizedMessage());
			return "";
		}

		return text;
	}
	
	
	/*---[ Native calls ]----------------------------------------------------*/
	private native int processEvents();

	// private native int do_connect();
	private native void set_connection_status(boolean state);

	// Misc calls
	public native String getEncoding();

	public native String getVersion();

	public native String getRevision();

	public native void setToggle(String name, boolean state);

	// Network I/O
	public native void procRecvdata(byte[] data, int len);

	// Connect/Disconnect status
	public native void setHost(String host);

	public native String getHost();

	public native boolean isConnected();

	public native boolean isTerminalReady();

	// Timers
	protected void newTimer(long id, int msec)
	{
		Message msg = mHandler.obtainMessage();

		msg.what	= 9; // MSG_CREATETIMER
		msg.arg1	= msec;
		msg.obj		= new Long(id);

		mHandler.sendMessage(msg);

	}

	private native void timerFinish(long id);

	// Keyboard actions
	public native void sendEnter();

	public native void sendPFkey(int id);

	// Get/Set screen contents
	public native byte[] getHTML();

	public native byte[] getText();

	public native void setTextAt(int offset, byte[] str, int len);

}
