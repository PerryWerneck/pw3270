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

		connect)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.connect string:$2
			;;

		disconnect)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.disconnect
			;;

		quit)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.quit
			;;

		get)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.getScreenContents
			;;

		set)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.setTextAt int32:$2 int32:$3 string:$4
			;;

		enter)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.enter
			;;

		isconnected)
			dbus-send --session  --print-reply --dest=$DEST.$SESSION $BPATH $DEST.isConnected
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

