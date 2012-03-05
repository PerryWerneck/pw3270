#
# "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
# (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
# aplicativos mainframe. Registro no INPI sob o nome G3270.
#
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
# os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
# Free Software Foundation.
#
# Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
# GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
# A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
# obter mais detalhes.
#
# Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
# programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA, 02111-1307, USA
#
# Contatos:
#
# perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
# erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
# licinio@bb.com.br			(Licínio Luis Branco)
# kraucer@bb.com.br			(Kraucer Fernandes Mazuco)
#

PACKAGE_NAME=pw3270

#---[ Paths ]------------------------------------------------------------------

prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
datarootdir=${prefix}/share
localedir=${datarootdir}/locale
docdir=${datarootdir}/doc/${PACKAGE_TARNAME}
sysconfdir=${prefix}/etc

#---[ Sources ]----------------------------------------------------------------

include common/sources.mak
include v3270/sources.mak
include uiparser/sources.mak

#---[ Targets ]----------------------------------------------------------------

SOURCES=main.c mainwindow.c actions.c fonts.c \
		$(foreach SRC, $(V3270_SRC), v3270/$(SRC)) \
		$(foreach SRC, $(COMMON_SRC), common/$(SRC)) \
		$(foreach SRC, $(UI_PARSER_SRC), uiparser/$(SRC))

DEPENDS=*.h common/*.h uiparser/*.h v3270/*.h Makefile

VALGRIND=/usr/bin/valgrind

CFLAGS=-g -O2 -fvisibility=hidden -DGSEAL_ENABLE -pthread -I/usr/include/gtk-3.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng14   -DAPPDATA=\"$(datarootdir)/$(PACKAGE_NAME)\" -I../../src/include
LIBS=-lm  -pthread -lgtk-3 -lgdk-3 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0  

include ../include/rules.mak

#---[ Debug targets ]----------------------------------------------------------

Debug: $(BINDIR)/Debug/$(PACKAGE_TARNAME)$(EXEEXT)

$(BINDIR)/Debug/$(PACKAGE_TARNAME)$(EXEEXT):	$(foreach SRC, $(basename $(SOURCES)), $(OBJDIR)/Debug/$(SRC)$(OBJEXT))

	@echo $@ ...
	@$(MKDIR) `dirname $@`
	@$(LD) -Wl,--rpath,.bin/Debug -o $@ $^ $(LIBS) $(LIB3270_LIBS)

run: $(BINDIR)/Debug/$(PACKAGE_TARNAME)$(EXEEXT)
	@cd "$(ROOTDIR)" ; .bin/Debug/$(PACKAGE_TARNAME)$(EXEEXT)

memchk: $(BINDIR)/Debug/$(PACKAGE_TARNAME)$(EXEEXT)
ifeq ($(VALGRIND),no)
	@cd "$(ROOTDIR)" ; .bin/Debug/$(PACKAGE_TARNAME)$(EXEEXT)
else
	@cd "$(ROOTDIR)" ; G_DEBUG=gc-friendly G_SLICE=always-malloc \$(VALGRIND) --leak-check=full --suppressions=valgrind.suppression --gen-suppressions=all .bin/Debug/$(PACKAGE_TARNAME)$(EXEEXT)
#	@cd "$(ROOTDIR)" ; G_DEBUG=gc-friendly G_SLICE=always-malloc \$(VALGRIND) --leak-check=full --suppressions=valgrind.suppression .bin/Debug/$(PACKAGE_TARNAME)$(EXEEXT)
endif

#---[ Misc targets ]-----------------------------------------------------------

v3270/marshal.h: v3270/genmarshal
	@mkdir -p `dirname $@`
	@glib-genmarshal --prefix=$(PACKAGE_NAME) --header v3270/genmarshal > $@

v3270/marshal.c: v3270/genmarshal v3270/marshal.h
	@mkdir -p `dirname $@`
	@glib-genmarshal --prefix=$(PACKAGE_NAME) --body v3270/genmarshal > $@

clean: clean-common
	@rm -f v3270/marshal.c
	@rm -f v3270/marshal.h
	@rm -fr uiparser/.bin
	@rm -fr uiparser/.obj
	@rm -fr v3270/.bin
	@rm -fr v3270/.obj

