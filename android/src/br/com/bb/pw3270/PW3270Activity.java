package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
// import android.widget.TextView;
// import android.widget.Button;
// import android.widget.EditText;
import android.util.Log;
// import android.view.View;
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
	private Activity 		mainact;
	private ProgressDialog	dlgProgress;

	private class terminal extends lib3270
	{

		terminal()
		{
			SharedPreferences settings = getSharedPreferences(TAG, 0);
			hostname	= settings.getString("hostname",hostname);
			port		= settings.getInt("port",port);
			ssl			= settings.getBoolean("ssl",ssl);

	    	setToggle("dstrace",settings.getBoolean("dstrace",true));
	    	setToggle("screentrace",settings.getBoolean("screentrace",true));
	    	setToggle("eventtrace",settings.getBoolean("eventtrace",true));

		}

		public void hideProgressDialog()
		{
			dlgProgress.hide();
		}

		public void showProgressDialog(String msg)
		{
			dlgProgress.setMessage(msg);
			Log.v(TAG,msg);
			dlgProgress.show();
		}

		protected void updateScreen()
		{
			// showProgressDialog("Aguarde...");
			view.reload();
		}
		
		protected boolean showProgramMessage(int id) 
		{
			if(!super.showProgramMessage(id))
			{
				String message[] = res.getStringArray(R.array.program_msg);
				try
				{
					showProgressDialog(message[id]);
				} catch(Exception e)
				{
					Log.e(TAG,e.getLocalizedMessage());
					showProgressDialog("?");
				}
			}
			return true;
		}

		protected void popupMessage(int type, String title, String text, String info)
		{
			AlertDialog d = new AlertDialog.Builder(mainact).create();

			d.setTitle(title);
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
			} catch(Exception e) { text = ""; }

			return text;
		}

	};

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		res = getResources();

        dlgProgress = new ProgressDialog(this);
        dlgProgress.setMessage("Aguarde...");
        dlgProgress.setCancelable(false);
//        dlgProgress.show();

		// Reference:
		// http://developer.android.com/reference/android/webkit/WebView.html
		view = new WebView(this);

		view.setWebChromeClient(new WebChromeClient());

		view.getSettings().setBuiltInZoomControls(true);
		view.getSettings().setSupportZoom(true);
		view.getSettings().setUseWideViewPort(true);
		view.getSettings().setLoadWithOverviewMode(true);

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

		view.getSettings().setJavaScriptEnabled(true);

		setContentView(view);
		view.loadUrl("file:index.html");

		host = new terminal();
		view.addJavascriptInterface(host, "pw3270");
		host.connect();


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
        	break;

        default:
            return super.onOptionsItemSelected(item);
        }
        return true;
        
    }
    
}
