#!/bin/bash
myDIR=$(dirname $(readlink -f $0))

cleanup() 
{
    #
    # Apaga diretorio temporário caso o script seja interrompido
    #
	cd ${myDIR}
	rm -fr ${TEMPDIR}
	exit -1
}

failed()
{
	echo -e "\e]2;Failed!\a"
	echo $1
	cleanup	
}


#
# Gera pacote windows
#
# $1 = Arquitetura (x86_32/x86_64)
#
build()
{
	cd $(dirname $myDIR)
	echo -e "\e]2;${1}\a"

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

	export HOST_CC=/usr/bin/gcc
	export cache=${1}.cache

	./configure \
		--with-inet-ntop \
		--host=${host} \
		--prefix=${prefix} \
		--libdir=${prefix}/lib

	if [ "$?" != "0" ]; then
		failed "Erro ao configurar"
	fi

	. ./versions
	echo -e "\e]2;${PACKAGE_NAME} - ${1}\a"
 
	make clean
	rm -f *.exe

	make all
	if [ "$?" != "0" ]; then
		failed "Erro ao compilar fontes"
	fi

	rm -f ./win/*.exe

	mkdir -p ${DESTDIR}/${PACKAGE_NAME}/${1}

	if [ "${RUNTIME}" == "1" ]; then

		makensis ./win/${PACKAGE}.nsi
		if [ "$?" != "0" ]; then
			failed "Erro ao gerar instalador sem gtk"
		fi
	
		mv -f	./win/${PACKAGE}-${PACKAGE_VERSION}-requires-gtk-${GTK_MODVERSION}-${host_cpu}.exe \
				${DESTDIR}/${PACKAGE_NAME}/${1}

		if [ "$?" != "0" ]; then
			failed "Erro ao copiar instalador sem gtk para ${1}"
		fi

	fi

	if [ "${COMPLETE}" == "1" ]; then

		chmod +x ./win/makegtkruntime.sh
		./win/makegtkruntime.sh

		makensis -DWITHGTK ./win/${PACKAGE}.nsi
		if [ "$?" != "0" ]; then
			failed "Erro ao gerar instalador com runtime"
		fi

		mv -f	./win/${PACKAGE}-${PACKAGE_VERSION}-gtk-${GTK_MODVERSION}-${host_cpu}.exe \
				${DESTDIR}/${PACKAGE_NAME}/${1}

		if [ "$?" != "0" ]; then
			failed "Erro ao copiar instalador completo para ${1}"
		fi

		ln -sf	${1}/${PACKAGE}-${PACKAGE_VERSION}-gtk-${GTK_MODVERSION}-${host_cpu}.exe \
				${DESTDIR}/${PACKAGE_NAME}/${PACKAGE}-latest-${host_cpu}.exe

		if [ "$?" != "0" ]; then
			failed "Erro ao criar link para ${1}"
		fi
	fi

	make clean
	rm -fr .bin

}

TEMPDIR=$(mktemp -d)
ARCHS="x86_32 x86_64"
DESTDIR=${HOME}/public_html/win
RUNTIME=0
COMPLETE=1

rm -f	${myDIR}/*.exe \
		${myDIR}/*.zip

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

		FULL)
			COMPLETE=1
			RUNTIME=1
			;;

		RT)
			COMPLETE=0
			RUNTIME=1
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

#if [ "${RUNTIME}" == "1" ]; then
#
#	echo -e "\e]2;Baixando runtime\a"
#
#	mkdir -p ${TEMPDIR}/runtime
#	cd ${TEMPDIR}/runtime
#
#	#
#	# Puxo scripts de construção do GTK direto da sourceforge.
#	#
#	git clone http://git.code.sf.net/p/gtk3win/code .
#	if [ "$?" != "0" ]; then
#		echo "Erro ao baixar fontes do runtime"
#		exit -1
#	fi
#
#	for i in ${ARCHS}; do
#
#		echo -e "\e]2;gtk-runtime-${i}\a"
#
#		case ${i} in
#		x86_32)
#			host_cpu=i686
#			./win32.sh
#			if [ "$?" != "0" ]; then
#				exit -1
#			fi
#			;;
#
#		x86_64)
#			host_cpu=x86_64
#			./win64.sh
#			if [ "$?" != "0" ]; then
#				exit -1
#			fi
#			;;
#
#		*)
#			echo "Arquitetura desconhecida ${i}"
#			exit -1
#
#		esac
#
#		chmod +x ./win/makeruntime.sh
#
#		./win/makeruntime.sh
#		if [ "$?" != "0" ]; then
#			exit -1
#		fi
#
#		# Copia o pacote gerado
#		FILENAME=$(find . -maxdepth 1 -name "gtk-runtime-*-${host_cpu}.exe" | head --lines 1)
#
#		mkdir -p ${DESTDIR}/${host_cpu}
#
#		mv gtk-runtime-*-${host_cpu}.exe ${DESTDIR}/${host_cpu}
#		if [ "$?" != "0" ]; then
#			failed "Erro ao copiar instalador"
#		fi
#
#		ln -sf $(basename ${FILENAME}) "${DESTDIR}/${host_cpu}/gtk-runtime-latest-${host_cpu}.exe"
#		if [ "$?" != "0" ]; then
#			failed "Erro ao criar o link simbólico"
#		fi
#
#	done
#
#fi

cd $(dirname $myDIR)
rm -fr ${TEMPDIR}

# Gera pacotes para envio ao SPB
rm -f ${DESTDIR}/${PACKAGE}-latest.zip

zip -9 -r -j \
		${DESTDIR}/${PACKAGE}-latest.zip \
		$(readlink -f ${DESTDIR}/${PACKAGE}/${PACKAGE}-latest-i686.exe) \
		$(readlink -f ${DESTDIR}/${PACKAGE}/${PACKAGE}-latest-x86_64.exe) 

echo -e "\e]2;Success!\a"


