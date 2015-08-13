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
 * Este programa está nomeado como draw.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <math.h>
 #include <pw3270.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <pw3270/v3270.h>
 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

gboolean v3270_draw(GtkWidget * widget, cairo_t * cr)
{
	v3270 * terminal = GTK_V3270(widget);

	cairo_set_source_surface(cr,terminal->surface,0,0);
	cairo_paint(cr);

	if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_CROSSHAIR) && (terminal->cursor.show&2))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(widget, &allocation);

		gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_CROSS_HAIR);

		cairo_rectangle(cr,	0,terminal->cursor.rect.y+terminal->font.height,allocation.width,1);
		cairo_fill(cr);

		cairo_rectangle(cr,	terminal->cursor.rect.x,0,1,terminal->oia_rect->y-3);
		cairo_fill(cr);
	}

	if(terminal->cursor.show == 3)
	{
		cairo_set_source_surface(cr,terminal->cursor.surface,terminal->cursor.rect.x,terminal->cursor.rect.y);

		if(lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_INSERT))
		{
			cairo_rectangle(cr,	terminal->cursor.rect.x,
								terminal->cursor.rect.y,
								terminal->cursor.rect.width,
								terminal->cursor.rect.height );
		}
		else
		{
			cairo_rectangle(cr,	terminal->cursor.rect.x,
								terminal->cursor.rect.y+terminal->font.height,
								terminal->cursor.rect.width,
								terminal->font.descent );
		}

		cairo_fill(cr);
	}

	return FALSE;
}

#if( !GTK_CHECK_VERSION(3,0,0))
gboolean v3270_expose(GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t *cr = gdk_cairo_create(widget->window);
	v3270_draw(widget,cr);
    cairo_destroy(cr);
    return FALSE;
}
#endif // GTk3


static void get_element_colors(unsigned short attr, GdkRGBA **fg, GdkRGBA **bg, GdkRGBA *color)
{
	if(attr & LIB3270_ATTR_SELECTED)
	{
		*fg = color+V3270_COLOR_SELECTED_FG;
		*bg = color+V3270_COLOR_SELECTED_BG;
	}
	else
	{
		*bg = color+((attr & 0x00F0) >> 4);

		if(attr & LIB3270_ATTR_FIELD)
			*fg = color+(attr & 0x0003)+V3270_COLOR_FIELD;
		else
			*fg = color+(attr & 0x000F);
	}
}

void v3270_draw_element(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, guint height, GdkRectangle *rect, GdkRGBA *color)
{
	GdkRGBA *fg;
	GdkRGBA *bg;

	get_element_colors(attr,&fg,&bg,color);
	v3270_draw_char(cr,chr,attr,session,height,rect,fg,bg);

	if(attr & LIB3270_ATTR_UNDERLINE)
	{
		cairo_scaled_font_t		* font	= cairo_get_scaled_font(cr);
		cairo_font_extents_t	  extents;
		double					  sl;

		cairo_scaled_font_extents(font,&extents);

		sl = extents.descent/3;
		if(sl < 1)
			sl = 1;

		gdk_cairo_set_source_rgba(cr,fg);

		cairo_rectangle(cr,rect->x,rect->y+sl+extents.ascent+(extents.descent/2),rect->width,sl);
		cairo_fill(cr);

		cairo_stroke(cr);
	}

}

void v3270_draw_text(cairo_t *cr, const GdkRectangle *rect, guint height, const char *str) {

	cairo_status_t		 		  status;
	cairo_glyph_t				* glyphs			= NULL;
	int							  num_glyphs		= 0;
	cairo_text_cluster_t		* clusters			= NULL;
	int							  num_clusters		= 0;
	cairo_text_cluster_flags_t	  cluster_flags;
	cairo_scaled_font_t			* scaled_font		= cairo_get_scaled_font (cr);

	status = cairo_scaled_font_text_to_glyphs(
					scaled_font,
					(double) rect->x, (double) (rect->y+height),
					str, strlen(str),
					&glyphs, &num_glyphs,
					&clusters, &num_clusters, &cluster_flags );

	if (status == CAIRO_STATUS_SUCCESS) {
		cairo_show_text_glyphs (cr,str,strlen(str),glyphs, num_glyphs,clusters, num_clusters, cluster_flags);
	}

    if(glyphs)
		cairo_glyph_free(glyphs);

    if(clusters)
		cairo_text_cluster_free(clusters);

/*
	cairo_move_to(cr,rect->x,rect->y+height);
	cairo_show_text(cr, str);
*/

}

void v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, guint height, GdkRectangle *rect, GdkRGBA *fg, GdkRGBA *bg)
{
	// Clear element area
	gdk_cairo_set_source_rgba(cr,bg);
	cairo_rectangle(cr, rect->x, rect->y, rect->width, rect->height);
	cairo_fill(cr);

	// Set foreground color
	gdk_cairo_set_source_rgba(cr,fg);

	// Draw char
	if( (attr & LIB3270_ATTR_MARKER) && lib3270_get_toggle(session,LIB3270_TOGGLE_VIEW_FIELD) )
	{
		double sz = (double) rect->width;
		if(rect->height < rect->width)
			sz = (double) rect->height;

		cairo_save(cr);

		sz /= 10;

		cairo_translate(cr, rect->x + (rect->width / 2), rect->y + (rect->height / 2));
		cairo_scale(cr, sz, sz);
		cairo_arc(cr, 0., 0., 1., 0., 2 * M_PI);

		cairo_restore(cr);
	}
	else if(attr & LIB3270_ATTR_CG)
	{
		switch(chr)
		{
		case 0xd3: // CG 0xab, plus
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0xa2: // CG 0x92, horizontal line
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0x85: // CG 0x184, vertical line
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xd4: // CG 0xac, LR corner
			cairo_move_to(cr,rect->x, rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_rel_line_to(cr,0,-(rect->height/2));
			break;

		case 0xd5: // CG 0xad, UR corner
			cairo_move_to(cr,rect->x, rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_rel_line_to(cr,0,rect->height/2);
			break;

		case 0xc5: // CG 0xa4, UL corner
			cairo_move_to(cr,rect->x+rect->width,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_rel_line_to(cr,0,(rect->height/2));
			break;

		case 0xc4: // CG 0xa3, LL corner
			cairo_move_to(cr,rect->x+rect->width,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_rel_line_to(cr,0,-(rect->height/2));
			break;

		case 0xc6: // CG 0xa5, left tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width/2,0);
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xd6: // CG 0xae, right tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,-(rect->width/2),0);
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y);
			cairo_rel_line_to(cr,0,rect->height);
			break;

		case 0xc7: // CG 0xa6, bottom tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,0,-(rect->height/2));
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0xd7: // CG 0xaf, top tee
			cairo_move_to(cr,rect->x+(rect->width/2),rect->y+(rect->height/2));
			cairo_rel_line_to(cr,0,rect->height/2);
			cairo_move_to(cr,rect->x,rect->y+(rect->height/2));
			cairo_rel_line_to(cr,rect->width,0);
			break;

		case 0x8c: // CG 0xf7, less or equal "≤"
			v3270_draw_text(cr,rect,height,"≤");
			break;

		case 0xae: // CG 0xd9, greater or equal "≥"
			v3270_draw_text(cr,rect,height,"≥");
			break;

		case 0xbe: // CG 0x3e, not equal "≠"
			v3270_draw_text(cr,rect,height,"≠");
			break;

		case 0xad: // "["
			v3270_draw_text(cr,rect,height,"[");
			break;

		case 0xbd: // "]"
			v3270_draw_text(cr,rect,height,"]");
			break;

		default:
			cairo_rectangle(cr, rect->x+1, rect->y+1, rect->width-2, rect->height-2);
		}
	}
	else if(chr)
	{
		gchar *utf = g_convert((char *) &chr, 1, "UTF-8", lib3270_get_display_charset(session), NULL, NULL, NULL);

		if(utf)
		{
			v3270_draw_text(cr,rect,height,utf);
			g_free(utf);
		}
	}

	cairo_stroke(cr);
}

#if !GTK_CHECK_VERSION(2, 22, 0)
cairo_surface_t *gdk_window_create_similar_surface(GdkWindow *window, cairo_content_t content, int width, int height)
{
	cairo_t *cairoContext = gdk_cairo_create(window);
	cairo_surface_t *cairoSurface = cairo_get_target(cairoContext);
	cairo_surface_t *newSurface = cairo_surface_create_similar(cairoSurface, content, width, height);
	cairo_destroy(cairoContext);
	return newSurface;
}
#endif // GTK_CHECK_VERSION(2, 22, 0)

void v3270_reload(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

#if GTK_CHECK_VERSION(3,0,0)
	gint width	= gtk_widget_get_allocated_width(widget);
	gint height	= gtk_widget_get_allocated_height(widget);
#else
    gint width	= terminal->width;
    gint height	= terminal->height;
#endif

	GdkRectangle rect;
	int addr, cursor, r, rows, cols;

	cairo_t * cr;

	if(!gtk_widget_get_realized(widget))
		return;

	// Create new terminal image
	if(terminal->surface)
		cairo_surface_destroy(terminal->surface);

	terminal->surface = (cairo_surface_t *) gdk_window_create_similar_surface(gtk_widget_get_window(widget),CAIRO_CONTENT_COLOR,width,height);

	// Update the created image
	cr = cairo_create(terminal->surface);
	v3270_update_font_metrics(terminal, cr, width, height);

	gdk_cairo_set_source_rgba(cr,terminal->color+V3270_COLOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);
	cairo_stroke(cr);

    // Draw terminal contents
	lib3270_get_screen_size(terminal->host,&rows,&cols);

	memset(&rect,0,sizeof(rect));
	rect.y		= terminal->font.top;
	rect.width	= terminal->font.width;
	rect.height	= terminal->font.spacing;
	addr 		= 0;
	cursor		= lib3270_get_cursor_address(terminal->host);

	for(r = 0; r < rows; r++)
	{
		int c;

		rect.x = terminal->font.left;

		for(c=0;c < cols;c++)
		{
			unsigned char	  chr = 0;
			unsigned short	  attr;

			lib3270_get_contents(terminal->host,addr,addr,&chr,&attr);

			if(addr == cursor)
				v3270_update_cursor_rect(terminal,&rect,chr,attr);

			v3270_draw_element(cr,chr,attr,terminal->host,terminal->font.height,&rect,terminal->color);

			addr++;
			rect.x += rect.width;
		}

		rect.y += terminal->font.spacing;

	}

	cairo_set_scaled_font(cr,terminal->font.scaled);
	v3270_draw_oia(cr, terminal->host, rect.y, cols, &terminal->font, terminal->color,terminal->oia_rect);

    cairo_destroy(cr);

}

void v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor)
{
	v3270			* terminal = GTK_V3270(session->user_data);
	cairo_t			* cr;
	GdkRectangle	  rect;
	int				  rows,cols;

	if(!gtk_widget_get_realized(GTK_WIDGET(terminal)))
		return;

	if(!terminal->surface)
	{
		v3270_reload(GTK_WIDGET(terminal));
		gtk_widget_queue_draw(GTK_WIDGET(terminal));
		return;
	}

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	memset(&rect,0,sizeof(rect));
	rect.x          = terminal->font.left + ((addr % cols) * terminal->font.width);
	rect.y          = terminal->font.top  + ((addr / cols) * terminal->font.spacing);
	rect.width      = terminal->font.width;
	rect.height     = terminal->font.spacing;

//	trace("%s: c=%c attr=%04x addr=%d pos=%d,%d x=%d y=%d w=%d h=%d",__FUNCTION__,chr,(int) attr,addr,(addr / cols),(addr % cols),rect.x,rect.y,rect.width,rect.height);

	cr = cairo_create(terminal->surface);
	cairo_set_scaled_font(cr,terminal->font.scaled);
	v3270_draw_element(cr, chr, attr, terminal->host, terminal->font.height, &rect,terminal->color);
    cairo_destroy(cr);
	if(cursor)
		v3270_update_cursor_rect(terminal,&rect,chr,attr);

// #ifndef _WIN32
	gtk_widget_queue_draw_area(GTK_WIDGET(terminal),rect.x,rect.y,rect.width,rect.height);
// #endif // WIN32

}

void v3270_update_cursor_surface(v3270 *widget,unsigned char chr,unsigned short attr)
{
	if(widget->cursor.surface)
	{
		GdkRectangle	  rect	= widget->cursor.rect;
		cairo_t			* cr 	= cairo_create(widget->cursor.surface);
		GdkRGBA		* fg;
		GdkRGBA 		* bg;

		get_element_colors(attr,&fg,&bg,widget->color);

		cairo_set_scaled_font(cr,widget->font.scaled);

		rect.x = 0;
		rect.y = 0;
		v3270_draw_char(cr,chr,attr,widget->host,widget->font.height,&rect,bg,fg);

		cairo_destroy(cr);
	}


}

void v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, unsigned char chr, unsigned short attr)
{
	widget->cursor.chr  = chr;
	widget->cursor.rect = *rect;
	widget->cursor.attr = attr;
	widget->cursor.rect.height = widget->font.height + widget->font.descent;
	v3270_update_cursor_surface(widget,chr,attr);
}

