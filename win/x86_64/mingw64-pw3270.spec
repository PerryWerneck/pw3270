#
# spec file for package mingw64-pw3270
#
# Copyright (c) 2014 SUSE LINUX Products GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define __strip %{_mingw64_strip}
%define __objdump %{_mingw64_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw64_findrequires}
%define __find_provides %{_mingw64_findprovides}
%define __os_install_post %{_mingw64_debug_install_post} \
                          %{_mingw64_install_post}

#---[ Packaging ]-----------------------------------------------------------------------------------------------------

Name:           mingw64-pw3270
Version: 5.5.0
Release:        0
Summary:        IBM 3270 Terminal emulator for GTK
License:        GPL-2.0
Group:          System/X11/Terminals
Url:            http://www.softwarepublico.gov.br/dotlrn/clubs/pw3270
Source:         pw3270-%{version}.tar.xz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	inkscape
BuildRequires:	gettext-tools
BuildRequires:	glib2-devel
BuildRequires:	pkgconfig(gtk+-3.0)

BuildRequires:  desktop-file-utils
BuildRequires:	optipng
BuildRequires:	ImageMagick

BuildRequires:	mingw64-cross-binutils
BuildRequires:	mingw64-cross-gcc
BuildRequires:	mingw64-cross-gcc-c++
BuildRequires:	mingw64-cross-pkg-config
BuildRequires:	mingw64-filesystem
BuildRequires:	mingw64-libopenssl-devel
BuildRequires:	mingw64-zlib-devel
BuildRequires:	sed

BuildRequires:	mingw64(pkg:gtk+-win32-3.0)
BuildRequires:	mingw64(pkg:lib3270)
BuildRequires:	mingw64(pkg:libv3270)

%description
Open-source GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.
Based on the original x3270 code, pw3270 was originally created for Banco do Brasil, and is now used worldwide. 

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep

%setup -q -n pw3270-%{version}

NOCONFIGURE=1 ./autogen.sh
%{_mingw64_configure}

%build
make clean
make all
%{_mingw64_strip} --strip-all .bin/Release/*.exe

%install
%{make_install}
%_mingw64_find_lang pw3270 langfiles

%clean
rm -rf %{buildroot}

#---[ Files ]---------------------------------------------------------------------------------------------------------

%files -f langfiles
%defattr(-,root,root)
%doc AUTHORS LICENSE 
# %{_mingw64_mandir}/*/*

# Main application
%{_mingw64_bindir}/pw3270.*
%{_mingw64_datadir}/pw3270
%{_mingw64_datadir}/glib-2.0/schemas/*.xml

%changelog

