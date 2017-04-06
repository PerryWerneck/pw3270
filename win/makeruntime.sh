#!/bin/bash
myDIR=$(dirname $(readlink -f $0))

prefix="/usr/i686-w64-mingw32/sys-root/mingw"
PKG_CONFIG="/usr/bin/i686-w64-mingw32-pkg-config"
GTK_VERSION="gtk+-3.0"

GTK_PREFIX=$($PKG_CONFIG --variable=prefix ${GTK_VERSION})

TARGET="$(dirname ${myDIR})/.bin/runtime"

# Change to bin path
mkdir -p ${TARGET}
rm -fr ${TARGET}/*

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


