#!/bin/bash

OO_SDK_HOME=/usr/lib64/libreoffice/sdk
IDLC=/usr/lib64/libreoffice/sdk/bin/idlc
CPPUMAKER=/usr/lib64/libreoffice/sdk/bin/cppumaker
TYPES_RDB=/usr/lib64/libreoffice/ure/share/misc/types.rdb
REGMERGE=/usr/lib64/libreoffice/ure/bin/regmerge

$IDLC -C -I$OO_SDK_HOME/idl -O. pw3270.idl
if [ "$?" != "0" ]; then
	exit -1
fi


$REGMERGE pw3270.rdb /UCR pw3270.urd
if [ "$?" != "0" ]; then
	exit -1
fi


$CPPUMAKER -O./include -Tpw3270.lib3270 $TYPES_RDB pw3270.rdb
if [ "$?" != "0" ]; then
	exit -1
fi

# XWeak
$IDLC -C -I$OO_SDK_HOME/idl -O. /usr/share/idl/libreoffice/com/sun/star/uno/XWeak.idl
if [ "$?" != "0" ]; then
	exit -1
fi

$REGMERGE XWeak.rdb /UCR XWeak.urd
if [ "$?" != "0" ]; then
	exit -1
fi

$CPPUMAKER -O./include $TYPES_RDB XWeak.rdb
if [ "$?" != "0" ]; then
	exit -1
fi


echo ok

