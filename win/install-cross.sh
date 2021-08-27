#!/bin/bash
myDIR=$(dirname $(readlink -f ${0}))

install_packages() {

	TEMPFILE=$(mktemp)
	
	for spec in $(find ${myDIR} -name "${1}*.spec")
	do
		echo "Parsing ${spec}"
		grep -i "^Requires:" "${spec}" | grep -v "%" | cut -d: -f2- | tr -d '[:blank:]' | cut -d'>' -f1 >> ${TEMPFILE}
		grep -i "^BuildRequires:" "${spec}" | grep -v "%" | cut -d: -f2- | tr -d '[:blank:]'  | cut -d'>' -f1 >> ${TEMPFILE}
	done
	
	cat ${TEMPFILE} \
		| sort --unique \
		| xargs sudo zypper --non-interactive --verbose in

	rm -f ${TEMPFILE}

}

if [ -z ${1} ]; then
	echo "${0} [options]"
	echo ""
	echo "Options:"
	echo ""

	echo "  --ar	Install required OBS repositories for zypper"
	echo "  --32	Install cross compiler for 32 bits windows using zypper"
	echo "  --64	Install cross compiler for 64 bits windows using zypper"
	echo "  --all	Install cross compiler for 32 and 64 bits windows using zypper"
	exit -1
fi


until [ -z "${1}" ]
do
	if [ ${1:0:2} = '--' ]; then
		tmp=${1:2}
		parameter=${tmp%%=*}
		parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")

		case $parameter in

		AR)
			echo "Adding required repositories"
			sudo zypper ar obs://windows:mingw:win32 windows_mingw_win32
			sudo zypper ar obs://windows:mingw:win64 windows_mingw_win64
			sudo zypper ar obs://home:PerryWerneck:pw3270 home_PerryWerneck_pw3270
			;;

		32)
			install_packages mingw32
			;;

		64)
			install_packages mingw64
			;;

		ALL)
			install_packages mingw32
			install_packages mingw64
			;;


		*)
			value=${tmp##*=}
			eval $parameter=$value
		esac

	fi

	shift
done


