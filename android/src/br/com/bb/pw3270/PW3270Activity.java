package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.EditText;
import android.view.View;
import android.content.res.*;

public class PW3270Activity extends Activity implements View.OnClickListener
{
	
	private class terminal extends lib3270
	{
		TextView view;
		
		terminal(TextView view)
		{
			this.view = view;
			
		}

		protected void updateProgramMessage(int id)
		{
			 this.view.setText(message[id]);
		}

		public void popupMessage(int type, String title, String text, String info)
		{
			this.view.setText(title + "\n" + text + "\n" + info);
		}

		
	};
	
	private terminal 	host;
	private EditText 	uri;
	private Resources	res;
	private String[] 	message;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        res 	= getResources();
        message = res.getStringArray(R.array.program_msg);
        uri 	= (EditText) findViewById(R.id.hostname);
        
        // Set button
        Button btn = (Button) findViewById(R.id.connect);
        btn.setOnClickListener((View.OnClickListener) this);        
        
        host = new terminal((TextView) findViewById(R.id.text));
        
    }

    public void onClick(View v) 
    {
        // Perform action on click
    	// host.setHost(uri.getText().toString());
    	host.connect();
    }

}