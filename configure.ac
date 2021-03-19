dnl
dnl Software pw3270, desenvolvido com base nos códigos fontes do  C3270  e  X3270
dnl (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
dnl aplicativos mainframe. Registro no INPI sob o nome G3270.
dnl
dnl Copyright (C) <2008> <Banco do Brasil S.A.>
dnl
dnl Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
dnl os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
dnl Free Software Foundation.
dnl
dnl Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
dnl GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
dnl A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
dnl obter mais detalhes.
dnl
dnl Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
dnl programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple

dnl Place, Suite 330, Boston, MA, 02111-1307, USA
dnl
dnl Contatos:
dnl
dnl perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
dnl erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
dnl

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([pw3270], [5.4], [perry.werneck@gmail.com])

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

		# Windows and linux doesn't use the same defaults.
		AC_CONFIG_FILES(schemas/windows/application.gschema.xml)
		AC_CONFIG_FILES(schemas/windows/window.gschema.xml)
		AC_CONFIG_FILES(win/makeruntime.sh)
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

		;;

 	*)
		app_cv_osname="linux"
		app_rls_ldflags=""

		CFLAGS="$CFLAGS -pthread -DLIBDIR=\$(libdir)"
		LDFLAGS="$LDFLAGS -pthread"

		app_cv_static='no'

		# Windows and linux doesn't use the same window defaults.
		AC_CONFIG_FILES(schemas/linux/application.gschema.xml)
		AC_CONFIG_FILES(schemas/linux/window.gschema.xml)

esac

app_release_date=$(date +%Y-%m-%d)
AC_SUBST(RELEASE_DATE,$app_release_date)
AC_SUBST(OSNAME,$app_cv_osname)
AC_SUBST(RLS_LDFLAGS,$app_rls_ldflags)
AC_SUBST(LIBS)
AC_SUBST(LOGDIR)
AC_SUBST(DLLEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLL_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([CONVERT], [convert], [no])
dnl AC_PATH_TOOL([OPTIPNG],[optipng],[no])
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

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"IBM 3270 Terminal emulator")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor)

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%-y`.`date +%-m`.`date +%-d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "$app_cv_release")
AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor)

AC_SUBST(PACKAGE_RELEASE,$app_cv_release)
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

dnl ---------------------------------------------------------------------------
dnl Set application id & product name
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=`pkg-config --variable=product_name lib3270` ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, $app_cv_product)
AC_SUBST(PRODUCT_NAME,$app_cv_product)

AC_ARG_WITH([application-id], [AS_HELP_STRING([--application-id], [Set the application ID])], [ app_cv_application_id="$withval" ],[ app_cv_application_id="br.com.bb.$app_cv_product" ])

AC_DEFINE_UNQUOTED(APPLICATION_ID, $app_cv_application_id)
AC_SUBST(APPLICATION_ID,$app_cv_application_id)

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
	AC_DEFINE(ENABLE_UNSTABLE_FEATURES)
	AC_MSG_NOTICE([Enabling unstable features])
else
	AC_MSG_NOTICE([Using only stable features])
fi


dnl ---------------------------------------------------------------------------
dnl Check for LIBV3270
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIBV3270], [libv3270], AC_DEFINE(HAVE_LIBV3270), AC_MSG_ERROR([LIBV3270 not present.]))

AC_SUBST(LIBV3270_LIBS)
AC_SUBST(LIBV3270_CFLAGS)

AC_ARG_WITH([library-name], [AS_HELP_STRING([--with-library-name], [Set protocol library name])], [ app_cv_library="$withval" ],[ app_cv_library=`pkg-config --variable=library_name lib3270` ])
AC_DEFINE_UNQUOTED(LIBRARY_NAME, $app_cv_library)
AC_SUBST(LIBRARY_NAME,$app_cv_library)

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

GLIB_GSETTINGS

PKG_CHECK_MODULES( [GTK], [gtk+-3.0 glib-2.0 gmodule-2.0], AC_DEFINE(HAVE_GTK), AC_MSG_ERROR([GTK not present.]))

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
AC_SUBST(BASEDIR,$app_cv_buildir)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)
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


