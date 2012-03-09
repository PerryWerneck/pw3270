
#include <gtk/gtk.h>
#include "../common/common.h"
#include "parser.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

void activated(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
}

void toggled(GtkToggleAction *action, GtkWidget *widget)
{
	trace("Action %s toggled on widget %p",gtk_action_get_name(GTK_ACTION(action)),widget);
}

void ui_connect_action(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	g_signal_connect(action,"activate",G_CALLBACK(activated),widget);
}

void ui_connect_toggle(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	trace("Connecting action %s with toggle %s and widget %p",gtk_action_get_name(action),id,widget);
	g_signal_connect(action,"toggled",G_CALLBACK(toggled),widget);
}

void ui_connect_pfkey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	g_signal_connect(action,"activate",G_CALLBACK(activated),widget);
}

void ui_connect_pakey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	g_signal_connect(action,"activate",G_CALLBACK(activated),widget);
}

int main (int argc, char *argv[])
{
	static const gchar *groupname[] = {	"default",
										"online",
										"offline",
										"selection",
										"clipboard",
										"filetransfer",
										"paste",
										NULL };

	static const gchar *popupname[] = {	"default",
										"selection",
										"offline",
										NULL };
	GtkWidget 	* window;
	gchar		* path;

	gtk_init(&argc, &argv);
	configuration_init();

	path   = build_data_filename("ui",NULL);
	window = ui_parse_xml_folder(path,groupname,popupname,NULL,NULL);
	g_free(path);

	if(window)
	{
		gtk_widget_show(window);
		gtk_main();
	}

	configuration_deinit();
	return 0;
}


