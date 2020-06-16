
## GTK Based 3270 terminal emulator

pw3270 is a modern, GTK-based, completely free tn3270 emulator. 

Created originally for Banco do Brasil, it's now an official Brazilian Government Public Software project, and is used worldwide. 

See more details at https://softwarepublico.gov.br/social/pw3270/

## Installation

### Linux

For the supported distributions get the install repositories and instructions from https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270

### Windows


## Requirements

	* GTK+ 3.20 or later (https://www.gtk.org/)
	* libv3270 (../../../libv3270)

## Building for Linux

	```
	git clone https://github.com/PerryWerneck/pw3270.git
	cd pw3270
	./autogen.sh
	make all
	sudo make install
	```

## Building for Windows

### Cross-compiling on SuSE Linux (Native or WSL) - The easier way!

1. Add the MinGW Repositories

	```shell
	sudo zypper ar obs://windows:mingw:win32 mingw32
	sudo zypper ar obs://windows:mingw:win64 mingw64
	sudo zypper ref
	```

2. Run the build script

	```shell
	wget https://github.com/PerryWerneck/pw3270/blob/master/win/pack.sh
	./pack.sh --pre-reqs
	```

## Building for macOS (using homebrew)

1. Build and install [libv3270](../../../libv3270)

2. Install additional dependencies

	```shell
	$ brew install adwaita-icon-theme imagemagick
	```

3. Configure and build

	```shell
	$ ./autogen.sh
	$ make all
	````

4. Create app bundle

	```shell
	$ cd macos
	$ ./bundle
	````

