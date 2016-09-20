#!/bin/bash

PACKAGE_NAME="pw3270"

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
# Copia pacote gerado
#
# $1 = Arquitetura (i686/x86_64)
# $2 = Tipo do link
#
CopyPacket()
{
	#
	# Primeiro move a versão baseada no runtime
	#
	FILENAME=$(find nsi -maxdepth 1 -name "${PACKAGE_NAME}-*-requires-gtk-*-${1}.exe" | head --lines 1)

	if [ ! -z ${FILENAME} ]; then

		mkdir -p ${DESTDIR}/${1}

		echo "Copiando ${FILENAME} para ${DESTDIR}/${1}"

		mv "${FILENAME}" "${DESTDIR}/${1}"
		if [ "$?" != "0" ]; then
			echo "src=${FILENAME}"
			echo "dst=${DESTDIR}/${1}"
			failed "Erro ao copiar instalador sem o runtime"
		fi

		#
		# Cria link do pacote sem GTK para "latest"
		#
		ln -sf $(basename ${FILENAME}) ${DESTDIR}/${1}/${PACKAGE_NAME}-without-gtk-${2}-${1}.exe
		if [ "$?" != "0" ]; then
			failed "Erro ao criar o link simbólico"
		fi


	fi

	#
	# Depois copia o pacote completo
	#
	FILENAME=$(find nsi -maxdepth 1 -name "${PACKAGE_NAME}-*-gtk-*-${1}.exe" | head --lines 1)

	if [ ! -z ${FILENAME} ]; then

		mkdir -p ${DESTDIR}/${1}

		echo "Copiando ${FILENAME} para ${DESTDIR}/${1}"

		mv "${FILENAME}" "${DESTDIR}/${1}"
		if [ "$?" != "0" ]; then
			echo "src=${FILENAME}"
			echo "dst=${DESTDIR}/${1}"
			failed "Erro ao copiar instalador completo"
		fi

		#
		# Cria link do pacote completo para "latest"
		#
		ln -sf $(basename ${FILENAME}) ${DESTDIR}/${1}/${PACKAGE_NAME}-with-gtk-${2}-${1}.exe
		if [ "$?" != "0" ]; then
			failed "Erro ao criar o link simbólico"
		fi

	fi

}


#
# Gera pacote windows
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

	REXXCONFIG=$(which ${tools}-oorexx-config)
	if [ -z ${REXXCONFIG} ]; then
		echo "Desabilitando suporte ooRexx"
		ARGS="${ARGS} --disable-rexx"
	fi

	./configure \
		--cache-file=.${1}.cache \
		--host=${host} \
		--prefix=${prefix} \
		--disable-python \
		${ARGS}
 
	if [ "$?" != "0" ]; then
		failed "Erro ao configurar"
	fi

	make clean
	rm -f *.exe

	make -C nsi ${PACKAGE_NAME}-${host_cpu}.nsi
	if [ "$?" != "0" ]; then
		failed "Erro ao gerar script de empacotamento windows"
	fi

	make Release
	if [ "$?" != "0" ]; then
		failed "Erro ao compilar fontes"
	fi

	ln -sf .${prefix}/share/locale .bin/Release/locale
	if [ "$?" != "0" ]; then
		failed "Erro ao criar link para traduções"
	fi

	if [ "${COMPLETE}" != "0" ]; then

		# Gera pacote completo

		chmod +x makegtkruntime.sh
		./makegtkruntime.sh
		if [ "$?" != "0" ]; then
			failed "Erro ao construir runtime gtk"
		fi

		echo -e "\e]2;${PACKAGE_NAME}-install-${host_cpu}.exe\a"
		make -C nsi package
		if [ "$?" != "0" ]; then
			failed "Erro ao gerar pacote windows"
		fi

	fi

	if [ "${RUNTIME}" != "0" ]; then

		make -C nsi package-no-gtk
		if [ "$?" != "0" ]; then
			failed "Erro ao gerar pacote windows"
		fi

	fi

	CopyPacket ${host_cpu} "latest"

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

		FULL)
			RUNTIME=0
			COMPLETE=1
			;;

		RT)
			RUNTIME=1
			COMPLETE=0
			;;

		NAME)
			PACKAGE_NAME=$value
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
zip -9 -r -j	${HOME}/public_html/win/${PACKAGE_NAME}-latest.zip \
				${HOME}/public_html/win/x86_32/${PACKAGE_NAME}-with-gtk-latest-i686.exe \
				${HOME}/public_html/win/x86_64/${PACKAGE_NAME}-with-gtk-latest-x86_64.exe 

echo -e "\e]2;Success!\a"




