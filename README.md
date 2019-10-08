
GTK Based 3270 terminal emulator
================================

pw3270 is a modern, GTK-based, completely free tn3270 emulator. 

Created originally for Banco do Brasil, it's now an official Brazilian Government Public Software project, and is used worldwide. 

See more details at https://softwarepublico.gov.br/social/pw3270/

Installation repositories
=========================

The latest version packaged for many linux distributions can be found in SuSE Build Service (https://build.opensuse.org/project/show/home:PerryWerneck:pw3270)

Requirements
============

GTK-3
	https://www.gtk.org/


Building for Linux
==================



Building for Windows
===========================

Cross-compiling on SuSE Linux (Native or WSL) - The easier way!
---------------------------------------------------------------

1. First add the MinGW Repositories for your SuSE version from:

	* https://build.opensuse.org/project/show/windows:mingw:win32
	* https://build.opensuse.org/project/show/windows:mingw:win64

2. Run the build script

	* get it from https://github.com/PerryWerneck/pw3270/blob/master/win/pack.sh
	* Run the build script: ./pack.sh --pre-reqs



