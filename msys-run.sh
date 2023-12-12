#!/bin/bash

make all
if [ "$?" != "0" ]; then
	exit -1
fi

make DESTDIR=${PWD}/.build install 
if [ "$?" != "0" ]; then
	exit -1
fi

bash ./win/makeruntime.sh 
if [ "$?" != "0" ]; then
	exit -1
fi

.build/mingw64/bin/pw3270.exe
if [ "$?" != "0" ]; then
	exit -1
fi



