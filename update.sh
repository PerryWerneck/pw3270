#!/bin/bash

git pull
git fetch origin

git submodule update --remote --recursive

