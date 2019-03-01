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
VERSION="5.2"

install() {

	BUILD_DIR=${TEMPDIR}/${1}

	mkdir -p ${BUILD_DIR}
	if [ "$?" != "0" ]; then
		exit -1
	fi

	pushd ${BUILD_DIR}
	if [ "$?" != "0" ]; then
		exit -1
	fi

	wget https://github.com/PerryWerneck/${1}/releases/download/${VERSION}/PKGBUILD
	if [ "$?" != "0" ]; then
		exit -1
	fi

	makepkg
	if [ "$?" != "0" ]; then
		exit -1
	fi

	popd

	cp ${BUILD_DIR}/mingw*-${1}*.tar.xz .

	rm -fr ${BUILD_DIR}

	pacman -U mingw*-${1}*.tar.xz
	if [ "$?" != "0" ]; then
		exit -1
	fi

}

TEMPDIR=$(mktemp -d)

install lib3270
install libv3270

rm -fr ${TEMPDIR}
