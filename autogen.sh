#!/bin/bash

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

mkdir -p m4

aclocal
if test $? != 0 ; then
	echo "aclocal failed."
	exit -1
fi

autoconf
if test $? != 0 ; then
	echo "autoconf failed."
	exit -1
fi

mkdir -p scripts
automake --add-missing 2> /dev/null | true

cd "$olddir"
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"




