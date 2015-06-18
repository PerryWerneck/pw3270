#!/bin/bash
make -C ../.. Debug
make Debug
make .bin/java/${1}.class

LD_LIBRARY_PATH=../../.bin/Debug/lib/ java -Djava.library.path=/usr/local/lib64/java:.bin/Debug/lib -cp .bin/java/ ${1}

