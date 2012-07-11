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
import java.io.InputStream;

// import android.app.Dialog;

public class PW3270Activity extends Activity 
{
	private Resources	res;
	private WebView		view;
	private terminal 	host;
	private static final String TAG = "pw3270";
	
	private class terminal extends lib3270
	{
		
		Activity Main;
			
		terminal(Activity Main)
		{
			this.Main = Main;
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
			AlertDialog d = new AlertDialog.Builder(Main).create();
			
			d.setTitle(title);
			d.setMessage(text);

			d.setCancelable(true);
			d.show();
		}

		protected void redraw()
		{
			try
			{
				String text = new String(getText(),getEncoding());
//				Log.i(TAG,text);
			} catch(Exception e) { }
		}

		
	};

	/*
	private terminal 	host;
	private EditText 	uri;
	private Resources	res;
	private String[] 	message;
	private WebView		view;
	
*/
	
	
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
		
		view.setWebViewClient(new WebViewClient() 
		{
			
			@Override
			public WebResourceResponse shouldInterceptRequest(WebView view, String url)
			{
				int		id		= R.raw.index;
				String	mime	= "text/html";
				
				Log.i(TAG,"Loading [" + url + "]");
				
				if(url.equalsIgnoreCase("file://jsmain.js"))
				{
					id = R.raw.jsmain;
				}
				else if(url.equalsIgnoreCase("file://theme.css"))
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
		view.loadUrl("file://index.html");
        
        //        setContentView(R.layout.main);

//		Log.i("pw3270","Activity started");

		
//		view = (WebView) findViewById(R.id.view);
		
//		String summary = "<html><body>Welcome to <b>pw3270</b>.</body></html>" ;
				
				// res.getString(R.string.hello);
		
		// ;
		// view.loadData("<html><body>Welcome to <b>pw3270</b>.</body></html>", "text/html", null);
		 
		/*
        message = res.getStringArray(R.array.program_msg);
        uri 	= (EditText) findViewById(R.id.hostname);
        
        // Set button
        Button btn = (Button) findViewById(R.id.connect);
        btn.setOnClickListener((View.OnClickListener) this);        
        
        host = new terminal((TextView) findViewById(R.id.msgbox),this);
        
        */
    }

    /*
    public void onClick(View v) 
    {
        // Perform action on click
    	// host.setHost(uri.getText().toString());
    	host.connect();
    }
    */

}