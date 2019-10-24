/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>
 #include <pw3270/application.h>
 #include <pw3270/toolbar.h>
 #include <v3270.h>
 #include <v3270/trace.h>
 #include <lib3270/log.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

GtkWidget * pw3270_toolbar_new(void) {

	static const struct _item {
		const gchar *icon;
		const gchar *label;
	} itens[] = {

		{
			"gtk-connect",
			"_Connect"
		},

		{
			"gtk-disconnect",
			"_Disconnect"
		}

	};

	GtkWidget * toolbar = gtk_toolbar_new();
	size_t item;

	for(item = 0; item < G_N_ELEMENTS(itens); item++) {

		GtkToolItem * button = gtk_tool_button_new(gtk_image_new_from_icon_name(itens[item].icon,GTK_ICON_SIZE_LARGE_TOOLBAR),itens[item].label);
		gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(button),TRUE);

		gtk_widget_set_can_focus(GTK_WIDGET(button),FALSE);
		gtk_widget_set_can_default(GTK_WIDGET(button),FALSE);
		gtk_widget_set_focus_on_click(GTK_WIDGET(button),FALSE);

		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	}

	return toolbar;
}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = pw3270_application_new("br.com.bb.pw3270",G_APPLICATION_HANDLES_OPEN);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;

}



