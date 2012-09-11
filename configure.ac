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

dnl Load svn revision file info
m4_include([revision.m4])

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([pw3270], [5.0], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/lib3270/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

#--[ Check for tools & Libraries ]----------------------------------------------------------------------------------------------------------------------------

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S

AC_PATH_TOOL([REXXC], [rexxc], [cp])
AC_SUBST(REXXC)

AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([REXXCONFIG], [oorexx-config], [no])

AC_PATH_TOOL([MAKENSIS], [makensis], [no])
AC_PATH_TOOL([WINDRES], [windres], [no])

AC_CHECK_PROGS(GZIP,[gzip],no)
export GZIP;
if test $GZIP = "no" ; then
        AC_MSG_ERROR([Unable to find the gzip application]);
fi
AC_SUBST(GZIP)

AC_CHECK_PROGS(CONVERT,[rsvg-convert],no)
AC_SUBST(CONVERT)


#--[ Version & Revision ]-------------------------------------------------------------------------------------------------------------------------------------

AC_DEFINE(PACKAGE_REVISION,"SVN_REVISION")
AC_SUBST(PACKAGE_REVISION,"SVN_REVISION")

AC_DEFINE(PACKAGE_SOURCE,"SVN_URL")
AC_SUBST(PACKAGE_SOURCE,"SVN_URL")

AC_DEFINE(PACKAGE_LEVEL,"APP_LEVEL")
AC_SUBST(PACKAGE_LEVEL,"APP_LEVEL")

#--[ Set host based options ]---------------------------------------------------------------------------------------------------------------------------------

DLL_CFLAGS=""
DLL_FLAGS="-shared"
SOCKET_LIBS=""
LIB3270_REQUIRES=""

DLLEXT=".so"
OBJEXT=".o"
EXEEXT=""
DLLPREFIX="lib"

if test $host != $build; then
	AC_CHECK_PROGS(HOST_CC, gcc cc)
else
	HOST_CC=$CC
fi

case "$host" in
	*-mingw32)
		DBG_LDFLAGS="-L.bin/Debug/lib"
		RLS_LDFLAGS="-L.bin/Release/lib -Wl,--strip-all"
		DLLEXT=".dll"
		EXEEXT=".exe"
		SOCKET_LIBS="-lws2_32"
		CFLAGS="$CFLAGS -mms-bitfields"
		localedir="locale"
		DLL_FLAGS="-shared"
		LDSOFLAGS="-Wl,-soname,\`basename \$@\`"
		LDAPPFLAGS="-mwindows"
		DBGRPATH=""
		DLLDIR=""
		DBGCMDPREFIX=""
		LDARCH=""
		;;

	*-apple-*)
		DBG_LDFLAGS="-L.bin/Debug/lib"
		RLS_LDFLAGS="-L.bin/Release/lib -Wl,--strip-all"
		LDSOFLAGS=""
		DLLDIR="/lib"
		DBGRPATH=""
		DBGCMDPREFIX="DYLD_LIBRARY_PATH=\$(BINDIR)/Debug/lib"
		LDSOFLAGS="-Wl,-install_name,\`basename \$@\`"
		LDARCH="-arch $build_cpu"
		LDAPPFLAGS=""
		;;

	*)
		DLLDIR="/lib"
		DBG_LDFLAGS="-L.bin/Debug/$DLLDIR"
		RLS_LDFLAGS="-L.bin/Release/$DLLDIR"
		LDARCH=""
		LDSOFLAGS="-Wl,-soname,\`basename \$@\`"
		LDAPPFLAGS=""
		DBGRPATH="-Wl,--rpath,.bin/Debug/lib"
		DBGCMDPREFIX="PATH=\$(BINDIR)/Debug/lib:\$(PATH)"

esac

#--[ Check gettext ]------------------------------------------------------------------------------------------------------------------------------------------

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])
AC_PATH_TOOL([RPMBUILD], [rpmbuild], [no])
AC_PATH_TOOL([DPKGBUILD], [dpkg-buildpackage], [no])

AC_CHECK_HEADER(libintl.h, AC_DEFINE(HAVE_LIBINTL))
AC_CHECK_LIB(intl, gettext,[INTL_LIBS="-lintl"])

AC_SUBST(INTL_LIBS)

#--[ Check for ELF symbol visibility support ]----------------------------------------------------------------------------------------------------------------

# Try to compile PROGRAM, check for warnings
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
GLIB_CHECK_COMPILE_WARNINGS([AC_LANG_SOURCE([
void
__attribute__ ((visibility ("hidden")))
     f_hidden (void)
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
        return 0;
}
])],g_have_gnuc_visibility=yes,g_have_gnuc_visibility=no)
AC_MSG_RESULT($g_have_gnuc_visibility)
if test x$g_have_gnuc_visibility = xyes; then
  AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])
fi

# Check for compiler visibility flag
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
fi

#--[ Options ]------------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile PW3270 on some systems)])],
[
	app_cv_pic="$enableval"
],[
	if test "$host_cpu" = "x86_64"; then
		app_cv_pic="yes"
	else
		app_cv_pic="no"
	fi
])

if test "$app_cv_pic" == "yes"; then
	DLL_CFLAGS="$DLL_CFLAGS -fPIC"
fi

AC_ARG_ENABLE([strip],[AS_HELP_STRING([--disable-strip], [disable stripping of binaries])], [ app_cv_strip="$enableval" ],[ app_cv_strip="yes" ])

if test "$app_cv_strip" == "yes"; then
	AC_PATH_TOOL([STRIP], [strip], "true")
	AC_SUBST(STRIP)
else
	AC_SUBST(STRIP,"true")
fi


#--[ Check For Java ]-----------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_ENABLE([java],[AS_HELP_STRING([--disable-java], [Disable java extension & plugin])],[ app_cv_java="$enableval" ],[ app_cv_java="yes" ])

# Check for JDK_HOME
if test "_$JDK_HOME" = "_" ; then
	AC_CHECK_FILE("/usr/java/default/bin/javac",JDK_HOME="/usr/java/default",)
fi

if test "_$JDK_HOME" = "_" ; then
	app_cv_java="no"
fi

# Check for JRE_HOME
if test "_$JRE_HOME" = "_" ; then
	JRE_HOME="\$(JDK_HOME)/jre"
fi

if test "$app_cv_java" = "yes" ; then
	AC_PATH_TOOL([JAVAC], [javac], [\$(JDK_HOME)/bin/javac])
	AC_PATH_TOOL([JAVAH], [javah], [\$(JDK_HOME)/bin/javah])
	AC_PATH_TOOL([JAR], [jar], [\$(JDK_HOME)/bin/jar])
fi

AC_SUBST(JAVAC)
AC_SUBST(JAVAH)
AC_SUBST(JAR)
AC_SUBST(JDK_HOME)
AC_SUBST(JRE_HOME)

#--[ Misc options ]-------------------------------------------------------------------------------------------------------------------------------------------

AC_SUBST(MAJOR_VERSION,"$(echo $VERSION | cut -d. -f1)")
AC_SUBST(HOST_CC)
AC_SUBST(EXEEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLLEXT)
AC_SUBST(DLL_FLAGS)
AC_SUBST(DLL_CFLAGS)
AC_SUBST(DLLDIR)
AC_SUBST(LDSOFLAGS)
AC_SUBST(LDAPPFLAGS)
AC_SUBST(DBGRPATH)
AC_SUBST(DBGCMDPREFIX)
AC_SUBST(LDARCH)

#--[ Headers ]------------------------------------------------------------------------------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

AC_CHECK_FUNCS(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO) )

#--[ GTK 2/3 check ]------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_EXISTS

PKG_CHECK_MODULES( [GTK], [gtk+-3.0], [ GTK_VERSION="3.0" ], [ PKG_CHECK_MODULES( [GTK], [gtk+-2.0 >= 2.16 gthread-2.0 gmodule-2.0], [ GTK_VERSION="2.0" ] ) ], [  ] )

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_VERSION)

#--[ SSL ]----------------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIBSSL], [libssl], AC_DEFINE(HAVE_LIBSSL), AC_MSG_NOTICE([libssl not present.]) )

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

#--[ Libraries ]----------------------------------------------------------------------------------------------------------------------------------------------
PKG_CHECK_MODULES( [GTKMAC], [gtk-mac-integration], AC_DEFINE(HAVE_GTKMAC), AC_MSG_NOTICE([macosx integration not present.]) )
AC_SUBST(GTKMAC_LIBS)
AC_SUBST(GTKMAC_CFLAGS)

AC_SEARCH_LIBS( [sin], [m], AC_DEFINE(HAVE_LIBM), AC_MSG_NOTICE([libm not present.]))

AC_SUBST(SOCKET_LIBS)

#--[ Optional ]---------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_WITH([php-config], [AS_HELP_STRING([--with-php-config], [Path to php-config tool])], [ PHPCONFIG="$withval" ],[ AC_PATH_TOOL(PHPCONFIG, php-config, , $PATH) ])

if test -z "${PHPCONFIG}"; then
	AC_MSG_NOTICE([Program php-config not found. Use --with-php-config=path to specify absolute path to the php-config tool.])
else
	AC_DEFINE(HAVE_PHP)
fi

AC_SUBST(PHPCONFIG)

#--[ Pre-reqs ]-----------------------------------------------------------------------------------------------------------------------------------------------
AC_SUBST(LIB3270_REQUIRES)

#--[ Output files ]-------------------------------------------------------------------------------------------------------------------------------------------

AC_CONFIG_FILES([

	Makefile
	lib3270.pc
	pw3270.pc
	pw3270.spec
	pw3270.nsi
	src/include/rules.mak
	src/lib3270/Makefile
	src/lib3270/mkversion.sh
	src/pw3270/Makefile
	src/pw3270/uiparser/Makefile
	src/tools/Makefile
	man/Makefile
	makegtkruntime.sh
])

dnl Output the generated config.status script.
AC_OUTPUT

