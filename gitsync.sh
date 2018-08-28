#!/bin/bash
#
# https://help.github.com/articles/syncing-a-fork/
#
# https://help.github.com/articles/configuring-a-remote-for-a-fork/
#
# git remote add bitbucket https://bitbucket.org/pw3270/pw3270-application.git
#

git fetch origin
git checkout master
git merge origin/master

git push bitbucket

