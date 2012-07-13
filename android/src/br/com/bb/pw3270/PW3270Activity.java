package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
// import android.widget.TextView;
// import android.widget.Button;
// import android.widget.EditText;
import android.util.Log;
// import android.view.View;
import android.content.res.*;
import android.app.AlertDialog;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebResourceResponse;
import android.webkit.WebChromeClient;
// import java.io.InputStream;

// import android.app.Dialog;

public class PW3270Activity extends Activity 
{
	private static final String TAG = "pw3270";

	private Resources	res;
	private WebView		view;
	private terminal 	host;
	private Activity 	mainact;
	
	private class terminal extends lib3270
	{
		
		terminal()
		{
		}

		protected void updateProgramMessage(int id)
		{
			/*
			try
			{
			//	this.msgbox.setText(message[id]);
			} catch(Exception e) { this.msgbox.setText("Estado inesperado"); }
			*/
		}

		protected void popupMessage(int type, String title, String text, String info)
		{
			AlertDialog d = new AlertDialog.Builder(mainact).create();
			
			d.setTitle(title);
			d.setMessage(text);

			d.setCancelable(true);
			d.show();
		}

		public String getscreencontents()
		{
			String text;
			
			try
			{
				text = new String(getHTML(),getEncoding());
			} catch(Exception e) { text = ""; }
			
			return text;
		}
		
		protected void redraw()
		{
			view.reload();
		}
		
	};

	
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        
		res = getResources();

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

}