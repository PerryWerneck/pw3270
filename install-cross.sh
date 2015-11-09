#!/bin/bash

PREFIX="mingw32"

install_packages()
{

TEMPFILE=$(mktemp)

cat > ${TEMPFILE} << EOF
atk-devel
pango-devel
win_iconv-devel
pixman-devel
glib2-devel
cairo-devel
freetype-devel
winpthreads-devel
gtk3-devel
libopenssl-devel
libintl-devel
cross-gcc-c++
cross-pkg-config
cross-cpp
cross-binutils
cross-nsis
filesystem
gettext-tools
gtk3-data
gtk3-tools
headers
gnome-icon-theme
hicolor-icon-theme
gdk-pixbuf-loader-rsvg
libgdk_pixbuf-2_0-0
gdk-pixbuf-query-loaders
python-devel
EOF

# Instala o tema usado no pacote windows
sudo zypper --non-interactive in adwaita-icon-theme

while read FILE
do
	sudo zypper --non-interactive in ${1}-${FILE}
done < ${TEMPFILE}

rm -f ${TEMPFILE}

}

if [ -z ${1} ]; then
	echo "Use ${0} --32 for 32 bits cross-compiler"
	echo "Use ${0} --64 for 64 bits cross-compiler"
	exit -1
fi


until [ -z "${1}" ]
do
	if [ ${1:0:2} = '--' ]; then
		tmp=${1:2}
		parameter=${tmp%%=*}
		parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")

		case $parameter in

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


