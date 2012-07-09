package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.EditText;
import android.util.Log;
import android.view.View;
import android.content.res.*;
import android.app.AlertDialog;
import android.webkit.WebView;
// import android.app.Dialog;

public class PW3270Activity extends Activity implements View.OnClickListener
{
	private class terminal extends lib3270
	{
		private static final String TAG = "pw3270";
		
		TextView msgbox;
		Activity Main;
	
		
		terminal(TextView msgbox, Activity Main)
		{
			this.msgbox = msgbox;
			this.Main = Main;
		}

		protected void updateProgramMessage(int id)
		{
			try
			{
				this.msgbox.setText(message[id]);
			} catch(Exception e) { this.msgbox.setText("Estado inesperado"); }
		}

		public void popupMessage(int type, String title, String text, String info)
		{
			AlertDialog d = new AlertDialog.Builder(Main).create();
			
			d.setTitle(title);
			d.setMessage(text);

			d.setCancelable(true);
			d.show();
		}
		
		protected void redraw()
		{
			String text = getHTML();
			Log.i(TAG,text);
		}


		
	};
	
	private terminal 	host;
	private EditText 	uri;
	private Resources	res;
	private String[] 	message;
	private WebView		view;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
 
		Log.i("pw3270","Activity started");

        res 	= getResources();
        message = res.getStringArray(R.array.program_msg);
        uri 	= (EditText) findViewById(R.id.hostname);
        
        // Set button
        Button btn = (Button) findViewById(R.id.connect);
        btn.setOnClickListener((View.OnClickListener) this);        
        
        host = new terminal((TextView) findViewById(R.id.msgbox),this);
        
        
    }

    public void onClick(View v) 
    {
        // Perform action on click
    	host.setHost(uri.getText().toString());
    	host.connect();
    }

}