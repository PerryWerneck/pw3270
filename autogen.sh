#!/bin/bash
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Copyright (C) 2008 Banco do Brasil S.A.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

builddir=${PWD}

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

cd "$srcdir"

mkdir -p scripts
mkdir -p m4

LIBTOOLIZE=$(which libtoolize)
if [ -z ${LIBTOOLIZE} ]; then
	LIBTOOLIZE=$(which glibtoolize)
fi
if [ -z ${LIBTOOLIZE} ]; then
	echo "Can't find libtoolize"
	exit -1
fi

${LIBTOOLIZE} --force
if test $? != 0 ; then
	echo "libtoolize failed."
	exit -1
fi

aclocal
if test $? != 0 ; then
	echo "aclocal failed."
	exit -1
fi

#autoheader --force
#if test $? != 0 ; then
#	echo "autoheader failed."
#	exit -1
#fi

autoconf --force
if test $? != 0 ; then
	echo "autoconf failed."
	exit -1
fi

automake --add-missing 2> /dev/null | true

autopoint

cd "${builddir}"

test -n "$NOCONFIGURE" || "$srcdir/configure" --srcdir=${srcdir} $@




