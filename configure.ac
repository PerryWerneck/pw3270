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
dnl licinio@bb.com.br		(Licínio Luis Branco)
dnl kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
dnl macmiranda@bb.com.br	(Marco Aurélio Caldas Miranda)
dnl

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([pw3270], [5.2], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S
AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([GENMARSHAL], [glib-genmarshal], [glib-genmarshal])
AC_PATH_TOOL([HELP2MAN], [help2man], [no])

AC_LANG([C])

dnl ---------------------------------------------------------------------------
dnl Initialize defaults
dnl ---------------------------------------------------------------------------

DBG_CFLAGS="-g -fstack-check -DDEBUG=1"
RLS_CFLAGS="-DNDEBUG=1"
PLUGINS=""
APP_RESOURCES=""
APP_LDFLAGS=""
DLL_LDFLAGS="-shared"
STATIC_LDFLAGS=""

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

if test $host != $build; then
	AC_CHECK_PROGS(HOST_CC, gcc cc)
else
	HOST_CC=$CC
fi

case "$host" in
	*-mingw32)

		EXEEXT=".exe"
		DLLPREFIX=""
		DLLEXT=".dll"
		APP_RESOURCES="resources.rc"
		APP_LDFLAGS="-mwindows"

		DBG_CFLAGS="-DDEBUG=1"
		RLS_CFLAGS="-DNDEBUG=1"

		CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0600"
		LIBS="$LIBS -lws2_32 -lwtsapi32 -lcomdlg32"

		app_win32_revision=$(date +%y.%m.%d.%H)
		AC_SUBST(WIN32_VERSION,$app_win32_revision)

		dnl https://stackoverflow.com/questions/13768515/how-to-do-static-linking-of-libwinpthread-1-dll-in-mingw
		STATIC_LDFLAGS="-static-libstdc++ -static-libgcc -static"
		DLL_LDFLAGS="-shared -Wl,--add-stdcall-alias"

		AC_CONFIG_FILES(win/makeruntime.sh)
		AC_CONFIG_FILES(win/copydeps.sh)
		AC_CONFIG_FILES(win/$PACKAGE_TARNAME.nsi)
		AC_CONFIG_FILES(src/pw3270/resources.rc)

		PLUGINS="$PLUGINS hllapi"

		if test "$host_cpu" = "x86_64"; then
			app_cv_programfiles="PROGRAMFILES64"
			app_cv_winarch="64"
		else
			app_cv_programfiles="PROGRAMFILES"
			app_cv_winarch="32"
		fi

		AC_SUBST(PROGRAMFILES,$app_cv_programfiles)
		AC_SUBST(WINARCH,$app_cv_winarch)
		;;

	*-apple-*)
		EXEEXT=""
		DLLPREFIX="lib"
		DLLEXT=".so"

		DBG_CFLAGS="$DBG_CFLAGS -DDATADIR=\$(BASEDIR)"

		CFLAGS="$CFLAGS -Wall -Werror"

		LIBS="-pthread $LIBS"
		;;

 	*)
		EXEEXT=""
		DLLPREFIX="lib"
		DLLEXT=".so"

		DBG_CFLAGS="$DBG_CFLAGS -DDATADIR=\$(BASEDIR)"
		RLS_CFLAGS="$RLS_CFLAGS -DDATADIR=\$(datarootdir)/\$(PACKAGE_NAME)"

		# LIBS="-Wl,--no-allow-shlib-undefined,--as-needed,--no-undefined -pthread $LIBS"
		LIBS="-pthread $LIBS"

esac

AC_SUBST(DBG_CFLAGS)
AC_SUBST(RLS_CFLAGS)
AC_SUBST(EXEEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLLEXT)
AC_SUBST(APP_RESOURCES)
AC_SUBST(APP_LDFLAGS)
AC_SUBST(DLL_LDFLAGS)
AC_SUBST(STATIC_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([RPMBUILD], [rpmbuild], [no])
AC_PATH_TOOL([MAKENSIS], [makensis], [no])
AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([WINDRES], [windres], [no])
AC_PATH_TOOL([DBUSBINDINGTOOL],[dbus-binding-tool],[no])

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"3270 Emulator for gtk")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor)

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%y`.`date +%m`.`date +%d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, $app_cv_release)
AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor)

AC_SUBST(PACKAGE_RELEASE,$app_cv_release)
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

dnl ---------------------------------------------------------------------------
dnl Check for D-Bus
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [DBUS], [dbus-1], app_cv_libdbus=yes, AC_MSG_NOTICE([ No DBUS support.]) )
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)

PKG_CHECK_MODULES( [GDBUS], [dbus-glib-1], app_cv_dbus=yes, AC_MSG_NOTICE([No GDBUS support.])  )
AC_SUBST(GDBUS_LIBS)
AC_SUBST(GDBUS_CFLAGS)

if test "$app_cv_dbus" == "yes"; then

	AC_DEFINE(HAVE_DBUS)
	PLUGINS="$PLUGINS dbus3270"

fi

dnl ---------------------------------------------------------------------------
dnl Check for libintl
dnl ---------------------------------------------------------------------------

INTL_LIBS=""

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([SVG2PNG],[rsvg-convert],[no])
AC_PATH_TOOL([CONVERT], [convert], [no])
AC_PATH_TOOL([OPTIPNG],[optipng],[no])

AC_CHECK_HEADER(libintl.h, [
	AC_DEFINE(HAVE_LIBINTL, 1)

	case "$host" in
	*-mingw32)
		INTL_LIBS="-lintl"
		;;

	*)
		INTL_LIBS=""

	esac

])

AC_SUBST(INTL_LIBS)

dnl ---------------------------------------------------------------------------
dnl Check for ELF symbol visibility support
dnl ---------------------------------------------------------------------------
m4_define([GLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
glib_ac_compile_save="$ac_compile"
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext'
AS_IF([_AC_EVAL_STDERR($ac_compile) &&
         AC_TRY_COMMAND([(if test -s conftest.err; then false ; else true; fi)])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])
ac_compile="$glib_ac_compile_save"
rm -f conftest.$ac_objext conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# GLIB_CHECK_COMPILE_WARNINGS

AC_MSG_CHECKING(for GNUC visibility attribute)
GLIB_CHECK_COMPILE_WARNINGS([
void
__attribute__ ((visibility ("hidden")))
     f_hidden (void)
{
}
void
__attribute__ ((visibility ("internal")))
     f_internal (void)
{
}
void
__attribute__ ((visibility ("protected")))
     f_protected (void)
{
}
void
__attribute__ ((visibility ("default")))
     f_default (void)
{
}
int main (int argc, char **argv)
{
        f_hidden();
        f_internal();
        f_protected();
        f_default();
        return 0;
}
],g_have_gnuc_visibility=yes,g_have_gnuc_visibility=no)
AC_MSG_RESULT($g_have_gnuc_visibility)
if test x$g_have_gnuc_visibility = xyes; then
  AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])
fi

dnl ---------------------------------------------------------------------------
dnl Check for compiler visibility flag
dnl ---------------------------------------------------------------------------
AC_MSG_CHECKING(whether $CC accepts -fvisibility=hidden)
AC_CACHE_VAL(app_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_TRY_RUN([int main() { return 0; }],
  app_cv_fvisibility_ok=yes,
  app_cv_fvisibility_ok=no,
  app_cv_fvisibility_ok=no)
CC="$ac_save_cc"])
AC_MSG_RESULT($app_cv_fvisibility_ok)
if test $app_cv_fvisibility_ok = yes; then
  CFLAGS="$CFLAGS -fvisibility=hidden"
  CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
fi

dnl ---------------------------------------------------------------------------
dnl SSL Security options
dnl ---------------------------------------------------------------------------

AC_ARG_ENABLE([self-signed-certs],
	[AS_HELP_STRING([--disable-self-signed-certs], [disable SSL connection when host presents a self signed certificate])],
[
	app_cv_self_signed_certs="$enableval"
],[
	app_cv_self_signed_certs="yes"
])

AC_ARG_WITH([ssl-crl-check],
	[AS_HELP_STRING([--with-ssl-crl-check], [enable validation of the certificate revogation list in TN3270S connections])],
[
	app_cv_enable_crl_check="$withval"
],[
	app_cv_enable_crl_check="no"
])

if test "$app_cv_self_signed_certs" == "yes"; then
	AC_DEFINE(SSL_ALLOW_SELF_SIGNED_CERT)
fi

if test "$app_cv_enable_crl_check" == "yes"; then
	AC_DEFINE(SSL_ENABLE_CRL_CHECK)
fi


dnl ---------------------------------------------------------------------------
dnl Check for pic
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile on 64bits)])],
[
	app_cv_pic="$enableval"
],[

	case "$host_cpu" in
	x86_64)
		app_cv_pic="yes"
		;;

	aarch64)
		app_cv_pic="yes"
		;;

	*)
		app_cv_pic="no"
	esac

dnl	case "$host" in
dnl		*-mingw32)
dnl			app_cv_pic="no"
dnl			;;
dnl
dnl	*)
dnl		if test "$host_cpu" = "x86_64"; then
dnl			app_cv_pic="yes"
dnl		else
dnl			app_cv_pic="no"
dnl		fi
dnl	esac
])

if test "$app_cv_pic" == "yes"; then
	CFLAGS="$CFLAGS -fPIC"
	CXXFLAGS="$CXXFLAGS -fPIC"
fi

dnl ---------------------------------------------------------------------------
dnl Lib3270 flags
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([libname], [AS_HELP_STRING([--with-libname], [Setup library name])], [ app_cv_libname="$withval" ],[ app_cv_libname="3270" ])

AC_SUBST(LIB3270_INCLUDEPATH,$ac_pwd/modules/lib3270/src/include)
AC_SUBST(LIBV3270_INCLUDEPATH,$ac_pwd/modules/libv3270/src/include)

AC_SUBST(LIB3270_CFLAGS,-I$ac_pwd/modules/lib3270/src/include)
AC_SUBST(LIB3270_LIBS,-l$app_cv_libname)

AC_SUBST(LIBV3270_CFLAGS,-I$ac_pwd/modules/libv3270/src/include)
AC_SUBST(LIBV3270_LIBS,-lv$app_cv_libname)

AC_SUBST(PW3270_CFLAGS,-I$ac_pwd/src/include)

AC_SUBST(PW3270_LIBS,-l$PACKAGE_TARNAME)

AC_DEFINE(LIB3270_NAME,$app_cv_libname)
AC_SUBST(LIB3270_NAME,$app_cv_libname)

AX_SUBDIRS_CONFIGURE(
	[modules/lib3270],
	[--with-work-dir=$ac_pwd],
	[--with-libname=$app_cv_libname]
)

AX_SUBDIRS_CONFIGURE(
	[modules/libv3270],
	[LIB3270_CFLAGS=-I$ac_pwd/modules/lib3270/src/include],
	[LIB3270_LIBS=-l$app_cv_libname],
	[--with-work-dir=$ac_pwd]
)

dnl ---------------------------------------------------------------------------
dnl Check for headers
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

AC_CHECK_FUNCS(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO) )
AC_CHECK_FUNC(vasprintf, AC_DEFINE(HAVE_VASPRINTF) )
AC_CHECK_FUNC(strtok_r, AC_DEFINE(HAVE_STRTOK_R) )

AC_ARG_WITH([inet-ntop], [AS_HELP_STRING([--with-inet-ntop], [Assume that inet_nto() is available])], [ app_cv_inet_ntop="$withval" ],[ app_cv_inet_ntop="auto" ])

if test "$app_cv_inet_ntop" == "auto"; then

	case "$host" in
	*-mingw32)
		app_cv_inet_ntop="yes"
		;;

	*)
		AC_CHECK_FUNC(inet_ntop, app_cv_inet_ntop="yes" )

	esac

fi

if test "$app_cv_inet_ntop" == "yes"; then
	AC_DEFINE(HAVE_INET_NTOP)
fi

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GTK], [gtk+-3.0 gmodule-2.0], [ GTK_VERSION="3.0" ], [ PKG_CHECK_MODULES( [GTK], [gtk+-2.0 >= 2.14 gthread-2.0 gmodule-2.0], [ GTK_VERSION="2.0" ] ) ], [  ] )

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_MODVERSION,`$PKG_CONFIG --modversion gtk+-$GTK_VERSION`)

dnl ---------------------------------------------------------------------------
dnl Check for GLIB
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GLIB], [glib-2.0], AC_DEFINE(HAVE_GLIB) )
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for Libraries
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIBSSL], [libssl libcrypto], AC_DEFINE(HAVE_LIBSSL), AC_MSG_NOTICE([libssl not present.]) )

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

AC_SEARCH_LIBS([dlopen], [dl], AC_DEFINE(HAVE_LIBDL), AC_MSG_NOTICE([libdl not present.]))

dnl ---------------------------------------------------------------------------
dnl Output config
dnl ---------------------------------------------------------------------------

AC_SUBST(BASEDIR,$ac_pwd)
AC_SUBST(PLUGINS)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(src/pw3270/Makefile)
AC_CONFIG_FILES(src/pw3270/uiparser/Makefile)
AC_CONFIG_FILES(src/pw3270/common/Makefile)
AC_CONFIG_FILES(src/libpw3270cpp/Makefile)

AC_CONFIG_FILES(ui/Makefile)
AC_CONFIG_FILES(charsets/Makefile)
AC_CONFIG_FILES(branding/Makefile)
AC_CONFIG_FILES(locale/Makefile)

AC_CONFIG_FILES(src/plugins/hllapi/Makefile)
AC_CONFIG_FILES(src/plugins/dbus3270/Makefile)

AC_CONFIG_FILES(sdk/pw3270.pc)
AC_CONFIG_FILES(sdk/Makefile)

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(versions)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT


