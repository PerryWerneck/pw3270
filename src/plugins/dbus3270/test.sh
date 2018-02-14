#!/bin/bash

SESSION=a
DEST=br.com.bb.pw3270
BPATH=/br/com/bb/pw3270

run_command()
{

	case $1 in

		revision)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getRevision
			;;

		message)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getMessageID
			;;

		ssl)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getSecureState
			;;

		connect)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.connect string:$2 int32:10
			;;

		disconnect)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.disconnect
			;;

		url)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getURL
			;;

		quit)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.quit
			;;

		get)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getScreenContents
			;;

		gettext)
#			addr,len,lf
#			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getText int32:1 int32:-1 byte:0
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getText int32:1 int32:-1 byte:10
			;;

		protect)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getIsProtectedAt int32:$2 int32:$3 
			;;

		set)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.setTextAt int32:$2 int32:$3 string:$4
			;;

		action)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.action string:$2
			;;

		enter)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.enter
			;;

		isconnected)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.isConnected
			;;

		hostcharset)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getHostCharset
			;;

		displaycharset)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getDisplayCharset
			;;

		unlockdelay)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.setUnlockDelay int32:$2
			;;

		*)
			echo "Comando $1 desconhecido"
			;;

	esac
}



until [ -z "$1" ]
do
        if [ ${1:0:2} = '--' ]; then
                tmp=${1:2}
                parameter=${tmp%%=*}
                parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")
                value=${tmp##*=}

		case "$parameter" in
		SESSION)
			SESSION=$value
			;;
		HELP)
			echo "$0 options"
			echo ""
			echo "Options:"
			echo ""
			echo "	--session	pw3270's session manager"
			echo ""
			exit 0
			;;

		*)
			eval $parameter=$value
			;;

		esac

	else
		run_command $@
		exit 0
	fi

        shift
done

