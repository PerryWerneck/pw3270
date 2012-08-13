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
# programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA, 02111-1307, USA
#
# Contatos:
#
# perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
# erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
# licinio@bb.com.br		(Licínio Luis Branco)
# kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
#

touch ChangeLog

REV_TO=${1:-"HEAD"}
REV_LAST=`head -3 ChangeLog | tr -d '\r\n' | sed -e 's/.*svn\([0-9]*\).*/\1/'`

svn update

REV_MAX=`svn --xml info | tr -d '\r\n' | sed -e 's/.*<commit.*revision="\([0-9]*\)".*<\/commit>.*/\1/'`

REV_FROM=${2:-$(($REV_LAST + 1))}

if [ $REV_FROM -gt $REV_MAX ]; then
	echo "No update required from revision $REV_FROM to revision $REV_MAX" 
	exit 0;
fi

echo "Downloading svn-log from revision $REV_FROM to revision $REV_MAX" 

svn --verbose --xml log -r "$REV_TO:$REV_FROM" | xsltproc --stringparam strip-prefix "trunk"  --stringparam linelen "75" --stringparam groupbyday "no" --stringparam separate-daylogs "no"  --stringparam include-rev "yes" --stringparam breakbeforemsg "no" --stringparam reparagraph "no" --stringparam authorsfile "" --stringparam title "ChangeLog" --stringparam revision-link "#r" --stringparam ignore-message-starting "" --nowrite --nomkdir --nonet "ChangeLog.xsl" - > "ChangeLog.new"
if [ "$?" != "0" ]; then
	exit -1
fi

cat "ChangeLog" >> "ChangeLog.new"

mv "ChangeLog.new" "ChangeLog"
