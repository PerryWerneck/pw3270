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
# programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
# St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Contatos:
#
# perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
# erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
#
#

PRODUCT_NAME="pw3270"
LIBRARY_NAME="lib3270"
CORE_LIBRARIES="lib3270 libv3270 libipc3270"
PACKAGE_PLUGINS=""
PACKAGE_EXTRAS="libhllapi"
TARGET_ARCHS="x86_64 x86_32"
GIT_URL="https://github.com/PerryWerneck"
BUILD_UNSTABLE=0
MAKE_ZIP=1

PROJECTDIR=$(dirname $(dirname $(readlink -f ${0})))
WORKDIR=$(mktemp -d)
PUBLISH=0
GET_PREREQS=0
CERTS_DIR=${WORKDIR}/certs
PAUSE_ON_ERROR=0

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

failed()
{
	echo "$@"

	if [ "${PAUSE_ON_ERROR}" != "0" ]; then
		echo "Type exit to end build script"
		/bin/bash
	fi

	cleanup

	exit -1
}


#
# Get Sources from GIT
#
clone()
{
	echo -e "\e]2;Cloning ${1}\a"
	echo "Cloning ${1}"

	mkdir -p ${WORKDIR}/sources

	if [ "${BUILD_UNSTABLE}" == "1" ]; then
		BRANCH="develop"
	else
		TEMPVAR=${1}_branch
		BRANCH=${!TEMPVAR}
	fi

	if [ -z ${BRANCH} ]; then
		git clone --quiet ${GIT_URL}/${1}.git ${WORKDIR}/sources/${1}
	else
		echo -e "\e]2;Cloning ${1} ${BRANCH}\a"
		echo "Cloning ${1} ${BRANCH}"
		git clone --quiet --branch "${BRANCH}" ${GIT_URL}/${1}.git ${WORKDIR}/sources/${1}
	fi

	if [ "$?" != "0" ]; then
		failed "Can't get sources for ${1}"
	fi

}

prepare()
{
	echo -e "\e]2;Preparing ${1}\a"
	echo "Preparing ${1}"

	if [ -x ${PROJECTDIR}/win/prepare.${1} ]; then
		pushd ${WORKDIR}/sources/${1}
		${PROJECTDIR}/win/prepare.${1}
		if [ "$?" != "0" ]; then
			failed "Can't prepare ${1}"
		fi
		popd
	fi

	for ARCH in ${TARGET_ARCHS}
	do

		if [ -d ${WORKDIR}/sources/${1}/win/${ARCH} ]; then

			for spec in $(find ${WORKDIR}/sources/${1}/win/${ARCH} -name "*.spec")
			do
				grep -i buildrequires "${spec}" | grep -v "%" | cut -d: -f2- | tr -d '[:blank:]' >> ${WORKDIR}/sources/pre-reqs
			done
		
		fi

	done

}

#
# Configure
#
configure()
{

	if [ "${GET_PREREQS}" != "0" ]; then

		echo -e "\e]2;Installing pre-reqs\a"
		echo "Installing pre-reqs"

		echo "mingw32-cross-nsis" >> ${WORKDIR}/sources/pre-reqs 

		cat ${WORKDIR}/sources/pre-reqs \
			| grep -v 3270 \
			| sort --unique \
			| xargs sudo zypper --non-interactive --quiet in

	fi

	echo -e "\e]2;Creating configuration\a"
	echo "Creating configuration"
	for DIR in $(find ${WORKDIR}/sources -maxdepth 1 -type d)
	do
		echo ${DIR}
		cd ${DIR}

		if [ -x ./autogen.sh ]; then
			NOCONFIGURE=1 ./autogen.sh
			if [ "$?" != "0" ]; then
				failed "Erro em autogen.sh"
			fi
		fi
	done

}

#
# Build library
#
buildLibrary()
{
	echo "Building library ${1}"

	for ARCH in ${TARGET_ARCHS}
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
			failed "Arquitetura desconhecida: ${ARCH}"

		esac

		# Create install dirs
		mkdir -p ${WORKDIR}/build/${ARCH}
		mkdir -p ${WORKDIR}/cache/${ARCH}
		mkdir -p ${WORKDIR}/build/${ARCH}/locale
		mkdir -p ${WORKDIR}/build/${ARCH}/include

		export PKG_CONFIG_PATH=${WORKDIR}/build/${ARCH}/pkgconfig
		export cache=${WORKDIR}/cache/${ARCH}/${1}.cache

		cd ${WORKDIR}/sources/${1}

		if [ -x ${PROJECTDIR}/win/configure.${1} ]; then

			host="${host}" \
			prefix="${prefix}" \
			BUILDDIR="${WORKDIR}/build/${ARCH}" \
			CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			CXXFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			LDFLAGS="-L${WORKDIR}/build/${ARCH}" \
				${PROJECTDIR}/win/configure.${1}

		else

			./configure \
				CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
				CXXFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
				LDFLAGS="-L${WORKDIR}/build/${ARCH}" \
				--host=${host} \
				--prefix=${prefix} \
				--with-product-name="${PRODUCT_NAME}" \
				--bindir=${WORKDIR}/build/${ARCH} \
				--libdir=${WORKDIR}/build/${ARCH} \
				--localedir=${WORKDIR}/build/${ARCH}/locale \
				--includedir=${WORKDIR}/build/${ARCH}/include \
				--sysconfdir=${WORKDIR}/build/${ARCH} \
				--datadir=${WORKDIR}/build/${ARCH} \
				--datarootdir=${WORKDIR}/build/${ARCH}
		fi

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
# Build language binding
# 
buildExtraPackage() 
{

	echo "Building ${1}"

	for ARCH in ${TARGET_ARCHS}
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
			failed "Arquitetura desconhecida: ${ARCH}"

		esac

		# Required for lib3270 build tools
		export HOST_CC=/usr/bin/gcc

		# Required for .NET bindings
		export GACROOT=${WORKDIR}/build/${ARCH}/mono/gacroot
		export GAPIROOT=${WORKDIR}/build/${ARCH}/mono/gapi-2.0
		export MONOLIBPATH=${WORKDIR}/build/${ARCH}/mono/lib

		# Create install dirs
		mkdir -p ${WORKDIR}/build/${ARCH}
		mkdir -p ${WORKDIR}/cache/${ARCH}
		mkdir -p ${WORKDIR}/build/${ARCH}/locale
		mkdir -p ${WORKDIR}/build/${ARCH}/include

		export PKG_CONFIG_PATH=${WORKDIR}/build/${ARCH}/pkgconfig
		export cache=${WORKDIR}/cache/${ARCH}/${1}.cache

		cd ${WORKDIR}/sources/${1}

		if [ -x ${PROJECTDIR}/win/configure.${1} ]; then

			host="${host}" \
			prefix="${prefix}" \
			BUILDDIR="${WORKDIR}/build/${ARCH}" \
			CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			CXXFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			LDFLAGS="-static-libgcc -static-libstdc++ -L${WORKDIR}/build/${ARCH}" \
				${PROJECTDIR}/win/configure.${1}

		else

			./configure \
				CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
				CXXFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
				LDFLAGS="-static-libgcc -static-libstdc++ -L${WORKDIR}/build/${ARCH}" \
				--host=${host} \
				--prefix=${prefix} \
				--with-product-name="${PRODUCT_NAME}" \
				--bindir=${WORKDIR}/build/${ARCH} \
				--libdir=${WORKDIR}/build/${ARCH} \
				--localedir=${WORKDIR}/build/${ARCH}/locale \
				--includedir=${WORKDIR}/build/${ARCH}/include \
				--sysconfdir=${WORKDIR}/build/${ARCH} \
				--datadir=${WORKDIR}/build/${ARCH} \
				--datarootdir=${WORKDIR}/build/${ARCH}

		fi

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
	for ARCH in ${TARGET_ARCHS}
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
			failed "Arquitetura desconhecida: ${ARCH}"

		esac

		mkdir -p ${WORKDIR}/build/${ARCH}
		mkdir -p ${WORKDIR}/cache/${ARCH}
		mkdir -p ${WORKDIR}/build/${ARCH}/locale
		mkdir -p ${WORKDIR}/build/${ARCH}/include

		export HOST_CC=/usr/bin/gcc
		export PKG_CONFIG_PATH=${WORKDIR}/build/${ARCH}/pkgconfig
		export cache=${WORKDIR}/cache/${ARCH}/${1}.cache

		cd ${WORKDIR}/sources/${1}

		for NSI in $(find ./win -name '*.nsi.in')
		do
			SRCNAME="${PROJECTDIR}/win/$(basename ${NSI})"
			if [ -e "${SRCNAME}" ]; then
				cp "${SRCNAME}" "${NSI}"
				if [ "$?" != "0" ]; then
					failed "Can't copy ${SRCNAME}"
				fi
			fi
		done

		if [ -x ${PROJECTDIR}/win/configure.${1} ]; then

			host="${host}" \
			prefix="${prefix}" \
			BUILDDIR="${WORKDIR}/build/${ARCH}" \
			CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
			LDFLAGS="-L${WORKDIR}/build/${ARCH}" \
				${PROJECTDIR}/win/configure.${1}

		else

			./configure \
				CFLAGS="-I${WORKDIR}/build/${ARCH}/include" \
				LDFLAGS="-L${WORKDIR}/build/${ARCH}" \
				--host=${host} \
				--prefix=${prefix} \
				--bindir=${WORKDIR}/build/${ARCH} \
				--libdir=${WORKDIR}/build/${ARCH} \
				--localedir=${WORKDIR}/build/${ARCH}/locale \
				--includedir=${WORKDIR}/build/${ARCH}/include \
				--sysconfdir=${WORKDIR}/build/${ARCH} \
				--datadir=${WORKDIR}/build/${ARCH} \
				--datarootdir=${WORKDIR}/build/${ARCH}

		fi


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

		if [ -x ${PROJECTDIR}/win/install.${1} ]; then
			pushd "${WORKDIR}/build/${ARCH}"
			echo "Executando install.${1} em ${PWD}"
			${PROJECTDIR}/win/install.${1}
			if [ "$?" != "0" ]; then
				failed "Can't install ${1}"
			fi
			popd
		fi

		for NSI in $(find ./win -name '*.nsi')
		do
			cp "${NSI}" "${WORKDIR}/build/${ARCH}"
			if [ "$?" != "0" ]; then
				failed "Can't copy ${NSI}"
			fi
		done

		if [ -e ./win/makeruntime.sh ]; then
			cp "./win/makeruntime.sh" "${WORKDIR}/build/${ARCH}/${1}-makeruntime.sh"
			if [ "$?" != "0" ]; then
				failed "Can't copy ${1}.makeruntime.sh"
			fi
		fi

		if [ -e branding/${1}.svg ]; then
			convert -density 384 -background transparent branding/${1}.svg -define icon:auto-resize -colors 256 ${WORKDIR}/build/${ARCH}/${1}.ico
			if [ "$?" != "0" ]; then
				cleanup
				exit -1
			fi
		fi

		for doc in LICENSE LICENCA README.md AUTHORS
		do

			if [ -e ${doc} ]; then
				cp ${doc} ${WORKDIR}/build/${ARCH}

				if [ "$?" != "0" ]; then
					cleanup
					exit -1
				fi

			fi

		done

	done

}

#
# Make runtime
#
makeRuntime() 
{

	for ARCH in ${TARGET_ARCHS}
	do

		echo -e "\e]2;Building runtime for ${ARCH}\a"
		echo "Building runtime for ${ARCH}"

		rm -fr ${WORKDIR}/build/${ARCH}/runtime
		mkdir -p ${WORKDIR}/build/${ARCH}/runtime

		for SCRIPT in ${WORKDIR}/build/${ARCH}/*-makeruntime.sh
		do
			chmod +x ${SCRIPT}

			cd ${WORKDIR}/build/${ARCH}
			${SCRIPT} --output-dir="${WORKDIR}/build/${ARCH}/runtime" --bindir="${WORKDIR}/build/${ARCH}"
			if [ "$?" != "0" ]; then
				failed "Error on ${SCRIPT}"
			fi
		done

	done

}

#
# Make packages
#
makeInstaller()
{
	NSIS_ARGS="-DWITHGTK -DWITHIPC -DWITHPLUGINS -DWITHSDK"

	if [ -d ${CERTS_DIR} ]; then
		NSIS_ARGS="${NSIS_ARGS} -DWITHCERTS"
	fi

	if [ ! -z "${PACKAGE_EXTRAS}" ]; then
		NSIS_ARGS="${NSIS_ARGS} -DWITHEXTRAS"
	fi

	for ARG in $(echo ${PACKAGE_PLUGINS} | tr "[:lower:]" "[:upper:]") $(echo ${PACKAGE_EXTRAS} | tr "[:lower:]" "[:upper:]")
	do
		NSIS_ARGS="${NSIS_ARGS} -DWITH${ARG}"
	done

	for ARCH in ${TARGET_ARCHS}
	do

		echo -e "\e]2;Creating installers for ${ARCH}\a"
		echo "Creating installers for ${ARCH}"

		if [ -d ${CERTS_DIR} ]; then

			mkdir -p ${WORKDIR}/build/${ARCH}/sslcerts
			cp -rv ${CERTS_DIR}/* ${WORKDIR}/build/${ARCH}/sslcerts
			if [ "$?" != "0" ]; then
				failed "Can't copy certs"
			fi

		fi

		if [ -d ${PROJECTDIR}/ui ]; then
			mkdir -p ${WORKDIR}/build/${ARCH}/${PRODUCT_NAME}/ui
			cp -rv ${PROJECTDIR}/ui/* ${WORKDIR}/build/${ARCH}/${PRODUCT_NAME}/ui

			if [ "$?" != "0" ]; then
				failed "Can't copy UI files"
			fi
		fi

		cd ${WORKDIR}/build/${ARCH}

		TARCH=${ARCH}
		if [ "${TARCH}" == "x86_32" ]; then
			TARCH="i686"
		fi

		if [ ${BUILD_UNSTABLE} == "1" ]; then
			TARGET_PATH="/${PRODUCT_NAME}/unstable/${ARCH}"
		else
			TARGET_PATH="/${PRODUCT_NAME}/${ARCH}"
		fi

		if [ "${MAKE_ZIP}" == "1" ]; then

			ZIPNAME="${WORKDIR}/build/${ARCH}/${PRODUCT_NAME}-${ARCH}.zip"

			rm -f "${ZIPNAME}"
			zip -9 "${ZIPNAME}" *.dll *.exe *.ico

			pushd runtime
			zip -9 -r "${ZIPNAME}" .

			popd

			if [ -d ~/public_html ]; then
				mkdir -p ~/public_html/win/${TARGET_PATH}
				cp -v "${ZIPNAME}" ~/public_html/win/${TARGET_PATH}
				if [ "$?" != "0" ]; then
					failed "Can't copy zip to ~/public_html/win/${TARGET_PATH}"
				fi
			fi

			if [ "${PUBLISH}" == "1" ] && [ ! -z ${WIN_PACKAGE_SERVER} ]; then

				scp "${ZIPNAME}" ${WIN_PACKAGE_SERVER}/${TARGET_PATH}
				if [ "$?" != "0" ]; then
					failed "Can't publish zip to ${WIN_PACKAGE_SERVER}/${TARGET_PATH}"
				fi
			fi

			mv -f "${ZIPNAME}" ${PROJECTDIR}
			if [ "$?" != "0" ]; then
				failed "Can't move zip to ${PROJECTDIR}"
			fi

		fi

		for NSI in *.nsi
		do
			makensis ${NSIS_ARGS} ${NSI}
			if [ "$?" != "0" ]; then
				echo makensis ${NSIS_ARGS} ${NSI}
				failed "Error building ${ARCH} ${NSI}"
			fi

			if [ -d ~/public_html ]; then
				mkdir -p ~/public_html/win/${TARGET_PATH}
				cp -v *-[0-9]*-${TARCH}.exe ~/public_html/win/${TARGET_PATH}
				if [ "$?" != "0" ]; then
					failed "Can't copy binary to ~/public_html/win/${TARGET_PATH}"
				fi
			fi
			
			if [ "${PUBLISH}" == "1" ] && [ ! -z ${WIN_PACKAGE_SERVER} ]; then

				scp *-[0-9]*-${TARCH}.exe ${WIN_PACKAGE_SERVER}/${TARGET_PATH}
				if [ "$?" != "0" ]; then
					failed "Can't publish to ${WIN_PACKAGE_SERVER}/${TARGET_PATH}"
				fi
			fi

			mv -f *-[0-9]*-${TARCH}.exe ${PROJECTDIR}
			if [ "$?" != "0" ]; then
				failed "Can't move installer to ${PROJECTDIR}"
			fi

		done

	done

}

#
# Add repos
#
addRepos() {

	for ARCH in ${TARGET_ARCHS}
	do
		case ${ARCH} in
		x86_32)
			# https://download.opensuse.org/repositories/windows:/mingw:/win32/openSUSE_Leap_15.1/windows:mingw:win32.repo
			REPO_ARCH="win32"
			;;

		x86_64)
			# https://download.opensuse.org/repositories/windows:/mingw:/win64/openSUSE_Leap_15.1/windows:mingw:win64.repo
			REPO_ARCH="win64"
			;;

		*)
			failed "Arquitetura desconhecida: ${ARCH}"

		esac


		echo zypper ar "https://download.opensuse.org/repositories/windows:/mingw:/${REPO_ARCH}/$(echo ${PRETTY_NAME} | sed "s@ @_@g")" ${REPO_ARCH}

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
		NO-NOPUBLISH)
			PUBLISH=0
			;;

		PUBLISH)
			PUBLISH=1
			;;

		PRODUCT-NAME)
			PRODUCT_NAME=${value}
			;;

		SOURCES-FROM)
			GIT_URL=${value}
			;;

		CERTS-FROM)
			CERTS_DIR=${value}
			;;

		ADD-REPOS)
			addRepos
			exit 0
			;;

		CLEAR)
			if [ -d ~/public_html/win/${PRODUCT_NAME} ]; then
				rm -fr ~/public_html/win/${PRODUCT_NAME}/{x86_32,x86_64}
			fi

			;;

		EXTRA-PACKAGES)
			PACKAGE_EXTRAS=$(echo ${value} | sed "s@,@ @g")
			;;

		TARGET-ARCHS)
			TARGET_ARCHS=${value}
			;;

		NO-PRE-REQS)
			GET_PREREQS=0
			;;

		PRE-REQS)
			GET_PREREQS=1
			;;

		PROJECT-PATH)
			PROJECTDIR=$(readlink -f ${value})
			;;

		UNSTABLE)
			BUILD_UNSTABLE=1
			;;

		DEVELOP)
			BUILD_UNSTABLE=1
			;;

		SHELL-ON-ERROR)
			PAUSE_ON_ERROR=1
			;;

		NO_ZIP)
			MAKE_ZIP=0
			;;

		ZIP)
			MAKE_ZIP=1
			;;

		HELP)
			echo "${0} [options]"
			echo ""
			echo "Options:"
			echo ""

			echo "  --product-name	Set the product name (current is ${PRODUCT_NAME})"
			echo "  --project-path	Set the path for the customization data"
			echo "  --target-archs	Set the target architectures (current are ${TARGET_ARCHS})"
			echo "  --sources-from	Base URL of the git server with the sources (current is ${GIT_URL})"
			echo "  --no-pre-reqs		Don't try to install required packages"
			echo "  --pre-reqs		Install required packages"
			echo "  --shell-on-error	Open a shell when the build process failed"
			echo "  --unstable		Build unstable version"
			
			if [ "${MAKE_ZIP}" == "1" ]; then
				echo "  --no-zip		Don't create zip file"
			else
				echo "  --zip			Create zip file"
			fi

			if [ ! -z ${WIN_PACKAGE_SERVER} ]; then
				echo "  --no-publish		Don't publish binaries in ${WIN_PACKAGE_SERVER}/${PRODUCT_NAME}"
				echo "  --publish		Publish binaries in ${WIN_PACKAGE_SERVER}/${PRODUCT_NAME}"
			fi

			if [ -d ~/public_html/win/${PRODUCT_NAME} ]; then
				echo "  --clear		Replace the contents of ~/public_html/win/${PRODUCT_NAME}/{x86_32,x86_64}"
			fi

			echo ""
			exit 0
			
			;;
		
		esac
	fi

	shift

done

#
# Load customizations
#
if [ -e ${PROJECTDIR}/win/pack.conf ]; then
	. ${PROJECTDIR}/win/pack.conf
fi

#
# Download sources
#
for src in ${CORE_LIBRARIES}
do
	echo "Core library: ${src}"
	clone ${src}
	prepare ${src}
done

clone pw3270
prepare pw3270

for src in ${PACKAGE_PLUGINS}
do
	echo "Plugin module: ${src}"
	clone pw3270-plugin-${src}
	prepare pw3270-plugin-${src}
done

for src in ${PACKAGE_EXTRAS}
do
	echo "Extra package: ${src}"
	clone ${src}
	prepare ${src}
done

#
# Build packages
#
configure

for src in ${CORE_LIBRARIES}
do
	buildLibrary ${src}
done

buildApplication pw3270

for src in ${PACKAGE_PLUGINS}
do
	buildLibrary pw3270-plugin-${src}
done

for src in ${PACKAGE_EXTRAS}
do
	buildExtraPackage ${src}
done

#
# Create runtime
#
makeRuntime
makeInstaller

cleanup




