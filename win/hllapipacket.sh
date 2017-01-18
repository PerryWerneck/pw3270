#!/bin/bash


#
# Gera binários windows
#
# $1 = Arquitetura (x86_32/x86_64)
#
build()
{
	echo -e "\e]2;${PACKAGE_NAME}-${1}\a"

	case ${1} in
	x86_32)
		host=i686-w64-mingw32
		host_cpu=i686
		prefix=/usr/i686-w64-mingw32/sys-root/mingw
		tools=i686-w64-mingw32
		;;

	x86_64)
		host=x86_64-w64-mingw32
		host_cpu=x86_64
		prefix=/usr/x86_64-w64-mingw32/sys-root/mingw
		tools=x86_64-w64-mingw32
		;;


	*)
		failed "Arquitetura desconhecida: ${1}"

	esac

	# Detecto argumentos
	ARGS=""

	./configure \
		--cache-file=.${1}.cache \
		--host=${host} \
		--prefix=${prefix} \
		--disable-rexx
		--disable-java \
		--disable-office
 
	if [ "$?" != "0" ]; then
		failed "Erro ao configurar"
	fi

	make clean
	rm -f *.exe

	make Release
	if [ "$?" != "0" ]; then
		failed "Erro ao compilar fontes"
	fi

	mkdir -p ${TEMPDIR}/package/${host_cpu}

	cp -v .bin/Release/hllapi.dll* ${TEMPDIR}/package/${host_cpu}
	if [ "$?" != "0" ]; then
		failed "Erro ao copiar pacotes"
	fi

	make clean
	rm -f *.exe

}

myDIR=$(readlink -f $(dirname $0))
TEMPDIR=$(mktemp -d)
DESTDIR=${HOME}/public_html/win
RUNTIMEDIR=$(mktemp -d)
ARCHS="x86_32 x86_64"
RUNTIME=1
COMPLETE=1

trap cleanup INT 

until [ -z "$1" ]
do
	if [ ${1:0:2} = '--' ]; then
		tmp=${1:2}
		parameter=${tmp%%=*}
		parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")

		case $parameter in

		32)
			ARCHS="x86_32"
			;;

		64)
			ARCHS="x86_64"
			;;

		OUT)
			DESTDIR=$value
			;;

		ARCH)
			value=${tmp##*=}
			ARCHS=$value
			;;

		*)
			value=${tmp##*=}
			eval $parameter=$value
		esac

	fi

	shift
done

# Configura
aclocal
if [ "$?" != "0" ]; then
	exit -1
fi

autoconf
if [ "$?" != "0" ]; then
	exit -1
fi

# Gera pacotes
for i in ${ARCHS}; do

	build "${i}"

done

cd ${TEMPDIR}/package

zip -9 -m -r ~/public_html/win/hllapi_$(date "+%Y%m%d").zip .

