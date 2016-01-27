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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como oia.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <pw3270.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/log.h>
 #include <lib3270/config.h>
 #include <gtk/gtk.h>
 #include <string.h>
 #include <errno.h>
 #include <ctype.h>

 #ifdef HAVE_LIBM
	#include <math.h>
 #endif // HAVE_LIBM

 #include <pw3270/v3270.h>
 #include "private.h"
 #include "accessible.h"

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void draw_cursor_position(cairo_t *cr, GdkRectangle *rect, v3270FontInfo *metrics, int row, int col);

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 #include "locked.xbm"
 #include "unlocked.xbm"
 #include "negotiated.xbm"
 #include "warning.xbm"

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void short2string(char *ptr, unsigned short vlr, size_t sz)
{
	int f;

	for(f=sz-1;f>=0;f--)
	{
		ptr[f] = '0'+(vlr%10);
		vlr /= 10;
	}
}


#ifdef HAVE_LIBM
static gint draw_spinner(cairo_t *cr, GdkRectangle *r, GdkRGBA *color, gint step)
{
	static const guint num_steps	= 10;

	gdouble dx = r->width/2;
	gdouble dy = r->height/2;
	gdouble radius = MIN (r->width / 2, r->height / 2);
	gdouble half = num_steps / 2;
	gint i;

	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_rectangle(cr, r->x, r->y, r->width, r->height);
	cairo_clip(cr);
	cairo_translate(cr, r->x, r->y);

	step++;
	step %= num_steps;

	for (i = 0; i < num_steps; i++)
	{
		gint inset = 0.7 * radius;

		/* transparency is a function of time and intial value */
		gdouble t = (gdouble) ((i + num_steps - step) % num_steps) / num_steps;

		cairo_save(cr);

		cairo_set_source_rgba (cr,
							 color[V3270_COLOR_OIA_SPINNER].red,
							 color[V3270_COLOR_OIA_SPINNER].green,
							 color[V3270_COLOR_OIA_SPINNER].blue,
							 t);

		cairo_set_line_width (cr, 2.0);
		cairo_move_to (cr,
					 dx + (radius - inset) * cos (i * G_PI / half),
					 dy + (radius - inset) * sin (i * G_PI / half));
		cairo_line_to (cr,
					 dx + radius * cos (i * G_PI / half),
					 dy + radius * sin (i * G_PI / half));
		cairo_stroke (cr);

		cairo_restore (cr);
	}

	cairo_restore(cr);

 	return step;
}
#endif // HAVE_LIBM

static void setup_cursor_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	rect->width = metrics->width * 8;
	rect->x -= rect->width;

	if(lib3270_get_toggle(host,LIB3270_TOGGLE_CURSOR_POS))
	{
		int addr = lib3270_get_cursor_address(host);
		draw_cursor_position(cr,rect,metrics,addr/cols,addr%cols);
	}
}

static void setup_timer_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	char buffer[7];
	cairo_text_extents_t extents;

	short2string(buffer,0,2);
	buffer[2] = ':';
	short2string(buffer+3,0,2);
	buffer[5] = 0;

	cairo_text_extents(cr,buffer,&extents);
	rect->width = ((int) extents.width + 2);
	rect->x -= rect->width;
}

static void setup_spinner_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	rect->width = rect->height;
	rect->x -= rect->width;
//	draw_spinner(cr,rect,color,0);
}

static void setup_luname_position(GdkRectangle *rect, v3270FontInfo *font, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	const char *luname = lib3270_get_luname(host);

	rect->width *= 16;
	rect->x -= rect->width;

	cairo_save(cr);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_clip(cr);

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	if(luname)
	{
//		cairo_move_to(cr,rect->x,rect->y+font->height);

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_LUNAME);
		v3270_draw_text(cr,rect,font,luname);

//		cairo_show_text(cr,luname);
//		cairo_stroke(cr);
	}

	cairo_restore(cr);

}

static void setup_single_char_right(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	rect->x -= rect->width;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif

}

static void setup_insert_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	if(rect->width > rect->height)
	{
		rect->width = rect->height;
	}
	else if(rect->height > rect->width)
	{
		rect->y += (rect->height - rect->width)/2;
		rect->height = rect->width;
	}

	rect->x -= rect->width;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif

}



static void setup_double_char_position(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color)
{
	rect->width <<= 1;
	rect->x -= rect->width;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);
#endif

}

static void draw_undera(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect)
{
#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	if(lib3270_get_undera(host))
	{
		const gchar *chr = lib3270_in_e(host) ? "B" : "A";
		cairo_text_extents_t extents;
		int x,y;

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);

		cairo_text_extents(cr,chr,&extents);

		x = rect->x + ((rect->width/2) - ((extents.width+extents.x_bearing)/2));
		y = rect->y + extents.height+((rect->height/2) - (extents.height/2));

		cairo_move_to(cr,x,y);
		cairo_show_text(cr,chr);

		cairo_move_to(cr,x+extents.x_bearing,y+2);
		cairo_rel_line_to(cr,extents.width,0);

		cairo_stroke(cr);

	}

}

static void draw_centered_char(cairo_t *cr, v3270FontInfo *metrics, int x, int y, const gchar chr)
{
	char str[2] = { chr, 0 };

	cairo_text_extents_t extents;
	cairo_text_extents(cr,str,&extents);

	cairo_move_to(cr,x+(((metrics->width+2)/2)-(extents.width/2)),y+extents.height+( (metrics->spacing/2) - (extents.height/2)));
	cairo_show_text(cr,str);

}

void v3270_draw_connection(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, const GdkRectangle *rect)
{
 	gchar str = ' ';

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_stroke(cr);

	if(lib3270_get_oia_box_solid(host))
	{
		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);
		return;
	}

	if(lib3270_in_ansi(host))
		str = 'N';
	else if(lib3270_in_sscp(host))
		str = 'S';
	else
		str = '?';

	draw_centered_char(cr,metrics,rect->x,rect->y,str);

}

static void draw_xbm(cairo_t *cr, GdkRectangle *rect, int width, int height, unsigned char *bits)
{
	double			  sz	= rect->width < rect->height ? rect->width : rect->height;
	cairo_surface_t	* icon	= cairo_image_surface_create_for_data(
									bits,
									CAIRO_FORMAT_A1,
									width,height,
									cairo_format_stride_for_width(CAIRO_FORMAT_A1,width));

	cairo_save(cr);

	cairo_scale(cr,	sz / ((double) width),
					sz / ((double) height));

	cairo_mask_surface(cr,icon,(rect->width-sz)/2,(rect->height-sz)/2);

	cairo_surface_destroy(icon);

	cairo_restore(cr);
}

void v3270_draw_ssl_status(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect)
{
#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_translate(cr, rect->x, rect->y);
	cairo_rectangle(cr, 0, 0, rect->width, rect->height);
	cairo_fill(cr);

	switch(lib3270_get_secure(host))
	{
	case LIB3270_SSL_UNSECURE:	/**< No secure connection */
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);
		draw_xbm(cr,rect,unlocked_width,unlocked_height,unlocked_bits);
		break;

	case LIB3270_SSL_NEGOTIATING:	/**< Negotiating SSL */
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_STATUS_WARNING);
		draw_xbm(cr,rect,negotiated_width,negotiated_height,negotiated_bits);
		break;

	case LIB3270_SSL_NEGOTIATED:	/**< Connection secure, no CA or self-signed */
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_STATUS_OK);
		draw_xbm(cr,rect,locked_width,locked_height,locked_bits);
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_STATUS_WARNING);
		draw_xbm(cr,rect,warning_width,warning_height,warning_bits);
		break;

	case LIB3270_SSL_SECURE:	/**< Connection secure with CA check */
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_STATUS_OK);
		draw_xbm(cr,rect,locked_width,locked_height,locked_bits);
		break;

	default:
		return;

	}


}

static void draw_status_message(cairo_t *cr, LIB3270_MESSAGE id, v3270FontInfo *font, GdkRGBA *color, const GdkRectangle *r)
{
	#ifdef DEBUG
		#define OIA_MESSAGE(x,c,y) { #x, c, y }
	#else
		#define OIA_MESSAGE(x,c,y) { c, y }
	#endif

	static const struct _message
	{
	#ifdef DEBUG
		const gchar			* dbg;
	#endif
		enum V3270_COLOR
			  color;
		const gchar			* msg;
	} message[] =
 	{
		OIA_MESSAGE(	LIB3270_MESSAGE_NONE,
						V3270_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	LIB3270_MESSAGE_SYSWAIT,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X System" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_TWAIT,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X Wait" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_CONNECTED,
						V3270_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	LIB3270_MESSAGE_DISCONNECTED,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Not Connected" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_AWAITING_FIRST,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_MINUS,
						V3270_COLOR_OIA_STATUS_OK,
						N_( "X -f" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_PROTECTED,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Protected" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_NUMERIC,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Numeric" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_OVERFLOW,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Overflow" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_INHIBIT,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X Inhibit" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_KYBDLOCK,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X") ),

		OIA_MESSAGE(	LIB3270_MESSAGE_X,
						V3270_COLOR_OIA_STATUS_INVALID,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_RESOLVING,
						V3270_COLOR_OIA_STATUS_WARNING,
						N_( "X Resolving" ) ),

		OIA_MESSAGE(	LIB3270_MESSAGE_CONNECTING,
						V3270_COLOR_OIA_STATUS_WARNING,
						N_( "X Connecting" ) ),


	};

	GdkRectangle	  rect;
	const gchar		* msg = message[0].msg;

	memcpy(&rect,r,sizeof(GdkRectangle));

//	trace("%s: id=%d",__FUNCTION__,id);

	if(id >= 0 && id < G_N_ELEMENTS(message))
	{
		msg = message[id].msg;
#ifdef DEBUG
		if(!msg)
			msg = message[id].dbg;
#endif // DEBUG
	}

	// Limpa o bloco
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, rect.x, rect.y, rect.width, rect.height);
	cairo_fill(cr);

	if(msg && *msg)
	{
		msg = gettext(msg);
	}

	if(msg && *msg)
	{
		rect.x++;

		debug("%s(%s)",__FUNCTION__,msg);

		gdk_cairo_set_source_rgba(cr,color+message[id].color);

		if(*msg == 'X')
		{
			cairo_save(cr);

			cairo_move_to(cr,rect.x+1,rect.y+(font->height)-(font->ascent));
			cairo_rel_line_to(cr,font->width,font->ascent);
			cairo_rel_move_to(cr,-font->width,0);
			cairo_rel_line_to(cr,font->width,-font->ascent);

			cairo_stroke(cr);
			rect.x += font->width;
			msg++;

			cairo_restore(cr);
		}

		while(isspace(*msg))
		{
			msg++;
			rect.x += font->width;
		}

		if(*msg)
		{
			v3270_draw_text(cr,&rect, font, msg);
//			cairo_move_to(cr,x,rect->y+metrics->height);
//			cairo_show_text(cr,msg);
		}

	}

}

static void draw_insert(cairo_t *cr, H3270 *host, GdkRGBA *color, GdkRectangle *rect)
{
	if(lib3270_get_toggle(host,LIB3270_TOGGLE_INSERT))
	{
		double y = rect->y+(rect->height-2);

		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_clip(cr);

		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);

		cairo_move_to(cr,rect->x,y);
		cairo_rel_line_to(cr,rect->width/2,-(rect->height/1.7));
		cairo_line_to(cr,rect->x+rect->width,y);
		cairo_stroke(cr);
	}

}

void v3270_draw_oia(cairo_t *cr, H3270 *host, int row, int cols, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect)
{
	static const struct _right_fields
	{
		V3270_OIA_FIELD id;
		void (*draw)(GdkRectangle *rect, v3270FontInfo *metrics, cairo_t *cr, H3270 *host, int cols, GdkRGBA *color);
	} right[] =
	{
		{ V3270_OIA_CURSOR_POSITION,	setup_cursor_position 		},
		{ V3270_OIA_TIMER,				setup_timer_position		},
		{ V3270_OIA_SPINNER, 			setup_spinner_position		},
		{ V3270_OIA_LUNAME, 			setup_luname_position		},
#ifdef HAVE_PRINTER
		{ V3270_OIA_PRINTER,			setup_single_char_right		},
#endif // HAVE_PRINTER
		{ V3270_OIA_SCRIPT,				setup_single_char_right		},
		{ V3270_OIA_INSERT,				setup_insert_position		},
		{ V3270_OIA_TYPEAHEAD,			setup_single_char_right		},
		{ V3270_OIA_SHIFT,				setup_double_char_position	},
//		{ V3270_OIA_CAPS,				setup_single_char_right		},
		{ V3270_OIA_ALT,				setup_single_char_right		},
		{ V3270_OIA_SSL,				setup_double_char_position	},
	};

	int f;
	int rCol = metrics->left+(cols*metrics->width);
	int lCol = metrics->left+1;

	row += OIA_TOP_MARGIN;
	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_SEPARATOR);
	cairo_rectangle(cr, metrics->left, row, cols*metrics->width, 1);
	cairo_fill(cr);

	row += 2;

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, metrics->left, row, cols*metrics->width, metrics->spacing);
	cairo_fill(cr);

	for(f=0;f<G_N_ELEMENTS(right);f++)
	{
		GdkRectangle *r = rect+right[f].id;

		memset(r,0,sizeof(GdkRectangle));
		r->x = rCol;
		r->y = row;
		r->width  = metrics->width;
		r->height = metrics->spacing;
		gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);
		right[f].draw(r,metrics,cr,host,cols,color);
		rCol = r->x - (metrics->width/3);
	}

	gdk_cairo_set_source_rgba(cr,color+V3270_COLOR_OIA_FOREGROUND);

	draw_centered_char(cr,metrics,lCol,row,'4');

	cairo_stroke(cr);
	cairo_rectangle(cr, lCol, row, metrics->width+2, metrics->spacing);
	cairo_stroke(cr);

	lCol += (metrics->width+5);

	// Undera indicator
	rect[V3270_OIA_UNDERA].x = lCol;
	rect[V3270_OIA_UNDERA].y = row;
	rect[V3270_OIA_UNDERA].width  = metrics->width+3;
	rect[V3270_OIA_UNDERA].height = metrics->spacing;
	draw_undera(cr,host,metrics,color,rect+V3270_OIA_UNDERA);

	lCol += (3 + rect[V3270_OIA_UNDERA].width);

	// Connection indicator
	rect[V3270_OIA_CONNECTION].x = lCol;
	rect[V3270_OIA_CONNECTION].y = row;
	rect[V3270_OIA_CONNECTION].width  = metrics->width+3;
	rect[V3270_OIA_CONNECTION].height = metrics->spacing;
	v3270_draw_connection(cr,host,metrics,color,rect+V3270_OIA_CONNECTION);

	lCol += (4 + rect[V3270_OIA_CONNECTION].width);

	memset(rect+V3270_OIA_MESSAGE,0,sizeof(GdkRectangle));

	if(lCol < rCol)
	{
		GdkRectangle *r = rect+V3270_OIA_MESSAGE;
		r->x = lCol;
		r->y = row;
		r->width  = rCol - lCol;
		r->height = metrics->spacing;
		draw_status_message(cr,lib3270_get_program_message(host),metrics,color,r);
	}

	cairo_save(cr);
	v3270_draw_ssl_status(cr,host,metrics,color,rect+V3270_OIA_SSL);
	cairo_restore(cr);

	cairo_save(cr);
	draw_insert(cr,host,color,rect+V3270_OIA_INSERT);
	cairo_restore(cr);
}

/**
 * Begin update of a specific OIA field.
 *
 * @param terminal	3270 terminal widget.
 * @param r			Rectangle to receive updated region.
 * @param id		Field id.
 *
 * @return cairo object for drawing.
 *
 */
static cairo_t * set_update_region(v3270 * terminal, GdkRectangle **r, V3270_OIA_FIELD id)
{
	GdkRectangle	* rect		= terminal->oia_rect + id;
	cairo_t 		* cr		= cairo_create(terminal->surface);

	cairo_set_scaled_font(cr,terminal->font.scaled);

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_clip(cr);

	*r = rect;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_FOREGROUND);

	return cr;
}

void v3270_update_luname(GtkWidget *widget,const gchar *name)
{
	cairo_t 		* cr;
	GdkRectangle	* rect;
	v3270			* terminal = GTK_V3270(widget);

	if(terminal->surface)
	{
		cr = set_update_region(terminal,&rect,V3270_OIA_LUNAME);

		if(name)
		{
			cairo_move_to(cr,rect->x,rect->y+terminal->font.height);
			gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_OIA_LUNAME);
			cairo_show_text(cr,name);
			cairo_stroke(cr);
		}

		cairo_destroy(cr);

		v3270_queue_draw_area(GTK_WIDGET(terminal),rect->x,rect->y,rect->width,rect->height);
	}

#if GTK_CHECK_VERSION(2,26,0)
	g_object_notify_by_pspec(G_OBJECT(widget), v3270_properties[PROP_LUNAME]);
#else
	g_object_notify(G_OBJECT(widget),"luname");
#endif // GTK_CHECK_VERSION

}

void v3270_update_message(v3270 *widget, LIB3270_MESSAGE id)
{
	cairo_t 		* cr;
	GdkRectangle	* rect;

	if(!widget->surface)
		return;

	cr = set_update_region(widget,&rect,V3270_OIA_MESSAGE);

	draw_status_message(cr,id,&widget->font,widget->color,rect);

    cairo_destroy(cr);

	v3270_queue_draw_area(GTK_WIDGET(widget),rect->x,rect->y,rect->width,rect->height);

	if(widget->accessible)
		v3270_acessible_set_state(widget->accessible,id);

}

static void draw_cursor_position(cairo_t *cr, GdkRectangle *rect, v3270FontInfo *metrics, int row, int col)
{
	cairo_text_extents_t extents;
	char buffer[10];

	short2string(buffer,row+1,3);
	buffer[3] = '/';
	short2string(buffer+4,col+1,3);
	buffer[7] = 0;

	cairo_text_extents(cr,buffer,&extents);

	cairo_move_to(cr,(rect->x+rect->width)-(extents.width+2),rect->y+metrics->height);
	cairo_show_text(cr, buffer);
	cairo_stroke(cr);
}

void v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr)
{
	v3270				* terminal = GTK_V3270(session->user_data);
	GdkRectangle		  saved;

	if(!terminal->surface)
		return;

	// Update cursor rectangle
	saved = terminal->cursor.rect;

	terminal->cursor.rect.x          = terminal->font.left + (col * terminal->cursor.rect.width);
	terminal->cursor.rect.y          = terminal->font.top  + (row * terminal->font.spacing);
	terminal->cursor.rect.width      = terminal->font.width;
	terminal->cursor.rect.height     = terminal->font.height+terminal->font.descent;
	terminal->cursor.show |= 1;

	v3270_queue_draw_area( GTK_WIDGET(terminal),	saved.x,
													saved.y,
													saved.width,
													saved.height);


	v3270_update_cursor_surface(terminal,c,attr);

	v3270_queue_draw_area(	GTK_WIDGET(terminal),
							terminal->cursor.rect.x,terminal->cursor.rect.y,
							terminal->cursor.rect.width,terminal->cursor.rect.height);

	if(lib3270_get_toggle(session,LIB3270_TOGGLE_CROSSHAIR))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(GTK_WIDGET(terminal), &allocation);

		v3270_queue_draw_area(GTK_WIDGET(terminal),0,saved.y+terminal->font.height,allocation.width,1);
		v3270_queue_draw_area(GTK_WIDGET(terminal),saved.x,0,1,terminal->oia_rect->y-3);

		v3270_queue_draw_area(GTK_WIDGET(terminal),0,terminal->cursor.rect.y+terminal->font.height,allocation.width,1);
		v3270_queue_draw_area(GTK_WIDGET(terminal),terminal->cursor.rect.x,0,1,terminal->oia_rect->y-3);
	}

	if(lib3270_get_toggle(session,LIB3270_TOGGLE_CURSOR_POS))
	{
		// Update OIA
		GdkRectangle	* rect;
		cairo_t 		* cr;

		cr = set_update_region(terminal,&rect,V3270_OIA_CURSOR_POSITION);

		draw_cursor_position(cr,rect,&terminal->font,row,col);

		cairo_destroy(cr);

		v3270_queue_draw_area(GTK_WIDGET(terminal),rect->x,rect->y,rect->width,rect->height);
	}

	if(terminal->accessible)
		g_signal_emit_by_name(ATK_TEXT(terminal->accessible),"text-caret-moved",lib3270_get_cursor_address(session));

}

struct timer_info
{
	time_t	  start;
	time_t	  last;
#ifdef HAVE_LIBM
	gint	  step;
#endif // HAVE_LIBM
	v3270	* terminal;
};

static void release_timer(struct timer_info *info)
{
//	trace("Timer %p stops",info);
	info->terminal->timer = NULL;

	if(info->terminal->surface)
	{
		// Erase timer info
		static const int id[] = {	V3270_OIA_TIMER,
#ifdef HAVE_LIBM
									V3270_OIA_SPINNER
#endif // HAVE_LIBM
								};
		int f;

		cairo_t *cr = cairo_create(info->terminal->surface);

#ifdef DEBUG
		cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

		for(f=0;f<G_N_ELEMENTS(id);f++)
		{
			GdkRectangle *rect = info->terminal->oia_rect + id[f];
			cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
			cairo_fill(cr);
			v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);
		}
		cairo_destroy(cr);
	}

	g_free(info);
}

void v3270_draw_shift_status(v3270 *terminal)
{
	GdkRectangle *r;
	cairo_t *cr;

	if(!terminal->surface)
		return;

	cr = set_update_region(terminal,&r,V3270_OIA_SHIFT);
	cairo_translate(cr, r->x, r->y+1);

	trace("%s: %s",__FUNCTION__,(terminal->keyflags & KEY_FLAG_SHIFT) ? "Yes" : "No");

	if(r->width > 2 && r->height > 7 && (terminal->keyflags & KEY_FLAG_SHIFT))
	{
		int b,x,y,w,h,l;
		int height = r->height-6;

		if(height > r->width)
		{
			w = r->width;
			h = w*1.5;
		}
		else // width > height
		{
			h = height;
			w = h/1.5;
		}

		// Set image position
		x = (r->width - w)/2;
		y = (height - h)/2;
		l = (w/3);
		b = y+(w/1.5);

		cairo_move_to(cr,x+(w/2),y);
		cairo_line_to(cr,x+w,b);
		cairo_line_to(cr,(x+w)-l,b);
		cairo_line_to(cr,(x+w)-l,y+h);
		cairo_line_to(cr,x+l,y+h);
		cairo_line_to(cr,x+l,b);
		cairo_line_to(cr,x,b);
		cairo_close_path(cr);

		cairo_stroke(cr);

	}

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);

}

static void update_text_field(v3270 *terminal, gboolean flag, V3270_OIA_FIELD id, const gchar chr)
{
	GdkRectangle	* r;
	cairo_t 		* cr;
	gchar			  text[] = { chr, 0 };

	if(!terminal->surface)
		return;

	cr = set_update_region(terminal,&r,id);
	cairo_translate(cr, r->x, r->y);

	if(flag)
	{
		cairo_move_to(cr,0,terminal->font.height);
		cairo_show_text(cr, text);
		cairo_stroke(cr);
	}

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
}

void v3270_draw_alt_status(v3270 *terminal)
{
#ifdef KEY_FLAG_ALT
	update_text_field(terminal,terminal->keyflags & KEY_FLAG_ALT,V3270_OIA_ALT,'A');
#endif // KEY_FLAG_ALT
}

void v3270_draw_ins_status(v3270 *terminal)
{
	GdkRectangle *r;
	cairo_t *cr;

	if(!terminal->surface)
		return;

	cr = set_update_region(terminal,&r,V3270_OIA_INSERT);

	draw_insert(cr,terminal->host,terminal->color,r);

    cairo_destroy(cr);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);

}

static gboolean update_timer(struct timer_info *info)
{
	cairo_t			* cr;
	time_t			  now = time(0);
	GdkRectangle	* rect;

	if(!info->terminal->surface)
		return TRUE;

	cr = cairo_create(info->terminal->surface);

	if(now != info->last)
	{
		time_t seconds = now - info->start;
		char buffer[7];

		rect = info->terminal->oia_rect + V3270_OIA_TIMER;

#ifdef DEBUG
		cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

		cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
		cairo_fill(cr);

		gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_FOREGROUND);

		short2string(buffer,seconds/60,2);
		buffer[2] = ':';
		short2string(buffer+3,seconds%60,2);
		buffer[5] = 0;

		cairo_set_scaled_font(cr,info->terminal->font.scaled);
		cairo_move_to(cr,rect->x,rect->y+info->terminal->font.height);
		cairo_show_text(cr, buffer);
		cairo_stroke(cr);

		info->last = now;
		v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);
	}

#ifdef HAVE_LIBM

    rect = info->terminal->oia_rect + V3270_OIA_SPINNER;

#ifdef DEBUG
	cairo_set_source_rgb(cr,0.1,0.1,0.1);
#else
	gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_BACKGROUND);
#endif

    cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
    cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr,info->terminal->color+V3270_COLOR_OIA_FOREGROUND);

    info->step = draw_spinner(cr, rect, info->terminal->color, info->step);

    v3270_queue_draw_area(GTK_WIDGET(info->terminal),rect->x,rect->y,rect->width,rect->height);

#endif // HAVE_LIBM

    cairo_destroy(cr);

	return TRUE;
}

void v3270_start_timer(GtkWidget *widget)
{
	struct timer_info *info;
	v3270 *terminal = GTK_V3270(widget);

	if(terminal->timer)
	{
		g_source_ref(terminal->timer);
		return;
	}

	info = g_new0(struct timer_info,1);
	info->terminal	= terminal;
	info->start		= time(0);

	terminal->timer = g_timeout_source_new(100);
	g_source_set_callback(terminal->timer,(GSourceFunc) update_timer, info, (GDestroyNotify) release_timer);

	g_source_attach(terminal->timer, NULL);
	g_source_unref(terminal->timer);

	trace("Timer %p starts",info);
}

void v3270_stop_timer(GtkWidget *widget)
{
	v3270 *terminal = GTK_V3270(widget);

	if(!terminal->timer)
		return;

//	trace("Timer=%p",terminal->timer);
	if(terminal->timer->ref_count < 2)
		g_source_destroy(terminal->timer);

	if(terminal->timer)
		g_source_unref(terminal->timer);

}

void v3270_update_ssl(H3270 *session, LIB3270_SSL_STATE state)
{
	v3270 			* terminal = GTK_V3270(session->user_data);
	cairo_t			* cr;
	GdkRectangle	* r;

	if(!terminal->surface)
		return;

	cr = set_update_region(terminal,&r,V3270_OIA_SSL);
	v3270_draw_ssl_status(cr,terminal->host,&terminal->font,terminal->color,r);
	v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
	cairo_destroy(cr);

}

void v3270_update_oia(H3270 *session, LIB3270_FLAG id, unsigned char on)
{
	cairo_t *cr;
	GdkRectangle *r;

	v3270 *terminal = GTK_V3270(session->user_data);

	if(!(terminal->surface && terminal->drawing))
		return;

	switch(id)
	{
	case LIB3270_FLAG_BOXSOLID:
		debug("%s",__FUNCTION__);
		cr = set_update_region(terminal,&r,V3270_OIA_CONNECTION);
		v3270_draw_connection(cr,terminal->host,&terminal->font,terminal->color,r);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		cairo_destroy(cr);
		break;

	case LIB3270_FLAG_UNDERA:
		debug("%s",__FUNCTION__);
		cr = set_update_region(terminal,&r,V3270_OIA_UNDERA);
		draw_undera(cr,terminal->host,&terminal->font,terminal->color,r);
		v3270_queue_draw_area(GTK_WIDGET(terminal),r->x,r->y,r->width,r->height);
		cairo_destroy(cr);
		break;

	case LIB3270_FLAG_TYPEAHEAD:
		debug("%s",__FUNCTION__);
		update_text_field(terminal,on,V3270_OIA_TYPEAHEAD,'T');
		break;

#ifdef HAVE_PRINTER
	case LIB3270_FLAG_PRINTER:
		debug("%s",__FUNCTION__);
		update_text_field(terminal,on,V3270_OIA_PRINTER,'P');
		break;
#endif // HAVE_PRINTER

/*
	case LIB3270_FLAG_SCRIPT:
		update_text_field(terminal,on,V3270_OIA_SCRIPT,terminal->script_id);
		break;
*/

	default:
		return;
	}

	debug("%s",__FUNCTION__);
}

static gboolean blink_script(v3270 *widget)
{
	if(!widget->script.id)
		return FALSE;

	update_text_field(widget,1,V3270_OIA_SCRIPT,widget->script.blink ? 'S' : ' ');
	widget->script.blink = !widget->script.blink;

	return TRUE;
}

static void release_script(v3270 *widget)
{
	widget->script.timer = NULL;
	widget->script.id = 0;
}

LIB3270_EXPORT int v3270_set_script(GtkWidget *widget, const gchar id, gboolean on)
{
	v3270 *terminal;
	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	terminal = GTK_V3270(widget);

	if(terminal->script.id && id != terminal->script.id)
		return EBUSY;

	terminal->script.id = on ? id : 0;
	update_text_field(terminal,on,V3270_OIA_SCRIPT,'S');

	if(on)
	{
		if(!terminal->script.timer)
		{
			terminal->script.timer = g_timeout_source_new(500);
			g_source_set_callback(terminal->script.timer,(GSourceFunc) blink_script, terminal, (GDestroyNotify) release_script);

			g_source_attach(terminal->script.timer, NULL);
			g_source_unref(terminal->script.timer);
		}
	}
	else if(terminal->script.timer)
	{
		if(terminal->script.timer->ref_count < 2)
			g_source_destroy(terminal->script.timer);

		if(terminal->timer)
			g_source_unref(terminal->script.timer);
	}

	return 0;
}
