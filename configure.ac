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
AC_PREREQ(2.59)

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

AC_PATH_TOOL([STRIP], [strip])
AC_SUBST(STRIP)

AC_PATH_TOOL([REXXC], [rexxc], [cp])
AC_SUBST(REXXC)

AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([REXXCONFIG], [oorexx-config], [no])

#--[ Version & Revision ]-------------------------------------------------------------------------------------------------------------------------------------

AC_DEFINE(PACKAGE_REVISION,"SVN_REV")
AC_SUBST(PACKAGE_REVISION, "SVN_REV")

#--[ Set host based options ]---------------------------------------------------------------------------------------------------------------------------------

DLL_FLAGS="-shared -Wl,-soname,\$(@F)"
SOCKET_LIBS=""

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
		DLLEXT=".dll"
		EXEEXT=".exe"
		SOCKET_LIBS="-lws2_32"
		AC_PATH_TOOL([STRIP], [strip])
		;;

	*)
		AC_PATH_TOOL([STRIP], [strip])

esac


AC_SUBST(HOST_CC)
AC_SUBST(EXEEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLLEXT)
AC_SUBST(DLL_FLAGS)
AC_SUBST(SOCKET_LIBS)

#--[ Debug/Release ]------------------------------------------------------------------------------------------------------------------------------------------

DBG_LDFLAGS="-L.bin/Debug/lib"
RLS_LDFLAGS="-L.bin/Release/lib -Wl,--strip-all"

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

#--[ Headers ]------------------------------------------------------------------------------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

#--[ GTK 2/3 check ]------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_EXISTS

PKG_CHECK_MODULES( [GTK], [gtk+-3.0], [ ], [ PKG_CHECK_MODULES( [GTK], [gtk+-2.0 >= 2.8]) ], [ ] )

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

#--[ SSL ]----------------------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_MODULES( [SSL], [libssl], AC_DEFINE(HAVE_SSL), AC_MSG_NOTICE([libssl not present.]) )

AC_SUBST(SSL_LIBS)
AC_SUBST(SSL_CFLAGS)

#--[ Libraries ]----------------------------------------------------------------------------------------------------------------------------------------------
AC_SEARCH_LIBS( [sin], [m], AC_DEFINE(HAVE_LIBM), AC_MSG_NOTICE([libm not present.]))

#--[ Output files ]-------------------------------------------------------------------------------------------------------------------------------------------

AC_CONFIG_FILES([

	Makefile
	src/include/rules.mak
	src/lib3270/Makefile
	src/lib3270/mkversion.sh
	src/gtk/Makefile
	src/gtk/uiparser/Makefile

])

dnl Output the generated config.status script.
AC_OUTPUT

