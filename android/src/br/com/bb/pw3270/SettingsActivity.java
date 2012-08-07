package br.com.bb.pw3270;

import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.app.Activity;

public class SettingsActivity extends Activity 
{
	public static class SettingsFragment extends PreferenceFragment 
	{
	    @Override
	    public void onCreate(Bundle savedInstanceState) 
	    {
	        super.onCreate(savedInstanceState);

	        // Load the preferences from an XML resource
	        addPreferencesFromResource(R.xml.preferences);
	    }
	}

	@Override
    protected void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);

        // Display the fragment as the main content.
        getFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsFragment())
                .commit();
    }

}



