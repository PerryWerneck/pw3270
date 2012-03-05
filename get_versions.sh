#!/bin/bash

REVISION=`date +%y%m%d%H%M`

if test -d ".svn" ; then

	SVN=`which svn 2> /dev/null`

	if test -x "$SVN" ; then

		TEMPFILE=$(mktemp)
		LANG="EN_US"
		$SVN info > $TEMPFILE 2> /dev/null
		REVISION=$(cat $TEMPFILE | grep "^Revision: " | cut -d" " -f2)
		REPOSITORY=$(cat $TEMPFILE | grep "^URL: " | cut -d" " -f2)

		rm -f $TEMPFILE
	fi


fi

echo "REVISION=$REVISION"
echo "REPOSITORY=$REPOSITORY"

