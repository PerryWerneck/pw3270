#ifndef _PW3270_DBUS_SERVICE_H

	#define _PW3270_DBUS_SERVICE_H

	#include <glib.h>
	#include <dbus/dbus-glib.h>
	#include <dbus/dbus-glib-bindings.h>
	#include <dbus/dbus-glib-lowlevel.h>

	#define PW3270_DBUS_SERVICE_PATH	"/br/com/bb/pw3270"
	#define PW3270_DBUS_SERVICE			"br.com.bb.pw3270"

	#define PW3270_TYPE_DBUS			(pw3270_dbus_get_type ())
	#define PW3270_DBUS(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object), PW3270_TYPE_DBUS, PW3270Dbus))
	#define PW3270_DBUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_DBUS, PW3270DbusClass))
	#define IS_PW3270_DBUS(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), PW3270_TYPE_DBUS))
	#define IS_PW3270_DBUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_DBUS))
	#define PW3270_DBUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_DBUS, PW3270DbusClass))

	G_BEGIN_DECLS

	typedef struct _PW3270Dbus		PW3270Dbus;
	typedef struct _PW3270DbusClass	PW3270DbusClass;

	struct _PW3270Dbus
	{
			GObject parent;
	};

	struct _PW3270DbusClass
	{
			GObjectClass parent;
	};

	PW3270Dbus	* pw3270_dbus_new (void);
	GType 		  pw3270_dbus_get_type (void);

	void		  pw3270_dbus_get_revision(PW3270Dbus *object, DBusGMethodInvocation *context);

	G_END_DECLS

#endif // _PW3270_DBUS_SERVICE_H
