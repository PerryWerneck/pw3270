#!/bin/bash
G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full --suppressions=src/gtk/valgrind.suppression --gen-suppressions=all .bin/Debug/pw3270

