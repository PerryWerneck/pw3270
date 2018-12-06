
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

Cross-compiling on SuSE Linux (Native or WSL)
---------------------------------------------

1. First add the MinGW Repositories for your SuSE version from:

	* 32 bits: https://build.opensuse.org/project/show/windows:mingw:win32
	* 64 bits: https://build.opensuse.org/project/show/windows:mingw:win64

2. Get pw3270 sources from git

	* git clone http://softwarepublico.gov.br/gitlab/pw3270/principal.git ./pw3270

3. Install cross compilers

	* ./pw3270/win/install-cross.sh --all

4. Build pw3270 windows installers

	* cd pw3270/ && ./win/pack.sh


