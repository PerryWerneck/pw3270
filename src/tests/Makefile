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

include ../include/rules.mak
include ../lib3270/sources.mak

DEBUG_CFLAGS=-DDEBUG=1 -g -Wall
LIBS=-lm  -lssl -lcrypto

#---[ Rules ]------------------------------------------------------------------

$(OBJDBG)/%.o: %.c
	@echo "  CC   `basename $@`"
	@$(MKDIR) `dirname $@`
	@$(CC) $(DEBUG_CFLAGS) $(CFLAGS) $(SSL_CFLAGS) -I../include -I../lib3270 -o $@ -c $<


$(OBJDBG)/%.o: ../lib3270/%.c
	@echo "  CC   `basename $@`"
	@$(MKDIR) `dirname $@`
	@$(CC) $(DEBUG_CFLAGS) $(CFLAGS) $(SSL_CFLAGS) -I../include -I../lib3270 -o $@ -c $<

#---[ Tags ]-------------------------------------------------------------------

Debug: $(BINDBG)/rpqtest$(EXEEXT)

$(BINDBG)/rpqtest$(EXEEXT): $(OBJDBG)/rpqtest$(OBJEXT) $(foreach SRC, $(basename $(SOURCES)), $(OBJDBG)/$(SRC)$(OBJEXT))
	@echo "  CCLD `basename $@`"
	@$(MKDIR) `dirname $@`
	@$(LD) -o $@ $(OBJDBG)/rpqtest$(OBJEXT) $(foreach SRC, $(basename $(SOURCES)), $(OBJDBG)/$(SRC)$(OBJEXT)) $(LIBS)

cleanDebug:
	@rm -fr $(OBJDBG)
	@rm -fr $(BINDBG)

clean:
	@rm -fr $(OBJDBG)
	@rm -fr $(BINDBG)
