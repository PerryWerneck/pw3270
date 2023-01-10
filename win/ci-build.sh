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

cd $(dirname $(dirname $(readlink -f ${0})))

rm -fr .build

#
# Build LIB3270
#
echo "Building lib3270"
mkdir -p .build/lib3270
git clone https://github.com/PerryWerneck/lib3270.git ./.build/lib3270 > $LOGFILE 2>&1 || die "clone lib3270 failure"
pushd ./.build/lib3270
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"
popd

export LIB3270_CFLAGS="-I./.build/lib3270/src/include"
export LIB3270_LIBS="-L./.build/lib3270/.bin/Release -l3270.delayed"

#
# Build LIBV3270
#
echo "Building libv3270"
mkdir -p .build/libv3270
git clone https://github.com/PerryWerneck/libv3270.git ./.build/libv3270 > $LOGFILE 2>&1 || die "clone libv3270 failure"
pushd ./.build/libv3270
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"
popd

export LIBV3270_CFLAGS="-I./.build/libv3270/src/include"
export LIBV3270_LIBS="-L./.build/libv3270/.bin/Release -lv3270"

#
# Build PW3270
#
echo "Building PW3270"
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"

echo "Build complete"

