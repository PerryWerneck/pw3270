#!/bin/bash
myDIR=$(dirname $(readlink -f $0))

cleanup() 
{
    #
    # Apaga diretorio temporário caso o script seja interrompido
    #
	cd ${myDIR}
	rm -fr ${TEMPDIR}
	rm -fr ${RUNTIMEDIR}
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

	makensis ./win/pw3270.nsi
	if [ "$?" != "0" ]; then
		failed "Erro ao gerar instalador sem runtime"
	fi

	if [ "${COMPLETE}" == "1" ]; then
		chmod +x ./win/makegtkruntime.sh
		./win/makegtkruntime.sh
		makensis -DWITHGTK ./win/pw3270.nsi
		if [ "$?" != "0" ]; then
			failed "Erro ao gerar instalador com runtime"
		fi
	fi

	mkdir -p ${DESTDIR}/${PACKAGE_NAME}/${1}
	mv -f ./win/*.exe ${DESTDIR}/${PACKAGE_NAME}/${1}
	if [ "$?" != "0" ]; then
		failed "Erro ao copiar pacotes de instalação"
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
			;;

		RT)
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

if [ "${RUNTIME}" == "1" ]; then

	echo -e "\e]2;Baixando runtime\a"

	#
	# Puxo scripts de construção do GTK direto da sourceforge.
	#
	cd ${RUNTIMEDIR}
	git clone http://git.code.sf.net/p/gtk3win/code .
	if [ "$?" != "0" ]; then
		exit -1
	fi

	for i in ${ARCHS}; do

		echo -e "\e]2;gtk-runtime-${i}\a"

		case ${i} in
		x86_32)
			host_cpu=i686
			./win32.sh
			if [ "$?" != "0" ]; then
				exit -1
			fi
			;;

		x86_64)
			host_cpu=x86_64
			./win64.sh
			if [ "$?" != "0" ]; then
				exit -1
			fi
			;;

		*)
			echo "Arquitetura desconhecida ${i}"
			exit -1

		esac

		rm -f gtk-runtime-*-${host_cpu}.exe


		chmod +x ./makeruntime.sh

		./makeruntime.sh
		if [ "$?" != "0" ]; then
			exit -1
		fi

		# Copia o pacote gerado
		FILENAME=$(find . -maxdepth 1 -name "gtk-runtime-*-${host_cpu}.exe" | head --lines 1)

		mkdir -p ${DESTDIR}/${host_cpu}

		mv gtk-runtime-*-${host_cpu}.exe ${DESTDIR}/${host_cpu}
		if [ "$?" != "0" ]; then
			failed "Erro ao copiar instalador"
		fi

		ln -sf $(basename ${FILENAME}) "${DESTDIR}/${host_cpu}/gtk-runtime-latest-${host_cpu}.exe"
		if [ "$?" != "0" ]; then
			failed "Erro ao criar o link simbólico"
		fi

	done

fi

cd $myDIR
rm -fr ${TEMPDIR}
rm -fr ${RUNTIMEDIR}

# Gera pacotes para envio ao SPB
#zip -9 -r -j	${DESTDIR}/${PACKAGE}-latest.zip \
#				${DESTDIR}/${PACKAGE}/x86_32/${PACKAGE_NAME}-with-gtk-latest-i686.exe \
#				${DESTDIR}/${PACKAGE}/x86_64/${PACKAGE_NAME}-with-gtk-latest-x86_64.exe 

echo -e "\e]2;Success!\a"




