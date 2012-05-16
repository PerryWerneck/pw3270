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
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include ../src/lib3270/sources.mak

LOCAL_CFLAGS=-I../src/include
LOCAL_DEFAULT_CPP_EXTENSION := cpp
LOCAL_MODULE    := lib3270
LOCAL_LDLIBS 	:= -llog
LOCAL_SRC_FILES := $(foreach SRC, $(TERMINAL_SOURCES) $(NETWORK_SOURCES), ../../src/lib3270/$(SRC)) \
					main.cpp misc.cpp

include $(BUILD_SHARED_LIBRARY)

