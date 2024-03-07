#!/bin/bash

PROJECT_NAME=$(grep AC_INIT configure.ac | cut -d[ -f2 | cut -d] -f1)
VERSION=$(grep AC_INIT configure.ac | cut -d[ -f3 | cut -d] -f1)

unpack() {

	echo "Unpacking ${1}"

	tar -C $(brew --cellar) -Jxf macos-${1}.tar.xz 
	if [ "$?" != "0" ]; then
		exit -1
	fi

	brew link ${1}
	if [ "$?" != "0" ]; then
		exit -1
	fi

	rm -f macos-${1}.tar.xz
	
}

unpack lib3270
unpack libv3270

./autogen.sh --prefix="/${PROJECT_NAME}/${VERSION}"
if [ "$?" != "0" ]; then
	exit -1
fi

make all
if [ "$?" != "0" ]; then
	exit -1
fi

make DESTDIR=.bin/package install
tar --create --xz --file=macos-${PROJECT_NAME}.tar.xz --directory=.bin/package --verbose .

find . -iname *.tar.xz


