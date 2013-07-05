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
#

LIBNAME=pw3270cxx.a

#---[ Sources ]----------------------------------------------------------------

SOURCES=main.cc session.cc exception.cc local.cc remote.cc

#---[ Paths ]------------------------------------------------------------------

ROOTDIR ?= .
OBJDIR  ?= $(ROOTDIR)/.obj
BINDIR  ?= $(ROOTDIR)/.bin
BINDBG  ?= $(BINDIR)/Debug
BINRLS  ?= $(BINDIR)/Release

OBJDBG = $(OBJDIR)/Debug
OBJRLS = $(OBJDIR)/Release

#---[ Tools ]------------------------------------------------------------------

MKDIR = mkdir -p

#---[ Flags ]------------------------------------------------------------------

LIBS=-ldl -ldbus-glib-1 -ldbus-1
CFLAGS=-fPIC -I../include -I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include
DEBUG_CFLAGS=-DDEBUG=1 -g -Wall


#---[ Rules ]------------------------------------------------------------------

CXX=g++
LD=g++

DEPENDS=*.h ../include/*.h ../include/lib3270/*.h Makefile

$(OBJDBG)/%.o: %.cc $(DEPENDS)
	@echo "  CC   `basename $@`"
	@$(MKDIR) `dirname $@`
	@$(CXX) $(CFLAGS) $(DEBUG_CFLAGS) -o $@ -c $<


#---[ Debug targets ]----------------------------------------------------------

Debug: $(BINDBG)/testprogram$(EXEEXT)

run: $(BINDBG)/testprogram$(EXEEXT)
	$(BINDBG)/testprogram$(EXEEXT)

$(BINDBG)/testprogram$(EXEEXT): $(OBJDBG)/testprogram.o $(BINDBG)/$(LIBNAME)
	@echo "  CCLD `basename $@`"
	@$(MKDIR) `dirname $@`
	@$(LD) -o $@ $^ $(LIBS)

$(BINDBG)/$(LIBNAME): $(foreach SRC, $(basename $(SOURCES)), $(OBJDBG)/$(SRC).o)
	@echo "  CCLD `basename $@`"
	@$(MKDIR) `dirname $@`
	@ar rs $@ $^

clean:
	@rm -fr $(OBJDIR)
	@rm -fr $(BINDIR)
	@rm -f testprogram
	@find . -name "*~" -exec rm -f {} \;
