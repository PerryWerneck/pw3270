#!/bin/bash

cleanup() 
{
    #
    # Apaga diretorio temporário caso o script seja interrompido
    #
	cd ${myDIR}
	rm -fr ${TEMPDIR}
	rm -fr ${RUNTIMEDIR}
	rm -fr ${DESTDIR}
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
# $1 = Arquitetura (x86_32/x86_64)
# $2 = Tipo do link
#
CopyPacket()
{

	FILENAME=$(find . -maxdepth 1 -name "*.exe" | head --lines 1 | sed "s@./@@g")

	mkdir -p ${DESTDIR}/${1}

	cp ${FILENAME} ${DESTDIR}/${1}
	if [ "$?" != "0" ]; then
		failed "Erro ao copiar instalador"
	fi

	ln -sf ${FILENAME} ${DESTDIR}/${1}/pw3270-${1}-${2}.exe
	if [ "$?" != "0" ]; then
		failed "Erro ao criar o link simbólico"
	fi

}


#
# Gera pacote windows
#
# $1 = Arquitetura (x86_32/x86_64)
#
build()
{
	echo -e "\e]2;pw3270-${1}\a"

	case ${1} in
	x86_32)
		host=i686-w64-mingw32
		host_cpu=i686
		prefix=/usr/i686-w64-mingw32/sys-root/mingw
		;;

	x86_64)
		host=x86_64-w64-mingw32
		host_cpu=x86_64
		prefix=/usr/x86_64-w64-mingw32/sys-root/mingw
		;;


	*)
		failed "Arquitetura desconhecida: ${1}"

	esac

	./configure \
		--cache-file=${1}.cache \
		--host=${host} \
		--prefix=${prefix} \
		--disable-java

	if [ "$?" != "0" ]; then
		failed "Erro ao configurar"
	fi

	make clean
	rm -f *.exe

	make -C nsi pw3270-${host_cpu}.nsi
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

	chmod +x makegtkruntime.sh
	./makegtkruntime.sh
	if [ "$?" != "0" ]; then
		failed "Erro ao construir runtime gtk"
	fi

	echo -e "\e]2;pw3270-install-${host_cpu}.exe\a"
	cat nsi/pw3270-${host_cpu}.nsi | makensis \
                -DLOCALEDIR=".bin/Release${prefix}/share/locale" \
				-

#                -Onsis-${host_cpu}.log -

	if [ "$?" != "0" ]; then
		failed "Erro ao gerar pacote windows"
	fi

	CopyPacket ${1} "latest"

	make clean
	rm -f *.exe

}

myDIR=$(readlink -f $(dirname $0))
TEMPDIR=$(mktemp -d)
DESTDIR=${HOME}/public_html/win
RUNTIMEDIR=$(mktemp -d)
SRC=svn
ARCHS="x86_32 x86_64"
RUNTIME=0
COMPLETE=1

trap cleanup INT 

until [ -z "$1" ]
do
	if [ ${1:0:2} = '--' ]; then
		tmp=${1:2}
		parameter=${tmp%%=*}
		parameter=$(echo $parameter | tr "[:lower:]" "[:upper:]")

		case $parameter in

		SRC)
			value=${tmp##*=}
			eval SRC=$value
			;;

		FULL)
			RUNTIME=0
			COMPLETE=1
			;;

		RT)
			RUNTIME=1
			COMPLETE=0
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

		rm -f *.exe

		echo -e "\e]2;gtk-runtime-${i}\a"

		case ${i} in
		x86_32)
			./win32.sh
			if [ "$?" != "0" ]; then
				exit -1
			fi
			;;

		x86_64)
			./win64.sh
			if [ "$?" != "0" ]; then
				exit -1
			fi
			;;

		*)
			echo "Arquitetura desconhecida ${i}"
			exit -1

		esac

		chmod +x ./makeruntime.sh

		./makeruntime.sh
		if [ "$?" != "0" ]; then
			exit -1
		fi

		cp *.exe ${DESTDIR}/${i}
		if [ "$?" != "0" ]; then
			failed "Erro ao copiar instalador"
		fi

	done

fi

cd $myDIR
rm -fr ${TEMPDIR}
rm -fr ${RUNTIMEDIR}

zip -r -j ${HOME}/public_html/win/pw3270-latest.zip \
			${HOME}/public_html/win/x86_32/pw3270-x86_32-latest.exe \
			${HOME}/public_html/win/x86_64/pw3270-x86_64-latest.exe 

echo -e "\e]2;Success!\a"




