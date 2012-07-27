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
import android.os.Handler;
import android.os.Message;
import android.os.CountDownTimer;
import android.util.Log;
import javax.net.SocketFactory;
import java.net.Socket;
import javax.net.ssl.SSLSocketFactory;
import java.io.DataInputStream;
import java.io.DataOutputStream;

// import java.util.Vector;

public class lib3270
{
	private NetworkThread mainloop = null;
	private static final String TAG = "lib3270";

	protected int screenState = 0;
	private boolean connected = false;
	private boolean refresh = true;
	private Socket sock = null;

	DataOutputStream outData = null;
	DataInputStream inData = null;

	protected String hostname = "3270.df.bb";
	protected int port = 8023;
	protected boolean ssl = false;

	// Define the Handler that receives messages from the thread
	final Handler mHandler = new Handler()
	{
		public void handleMessage(Message msg)
		{
			switch (msg.what)
			{
			case 0: // Connected/Disconnected
				set_connection_status(connected);
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
				popupMessage(msg.arg1, popup.title, popup.text, popup.info);
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

			case 6: // recv_data
				procRecvdata(((byteMessage) msg.obj).getMessage(),((byteMessage) msg.obj).getLength());
				break;

			case 7: // ready
				hideProgressDialog();
				break;

			case 8: // busy
				showProgressDialog("Aguarde...");
				break;
			}
		}
	};


	static
	{
		System.loadLibrary("3270");
		init();
	}

	lib3270()
	{
		screenState = 0;
		mainloop = null;
	}

	private class timer extends CountDownTimer
	{
		private long id;
		private lib3270 terminal;

		timer(lib3270 session, long timer_id, int msec) {
			super(msec, msec);

			terminal = session;
			id = timer_id;

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

		popupMessageInfo(String title, String text, String info) {
			this.title = title;
			this.text = text;
			this.info = info;
		}
	}

	private class byteMessage
	{
		byte[] msg;
		int sz;

		byteMessage(byte[] contents, int len) {
			msg = contents;
			sz = len;
		}

		byte[] getMessage() {
			return msg;
		}

		int getLength() {
			return sz;
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
				msg = "Erro indefinido";

			Log.i(TAG, "Erro ao enviar dados: " + msg);

			postPopup(0, "Erro na comunicação", "Não foi possível enviar dados", msg);

		}
		return -1;
	}

	// Main Thread
	private class NetworkThread extends Thread
	{
		private boolean connect()
		{
			// Connecta no host
			SocketFactory socketFactory;

			if (hostname == "")
				return false;

			postMessage(1, 14, 0);

			if (ssl) 
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
					msg = "Erro indefinido";

				Log.i(TAG, "Erro ao conectar: " + msg);

				postPopup(0, "Erro na conexão", "Não foi possível conectar", msg);

				postMessage(0, 0, 0);

				return false;
			}

			Log.i(TAG, "Conectado ao host");
			return true;

		}

		public void run()
		{

			info(TAG, "Network thread started");
			connected = connect();

			if (connected)
			{
				postMessage(0, 0, 0);

				while (connected)
				{
					byte[] in = new byte[4096];
					int sz = -1;

					try 
					{
						sz = inData.read(in, 0, 4096);

						Log.i(TAG, Integer.toString(sz) + " bytes recebidos");

						if (sz > 0) 
						{
							Message msg = mHandler.obtainMessage();
							msg.what = 6;
							msg.obj = new byteMessage(in, sz);
							mHandler.sendMessage(msg);
						}
					
					} catch (Exception e) 
					{
						Log.i(TAG, "Erro ao receber dados do host: " + e.getLocalizedMessage());
						connected = false;
					}
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

			postMessage(0, 0, 0);

			mainloop = null;
			info(TAG, "Network thread stopped");
		}

		/*
		public void postMessage(int what, int arg1, int arg2)
		{
			Message msg = mHandler.obtainMessage();
			msg.what = what;
			msg.arg1 = arg1;
			msg.arg2 = arg2;
			mHandler.sendMessage(msg);
		}
		*/

		public void postPopup(int type, String title, String text, String info)
		{
			Message msg = mHandler.obtainMessage();

			msg.what = 3;
			msg.arg1 = type;
			msg.obj = new popupMessageInfo(title, text, info);
			mHandler.sendMessage(msg);
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
		Log.d(TAG, "Type:" + type);
		Log.d(TAG, "Title:" + title);
		Log.d(TAG, "Text:" + text);
		Log.d(TAG, "Info:" + info);
		mainloop.postPopup(type, title, text, info);
	}

	/*---[ Signal methods ]--------------------------------------------------*/

	protected boolean showProgramMessage(int id)
	{
		switch(id)
		{
		case 0:	// LIB3270_MESSAGE_NONE
			if(screenState != 0)
			{
				screenState = 0;
				Log.v(TAG, "Status changed to NONE");
				updateScreen();
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
			return false;
		}

		return true;
	}

	protected void popupMessage(int type, String title, String text, String info)
	{
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

	protected void updateScreen()
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

	public void hideProgressDialog()
	{
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
		connected = false;

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

	/*---[ Native calls ]----------------------------------------------------*/
	static private native int init();

	static private native int deinit();

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
		new timer(this, id, msec);
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
