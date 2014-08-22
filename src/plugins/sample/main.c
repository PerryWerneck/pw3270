/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 *
 */

 #include <pw3270.h>
 #include <pw3270/plugin.h>

/*--[ Implement ]---------------------------------------------------------------------------------------------------------*/

 /**
  * @brief Method called when the terminal connects to host.
  *
  * @param terminal	Terminal widget.
  * @param host		Connected host.
  * @param window		Application window.
  *
  */
 static void connected(GtkWidget *terminal, const gchar *host, GtkWidget * window) {

	g_message("%s - %s",__FUNCTION__,host);

 }

 /**
  * @brief Method called when the terminal loses connection with the host.
  *
  * @param terminal	Terminal widget.
  * @param window		Application window.
  *
  */
 static void disconnected(GtkWidget *terminal, GtkWidget * window) {

	g_message("%s",__FUNCTION__);

 }

 /**
  * @brief Plugin has started.
  *
  */
 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window) {

	GtkWidget * terminal = pw3270_get_terminal_widget(window);

	g_message("%s",__FUNCTION__);

	g_signal_connect(terminal,"disconnected",G_CALLBACK(disconnected),window);
	g_signal_connect(terminal,"connected",G_CALLBACK(connected),window);

	return 0;
 }



