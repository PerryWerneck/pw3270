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
 * Este programa está nomeado como rexx_methods.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 *
 * Referencias:
 *
 * * http://www.oorexx.org/docs/rexxpg/x2950.htm
 *
 */

 #include "rx3270.h"
 #include <gtk/gtk.h>
 #include <time.h>
 #include <string.h>
 #include <ctype.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

RexxMethod1(int, rx3270_method_init, CSTRING, type)
{
	// Set session class in rexx object
	try
	{
		RexxPointerObject sessionPtr = context->NewPointer(rx3270::create(type));
		context->SetObjectVariable("CSELF", sessionPtr);
	}
	catch(rx3270::exception e)
	{
		e.RaiseException(context);
	}

    return 0;
}

RexxMethod1(int, rx3270_method_uninit, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;

    trace("rx3270_method_uninit hSession=%p",hSession);

	if(hSession)
		delete hSession;

    trace("%s","rx3270_method_uninit");
	return 0;
}

RexxMethod1(RexxStringObject, rx3270_method_version, CSELF, sessionPtr)
{
	rx3270	* session 	= (rx3270 *) sessionPtr;

	if(session)
	{
		char				* version	= session->get_version();
		RexxStringObject	  ret 		= context->String((CSTRING) (version ? version : "ERROR:"));
		free(version);
		return ret;
	}

	return context->String((CSTRING) PACKAGE_VERSION);
}

RexxMethod1(RexxStringObject, rx3270_method_revision, CSELF, sessionPtr)
{
	rx3270	* session 	= (rx3270 *) sessionPtr;

	if(session)
	{
		char				* version	= session->get_revision();
		RexxStringObject	  ret 		= context->String((CSTRING) (version ? version : PACKAGE_REVISION));
		free(version);
		return ret;
	}

	return context->String((CSTRING) PACKAGE_REVISION);
}

RexxMethod3(int, rx3270_method_connect, CSELF, sessionPtr, CSTRING, uri, OPTIONAL_int, wait)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->connect(uri,wait != 0);
}

RexxMethod1(int, rx3270_method_disconnect, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->disconnect();
}

RexxMethod2(int, rx3270_method_sleep, CSELF, sessionPtr, int, seconds)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->wait(seconds);
}

RexxMethod1(logical_t, rx3270_method_is_connected, CSELF, sessionPtr)
{
	try
	{
		rx3270 *hSession = (rx3270 *) sessionPtr;
		if(!hSession)
			return false;
		return hSession->is_connected();
	}
	catch(rx3270::exception e)
	{
		e.RaiseException(context);
	}

	return 0;
}

RexxMethod1(logical_t, rx3270_method_is_ready, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return false;
	return hSession->is_ready();
}

RexxMethod2(int, rx3270_method_wait_for_ready, CSELF, sessionPtr, OPTIONAL_int, seconds)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->wait_for_ready(seconds > 0 ? seconds : 60);
}

RexxMethod3(int, rx3270_method_set_cursor, CSELF, sessionPtr, int, row, int, col)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->set_cursor_position(row,col);
}

RexxMethod1(int, rx3270_method_get_cursor_addr, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->get_cursor_addr();
}

RexxMethod2(int, rx3270_method_set_cursor_addr, CSELF, sessionPtr, int, addr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->set_cursor_addr(addr);
}

RexxMethod1(int, rx3270_method_enter, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->enter();
}

RexxMethod2(int, rx3270_method_pfkey, CSELF, sessionPtr, int, key)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->pfkey(key);
}

RexxMethod2(int, rx3270_method_pakey, CSELF, sessionPtr, int, key)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->pakey(key);
}

RexxMethod4(RexxStringObject, rx3270_method_get_text_at, CSELF, sessionPtr, int, row, int, col, int, sz)
{
	rx3270	* session 	= (rx3270 *) sessionPtr;

	if(session)
	{
		char * str = session->get_text_at(row,col,sz);

		if(str)
		{
			char				* text	= session->get_local_string(str);
			RexxStringObject	  ret	= context->String((CSTRING) text);
			free(str);
			free(text);
			return ret;
		}
	}

	return context->String("");
}


RexxMethod4(int, rx3270_method_set_text_at, CSELF, sessionPtr, int, row, int, col, CSTRING, text)
{
	rx3270 * session = (rx3270 *) sessionPtr;

	if(session)
	{
		char	* str		= session->get_3270_string(text);
		int		  rc;
		rc = session->set_text_at(row,col,str);
		free(str);
		return rc;
	}
	return -1;
}

RexxMethod2(int, rx3270_method_input_text, CSELF, sessionPtr, CSTRING, text)
{
	rx3270	* session = (rx3270 *) sessionPtr;

	if(session)
	{
		char	* str	= session->get_3270_string(text);
		int		  rc    = session->emulate_input(str);
		free(str);
		return rc;
	}

	return -1;
}

RexxMethod4(int, rx3270_method_cmp_text_at, CSELF, sessionPtr, int, row, int, col, CSTRING, key)
{
	int		  rc		= 0;
	rx3270	* session	= (rx3270 *) sessionPtr;

	if(session)
	{
		char * str = session->get_text_at(row,col,strlen(key));
		if(str)
		{
			char * text	= session->get_3270_string(key);
			rc = strcasecmp(str,text);
			free(text);
		}
		free(str);
	}

	return rc;
}

RexxMethod2(int, rx3270_method_event_trace, CSELF, sessionPtr, int, flag)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	hSession->set_toggle(LIB3270_TOGGLE_EVENT_TRACE,flag);
	return 0;
}

RexxMethod2(int, rx3270_method_screen_trace, CSELF, sessionPtr, int, flag)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	hSession->set_toggle(LIB3270_TOGGLE_SCREEN_TRACE,flag);
	return 0;

}

RexxMethod2(int, rx3270_method_ds_trace, CSELF, sessionPtr, int, flag)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	hSession->set_toggle(LIB3270_TOGGLE_DS_TRACE,flag);
	return 0;
}

RexxMethod3(int, rx3270_method_set_option, CSELF, sessionPtr, CSTRING, name, int, flag)
{
	static const struct _toggle_info
	{
		const char		* name;
		LIB3270_TOGGLE	  id;
	}
	toggle[LIB3270_TOGGLE_COUNT] =
	{
			{ "monocase",		LIB3270_TOGGLE_MONOCASE				},
			{ "cursorblink",	LIB3270_TOGGLE_CURSOR_BLINK			},
			{ "showtiming",		LIB3270_TOGGLE_SHOW_TIMING			},
			{ "cursorpos",		LIB3270_TOGGLE_CURSOR_POS			},
			{ "dstrace",		LIB3270_TOGGLE_DS_TRACE				},
			{ "linewrap",		LIB3270_TOGGLE_LINE_WRAP			},
			{ "blankfill",		LIB3270_TOGGLE_BLANK_FILL			},
			{ "screentrace",	LIB3270_TOGGLE_SCREEN_TRACE			},
			{ "eventtrace",		LIB3270_TOGGLE_EVENT_TRACE			},
			{ "marginedpaste",	LIB3270_TOGGLE_MARGINED_PASTE		},
			{ "rectselect",		LIB3270_TOGGLE_RECTANGLE_SELECT		},
			{ "crosshair",		LIB3270_TOGGLE_CROSSHAIR			},
			{ "fullscreen",		LIB3270_TOGGLE_FULL_SCREEN			},
			{ "reconnect",		LIB3270_TOGGLE_RECONNECT			},
			{ "insert",			LIB3270_TOGGLE_INSERT				},
			{ "smartpaste",		LIB3270_TOGGLE_SMART_PASTE			},
			{ "bold",			LIB3270_TOGGLE_BOLD					},
			{ "keepselected",	LIB3270_TOGGLE_KEEP_SELECTED		},
			{ "underline",		LIB3270_TOGGLE_UNDERLINE			},
			{ "autoconnect",	LIB3270_TOGGLE_CONNECT_ON_STARTUP	},
			{ "kpalternative",	LIB3270_TOGGLE_KP_ALTERNATIVE		},
			{ "beep",			LIB3270_TOGGLE_BEEP					},
			{ "fieldattr",		LIB3270_TOGGLE_VIEW_FIELD			},
			{ "altscreen",		LIB3270_TOGGLE_ALTSCREEN			}
	};

	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(hSession)
	{
		for(int f = 0; f < LIB3270_TOGGLE_COUNT; f++)
		{
			if(!strcasecmp(name,toggle[f].name))
			{
				hSession->set_toggle(toggle[f].id,flag);
				return 0;
			}
		}
		return ENOENT;
	}
	return -1;
}


RexxMethod4(logical_t, rx3270_method_test, CSELF, sessionPtr, CSTRING, key, int, row, int, col)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(!hSession)
		return false;

	if(!hSession->is_ready())
		hSession->iterate(false);

	if(hSession->is_ready())
	{
		bool	  rc	= false;
		char	* str	= hSession->get_text_at(row,col,strlen(key));
		if(str)
		{
			char * text	= hSession->get_3270_string(key);
			rc = (strcasecmp(str,text) == 0);
			free(text);
		}
		free(str);
		return rc;
	}

	return false;
}

RexxMethod5(int, rx3270_method_wait_for_text_at, CSELF, sessionPtr, int, row, int, col, CSTRING, key, int, timeout)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(hSession)
		return hSession->wait_for_text_at(row,col,key,timeout);

	return -1;
}

RexxMethod3(RexxStringObject, rx3270_method_get_text, CSELF, sessionPtr, OPTIONAL_int, baddr, OPTIONAL_int, sz)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(hSession)
	{
		char *str = hSession->get_text(baddr,sz > 0 ? sz : -1);
		if(str)
		{
			char				* text	= hSession->get_local_string(str);
			RexxStringObject	  ret	= context->String((CSTRING) text);
			free(str);
			free(text);
			return ret;
		}
	}

	return context->String("");
}


RexxMethod2(int, rx3270_method_get_field_len, CSELF, sessionPtr, OPTIONAL_int, baddr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->get_field_len(baddr);
}

RexxMethod2(int, rx3270_method_get_field_start, CSELF, sessionPtr, OPTIONAL_int, baddr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->get_field_start(baddr)+1;
}

RexxMethod2(int, rx3270_method_get_next_unprotected, CSELF, sessionPtr, OPTIONAL_int, baddr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;

	baddr = hSession->get_next_unprotected(baddr);
	if(baddr < 1)
        return -1;

	return baddr;
}

RexxMethod1(RexxStringObject, rx3270_method_get_selection, CSELF, sessionPtr)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(hSession)
	{
		char *str = hSession->get_copy();
		if(str)
		{
			char				* text	= hSession->get_local_string(str);
			RexxStringObject	  ret	= context->String((CSTRING) text);
			free(str);
			free(text);
			return ret;
		}
	}

	return context->String("");
}

RexxMethod2(int, rx3270_method_set_selection, CSELF, sessionPtr, CSTRING, text)
{
	rx3270 * session = (rx3270 *) sessionPtr;

	if(session)
	{
		char	* str		= session->get_3270_string(text);
		int		  rc;
		rc = session->set_copy(str);
		free(str);
		return rc;
	}
	return -1;
}

RexxMethod1(RexxStringObject, rx3270_method_get_clipboard, CSELF, sessionPtr)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(hSession)
	{
		char *str = hSession->get_clipboard();

		trace("str=%p (%s)",str,str);

		if(str)
		{
			RexxStringObject ret = context->String((CSTRING) str);
			hSession->free(str);
			return ret;
		}
	}

    trace("%s","rx3270_method_get_clipboard: Clipboard is empty");
	return context->String("");
}

RexxMethod2(int, rx3270_method_set_clipboard, CSELF, sessionPtr, CSTRING, text)
{
	rx3270	* hSession = (rx3270 *) sessionPtr;

	if(hSession)
		return hSession->set_clipboard(text);

	return -1;
}


RexxMethod5(int, rx3270_method_popup, CSELF, sessionPtr, CSTRING, s_id, CSTRING, title, CSTRING, message, OPTIONAL_CSTRING, det)
{
    LIB3270_NOTIFY    id        = LIB3270_NOTIFY_INFO;
	rx3270          * hSession  = (rx3270 *) sessionPtr;

    if(!hSession)
        return -1;

    if(*s_id)
    {
        static const struct _descr
        {
            char            str;
            LIB3270_NOTIFY  id;
        } descr[] =
        {
            { 'I', LIB3270_NOTIFY_INFO       },
            { 'W', LIB3270_NOTIFY_WARNING    },
            { 'E', LIB3270_NOTIFY_ERROR      },
            { 'C', LIB3270_NOTIFY_CRITICAL   },
        };

        for(int f=0;f<4;f++)
        {
            if(toupper(*s_id) == descr[f].str)
            {
                id = descr[f].id;
                trace("Using mode %c (%d)",toupper(*s_id),(int) id);
            }
        }
    }

    return hSession->popup_dialog(id, title, message, "%s", det ? det : "");
}

RexxMethod5(RexxStringObject, rx3270_method_get_filename, CSELF, sessionPtr, CSTRING, action_name, CSTRING, title, OPTIONAL_CSTRING, extension, OPTIONAL_CSTRING, filename)
{
    static const struct _action
    {
        const gchar             * action_name;
        GtkFileChooserAction      id;
    } action[] =
    {
        { "open",           GTK_FILE_CHOOSER_ACTION_OPEN            },
        { "save",           GTK_FILE_CHOOSER_ACTION_SAVE            },
        { "folder",         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER   },
        { "select_folder",  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER   },
        { "create_folder",  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER   }
    };

    GtkFileChooserAction      id = GTK_FILE_CHOOSER_ACTION_OPEN;
    char                    * ret;

    for(int f=0;f<5;f++)
    {
        if(!strcasecmp(action_name,action[f].action_name))
        {
            id = action[f].id;
            break;
        }
    }

    ret = ((rx3270 *) sessionPtr)->file_chooser_dialog(id, title, extension,filename);
    if(ret)
    {
        RexxStringObject obj = context->String(ret);
        ((rx3270 *) sessionPtr)->free(ret);
        return obj;
    }

	return context->String("");
}

