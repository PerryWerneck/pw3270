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

git push

git fetch origin
git merge

REPOS=$(git remote -v | grep -v origin | grep "(push)" | awk '{print $1}')

for repo in ${REPOS}
do
	echo "Getting updates from ${repo} ..."
	git fetch ${repo}
	git merge
done

for repo in ${REPOS}
do
	echo "Updating ${repo} ..."
	git push ${repo}
done

