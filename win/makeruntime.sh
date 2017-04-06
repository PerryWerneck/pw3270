#!/bin/bash
#
# "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
# (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
# aplicativos mainframe. Registro no INPI sob o nome G3270.
#
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
# os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
# Free Software Foundation.
#
# Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
# GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
# A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
# obter mais detalhes.
#
# Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
# programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA, 02111-1307, USA
#
# Contatos:
#
# perry.werneck@gmail.com       (Alexandre Perry de Souza Werneck)
# erico.mendonca@gmail.com      (Erico Mascarenhas de Mendonça)
#


myDIR=$(dirname $(readlink -f $0))

prefix="/usr/i686-w64-mingw32/sys-root/mingw"
PKG_CONFIG="/usr/bin/i686-w64-mingw32-pkg-config"
GTK_VERSION="gtk+-3.0"

GTK_PREFIX=$($PKG_CONFIG --variable=prefix ${GTK_VERSION})
GDK_LOADERS=$(${PKG_CONFIG} --variable=gdk_pixbuf_binarydir gdk-pixbuf-2.0 | sed -e "s@${prefix}@@g")

TARGET="$(dirname ${myDIR})/.bin/runtime"

# Change to bin path
mkdir -p ${TARGET}
rm -fr ${TARGET}/*

copy_dll() {

	AGAIN=1
	until [  $AGAIN = 0 ]; do

		SOURCES=$(mktemp)
		REQUIRES=$(mktemp)

		find "$(dirname ${myDIR})/.bin/Release/" -iname "*.dll" >	${SOURCES}
		find "$(dirname ${myDIR})/.bin/Release/" -iname "*.exe" >>	${SOURCES}
		find "${TARGET}" -iname *.dll >> ${SOURCES}

		while read FILENAME
		do
			objdump -p ${FILENAME} | grep "DLL Name:" | cut -d: -f2 | tr "[:upper:]" "[:lower:]" >> ${REQUIRES}
		done < ${SOURCES}

		libs_to_exclude="
			advapi32
			cfgmgr32
			comctl32
			comdlg32
			crypt32
			d3d8
			d3d9
			ddraw
			dnsapi
			dsound
			dwmapi
			gdi32
			gdiplus
			glu32
			glut32
			imm32
			iphlpapi
			kernel32
			ksuser
			mpr
			mscms
			mscoree
			msimg32
			msvcr71
			msvcr80
			msvcr90
			msvcrt
			mswsock
			netapi32
			odbc32
			ole32
			oleacc
			oleaut32
			opengl32
			psapi
			rpcrt4
			secur32
			setupapi
			shell32
			shlwapi
			user32
			usp10
			version
			wininet
			winmm
			wldap32
			ws2_32
			wsock32
			winspool.drv
		"

		# Excluo DLLs do sistema
		for i in $libs_to_exclude; do
			sed -i -e "/${i}/d" ${REQUIRES}
		done

		# Procuro pelas DLLs que faltam
		AGAIN=0
		while read FILENAME
		do
			if [ ! -e "${TARGET}/${FILENAME}" ]; then

				COUNT=$(find "$(dirname ${myDIR})/.bin/Release/" -iname ${FILENAME} | wc --lines)
				if [ "${COUNT}" == "0" ]; then

					if [ -e ${prefix}/bin/${FILENAME} ]; then

						echo "Copiando $(basename ${FILENAME})..."

						AGAIN=1
						cp -v "${prefix}/bin/${FILENAME}" "${TARGET}/${FILENAME}"
						if [ "$?" != "0" ]; then
							exit -1
						fi
					else 

						echo "Can't find ${FILENAME}"

					fi

				fi


			fi

		done < ${REQUIRES}

		rm -f ${SOURCES}
		rm -f ${REQUIRES}

	done

}

copy_locale() {

	rm -fr ${TARGET}/share/locale/pt_BR/LC_MESSAGES
	mkdir -p ${TARGET}/share/locale/pt_BR/LC_MESSAGES

	locales="
		gettext-runtime.mo
		gettext-tools.mo
		glib20.mo
		gtk30.mo
		gtk30-properties.mo
	"

	for i in $locales; do
		if [ -e "${GTK_PREFIX}/share/locale/pt_BR/LC_MESSAGES/${i}" ]; then
			echo "${GTK_PREFIX}/share/locale/pt_BR/LC_MESSAGES/${i} ..."
			cp "${GTK_PREFIX}/share/locale/pt_BR/LC_MESSAGES/${i}" "${TARGET}/share/locale/pt_BR/LC_MESSAGES"
			if [ "$?" != "0" ]; then
				exit -1
			fi
		fi
	done

}

copy_loaders() {

	mkdir -p "${TARGET}/${GDK_LOADERS}"
	cp -rv "${prefix}/${GDK_LOADERS}/loaders" "${TARGET}/${GDK_LOADERS}"
	if [ "$?" != "0" ]; then
		exit -1
	fi

}

copy_theme() {

	mkdir -p "${TARGET}/etc"
	cp -rv "${prefix}/etc/gtk-3.0" "${TARGET}/etc"

	rm -f $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "[Settings]" >> $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "gtk-theme-name = MS-Windows" >> $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "gtk-icon-theme-name = ${1}" >> $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "gtk-fallback-icon-theme = ${1}" >> $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "gtk-font-name = Sans 10" >> $TARGET_PATH/etc/gtk-3.0/settings.ini
	echo "gtk-button-images = 1" >> $TARGET_PATH/etc/gtk-3.0/settings.ini

	mkdir -p ${TARGET}/share/icons
	if [ "$?" != 0 ]; then
		echo "Can´t create icons folder"
		exit -1
	fi

	cp -rv /usr/share/icons/${1} ${TARGET}/share/icons
	if [ "$?" != 0 ]; then
		echo "Can´t copy ${1} icons"
		exit -1
	fi

	mkdir -p ${TARGET}/share/themes
	if [ "$?" != 0 ]; then
		echo "Can´t create themes folder"
		exit -1
	fi

	cp -rv /usr/share/themes/${1} ${TARGET}/share/themes
	if [ "$?" != 0 ]; then
		echo "Can´t copy ${1} theme"
		exit -1
	fi

}

copy_dll
copy_locale
copy_loaders
copy_theme "Adwaita"

# Otimiza todos os pngs
#echo "Optimizing..."
#find ${TARGET} -iname *.png -exec optipng -o7 -quiet {} \; 2>&1 > /dev/null



echo "Runtime ok"
