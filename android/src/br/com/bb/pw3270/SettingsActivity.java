package br.com.bb.pw3270;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.app.Activity;

public class SettingsActivity extends Activity 
{
	public static class SettingsFragment extends PreferenceFragment
	{
		private class stringSetting implements Preference.OnPreferenceChangeListener
		{
		
			public stringSetting(Preference p)
			{
				p.setOnPreferenceChangeListener(this);
				this.onPreferenceChange(p,null);
			}
			
			public boolean onPreferenceChange(Preference p, Object arg1) 
			{
				p.setSummary(p.getSharedPreferences().getString(p.getKey(),""));
				return false;
			}			
		}
		
	    @Override
	    public void onCreate(Bundle savedInstanceState) 
	    {
	        super.onCreate(savedInstanceState);

	        // Load the preferences from an XML resource
	        addPreferencesFromResource(R.xml.preferences);

	        // Update summary from settings
	        new stringSetting(findPreference("hostname"));
	        new stringSetting(findPreference("port"));
	        
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



