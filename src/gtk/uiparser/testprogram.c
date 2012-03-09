
#include <gtk/gtk.h>
#include "../common/common.h"
#include "parser.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static GtkWidget **popup = NULL;

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

void show_popup(GtkWidget *button, GtkWidget *menu)
{
	trace("Showing popup %p",popup);

	gtk_widget_show_all(menu);
	gtk_menu_set_screen(GTK_MENU(menu), gtk_widget_get_screen(button));
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, 0);

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
										"dock",
										NULL };
	int			  f;
	GtkWidget 	* window;
	gchar		* path;
	GtkWidget	* vbox;
	GtkWidget	* hbox;

	gtk_init(&argc, &argv);
	configuration_init();

	hbox   = gtk_hbox_new(FALSE,5);
	vbox   = gtk_vbox_new(FALSE,5);
	path   = build_data_filename("ui",NULL);
	window = ui_parse_xml_folder(path,groupname,popupname,vbox,NULL);
	g_free(path);

	popup = g_object_get_data(G_OBJECT(window),"popup_menus");

	for(f=0;popupname[f];f++)
	{
		if(popup[f])
		{
			GtkWidget *button = gtk_button_new_with_label(popupname[f]);
			gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,5);
			g_signal_connect(button,"clicked",G_CALLBACK(show_popup),popup[f]);
		}
	}

	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,5);

	gtk_widget_show_all(vbox);

	if(window)
	{
		gtk_widget_show(window);
		gtk_main();
	}

	configuration_deinit();
	return 0;
}


