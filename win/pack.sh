#!/bin/bash

PROJECTDIR=$(dirname $(dirname $(readlink -f ${0})))
WORKDIR=$(mktemp -d)
PUBLISH=0

if [ -e /etc/os-release ]; then
	. /etc/os-release
fi

#
# Limpa diretório temporário
#
cleanup()
{
	rm -fr ${WORKDIR}
}

#
# Monta projeto no diretório corrente.
#
build()
{
	make clean

	make all
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv .bin/Release/* ${WORKDIR}/build/bin
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	make DESTDIR=${WORKDIR}/build install
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

}

build_plugin()
{

	echo -e "\e]2;${2}-${1}\a"

	cd ${WORKDIR}/sources/pw3270-plugin-${2}
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	export cache=${WORKDIR}/cache/pw3270-plugin-${2}.cache

	./configure \
		CFLAGS=${CFLAGS} \
		LDFLAGS=${LDFLAGS} \
		LIB3270_CFLAGS="${LIB3270_CFLAGS}" \
		LIB3270_LIBS="${LIB3270_LIBS}" \
		LIBV3270_CFLAGS="${LIBV3270_CFLAGS}" \
		LIBV3270_LIBS="${LIBV3270_LIBS}" \
		--host=${host} \
		--prefix=${prefix} \
		--libdir=${prefix}/lib

	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	make all
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv .bin/Release/* ${WORKDIR}/build/bin
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

}

#
# Monta binários
#
# $1 = Arquitetura (x86_32/x86_64)
#
pack()
{

	echo -e "\e]2;pw3270-${1}\a"

	case ${1} in
	x86_32)
		host=i686-w64-mingw32
		host_cpu=i686
		prefix=/usr/i686-w64-mingw32/sys-root/mingw
		tools=i686-w64-mingw32
		pkg_config=/usr/bin/i686-w64-mingw32-pkg-config
		mingw_name=mingw32
		;;

	x86_64)
		host=x86_64-w64-mingw32
		host_cpu=x86_64
		prefix=/usr/x86_64-w64-mingw32/sys-root/mingw
		tools=x86_64-w64-mingw32
		pkg_config=/usr/bin/x86_64-w64-mingw32-pkg-config
		mingw_name=mingw64
		;;

	*)
		failed "Arquitetura desconhecida: ${1}"

	esac

#	sudo zypper \
#		--non-interactive \
#		in \
#		${mingw_name}-libcurl-devel \
#		${mingw_name}-curl \
#		${mingw_name}-libopenssl-devel \
#		${mingw_name}-libintl-devel \
#		${mingw_name}-atk-devel \
#		${mingw_name}-pango-devel \
#		${mingw_name}-win_iconv-devel \
#		${mingw_name}-pixman-devel \
#		${mingw_name}-glib2-devel \
#		${mingw_name}-cairo-devel \
#		${mingw_name}-freetype-devel \
#		${mingw_name}-winpthreads-devel \
#		${mingw_name}-gtk3-devel \
#		${mingw_name}-cross-gcc-c++ \
#		${mingw_name}-cross-pkg-config \
#		${mingw_name}-cross-cpp \
#		${mingw_name}-cross-binutils \
#		${mingw_name}-cross-nsis

	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	export HOST_CC=/usr/bin/gcc

	rm -fr ${WORKDIR}/cache
	mkdir -p ${WORKDIR}/cache

	rm -fr ${WORKDIR}/build
	mkdir -p ${WORKDIR}/build/src/include
	mkdir -p ${WORKDIR}/build/.bin/Release

	#
	# Setup Target dir
	#
	mkdir -p ${WORKDIR}/build/bin

	export CFLAGS=-I${WORKDIR}/build/${prefix}/include -DWIN32 -D_WIN32
	export LDFLAGS=-L${WORKDIR}/build/bin
	export PKG_CONFIG_PATH=${WORKDIR}/build/${prefix}/lib/pkgconfig

	#
	# Build lib3270
	#
	echo -e "\e]2;lib3270-${1}\a"

	cd ${WORKDIR}/sources/lib3270
	export cache=${WORKDIR}/cache/lib3270.cache

	./configure \
		--host=${host} \
		--prefix=${prefix} \
		--libdir=${prefix}/lib

	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	build

	export LIB3270_CFLAGS="-DLIB3270_NAME=3270"
	export LIB3270_LIBS="-l3270"

	#
	# Build libv3270
	#
	echo -e "\e]2;libv3270-${1}\a"

	cd ${WORKDIR}/sources/libv3270
	export cache=${WORKDIR}/cache/libv3270.cache

	./configure \
		CFLAGS=${CFLAGS} \
		LDFLAGS=${LDFLAGS} \
		LIB3270_CFLAGS="${LIB3270_CFLAGS}" \
		LIB3270_LIBS="${LIB3270_LIBS}" \
		--host=${host} \
		--prefix=${prefix} \
		--libdir=${prefix}/lib

	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	build

	export LIBV3270_CFLAGS="-DLIBV3270_MODE=3270"
	export LIBV3270_LIBS="-lv3270"

	#
	# Build main application
	#
	echo -e "\e]2;pw3270-${1}\a"

	cd ${WORKDIR}/sources/pw3270
	export cache=${WORKDIR}/cache/application.cache

	./configure \
		CFLAGS=${CFLAGS} \
		LDFLAGS=${LDFLAGS} \
		LIB3270_CFLAGS="${LIB3270_CFLAGS}" \
		LIB3270_LIBS="${LIB3270_LIBS}" \
		LIBV3270_CFLAGS="${LIBV3270_CFLAGS}" \
		LIBV3270_LIBS="${LIBV3270_LIBS}" \
		--host=${host} \
		--prefix=${prefix} \
		--libdir=${prefix}/lib \
		--with-source-locales=${WORKDIR}/locale

	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	mkdir -p ${WORKDIR}/locale

	cp ${WORKDIR}/sources/lib3270/.pot/*.pot ${WORKDIR}/locale
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp ${WORKDIR}/sources/libv3270/.pot/*.pot ${WORKDIR}/locale
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	build

	#
	# Build plugins
	#
	build_plugin ${1} hllapi

	#
	# Install data & icons
	#
	echo -e "\e]2;pw3270-icons-${1}\a"

	cd ${WORKDIR}/sources/pw3270

	make -C ${WORKDIR}/sources/pw3270 locale
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv .bin/locale ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	mkdir -p ${WORKDIR}/build/win

	mkdir -p ${WORKDIR}/sources/pw3270/.bin/Release
	cp -rv ${WORKDIR}/build/bin/* ${WORKDIR}/sources/pw3270/.bin/Release

	chmod +x ${WORKDIR}/sources/pw3270/win/makeruntime.sh
	${WORKDIR}/sources/pw3270/win/makeruntime.sh
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	mkdir -p ${WORKDIR}/build/bin
	cp -rv ${WORKDIR}/sources/pw3270/.bin/runtime ${WORKDIR}/build/bin
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	#
	# Copy branding
	#
	cp ${WORKDIR}/branding/*.ico ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp ${WORKDIR}/branding/*.png ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp ${WORKDIR}/branding/AUTHORS ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp ${WORKDIR}/branding/LICENSE ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv ${WORKDIR}/branding/ui ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv ${WORKDIR}/branding/*.conf ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -rv ${WORKDIR}/sources/pw3270/charsets/*.xml ${WORKDIR}/build
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	#
	# Create installation package
	#
	echo -e "\e]2;pw3270-package-${1}\a"

	cd ${WORKDIR}/build

	cp ${WORKDIR}/sources/pw3270/win/pw3270.nsi ./pw3270.nsi
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	makensis -DWITHGTK pw3270.nsi
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cp -v *.exe ${PROJECTDIR}
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	if [ -d ~/public_html/win/pw3270 ]; then
		mkdir -p ~/public_html/win/pw3270/${1}
		cp -v *.exe ~/public_html/win/pw3270/${1}
		if [ "$?" != "0" ]; then
			cleanup
			exit -1
		fi
	fi

	if [ "${PUBLISH}" == "1" ] && [ ! -z ${WIN_PACKAGE_SERVER} ]; then
		scp *.exe ${WIN_PACKAGE_SERVER}/pw3270
		if [ "$?" != "0" ]; then
			cleanup
			exit -1
		fi
	fi

}

#
# Setup options
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
			if [ -d ~/public_html/win/pw3270 ]; then
				rm -fr ~/public_html/win/pw3270{x86_32,x86_64}
			fi
			;;

		HELP)
			echo "${0} [OPTIONS]"
			echo ""
			echo "Options:"
			echo ""

			if [ ! -z ${WIN_PACKAGE_SERVER} ]; then
				echo "  --nopublish	Don't send packages to ${WIN_PACKAGE_SERVER}/pw3270"
				echo "  --publish	Send packages to ${WIN_PACKAGE_SERVER}/pw3270"
			fi


			if [ -d ~/public_html/win/sisbb ]; then
				echo "  --clear	Remove directories ~/public_html/win/pw3270{x86_32,x86_64}"
			fi

			echo ""
			exit 0
			
			;;
		
		esac
	fi

	shift

done



#
# Get sources from GIT
#
mkdir -p ${WORKDIR}/sources

for src in lib3270 libv3270 pw3270 pw3270-plugin-hllapi; do

	echo "Baixando ${src}..."
	echo -e "\e]2;Downloading ${src}\a"

	git clone https://github.com/PerryWerneck/${src}.git ${WORKDIR}/sources/${src}
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi

	cd ${WORKDIR}/sources/${src}

	NOCONFIGURE=1 ./autogen.sh
	if [ "$?" != "0" ]; then
		cleanup
		exit -1
	fi


done

#
# Setup branding
#
echo -e "\e]2;Branding\a"

mkdir -p ${WORKDIR}/branding

BRANDING_SOURCES=${WORKDIR}/sources/pw3270/branding

cp -rv ${BRANDING_SOURCES}/* ${WORKDIR}/branding
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

convert -density 384 -background transparent ${BRANDING_SOURCES}/pw3270.svg -define icon:auto-resize -colors 256 ${WORKDIR}/branding/pw3270.ico
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

convert -background transparent ${BRANDING_SOURCES}/pw3270.svg ${WORKDIR}/branding/pw3270.png
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

optipng -o7 ${WORKDIR}/branding/pw3270.png
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

convert -background transparent ${BRANDING_SOURCES}/pw3270-logo.svg ${WORKDIR}/branding/pw3270-logo.png
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

optipng -o7 ${WORKDIR}/branding/pw3270-logo.png
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

cp ${WORKDIR}/sources/pw3270/AUTHORS ${WORKDIR}/branding
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

cp ${WORKDIR}/sources/pw3270/LICENSE ${WORKDIR}/branding
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

cp ${WORKDIR}/sources/pw3270/conf/colors.conf ${WORKDIR}/branding
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

cp -rv ${WORKDIR}/sources/pw3270/ui ${WORKDIR}/branding
if [ "$?" != "0" ]; then
	cleanup
	exit -1
fi

#
# Create installers 
#
pack x86_32
pack x86_64

cleanup
















