#
# spec file for package pw3270
#
# Copyright (c) 2024 SUSE LLC
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

# Please submit bugfixes or comments via https://github.com/PerryWerneck/pw3270/issues
#


%define _product %(pkg-config --variable=product_name lib3270)

%define plugindir %(pkg-config --variable=plugin_path lib3270)
%if "%{plugindir}" == ""
	%define plugindir %{_libdir}/pw3270-plugins
%endif

#---[ Packaging ]-----------------------------------------------------------------------------------------------------

Name:			pw3270
Version:		5.5.0
Release:		0
Summary:		IBM 3270 Terminal emulator for GTK
License:		LGPL-3.0-only
Group:			System/X11/Terminals
URL:			https://github.com/PerryWerneck/pw3270

Source:			%{name}-%{version}.tar.xz

BuildRoot:		%{_tmppath}/%{name}-%{version}-build

Requires:		%{name}-branding
Requires:		shared-mime-info

Recommends:		libv3270-config

#--[ Setup by distribution ]------------------------------------------------------------------------------------------
#
# References:
#
# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto#Detect_a_distribution_flavor_for_special_code
#

#--[ CentOS ]---------------------------------------------------------------------------------------------------------

%if 0%{?centos_version}

BuildRequires:	glib2-devel
BuildRequires:	gtk3-devel
BuildRequires:	libv3270-devel >= 5.4

%endif

#--[ Fedora ]---------------------------------------------------------------------------------------------------------

%if 0%{?fedora}

BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(gtk+-3.0)
BuildRequires:	pkgconfig(libv3270) >= 5.4

%endif

#--[ SuSE ]-----------------------------------------------------------------------------------------------------------

%if 0%{?suse_version}

BuildRequires:	appstream-glib
BuildRequires:	update-desktop-files
BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(gtk+-3.0)
BuildRequires:	pkgconfig(libv3270) >= 5.4

%glib2_gsettings_schema_requires

%endif

#---------------------------------------------------------------------------------------------------------------------

BuildRequires:	desktop-file-utils
BuildRequires:	fdupes
BuildRequires:	gcc-c++
BuildRequires:	gettext-devel
BuildRequires:	libtool
BuildRequires:	m4
BuildRequires:	pkgconfig
BuildRequires:	scour

%if 0%{?suse_version} == 01500
BuildRequires:	meson >= 0.61.4
%else
BuildRequires:	meson
%endif

%description
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

Based on the original x3270 code, pw3270 was originally created for Banco do Brasil, and is now used worldwide.


#--[ Configuration & Branding ]---------------------------------------------------------------------------------------

%package branding-upstream
Summary:		Upstream branding for %{name}
Group:			System/X11/Terminals

Requires:		%{name} = %{version}
BuildArch:		noarch

Provides:		%{name}-branding
Conflicts:		otherproviders(%{name}-branding)

Requires(post):		desktop-file-utils
Requires(postun):	desktop-file-utils

%description branding-upstream
GTK-based IBM 3270 terminal emulator with many advanced features. It can be used to communicate with any IBM host that supports 3270-style connections over TELNET.

This package contains the upstream branding for %{name}.

%lang_package -n %{name}

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%autosetup
%meson

%build
%meson_build

%install
%meson_install

%find_lang pw3270 langfiles

%if 0%{?suse_version}
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.metainfo.xml
%endif

%fdupes %{buildroot}/%{_prefix}

%files
%defattr(-,root,root)
%license LICENSE
%doc AUTHORS README.md

# Main application

%{_bindir}/%{_product}

# Desktop files
%{_datadir}/applications/*.desktop
%{_datadir}/metainfo/*.metainfo.xml

# Configuration & Themes
%{_datadir}/glib-2.0/schemas/*.xml
%{_datadir}/mime/packages/*.xml

# Customized icons
%dir %{_datadir}/%{_product}/icons
%{_datadir}/%{_product}/icons/*.svg

%files branding-upstream
%defattr(-,root,root)
%dir %{_datadir}/%{_product}
%{_datadir}/%{_product}/*.ui.xml
%{_datadir}/%{_product}/*.svg


# Icons
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%{_datadir}/icons/hicolor/symbolic/apps/*.svg


%files -n %{name}-lang -f langfiles

%posttrans
/usr/bin/update-desktop-database

%postun
/usr/bin/update-desktop-database

%changelog
