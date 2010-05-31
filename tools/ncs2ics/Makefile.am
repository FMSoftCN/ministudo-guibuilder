
TOP=../..

ETCDIR=$(prefix)/etc/guibuilder/tools/ncs2ics

INCLUDES=-I. -I.. -I$(TOP)/include

AUTOMAKE_OPTIONS=foreign

SUBDIRS=trans

LIBS=-lxml2
LDFLAGS=$(LIBS)
bin_PROGRAMS=ncs2ics
ncs2ics_SOURCES=trans.cpp trans.h  main.cpp $(TOP)/common/xmlhelp.cpp

ncs2ics_LDFLAGS=

DataFiles=trans.list

EXTRA_DIST=$(DataFiles)

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



