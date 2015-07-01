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
 * Este programa está nomeado como call.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */


 #include "private.h"
 #include <pw3270/v3270.h>


/*---[ Implement ]----------------------------------------------------------------------------------*/

namespace PW3270_NAMESPACE {


	void java::call(GtkWidget *widget, const char *filename) {

		if(!trylock()) {

			failed(widget, _( "Can't access java virtual machine" ), "%s", strerror(EBUSY));
			return;

		}

		if(jvm || load_jvm(widget)) {

			v3270_set_script(widget,'J',TRUE);

			gchar * dirname		= g_path_get_dirname(filename);
			gchar * classname	= g_path_get_basename(filename);

			gchar * ptr			= strrchr(classname,'.');
			if(ptr) {
				*ptr = 0;
			}



			try {

				jclass		cls;
				jmethodID	mid;

				// Atualizar o classpath
				cls = env->FindClass("java.lang.System");
				if(!cls) {
					throw exception( _(  "Can't find class %s" ), "java.lang.System");
				}

				mid = env->GetStaticMethodID(cls, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String");
				if(!mid) {
					throw exception( _(  "Can't find method %s/%d" ), "java.lang.System","setProperty");
				}



			} catch(std::exception &e) {

				failed(widget,_("Can't start java application"),"%s", e.what());

			}

			g_free(dirname);
			g_free(classname);

			v3270_set_script(widget,'J',FALSE);

		}

		unlock();


	}

}
