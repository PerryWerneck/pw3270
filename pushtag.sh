#!/bin/bash
#
# https://help.github.com/articles/syncing-a-fork/
#
# https://help.github.com/articles/configuring-a-remote-for-a-fork/
#
# https://www.opentechguides.com/how-to/article/git/177/git-sync-repos.html
#
# Setup:
#
# git remote add github https://github.com/PerryWerneck/lib3270.git
#
#

if [ -z ${1} ]; then
	echo "Inform target tag"
	exit -1
fi

git push

git fetch origin
git checkout master
git merge origin/master

git tag -f ${1}
git push -f --tags

for repo in $(git remote -v | grep -v origin | grep "(push)" | awk '{print $1}')
do
	echo "Updating ${repo} ..."
	git push ${repo} -f --tags
done

