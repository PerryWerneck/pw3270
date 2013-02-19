#!/bin/bash

case $1 in

	revision)
		dbus-send --session  --print-reply --dest=br.com.bb.pw3270 /br/com/bb/pw3270 br.com.bb.pw3270.getRevision
		;;

	connect)
		dbus-send --session  --print-reply --dest=br.com.bb.pw3270 /br/com/bb/pw3270 br.com.bb.pw3270.connect string:$2
		;;

	disconnect)
		dbus-send --session  --print-reply --dest=br.com.bb.pw3270 /br/com/bb/pw3270 br.com.bb.pw3270.disconnect
		;;

	quit)
		dbus-send --session  --print-reply --dest=br.com.bb.pw3270 /br/com/bb/pw3270 br.com.bb.pw3270.quit
		;;

	*)
		dbus-send --session  --print-reply --dest=br.com.bb.pw3270 /br/com/bb/pw3270 br.com.bb.pw3270.getRevision
		;;

esac

