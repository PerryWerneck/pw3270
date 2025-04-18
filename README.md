## GTK Based 3270 terminal emulator

<img src="https://raw.githubusercontent.com/PerryWerneck/PerryWerneck/master/screenshots/mvs-tk4.png" alt="Screenshot">

pw3270 is a modern, GTK-based, completely free tn3270 emulator. 

Created originally for Banco do Brasil, it's now an official Brazilian Government Public Software project, and is used worldwide. 

<!-- https://github.com/igrigorik/ga-beacon -->

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![CodeQL](https://github.com/PerryWerneck/pw3270/workflows/CodeQL/badge.svg?branch=master)
[![build result](https://build.opensuse.org/projects/home:PerryWerneck:pw3270/packages/pw3270/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:PerryWerneck:pw3270/pw3270)

## Installation

You can download installation package for supported linux distributions in [Open Build Service](https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270), the flatpak version from flathub and windows installer from [Releases](../../releases).

[<img src="https://raw.githubusercontent.com/PerryWerneck/pw3270/develop/branding/obs-badge-en.svg" alt="Download from open build service" height="80px">](https://software.opensuse.org/download.html?project=home%3APerryWerneck%3Apw3270&package=pw3270)
[<img src="https://flathub.org/assets/badges/flathub-badge-en.svg" alt="Download from flathub" height="80px">](https://flathub.org/apps/details/br.app.pw3270.terminal)
[<img src="https://github.com/PerryWerneck/pw3270/blob/develop/branding/release-badge-en.svg" alt="Download from github releases" height="80px">](../../releases)

Alternative windows installers for stable and unstable versions are already available on Dropbox and one drive.

[<img src="https://raw.githubusercontent.com/PerryWerneck/pw3270/develop/branding/dropbox-badge-en.svg" alt="Get from dropbox" height="80px">](https://www.dropbox.com/sh/2qy3s6b5s4o4bws/AAAubHE8SBG7r6CJSKPflKN0a?dl=0)
[<img src="https://raw.githubusercontent.com/PerryWerneck/pw3270/develop/branding/onedrive-badge-en.svg" alt="Get from Microsoft One Drive" height="80px">](https://onedrive.live.com/?id=D8B46DA0372A6F1A%212208&cid=D8B46DA0372A6F1A)

## Building for Linux

	```shell
	git clone https://github.com/PerryWerneck/pw3270.git
	cd pw3270
	./autogen.sh
	make all
	sudo make install
	```

## Building for Windows

### Cross-compiling on SuSE Linux (Native or WSL)

1. Get pw3270 sources

	```shell
	git clone https://github.com/PerryWerneck/pw3270.git
	```

2. Add obs repositories

	```shell
	./win/bundle.cross --install-repositories
	```

3. Install required packages

	```shell
	./win/bundle.cross --install-requires
	```

4. Build NSI installer

	```shell
	./win/bundle.cross --nsi
	```

### Windows native with MSYS2

1. Build and install [libv3270](../../../libv3270)

2. Install required packages

	```shell
	pacman -S mingw-w64-x86_64-imagemagick mingw-w64-x86_64-optipng mingw-w64-x86_64-inkscape
	```

2. Get pw3270 sources from git

	```
	git clone https://github.com/PerryWerneck/pw3270.git ./pw3270
	```

3. Build library using the mingw shell

	```
	cd pw3270
	./autogen.sh
	make all
	```

## Building for macOS 

### Using homebrew

1. Build and install [libv3270](../../../libv3270)

2. Install additional dependencies

	```shell
	$ brew install adwaita-icon-theme imagemagick scour
	```

3. Get pw3270 sources from git

	```shell
	git clone https://github.com/PerryWerneck/pw3270.git ./pw3270
	cd pw3270
	```

4. Configure and build

	```shell
	meson setup --prefix=$(brew --prefix)/Cellar/pw3270/$(grep 'version:' meson.build | cut -d: -f2 | cut -d\' -f2) --reconfigure --wipe .build
	meson compile -C .build
	meson install -C .build
	brew link pw3270
	````

5. Create app bundle

	```shell
	./mac/bundle
	````
	
### Using jhbuild

1. Install jhbuild and GTK-OSX

	https://wiki.gnome.org/Projects/GTK/OSX/Building
	
2. build

	```shell
	jhbuild --moduleset=https://raw.githubusercontent.com/PerryWerneck/pw3270/master/mac/pw3270.modules build pw3270
	```


