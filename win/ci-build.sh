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

<<<<<<< HEAD
cd $(dirname $(dirname $(readlink -f ${0})))

rm -fr .build
=======
myDIR=$(dirname $(dirname $(readlink -f ${0})))
echo "myDIR=${myDIR}"

cd ${myDIR}
rm -fr ${myDIR}/.build
>>>>>>> master

#
# Build LIB3270
#
echo "Building lib3270"
<<<<<<< HEAD
mkdir -p .build/lib3270
git clone https://github.com/PerryWerneck/lib3270.git ./.build/lib3270 > $LOGFILE 2>&1 || die "clone lib3270 failure"
pushd ./.build/lib3270
=======
mkdir -p ${myDIR}/.build/lib3270
git clone https://github.com/PerryWerneck/lib3270.git ${myDIR}/.build/lib3270 > $LOGFILE 2>&1 || die "clone lib3270 failure"
pushd  ${myDIR}/.build/lib3270
>>>>>>> master
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"
<<<<<<< HEAD
popd

export LIB3270_CFLAGS="-I./.build/lib3270/src/include"
export LIB3270_LIBS="-L./.build/lib3270/.bin/Release -l3270.delayed"

=======
make install  > $LOGFILE 2>&1 || die "Install failure"
popd

>>>>>>> master
#
# Build LIBV3270
#
echo "Building libv3270"
<<<<<<< HEAD
mkdir -p .build/libv3270
git clone https://github.com/PerryWerneck/libv3270.git ./.build/libv3270 > $LOGFILE 2>&1 || die "clone libv3270 failure"
pushd ./.build/libv3270
=======
mkdir -p  ${myDIR}/.build/libv3270
git clone https://github.com/PerryWerneck/libv3270.git ${myDIR}/.build/libv3270 > $LOGFILE 2>&1 || die "clone libv3270 failure"
pushd ${myDIR}/.build/libv3270
>>>>>>> master
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"
<<<<<<< HEAD
popd

export LIBV3270_CFLAGS="-I./.build/libv3270/src/include ${LIB3270_CFLAGS}"
export LIBV3270_LIBS="-L./.build/libv3270/.bin/Release -lv3270 ${LIB3270_LIBS}"

=======
make install  > $LOGFILE 2>&1 || die "Install failure"
popd

>>>>>>> master
#
# Build PW3270
#
echo "Building PW3270"
<<<<<<< HEAD
=======
cd ${myDIR}
>>>>>>> master
./autogen.sh > $LOGFILE 2>&1 || die "Autogen failure"
./configure > $LOGFILE 2>&1 || die "Configure failure"
make clean > $LOGFILE 2>&1 || die "Make clean failure"
make all  > $LOGFILE 2>&1 || die "Make failure"

echo "Build complete"

