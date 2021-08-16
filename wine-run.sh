#!/bin/bash

make Debug
make gschemas.compiled

cp gschemas.compiled .bin/Debug

wine .bin/Debug/pw3270.exe

