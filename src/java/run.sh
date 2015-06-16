#!/bin/bash
make -C ../.. Debug
make .bin/java/${1}.class

LD_LIBRARY_PATH=../../.bin/Debug/lib/ java -Djava.library.path=.bin/Debug -cp .bin/java/ ${1}

