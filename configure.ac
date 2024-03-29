dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl
dnl Copyright (C) 2008 Banco do Brasil S.A.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.
dnl

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([pw3270], [5.5], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Recommended for gtk-doc (https://developer.gnome.org/gtk-doc-manual/stable/settingup_autoconf.html.en)
AC_CONFIG_MACRO_DIR(m4)

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

LT_INIT

dnl Set gettext version
AM_GNU_GETTEXT_VERSION([0.14])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C])

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

case "$host" in
	*-mingw32|*-pc-msys)
		app_cv_osname="windows"
		app_rls_ldflags="-mwindows"

		CFLAGS="$CFLAGS -pthread -D_WIN32_WINNT=0x0600"
		LDFLAGS="$LDFLAGS -pthread"
		LIBS="$LIBS -lws2_32 -lwtsapi32 -lcomdlg32 -lole32 -luuid"

		app_win32_revision=$(date +%-y.%-m.%-d.%-H)
		AC_SUBST(WIN32_VERSION,$app_win32_revision)

		app_win32_file_version=$(date +%-y,%-m,%-d,%-H)
		AC_SUBST(WIN32_FILE_VERSION,$app_win32_file_version)

		AC_CONFIG_FILES(src/main/windows/resources.rc)
		AC_CONFIG_FILES(src/main/windows/app-manifest.xml)

		AC_CONFIG_FILES(win/pw3270.nsi)

		if test "$host_cpu" = "x86_64"; then
			app_cv_programfiles="PROGRAMFILES64"
			app_cv_winarch="64"
			app_cv_nsisredir="DisableX64FSRedirection"
		else
			app_cv_programfiles="PROGRAMFILES"
			app_cv_winarch="32"
			app_cv_nsisredir="EnableX64FSRedirection"
		fi

		AC_SUBST(PROGRAMFILES,$app_cv_programfiles)
		AC_SUBST(WINARCH,$app_cv_winarch)
		AC_SUBST(NSISREDIR,$app_cv_nsisredir)

		# Windows and unix doesn't use the same defaults.
		AC_SUBST(DEFAULT_UI_STYLE,0)
		AC_SUBST(DEFAULT_MENUBAR_VISIBLE,"true")
		;;

	*-apple-darwin*)
		app_cv_osname="macos"
		app_rls_ldflags=""

		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"

		# Windows and unix doesn't use the same defaults.
		AC_SUBST(DEFAULT_UI_STYLE,1)
		AC_SUBST(DEFAULT_MENUBAR_VISIBLE,"false")
		;;

 	*)
		app_cv_osname="linux"
		app_rls_ldflags=""

		CFLAGS="$CFLAGS -pthread -DLIBDIR=\$(libdir) -DDATADIR=\$(datadir)"
		LDFLAGS="$LDFLAGS -pthread"

		# Windows and unix doesn't use the same defaults.
		AC_SUBST(DEFAULT_UI_STYLE,1)
		AC_SUBST(DEFAULT_MENUBAR_VISIBLE,"false")

esac

DATE_FMT="+%Y-%m-%d"
SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(date +%s)}"
app_release_date=$(date -u -d "@$SOURCE_DATE_EPOCH" "$DATE_FMT" 2>/dev/null || date -u -r "$SOURCE_DATE_EPOCH" "$DATE_FMT" 2>/dev/null || date -u "$DATE_FMT")
AC_SUBST(RELEASE_DATE,$app_release_date)
AC_SUBST(OSNAME,$app_cv_osname)
AC_SUBST(RLS_LDFLAGS,$app_rls_ldflags)
AC_SUBST(LIBS)
AC_SUBST(LOGDIR)
AC_SUBST(DLLEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLL_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for headers
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))
AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R, [], [Do we have localtime_r?]) )
AC_CHECK_FUNC(strtok_r, AC_DEFINE(HAVE_STRTOK_R, [], [Do we have strtok_r?]) )

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([CONVERT], [convert], [no])
AC_PATH_TOOL([ZIP],[zip],[no])
AC_PATH_TOOL([DLLTOOL],[dlltool],[no])
AC_PATH_TOOL([WINDRES], [windres], [no])

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])

AC_PATH_TOOL([SCOUR], [scour], [no])
AC_PATH_TOOL([APPSTREAMCLI],[appstreamcli], [no])
AC_PATH_TOOL([STRIP], [strip], [true])

AC_PATH_TOOL([DESKTOP_INSTALL],[desktop-file-install],[no])
AC_PATH_TOOL([GLIB_SCHEMA_COMPILER],[glib-compile-schemas],[false])

AC_PATH_TOOL([OPTIPNG], [optipng], [true])

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"IBM 3270 Terminal emulator")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major, [The package major version])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor, [The package minor version])

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%-y`.`date +%-m`.`date +%-d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "$app_cv_release",[The package release])
AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major,[The package major release])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor,[The package minor release])

AC_SUBST(PACKAGE_RELEASE,$app_cv_release)
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

dnl ---------------------------------------------------------------------------
dnl Set application id & product name
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=`pkg-config --variable=product_name lib3270` ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, $app_cv_product,[The product name])
AC_SUBST(PRODUCT_NAME,$app_cv_product)

AC_ARG_WITH([product-id], [AS_HELP_STRING([--product-id], [Set the product ID])], [ app_cv_product_id="$withval" ],[ app_cv_product_id=`$PKG_CONFIG --variable=product_id lib3270` ])
AC_DEFINE_UNQUOTED(PRODUCT_ID, $app_cv_product_id,[The product ID])
AC_SUBST(PRODUCT_ID,$app_cv_product_id)

app_cv_object_path="/"`echo $app_cv_product_id | sed -e "s@\.@/@g"`"/"
AC_DEFINE_UNQUOTED(PRODUCT_PATH, "$app_cv_object_path",[The schema path])
AC_SUBST(PRODUCT_PATH,$app_cv_object_path)

AC_ARG_WITH([application-id], [AS_HELP_STRING([--application-id], [Set the application ID])], [ app_cv_application_id="$withval" ],[ app_cv_application_id=$app_cv_product_id.terminal ])
AC_DEFINE_UNQUOTED(APPLICATION_ID, $app_cv_application_id,[The application ID])
AC_SUBST(APPLICATION_ID,$app_cv_application_id)

AC_ARG_WITH([plugin-dir], [AS_HELP_STRING([--with-plugin-dir], [The application plugin dir])], [ app_cv_plugin_path="$withval" ],[ app_cv_plugin_path=`pkg-config --variable=plugin_path lib3270` ])
AC_DEFINE_UNQUOTED(PLUGIN_DIR, "$app_cv_plugin_path", [The product name])
AC_SUBST(PLUGIN_DIR,$app_cv_plugin_path)

dnl ---------------------------------------------------------------------------
dnl Check for unstable version
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([unstable],
	[AS_HELP_STRING([--enable-unstable], [enable unstable features])],
[
	app_cv_unstable="$enableval"
],[

	app_cv_unstable="no"

])

if test "$app_cv_unstable" == "yes"; then
	AC_DEFINE(ENABLE_UNSTABLE_FEATURES,,[Are we using the unstable features?])
	AC_MSG_NOTICE([Enabling unstable features])
else
	AC_MSG_NOTICE([Using only stable features])
fi


dnl ---------------------------------------------------------------------------
dnl Check for LIBV3270
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIBV3270], [libv3270], AC_DEFINE(HAVE_LIBV3270,,[Do we have libv3270?]), AC_MSG_ERROR([LIBV3270 not present.]))

AC_SUBST(LIBV3270_LIBS)
AC_SUBST(LIBV3270_CFLAGS)

AC_ARG_WITH([library-name], [AS_HELP_STRING([--with-library-name], [Set protocol library name])], [ app_cv_library="$withval" ],[ app_cv_library=`pkg-config --variable=library_name lib3270` ])
AC_DEFINE_UNQUOTED(LIBRARY_NAME, $app_cv_library,[The tn3270 library name])
AC_SUBST(LIBRARY_NAME,$app_cv_library)

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

GLIB_GSETTINGS

PKG_CHECK_MODULES( [GTK], [gtk+-3.0 glib-2.0 gmodule-2.0], AC_DEFINE(HAVE_GTK,1,[Do we have gtk?]), AC_MSG_ERROR([GTK not present.]))

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_MODVERSION,`$PKG_CONFIG --modversion gtk+-3.0`)

dnl ---------------------------------------------------------------------------
dnl Check for pie
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([pie],
	[AS_HELP_STRING([--disable-pie], [disable support for position independent executable])],
[
	app_cv_pie="$enableval"
],[

	if test "$app_cv_osname" == "linux"; then
		app_cv_pie="yes"
	else
		app_cv_pie="no"
	fi
	
])

if test "$app_cv_pie" == "yes"; then
	CFLAGS="$CFLAGS -fpie"
	LDFLAGS="$LDFLAGS -pie"
	AC_MSG_NOTICE([Using position independent executable])
else
	AC_MSG_NOTICE([Not using position independent executable])
fi

dnl ---------------------------------------------------------------------------
dnl Directory config
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([build-dir], [AS_HELP_STRING([--with-build-dir], [Setup build path])], [ app_cv_buildir="$withval" ],[ app_cv_buildir="$ac_pwd" ])
AC_SUBST(BUILDDIR,$app_cv_buildir)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)

AC_CONFIG_FILES(schemas/application.gschema.xml)
AC_CONFIG_FILES(schemas/window.gschema.xml)
AC_CONFIG_FILES(schemas/Makefile)

AC_CONFIG_FILES(locale/Makefile)

AC_CONFIG_FILES(branding/Makefile)
AC_CONFIG_FILES(branding/launcher.desktop)
AC_CONFIG_FILES(branding/mime.xml)
AC_CONFIG_FILES(branding/metainfo.xml)
dnl AC_CONFIG_FILES(branding/keypad.metainfo.xml)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT


