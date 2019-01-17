#!/bin/bash
#
# https://help.github.com/articles/syncing-a-fork/
#
# https://help.github.com/articles/configuring-a-remote-for-a-fork/
#
# https://www.opentechguides.com/how-to/article/git/177/git-sync-repos.html
#
# git remote add bitbucket https://bitbucket.org/pw3270/pw3270-application.git
# git remote add github	https://github.com/PerryWerneck/pw3270.git
#

# git remote add github https://github.com/PerryWerneck/pw3270.git
# git push github --all
#
# SUBTREES: 
#
# https://www.atlassian.com/blog/git/alternatives-to-git-submodule-git-subtree
# 
#

# git fetch lib3270

git push

git fetch origin
git checkout master
git merge origin/master

git push github
git push bitbucket


