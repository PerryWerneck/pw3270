#!/bin/bash
G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full --suppressions=../valgrind.suppression --gen-suppressions=all .bin/Debug/uiparser

