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
# Unpack lib3270
#
echo "Unpacking lib3270"
tar -C / -Jxf mingw-lib3270.${MSYSTEM_CARCH}.tar.xz > $LOGFILE 2>&1 || die "lib3270 unpack failure"

#
# Unpack libv3270
#
echo "Unpacking lib3270"
tar -C / -Jxf mingw-libv3270.${MSYSTEM_CARCH}.tar.xz > $LOGFILE 2>&1 || die "libv3270 unpack failure"

#
# Build PW3270
#
echo "Building PW3270"
cd ${myDIR}
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"

echo "Build complete"

