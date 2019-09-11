#
# spec file for package pw3270
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
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

#---[ Versions ]------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION 5
%define MINOR_VERSION 2

%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

#---[ Selected modules ]----------------------------------------------------------------------------------------------

%define _dbus     	1
%define _help2man  	1

#---[ Packaging ]-----------------------------------------------------------------------------------------------------

Name:			pw3270
Version:		5.2
Release:		0
Summary:		IBM 3270 Terminal emulator for GTK
License:		GPL-2.0
Group:			System/X11/Terminals
Url:			https://portal.softwarepublico.gov.br/social/pw3270/

Source:			pw3270-%{version}.tar.xz

BuildRoot:		%{_tmppath}/%{name}-%{version}-build

Requires:		shared-mime-info
Requires:		%{name}-branding >= 5.2

#--[ Setup by distribution ]------------------------------------------------------------------------------------------
# 
# References:
#
# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto#Detect_a_distribution_flavor_for_special_code
#

#--[ Red HAT ]--------------------------------------------------------------------------------------------------------

%if 0%{?rhel_version}

%define _help2man 0

BuildRequires:	dbus-devel
BuildRequires:	dbus-glib-devel
BuildRequires:	openssl-devel
BuildRequires:	gtk3-devel
BuildRequires:	librsvg2-tools
BuildRequires:	lib3270-devel
BuildRequires:	libv3270-devel

%endif

#--[ CentOS ]---------------------------------------------------------------------------------------------------------

%if 0%{?centos_version}

%define _help2man  	0

BuildRequires:	dbus-devel
BuildRequires:	dbus-glib-devel
BuildRequires:	openssl-devel
BuildRequires:	gtk3-devel
BuildRequires:	librsvg2-tools
BuildRequires:	lib3270-devel
BuildRequires:	libv3270-devel

# CENTOS Genmarshal doesn't depends on python!
BuildRequires:	python

%endif

#--[ Fedora ]---------------------------------------------------------------------------------------------------------

%if 0%{?fedora}

BuildRequires:	pkgconfig(dbus-1)
BuildRequires:	pkgconfig(dbus-glib-1)
BuildRequires:	pkgconfig(openssl)
BuildRequires:	pkgconfig(gtk+-3.0)
BuildRequires:	pkgconfig(lib3270)
BuildRequires:	pkgconfig(libv3270)
BuildRequires:	librsvg2-tools
BuildRequires:	autoconf-archive

%endif

#--[ SuSE ]-----------------------------------------------------------------------------------------------------------

%if 0%{?suse_version}

BuildRequires:	pkgconfig(openssl)
BuildRequires:	pkgconfig(dbus-1)
BuildRequires:	pkgconfig(dbus-glib-1)
BuildRequires:	pkgconfig(gtk+-3.0)
BuildRequires:	pkgconfig(lib3270)
BuildRequires:	pkgconfig(libv3270)
BuildRequires:	rsvg-view

# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto
%if 0%{suse_version} > 120100
BuildRequires:	autoconf-archive
BuildRequires:	update-desktop-files
%endif

%endif

#---------------------------------------------------------------------------------------------------------------------

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	desktop-file-utils
BuildRequires:	findutils
BuildRequires:	gcc-c++
BuildRequires:	gettext-devel
BuildRequires:	m4
BuildRequires:	pkgconfig
BuildRequires:	sed
BuildRequires:	optipng
BuildRequires:	fdupes
BuildRequires:	ImageMagick

%if 0%{?_help2man}
BuildRequires:	help2man
%endif

%description
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

Based on the original x3270 code, pw3270 was originally created for Banco do Brasil, and is now used worldwide.

#--[ Application library ]--------------------------------------------------------------------------------------------

%package -n libpw3270-%{_libvrs}
Summary:	PW3270 API for plugins
Group:		System/Libraries

%description -n libpw3270-%{_libvrs}
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the plugin support library.

#--[ Configuration & Branding ]---------------------------------------------------------------------------------------

%package branding
Summary:	Configuration and branding for %{name}
Group:		System/X11/Terminals
Requires:	%{name} = %{version}

Provides:	%{name}-config = %{version}
Conflicts:	otherproviders(%{name}-config)


%description branding
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the default configuration and branding for %{name}.

#--[ Devel ]----------------------------------------------------------------------------------------------------------

%package devel
Summary:	Files required for development of %{name} plugins
Group:		Development/Libraries/C and C++

Requires:	pkgconfig(lib3270)
Requires:	pkgconfig(libv3270)
Requires:	pkgconfig(gtk+-3.0)
Requires:	%{name} = %{version}
Requires:	libpw3270-%{_libvrs}

%description -n %{name}-devel
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the development files for %{name}.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep

%setup -q -n pw3270-%{version}

NOCONFIGURE=1 ./autogen.sh

%configure \
	--with-release=%{release}

%build
make clean

# parallel build is broken
make all -j1

%install
%make_install

# Remove static library
rm -f %{buildroot}/%{_libdir}/*.a

%find_lang pw3270 langfiles

%fdupes %{buildroot}/%{_prefix}

#---[ Files ]---------------------------------------------------------------------------------------------------------

%files -f langfiles
%defattr(-,root,root)
%doc AUTHORS LICENSE README.md
%if 0%{?_help2man}
%{_mandir}/*/*
%endif

# Main application
%dir %{_datadir}/pw3270
%dir %{_datadir}/pw3270/ui
%dir %{_libdir}/pw3270-plugins

%{_bindir}/pw3270

%files -n libpw3270-%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libpw3270.so.%{MAJOR_VERSION}.%{MINOR_VERSION}
%{_libdir}/libpw3270.so.%{MAJOR_VERSION}

%files branding
%defattr(-,root,root)

%{_datadir}/applications/pw3270.desktop
%{_datadir}/pw3270/ui/00default.xml
%{_datadir}/pw3270/ui/10functions.xml
%{_datadir}/pw3270/ui/10keypad.xml
%{_datadir}/pw3270/pw3270.png
%{_datadir}/pw3270/pw3270-logo.png
%{_datadir}/pixmaps/pw3270.png

%files devel

%{_includedir}/pw3270.h
%{_includedir}/pw3270cpp.h
%{_includedir}/pw3270

%{_libdir}/libpw3270.so

%{_libdir}/pkgconfig/pw3270.pc
%{_datadir}/pw3270/locale

%{_datadir}/pw3270/ui/98trace.xml
%{_datadir}/pw3270/ui/99debug.xml

#---[ Scripts ]-------------------------------------------------------------------------------------------------------

%post   -n libpw3270-%{_libvrs} -p /sbin/ldconfig
%postun -n libpw3270-%{_libvrs} -p /sbin/ldconfig

%changelog

