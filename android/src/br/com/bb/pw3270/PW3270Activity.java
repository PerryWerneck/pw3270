/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como PW3270Activity.hava e possui - linhas de
 * código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.*;
import android.app.AlertDialog;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebResourceResponse;
import android.webkit.WebChromeClient;
import android.app.ProgressDialog;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

// import java.io.InputStream;

// import android.app.Dialog;

public class PW3270Activity extends Activity
{
	private static final String TAG = "pw3270";

	private Resources		res;
	private WebView			view;
	private terminal 		host;
	private Activity 		mainact = this;
	private ProgressDialog	dlgSysMessage;

	private class terminal extends lib3270
	{

		terminal(SharedPreferences settings)
		{
			super(settings);
		}

		public void hideProgressDialog()
		{
			dlgSysMessage.hide();
		}

		protected void updateScreen()
		{
			view.reload();
		}

		protected boolean showProgramMessage(int id)
		{
			if(!super.showProgramMessage(id))
			{
				String message[] = res.getStringArray(R.array.program_msg);
				try
				{
					dlgSysMessage.setMessage(message[id]);
				} catch(Exception e)
				{
					dlgSysMessage.setMessage(e.getLocalizedMessage());
				}
				dlgSysMessage.show();
			}
			return true;
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
			hideProgressDialog();
			d.show();
		}

		@SuppressWarnings("unused")
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

	};

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    	res = getResources();
		initUI();
    }
		
	protected void initUI()
	{
		boolean autoconnect = false;
		SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(this);

		Log.d(TAG, "Initializing UI");

		if(dlgSysMessage == null)
		{
			// Cria dialogo para as mensagems de sistema
			Log.d(TAG, "Creating dlgSysMessage");
			dlgSysMessage = new ProgressDialog(this);
			dlgSysMessage.setCancelable(false);
			dlgSysMessage.setTitle(res.getString(R.string.wait));
		}
		
		if(host == null)
		{
			Log.d(TAG, "Creating terminal object");
			host = new terminal(settings);
			autoconnect = settings.getString("hostname","") != "" && settings.getBoolean("autoconnect",false);			
		}

		if(view == null)
		{
			// Reference:
			// http://developer.android.com/reference/android/webkit/WebView.html
			Log.d(TAG, "Creating Webview");
			view = new WebView(this);
			
			view.addJavascriptInterface(host, "pw3270");
	
			view.setWebChromeClient(new WebChromeClient());
	
			view.getSettings().setBuiltInZoomControls(true);
			view.getSettings().setSupportZoom(true);
			view.getSettings().setUseWideViewPort(true);
			view.getSettings().setLoadWithOverviewMode(true);
			view.getSettings().setJavaScriptEnabled(true);
	
			view.setWebViewClient(new WebViewClient()
			{
	
				@Override
				public WebResourceResponse shouldInterceptRequest(WebView view, String url)
				{
					int		id		= R.raw.index;
					String	mime	= "text/html";
					int		pos		= url.lastIndexOf("/");
	
					if(pos >=0 )
						url = url.substring(pos+1);
	
					Log.i(TAG,"Loading [" + url + "]");
	
					if(url.equalsIgnoreCase("jsmain.js"))
					{
						id = R.raw.jsmain;
					}
					else if(url.equalsIgnoreCase("theme.css"))
					{
						mime = "text/css";
						id = R.raw.theme;
					}
	
					// http://developer.android.com/reference/android/webkit/WebResourceResponse.html
					return new WebResourceResponse(mime,"utf-8",getResources().openRawResource(id));
				}
	
			});
		}
		
		setContentView(view);
		view.loadUrl("file:index.html");
		
		if(autoconnect)
			host.connect();

    }

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		 super.onConfigurationChanged(newConfig);
		 initUI();
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        MenuInflater inflater = getMenuInflater();
		Log.d(TAG,"Popup menu");
        inflater.inflate(R.layout.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        // Handle item selection
        switch (item.getItemId())
        {
        case R.id.connect:
        	host.connect();
        	break;

        case R.id.disconnect:
        	host.disconnect();
        	break;

        case R.id.settings:
			Intent myIntent = new Intent(view.getContext(), SettingsActivity.class);
			startActivityForResult(myIntent, 0);
        	break;

        default:
            return super.onOptionsItemSelected(item);
        }
        return true;

    }

}
