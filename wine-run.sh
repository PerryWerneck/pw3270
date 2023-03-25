#!/bin/bash

make DESTDIR=${PWD}/.build install 
if [ "$?" != "0" ]; then
	exit -1
fi

bash ./win/makeruntime.sh 
if [ "$?" != "0" ]; then
	exit -1
fi

wine .build/usr/x86_64-w64-mingw32/sys-root/mingw/bin/pw3270.exe
if [ "$?" != "0" ]; then
	exit -1
fi



