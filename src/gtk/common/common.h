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
 * Este programa está nomeado como common.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 // Configuration
 void		  configuration_init(void);
 void		  configuration_deinit(void);

 gchar		* get_string_from_config(const gchar *group, const gchar *key, const gchar *def);
 gboolean	  get_boolean_from_config(const gchar *group, const gchar *key, gboolean def);
 gint 		  get_integer_from_config(const gchar *group, const gchar *key, gint def);

 void		  set_string_to_config(const gchar *group, const gchar *key, const gchar *fmt, ...);
 void		  set_boolean_to_config(const gchar *group, const gchar *key, gboolean val);
 void		  set_integer_to_config(const gchar *group, const gchar *key, gint val);

 gchar 		* build_data_filename(const gchar *first_element, ...);

