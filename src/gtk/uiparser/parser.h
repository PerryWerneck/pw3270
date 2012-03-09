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
 * Este programa está nomeado como parser.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 typedef struct _setup_item
 {
	const gchar * name;
	void		  (*setup)(GtkWidget *widget, GtkWidget *obj);
 } SETUP_ITEM;

 GtkWidget 		* ui_parse_xml_folder(const gchar *path, const gchar ** groupname, const gchar **popupname, GtkWidget *widget, const SETUP_ITEM *itn);
 void			  ui_connect_action(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id);
 void			  ui_connect_toggle(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id);
 void			  ui_connect_pfkey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id);
 void			  ui_connect_pakey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id);
