#!/bin/bash
valgrind \
	--leak-check=full \
	--track-origins=yes \
	--gen-suppressions=all \
	--suppressions=valgrind.suppression \
	.bin/Debug/PW3270\ Actions

