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
import android.app.AlertDialog;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.content.res.*;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.app.ProgressDialog;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.ImageView;
import android.widget.TextView;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

// import java.io.InputStream;

// import android.app.Dialog;

public class PW3270Activity extends Activity
{
	private static final String 	TAG		= "pw3270";
	private static lib3270 			host	= null;


    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
   		initUI();
    }
		
	protected void initUI()
	{
        if(host == null)
        {
        	host = new lib3270(this)
        	{
        		public String getProgramMessageText(int id)
        		{
        			String message[] = res.getStringArray(R.array.program_msg);
        			try
        			{
        				return message[id];
        			} catch(Exception e)
        			{
        				return e.getLocalizedMessage();
        			}
        			
        		}
        	};
        }
        else
        {
        	host.setActivity(this);
        }
        
		Log.d(TAG, "Initializing UI");

		host.dlgSysMessage = new ProgressDialog(this);
		host.dlgSysMessage.setCancelable(false);
		host.dlgSysMessage.setTitle(host.res.getString(R.string.wait));

		// Reference:
		// http://developer.android.com/reference/android/webkit/WebView.html
		host.setView(new WebView(this)).setWebViewClient(new WebViewClient()
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
				return new WebResourceResponse(mime,"utf-8",host.mainact.getResources().openRawResource(id));
			}

		});
		
		setContentView(host.view);
		
		host.view.loadUrl("file:index.html");
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
			Intent myIntent = new Intent(host.view.getContext(), SettingsActivity.class);
			startActivityForResult(myIntent, 0);
        	break;

        case R.id.reload:
        	host.view.reload();
        	break;
        	
        case R.id.about:
        	showAboutDialog();
        	break;
        	
        default:
            return super.onOptionsItemSelected(item);
        }
        return true;

    }

    private void showAboutDialog()
    {
    	AlertDialog.Builder builder;
    	AlertDialog alertDialog;
    	Context mContext = getApplicationContext();
    	
    	LayoutInflater inflater = (LayoutInflater) mContext.getSystemService(LAYOUT_INFLATER_SERVICE);
    	
    	View layout = inflater.inflate(R.layout.about, (ViewGroup) findViewById(R.id.layout_root));

    	TextView text = (TextView) layout.findViewById(R.id.text);
    	text.setText(host.res.getString(R.string.app_name) + " Vrs " + host.getVersion() + "-" + host.getRevision());
    	
    	ImageView image = (ImageView) layout.findViewById(R.id.image);
    	image.setImageResource(R.drawable.ic_launcher);

    	builder = new AlertDialog.Builder(this);
    	builder.setView(layout);
    	alertDialog = builder.create();    
    	
    	alertDialog.show();
    }
    
	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		Log.d(TAG, "Configuration Changed");
		super.onConfigurationChanged(newConfig);
		initUI();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);
		// Save the state of the WebView
		host.view.saveState(outState);
	}
 
	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState)
	{
		super.onRestoreInstanceState(savedInstanceState);
		// Restore the state of the WebView
		host.view.restoreState(savedInstanceState);
		host.view.reload();
	}

}
