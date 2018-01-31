
AM_CPPFLAGS= -I. -Iinclude

AUTOMAKE_OPTIONS=foreign

all:builder


SUBDIRS=include etc uieditor texteditor imgeditor mainframe mgfc rdreditor resideditor trans 

bin_PROGRAMS=guibuilder

SRCS=build_version.h mainframe/dlgtmpls.c
include common/src.mk
guibuilder_SOURCES = main.cpp func-win.cpp $(SRCS)

guibuilder_LDADD= mainframe/libmainframe.a \
	uieditor/libuieditor.a \
	texteditor/libtexteditor.a \
	texteditor/json/libjson.a \
	imgeditor/libimgeditor.a \
	rdreditor/librdreditor.a \
	resideditor/libresideditor.a \
	mgfc/libmgfc.a \
	trans/libintl.a 

EXTRA_DIST=

build_version.h:
	./rebuild-version

with-builder:builder all

builder:
	./rebuild-version

