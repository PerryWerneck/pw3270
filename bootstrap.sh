#!/bin/bash

if test -z $1; then
	out=`dirname $0`
else
	out="$1"
fi

if test -e revision ; then
	. revision
fi

PACKAGE_REVISION=`date +%y%m%d`

if test -d ".svn" ; then

	SVN=`which svn 2> /dev/null`

	if test -x "$SVN" ; then

		TEMPFILE=.bootstrap.tmp
		LANG="EN_US"
		"$SVN" info > $TEMPFILE 2>&1

		if [ "$?" == "0" ]; then
			PACKAGE_REVISION=$(cat $TEMPFILE | grep "^Revision: " | cut -d" " -f2)
			PACKAGE_SOURCE=$(cat $TEMPFILE | grep "^URL: " | cut -d" " -f2)
		fi

		rm -f $TEMPFILE
	fi


fi

echo "PACKAGE_REVISION=$PACKAGE_REVISION" > $out/revision
echo "PACKAGE_SOURCE=$PACKAGE_SOURCE" >> $out/revision

echo "m4_define([SVN_REVISION], $PACKAGE_REVISION)" > $out/revision.m4
echo "m4_define([SVN_URL], $PACKAGE_SOURCE)" >> $out/revision.m4

