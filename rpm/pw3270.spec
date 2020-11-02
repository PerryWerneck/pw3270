#
# spec file for package pw3270
#
# Copyright (c) 2019 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (c) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#

#---[ Versions ]------------------------------------------------------------------------------------------------------

%define _product %(pkg-config --variable=product_name lib3270)

#---[ Packaging ]-----------------------------------------------------------------------------------------------------

Name:			pw3270
Version:		5.3
Release:		0
Summary:		IBM 3270 Terminal emulator for GTK
License:		GPL-2.0
Group:			System/X11/Terminals
Url:			https://github.com/PerryWerneck/pw3270

Source:		pw3270-%{version}.tar.xz

BuildRoot:		%{_tmppath}/%{name}-%{version}-build

Requires:		shared-mime-info
Requires:		%{name}-branding = %{version}

BuildRequires:	update-desktop-files

%glib2_gsettings_schema_requires

#--[ Setup by distribution ]------------------------------------------------------------------------------------------
# 
# References:
#
# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto#Detect_a_distribution_flavor_for_special_code
#

#--[ Red HAT ]--------------------------------------------------------------------------------------------------------

%if 0%{?rhel_version}

BuildRequires:  gtk3-devel
BuildRequires:  glib2-devel
BuildRequires:  librsvg2-tools
BuildRequires:  libv3270-devel >= 5.3

%endif

#--[ CentOS ]---------------------------------------------------------------------------------------------------------

%if 0%{?centos_version}

BuildRequires:  gtk3-devel
BuildRequires:  glib2-devel
BuildRequires:  libv3270-devel >= 5.3

# Required for genmarshal
BuildRequires:  python

%endif

#--[ Fedora ]---------------------------------------------------------------------------------------------------------

%if 0%{?fedora}

BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libv3270) >= 5.3

%endif

#--[ SuSE ]-----------------------------------------------------------------------------------------------------------

%if 0%{?suse_version}

BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libv3270) >= 5.3

%endif

#---------------------------------------------------------------------------------------------------------------------

BuildRequires:  autoconf >= 2.61
BuildRequires:  automake
BuildRequires:  binutils
BuildRequires:  coreutils
BuildRequires:  desktop-file-utils
BuildRequires:  findutils
BuildRequires:  gcc-c++
BuildRequires:  gettext-devel
BuildRequires:  gettext-tools
BuildRequires:  m4
BuildRequires:  pkgconfig
BuildRequires:  sed
BuildRequires:  optipng
BuildRequires:  fdupes
BuildRequires:  ImageMagick
BuildRequires:  autoconf-archive
%glib2_gsettings_schema_requires

%description
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

Based on the original x3270 code, pw3270 was originally created for Banco do Brasil, and is now used worldwide.

#--[ Configuration & Branding ]---------------------------------------------------------------------------------------

%package branding
Summary:        Default branding for %{name}
Group:          System/X11/Terminals

Requires:       	%{name} = %{version}
Requires(post):	desktop-file-utils
Requires(postun):	desktop-file-utils

%description branding
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the default branding for %{name}.

%package keypads
Summary:	Keypads for %{name}
Group:		System/X11/Terminals
Requires:	%{name} = %{version}
BuildArch:	noarch

Conflicts:	otherproviders(pw3270-keypads)
Enhances:	%{name}

%description keypads
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the keypads for %{name}.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

%global _lto_cflags %{_lto_cflags} -ffat-lto-objects
NOCONFIGURE=1 ./autogen.sh

%configure --with-release=%{release} CFLAGS="${CFLAGS} -fpie" LDFLAGS="${LDFLAGS} -pie"

%build
make %{?_smp_mflags} clean

# parallel build is broken
make all -j1

%install
%make_install

%find_lang pw3270 langfiles

%fdupes %{buildroot}/%{_prefix}

%files -f langfiles
%defattr(-,root,root)
%license LICENSE
%doc AUTHORS README.md

# Main application
%dir %{_datadir}/%{_product}
%dir %{_datadir}/%{_product}/ui
%dir %{_datadir}/%{_product}/keypad
%dir %{_libdir}/%{_product}-plugins
%dir %{_datadir}/%{_product}/icons

%{_bindir}/%{_product}

# Configuration & Themes
%{_datadir}/glib-2.0/schemas/*.xml
%{_datadir}/%{_product}/icons/gtk-*.svg

%{_datadir}/pixmaps/*.png

%{_datadir}/applications/*.desktop
%{_datadir}/appdata/%{_product}.appdata.xml

%{_datadir}/mime/packages/*.xml

%files branding
%defattr(-,root,root)

%{_datadir}/%{_product}/ui/*
%{_datadir}/%{_product}/*.png
%{_datadir}/%{_product}/icons/%{_product}.svg

%files keypads
%{_datadir}/%{_product}/keypad/*

%posttrans
/usr/bin/update-desktop-database

%postun
/usr/bin/update-desktop-database

%changelog
