#!/bin/bash

PROJECT_NAME="pw3270"
LIBRARY_NAME="lib3270"
CORE_LIBRARIES="lib3270 libv3270"
PACKAGE_PLUGINS="ipc"
PACKAGE_LANGUAGE_BINDINGS="hllapi"
TARGET_ARCHS="x86_32"
GIT_URL="https://github.com/PerryWerneck"

PROJECTDIR=$(dirname $(dirname $(readlink -f ${0})))
WORKDIR=$(mktemp -d)
PUBLISH=0
GET_PREREQS=0

if [ -e /etc/os-release ]; then
	. /etc/os-release
fi

if [ -e ~/.config/pw3270.build.conf ]; then
	. ~/.config/pw3270.build.conf
fi

#
# Limpa diretório temporário
#
cleanup()
{
	rm -fr ${WORKDIR}
}

failed()
{
	echo "$@"
	cleanup
	exit -1
}

#
# Get pre requisites from spec
#
getBuildRequires()
{
	for required in $(grep -i buildrequires "${1}" | grep -v "%" | cut -d: -f2-)
	do
		echo "Installing ${required}"
		sudo zypper --non-interactive --quiet in "${required}"
	done

}

#
# Get Sources from GIT
#
getSource()
{
	echo -e "\e]2;Getting sources for ${1}\a"
	echo "Getting sources for ${1}"

	mkdir -p ${WORKDIR}/sources

	git clone ${GIT_URL}/${1}.git ${WORKDIR}/sources/${1}
	if [ "$?" != "0" ]; then
		faile "Can't get sources for ${1}"
	fi

	if [ "${GET_PREREQS}" != "0" ]; then
		for ARCH in ${ARCHS}
		do

			if [ -d ${WORKDIR}/sources/${1}/win/${ARCH} ]; then

				for spec in $(find ${WORKDIR}/sources/${1}/win/${ARCH} -name "*.spec")
				do
					getBuildRequires "${spec}"
				done

			fi


		done
	fi

	cd ${WORKDIR}/sources/${1}

	NOCONFIGURE=1 ./autogen.sh
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi


}

#
# Build library
#
buildLibrary()
{
	for ARCH in ${ARCHS}
	do

		echo -e "\e]2;Building ${1} for ${ARCH}\a"
		echo "Building ${1} for ${ARCH}"

		case ${ARCH} in
		x86_32)
			host=i686-w64-mingw32
			host_cpu=i686
			prefix=/usr/i686-w64-mingw32/sys-root/mingw
			tools=i686-w64-mingw32
			pkg_config=/usr/bin/i686-w64-mingw32-pkg-config
			;;

		x86_64)
			host=x86_64-w64-mingw32
			host_cpu=x86_64
			prefix=/usr/x86_64-w64-mingw32/sys-root/mingw
			tools=x86_64-w64-mingw32
			pkg_config=/usr/bin/x86_64-w64-mingw32-pkg-config
			;;

		*)
			failed "Arquitetura desconhecida: ${1}"

		esac

		export HOST_CC=/usr/bin/gcc

		mkdir -p ${WORKDIR}/build/${ARCH}
		mkdir -p ${WORKDIR}/cache/${ARCH}
		mkdir -p ${WORKDIR}/build/${ARCH}/bin
		mkdir -p ${WORKDIR}/build/${ARCH}/lib
		mkdir -p ${WORKDIR}/build/${ARCH}/locale
		mkdir -p ${WORKDIR}/build/${ARCH}/include
		mkdir -p ${WORKDIR}/build/${ARCH}/sysconfig
		mkdir -p ${WORKDIR}/build/${ARCH}/data

		export PKG_CONFIG_PATH=${WORKDIR}/build/${ARCH}/lib/pkgconfig
		export cache=${WORKDIR}/cache/${ARCH}/${1}.cache

		cd ${WORKDIR}/sources/${1}

		./configure \
			CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			LDFLAGS="-L${WORKDIR}/build/${ARCH}/lib" \
			--host=${host} \
			--prefix=${prefix} \
			--bindir=${WORKDIR}/build/${ARCH}/bin \
			--libdir=${WORKDIR}/build/${ARCH}/lib \
			--localedir=${WORKDIR}/build/${ARCH}/locale \
			--includedir=${WORKDIR}/build/${ARCH}/include \
			--sysconfdir=${WORKDIR}/build/${ARCH}/sysconfig \
			--datadir=${WORKDIR}/build/${ARCH}/data \
			--datarootdir=${WORKDIR}/build/${ARCH}/data

		if [ "$?" != "0" ]; then
			failed "Can't configure ${1}"
		fi

		make all
		if [ "$?" != "0" ]; then
			failed "Can't buid ${1}"
		fi

		make install
		if [ "$?" != "0" ]; then
			failed "Can't install ${1}"
		fi

	done

}

#
# Build main application
#
buildApplication()
{
	for ARCH in ${ARCHS}
	do

		echo -e "\e]2;Building ${1} for ${ARCH}\a"
		echo "Building ${1} for ${ARCH}"

		case ${ARCH} in
		x86_32)
			host=i686-w64-mingw32
			host_cpu=i686
			prefix=/usr/i686-w64-mingw32/sys-root/mingw
			tools=i686-w64-mingw32
			pkg_config=/usr/bin/i686-w64-mingw32-pkg-config
			;;

		x86_64)
			host=x86_64-w64-mingw32
			host_cpu=x86_64
			prefix=/usr/x86_64-w64-mingw32/sys-root/mingw
			tools=x86_64-w64-mingw32
			pkg_config=/usr/bin/x86_64-w64-mingw32-pkg-config
			;;

		*)
			failed "Arquitetura desconhecida: ${1}"

		esac

		export HOST_CC=/usr/bin/gcc

		mkdir -p ${WORKDIR}/build/${ARCH}
		mkdir -p ${WORKDIR}/cache/${ARCH}
		mkdir -p ${WORKDIR}/build/${ARCH}/bin
		mkdir -p ${WORKDIR}/build/${ARCH}/lib
		mkdir -p ${WORKDIR}/build/${ARCH}/locale
		mkdir -p ${WORKDIR}/build/${ARCH}/include
		mkdir -p ${WORKDIR}/build/${ARCH}/sysconfig
		mkdir -p ${WORKDIR}/build/${ARCH}/data

		export PKG_CONFIG_PATH=${WORKDIR}/build/${ARCH}/lib/pkgconfig
		export cache=${WORKDIR}/cache/${ARCH}/${1}.cache

		cd ${WORKDIR}/sources/${1}

		./configure \
			CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			LDFLAGS="-L${WORKDIR}/build/${ARCH}/lib" \
			--host=${host} \
			--prefix=${prefix} \
			--bindir=${WORKDIR}/build/${ARCH}/bin \
			--libdir=${WORKDIR}/build/${ARCH}/lib \
			--localedir=${WORKDIR}/build/${ARCH}/locale \
			--includedir=${WORKDIR}/build/${ARCH}/include \
			--sysconfdir=${WORKDIR}/build/${ARCH}/sysconfig \
			--datadir=${WORKDIR}/build/${ARCH}/data \
			--datarootdir=${WORKDIR}/build/${ARCH}/data

		if [ "$?" != "0" ]; then
			failed "Can't configure ${1}"
		fi

		make all
		if [ "$?" != "0" ]; then
			failed "Can't buid ${1}"
		fi

		make install
		if [ "$?" != "0" ]; then
			failed "Can't install ${1}"
		fi

	done


}

#
# Check command line parameters
#
until [ -z "$1" ]
do
        if [ ${1:0:2} = '--' ]; then
                tmp=${1:2}
                parameter=${tmp%%=*}
                parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")
                value=${tmp##*=}

                case "$parameter" in
		NOPUBLISH)
			PUBLISH=0
			;;

		PUBLISH)
			PUBLISH=1
			;;

		CLEAR)
			if [ -d ~/public_html/win/${PROJECT_NAME} ]; then
				rm -fr ~/public_html/win/${PROJECT_NAME}/{x86_32,x86_64}
			fi

			;;
		HELP)
			echo "${0} [options]"
			echo ""
			echo "Options:"
			echo ""

			if [ ! -z ${WIN_PACKAGE_SERVER} ]; then
				echo "  --nopublish	Don't publish binaries in ${WIN_PACKAGE_SERVER}"
				echo "  --publish	Publish binaries in ${WIN_PACKAGE_SERVER}"
			fi


			if [ -d ~/public_html/win/${PROJECT_NAME} ]; then
				echo "  --clear	Remove ~/public_html/win/${PROJECT_NAME}/{x86_32,x86_64}"
			fi

			echo ""
			exit 0
			
			;;
		
		esac
	fi

	shift

done

#
# Download sources
#
for src in ${CORE_LIBRARIES}
do
	getSource ${src}
done

getSource pw3270

for src in ${PACKAGE_PLUGINS}
do
	getSource pw3270-plugin-${src}
done

for src in ${PACKAGE_LANGUAGE_BINDINGS}
do
	getSource lib3270-${src}-bindings
done

#
# Build packages
#
for src in ${CORE_LIBRARIES}
do
	buildLibrary ${src}
done

buildApplication pw3270

for src in ${PACKAGE_PLUGINS}
do
	buildLibrary pw3270-plugin-${src}
done

for src in ${PACKAGE_LANGUAGE_BINDINGS}
do
	buildLibrary lib3270-${src}-bindings
done

cd ${WORKDIR}/build
/bin/bash

cleanup




