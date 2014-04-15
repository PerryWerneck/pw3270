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

dnl Check for iconv
AM_ICONV

#--[ Check for tools & Libraries ]----------------------------------------------------------------------------------------------------------------------------

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S

AC_PATH_TOOL([VALGRIND], [valgrind], [no])

AC_PATH_TOOL([MAKENSIS], [makensis], [no])
AC_PATH_TOOL([WINDRES], [windres], [no])
AC_PATH_TOOL([DBUSBINDINGTOOL],[dbus-binding-tool],[no])

AC_PATH_TOOL([MACBUNDLER],[gtk-mac-bundler], [no])

AC_CHECK_PROGS(GZIP,[gzip],no)
export GZIP;
if test $GZIP = "no" ; then
        AC_MSG_ERROR([Unable to find the gzip application]);
fi
AC_SUBST(GZIP)

AC_CHECK_PROGS(ZIP,[zip],no)
export ZIP;
if test $GZIP = "no" ; then
        AC_MSG_ERROR([Unable to find the gzip application]);
fi
AC_SUBST(ZIP)

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
SYSDLL_FLAGS="-shared"
SOCKET_LIBS=""
LIB3270_REQUIRES=""

DLLEXT=".so"
OBJEXT=".o"
EXEEXT=""
DLLPREFIX="lib"
PLUGINS=""
APP_GUI_SRC=""
LDLIBFLAGS=""
EXTENSIONS=""

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
		DLL_FLAGS="-shared -Wl,--add-stdcall-alias,--enable-stdcall-fixup"

		# http://stackoverflow.com/questions/18138635/mingw-exe-requires-a-few-gcc-dlls-regardless-of-the-code
		SYSDLL_FLAGS="-shared -static-libgcc -static-libstdc++ -Wl,--add-stdcall-alias,--enable-stdcall-fixup"

		LDSOFLAGS="-Wl,-soname,\`basename \$@\`"
		LDAPPFLAGS="-mwindows"
		LDLIBFLAGS="-Wl,--output-def,\$@.def"
		DBGRPATH=""
		DLLDIR=""
		DBGCMDPREFIX=""
		LDARCH=""
		PLUGINS="hllapi"
		APP_GUI_SRC="resources.rc"
		DLLPREFIX=""
		;;

	*-apple-*)
		DBG_LDFLAGS="-L.bin/Debug/lib"
		RLS_LDFLAGS="-L.bin/Release/lib"
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
		RLS_LDFLAGS="-Wl,-rpath,\$(libdir)"
		LDARCH=""
		LDSOFLAGS="-Wl,-soname,\`basename \$@\`"
		LDAPPFLAGS=""
		DBGRPATH="-Wl,--rpath,.bin/Debug/lib"
		DBGCMDPREFIX="LD_LIBRARY_PATH=\$(BINDIR)/Debug/lib"

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
AC_CHECK_HEADER(syslog.h, AC_DEFINE(HAVE_SYSLOG))
AC_CHECK_LIB(intl, gettext,[INTL_LIBS="-lintl"])

AC_SUBST(INTL_LIBS)

#--[ Check for ELF symbol visibility support ]----------------------------------------------------------------------------------------------------------------

# Try to compile PROGRAM, check for warnings
m4_define([GLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
glib_ac_compile_save="$ac_compile"
ac_compile='$CC -c conftest.$ac_ext'
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
  OO_CXXFLAGS="$OO_CXXFLAGS -fvisibility=hidden"
  CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
fi

#--[ Options ]------------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile PW3270 on some systems)])],
[
	app_cv_pic="$enableval"
],[
	case "$host-$host_cpu" in
        *-mingw32-*)
		app_cv_pic="no"
		;;

	*-x86_64)
		app_cv_pic="yes"
		;;

	*)
		app_cv_pic="no"
	esac

])

AC_ARG_ENABLE([registry],
	[AS_HELP_STRING([--disable-registry], [disable configurations in windows registry])],
[
	app_cv_registry="$enableval"
],[
	case "$host" in
        *-mingw32)
			app_cv_registry="yes"
			;;

		*)
			app_cv_registry="no"
	esac

])


if test "$app_cv_pic" == "yes"; then
	DLL_CFLAGS="$DLL_CFLAGS -fPIC"
	OO_CXXFLAGS="-fPIC $OO_CXXFLAGS"
fi

AC_ARG_ENABLE([strip],[AS_HELP_STRING([--disable-strip], [disable stripping of binaries])], [ app_cv_strip="$enableval" ],[ app_cv_strip="yes" ])

if test "$app_cv_strip" == "yes"; then
	AC_PATH_TOOL([STRIP], [strip], "true")
	AC_SUBST(STRIP)
else
	AC_SUBST(STRIP,"true")
fi


#--[ Check For Java ]-----------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_VAR([JDK_HOME], [JDK home directory])
AC_ARG_VAR([JRE_HOME], [JRE home directory])

AC_ARG_ENABLE([java],[AS_HELP_STRING([--disable-java], [Disable java integration])],[ app_cv_java="$enableval" ],[ app_cv_java="yes" ])

if test "$app_cv_java" = "yes" ; then

	# Check for JAVA_HOME
	if test "_$JAVA_HOME" = "_" ; then
		app_cv_java="no"
	fi

	# Check for JDK_HOME
	if test "_$JDK_HOME" = "_" ; then
		JDK_HOME=$JAVA_HOME
	fi

	# Check for JRE_HOME
	if test "_$JRE_HOME" = "_" ; then
		JRE_HOME=$JAVA_HOME/jre
	fi
fi

if test "$app_cv_java" = "yes" ; then
	AC_PATH_TOOL([JAVA], [java], [no], [$JDK_HOME/bin:$JRE_HOME/bin:$PATH])
	AC_PATH_TOOL([JAVAC], [javac], [no], [$JDK_HOME/bin:$JRE_HOME/bin:$PATH])
	AC_PATH_TOOL([JAVAH], [javah], [no], [$JDK_HOME/bin:$JRE_HOME/bin:$PATH])
	AC_PATH_TOOL([JAR], [jar], [no], [$JDK_HOME/bin:$JRE_HOME/bin:$PATH])
fi

AC_SUBST(JAVAC)
AC_SUBST(JAVAH)
AC_SUBST(JAR)
AC_SUBST(JDK_HOME)
AC_SUBST(JRE_HOME)

#--[ Headers ]------------------------------------------------------------------------------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

AC_CHECK_FUNCS(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO) )
AC_CHECK_FUNC(vasprintf, AC_DEFINE(HAVE_VASPRINTF) )
AC_CHECK_FUNC(strtok_r, AC_DEFINE(HAVE_STRTOK_R) )

PKG_CHECK_EXISTS

#--[ GTK 2/3 check ]------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_MODULES( [GTK], [gtk+-3.0 gmodule-2.0], [ GTK_VERSION="3.0" ], [ PKG_CHECK_MODULES( [GTK], [gtk+-2.0 >= 2.14 gthread-2.0 gmodule-2.0], [ GTK_VERSION="2.0" ] ) ], [  ] )

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_MODVERSION,`$PKG_CONFIG --modversion gtk+-$GTK_VERSION`)

#--[ GLIB check ]---------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_MODULES( [GLIB], [glib-2.0], AC_DEFINE(HAVE_GLIB) )
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

#--[ Check DBUS ]---------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_MODULES( [DBUS], [dbus-1] )
AC_SUBST(DBUS_LIBS)
AC_SUBST(DBUS_CFLAGS)

PKG_CHECK_MODULES( [GDBUS], [dbus-glib-1], app_cv_dbus=yes, AC_MSG_NOTICE([No GDBUS support.])  )
AC_SUBST(GDBUS_LIBS)
AC_SUBST(GDBUS_CFLAGS)

if test "$app_cv_dbus" == "yes"; then
	AC_DEFINE(HAVE_DBUS)
	PLUGINS="$PLUGINS dbus3270"
fi

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

AC_SEARCH_LIBS([dlopen], [dl], AC_DEFINE(HAVE_LIBDL), AC_MSG_NOTICE([libdl not present.]))

#--[ Optional ]---------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_WITH([php-config], [AS_HELP_STRING([--with-php-config], [Path to php-config tool])], [ PHPCONFIG="$withval" ],[ AC_PATH_TOOL(PHPCONFIG, php-config, , $PATH) ])

case "$host" in
        *-mingw32)
		AC_MSG_NOTICE([PHP disabled on windows platforms.])
		;;

	*-apple-*)
		AC_MSG_NOTICE([PHP disabled on apple platforms.])
		;;

	*)
		if test -z "${PHPCONFIG}"; then
			AC_MSG_NOTICE([Program php-config not found. Use --with-php-config=path to specify absolute path to the php-config tool.])
		else
			AC_DEFINE(HAVE_PHP)
			EXTENSIONS="$EXTENSIONS php"
		fi
esac

AC_SUBST(PHPCONFIG)

#--[ ooRexx ]-----------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_ENABLE([rexx],[AS_HELP_STRING([--disable-rexx], [Disable ooRexx plugin & extension])],[ app_cv_rexx="$enableval" ],[ app_cv_rexx="yes" ])
AC_ARG_VAR([REXX_HOME], [ooRexx home])

if test -z "${REXX_HOME}"; then

	REXX_HOME="\$(datarootdir)/ooRexx"

	AC_PATH_TOOL([REXXC], [rexxc], [cp] )
	AC_PATH_TOOL([REXXCONFIG], [oorexx-config], [no])

	PKG_CHECK_MODULES( [REXX], [ooRexx], app_cv_rexx=yes, AC_MSG_NOTICE([No ooRexx on pkg-config.]) )

	if test "$REXXCONFIG" != "no"; then
		REXX_CFLAGS=`$REXXCONFIG --cflags`
		REXX_LIBS=`$REXXCONFIG --libs`
		REXX_HOME=`$REXXCONFIG --prefix`/share/ooRexx
		AC_MSG_NOTICE([Got settings from oorexx-config.])
	else
		AC_MSG_NOTICE([ooRexx SDK is NOT available.])
		app_cv_rexx="no"
	fi

else
	if test -e "${REXX_HOME}/api/oorexxapi.h"; then

		AC_MSG_NOTICE([Setting rexx sdk from REXX_HOME])

		REXX_CFLAGS="$CFLAGS -I\"$REXX_HOME/api\""
		REXX_LIBS="$LIBS -L\"$REXX_HOME\" -lrexxapi -lrexx"

		AC_PATH_TOOL([REXXC], [rexxc], [cp], path="$PATH:$REXX_HOME" )

	else
		AC_MSG_NOTICE([ooRexx SDK is NOT available.])
		app_cv_rexx="no"
	fi

fi

if test "$app_cv_rexx" == "yes"; then
	AC_DEFINE(HAVE_REXX)
	AC_DEFINE(HAVE_OOREXXAPI_H)
	PLUGINS="$PLUGINS rx3270"
	AC_MSG_NOTICE([ooRexx plugin & extension are enabled.])
else
	AC_MSG_NOTICE([ooRexx is NOT present or disabled.])
fi

AC_ARG_WITH([rexxlibdir], [AS_HELP_STRING([--with-rexxlibdir], [Path to rexx extension libs])], [ REXX_LIBDIR=$withval ],[ REXX_LIBDIR=$libdir ])

AC_SUBST(REXXC)
AC_SUBST(REXXCONFIG)
AC_SUBST(REXX_CFLAGS)
AC_SUBST(REXX_LIBS)
AC_SUBST(REXX_HOME)
AC_SUBST(REXX_LIBDIR)

#--[ Libreoffice ]--------------------------------------------------------------------------------------------------------------------------------------------

AC_ARG_ENABLE([office],[AS_HELP_STRING([--disable-office], [Disable libreoffice extension])],[ app_cv_office="$enableval" ],[ app_cv_office="yes" ])

AC_ARG_VAR([OFFICE_HOME], [Libreoffice home])
AC_ARG_VAR([OO_SDK_HOME], [Libreoffice SDK home])
AC_ARG_VAR([OO_SDK_URE_HOME], [Libreoffice URE home])

if test "_$OFFICE_HOME" == "_" ; then
	if test "$host_cpu" = "x86_64"; then
		OFFICE_HOME="/usr/lib64/libreoffice"
	else
		OFFICE_HOME="/usr/lib/libreoffice"
	fi
fi

if test "_$OO_SDK_HOME" == "_" ; then
	OO_SDK_HOME=$OFFICE_HOME/sdk
fi

if test "_$OO_SDK_URE_HOME" == "_" ; then
	if test -d $OFFICE_HOME/ure; then
		OO_SDK_URE_HOME="$OFFICE_HOME/ure"
	else
		if test "$host_cpu" = "x86_64"; then
			OO_SDK_URE_HOME="/usr/lib64/ure"
		else
			OO_SDK_URE_HOME="/usr/lib/ure"
		fi
	fi
fi

# Setup c++ compiler flags
if test -d "$prefix/include/libreoffice"; then
	OO_CXXFLAGS="$OO_CXXFLAGS -I$prefix/include/libreoffice"
	AC_MSG_NOTICE([Found libreoffice includes at $prefix/include/libreoffice])
elif test -d "/usr/include/libreoffice"; then
	OO_CXXFLAGS="$OO_CXXFLAGS -I/usr/include/libreoffice"
	AC_MSG_NOTICE([Found libreoffice includes at /usr/include/libreoffice])
elif test -d "$OO_SDK_HOME/include"; then
	OO_CXXFLAGS="$OO_CXXFLAGS -I$OO_SDK_HOME/include"
	AC_MSG_NOTICE([Found libreoffice includes at $OO_SDK_HOME/include])
else
	app_cv_office="no"
	AC_MSG_NOTICE([Can't libreoffice includes])
fi

OO_CXXFLAGS="-DCPPU_ENV=gcc3 -Wno-strict-aliasing $OO_CXXFLAGS"

# Test for IDL files
if test -d "$OO_SDK_HOME/idl"; then
	IDLDIR="$OO_SDK_HOME/idl"
	AC_MSG_NOTICE([Found idl at $IDLDIR])
elif test -d "/usr/share/idl/libreoffice"; then
	IDLDIR="/usr/share/idl/libreoffice"
	AC_MSG_NOTICE([Found idl at $IDLDIR])
elif test -d "$libdir/libreoffice/sdk/idl"; then
	IDLDIR="$libdir/libreoffice/sdk/idl"
	AC_MSG_NOTICE([Found idl at $IDLDIR])
else
	app_cv_office="no"
	AC_MSG_NOTICE([Can't find idl path])
fi

OO_SDK_URE_BIN_DIR=$OO_SDK_URE_HOME/bin
OO_SDK_URE_LIB_DIR=$OO_SDK_URE_HOME/lib

AC_PATH_TOOL([IDLC], idlc, "no", path=$PATH:$OO_SDK_HOME/bin)
if test "$IDLC" == "no" ; then
	app_cv_office="no"
fi

AC_PATH_TOOL([UNOPKG], unopkg, "no", path=$PATH:$OO_SDK_HOME/bin)
if test "$UNOPKG" == "no" ; then
	app_cv_office="no"
fi

AC_PATH_TOOL([REGMERGE], regmerge, "no", path=$PATH:$OO_SDK_URE_BIN_DIR)
if test "$REGMERGE" == "no" ; then
	app_cv_office="no"
fi

AC_PATH_TOOL([CPPUMAKER], cppumaker, "no", path=$PATH:$OO_SDK_HOME/bin)
if test "$CPPUMAKER" == "no" ; then
	app_cv_office="no"
fi

AC_PATH_TOOL([UNO], uno, "no", path=$PATH:$OO_SDK_URE_BIN_DIR)
if test "$UNO" == "no" ; then
	app_cv_office="no"
fi


# Check for OO_SDK_HOME
if test "_$OO_SDK_HOME" == "_" ; then
	app_cv_office="no"
fi

# Check for OFFICE_HOME
if test "_$OFFICE_HOME" == "_" ; then
	app_cv_office="no"
fi

# Set targets
if test "$app_cv_office" == "yes" ; then
	AC_MSG_NOTICE([libreoffice sdk is present.])
	EXTENSIONS="$EXTENSIONS oxt"
else
	AC_MSG_NOTICE([libreoffice sdk is NOT present or disabled.])
fi

AC_SUBST(OFFICE_HOME)
AC_SUBST(OO_SDK_HOME)
AC_SUBST(OO_SDK_URE_HOME)
AC_SUBST(IDLDIR)
AC_SUBST(OO_CXXFLAGS)

#--[ Pre-reqs ]-----------------------------------------------------------------------------------------------------------------------------------------------
AC_SUBST(LIB3270_REQUIRES)

#--[ Misc options ]-------------------------------------------------------------------------------------------------------------------------------------------

if test "$app_cv_registry" == "yes"; then
	AC_DEFINE(HAVE_WIN_REGISTRY)
fi

AC_SUBST(MAJOR_VERSION,"$(echo $VERSION | cut -d. -f1)")
AC_SUBST(HOST_CC)
AC_SUBST(EXEEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLLEXT)
AC_SUBST(DLL_FLAGS)
AC_SUBST(SYSDLL_FLAGS)
AC_SUBST(DLL_CFLAGS)
AC_SUBST(DLLDIR)
AC_SUBST(LDSOFLAGS)
AC_SUBST(LDAPPFLAGS)
AC_SUBST(LDLIBFLAGS)
AC_SUBST(DBGRPATH)
AC_SUBST(DBGCMDPREFIX)
AC_SUBST(LDARCH)
AC_SUBST(PLUGINS)
AC_SUBST(EXTENSIONS)
AC_SUBST(APP_GUI_SRC)

AC_SUBST(RLS_LDFLAGS)

#--[ Output files ]-------------------------------------------------------------------------------------------------------------------------------------------

AC_CONFIG_FILES([

	Makefile
	lib3270.pc
	pw3270.pc
	pw3270.spec
	pw3270-i686.nsi
	pw3270-x86_64.nsi
	src/include/rules.mak
	src/include/plugin.mak
	src/lib3270/Makefile
	src/lib3270/mkversion.sh
	src/pw3270/Makefile
	src/loffice/Makefile
	src/loffice/description.xml
	src/loffice/manifest.xml
	src/pw3270/uiparser/Makefile
	src/tools/Makefile
	src/plugins/hllapi/Makefile
	src/plugins/rx3270/Makefile
	src/plugins/dbus3270/Makefile
	src/classlib/Makefile
	src/classlib/class.mak
	man/Makefile
	makegtkruntime.sh
	src/sample/Makefile
	mac/pw3270.bundle
	src/php/Makefile
	src/php/php3270.h
])

dnl Output the generated config.status script.
AC_OUTPUT
