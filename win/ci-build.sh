#!/bin/bash
#
# References:
#
#	* https://www.msys2.org/docs/ci/
#
#
echo "Running ${0}"

LOGFILE=build.log
rm -f ${LOGFILE}

die ( ) {
	[ -s $LOGFILE ] && tail $LOGFILE
	[ "$1" ] && echo "$*"
	exit -1
}

myDIR=$(dirname $(dirname $(readlink -f ${0})))
echo "myDIR=${myDIR}"

cd ${myDIR}
rm -fr ${myDIR}/.build

#
# Build LIB3270
#
if [ -e mingw-lib3270.tar.xz ]; then

	echo "Unpacking lib3270"
	tar -C / -Jxvf mingw-lib3270.tar.xz 

else
	echo "Building lib3270"
	git clone https://github.com/PerryWerneck/lib3270.git ./.build/lib3270 || die "clone lib3270 failure"
	cd ./.build/lib3270
	./autogen.sh || die "Lib3270 autogen failure"
	./configure || die "Lib3270 Configure failure"
	make clean || die "Lib3270 Make clean failure"
	make all || die "Lib3270 Make failure"
	make install || die "Lib3270 Install failure"
	cd ../..
fi

#
# Build LIBV3270
#
if [ -e mingw-libv3270.tar.xz ]; then

	echo "Unpacking libv3270"
	tar -C / -Jxvf mingw-lib3270.tar.xz 

else
	echo "Building libv3270"
	mkdir -p  ${myDIR}/.build/libv3270
	git clone https://github.com/PerryWerneck/libv3270.git ${myDIR}/.build/libv3270 > $LOGFILE 2>&1 || die "clone libv3270 failure"
	pushd ${myDIR}/.build/libv3270
	./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
	./configure > $LOGFILE 2>&1 || die "Configure failure"
	make clean > $LOGFILE 2>&1 || die "Make clean failure"
	make all  > $LOGFILE 2>&1 || die "Make failure"
	make install  > $LOGFILE 2>&1 || die "Install failure"
	popd
fi

#
# Build PW3270
#
echo "Building PW3270"
cd ${myDIR}
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"

make DESTDIR=.bin/package install
tar --create --xz --file=mingw-pw3270.tar.xz --directory=.bin/package --verbose .

echo "Build complete"

