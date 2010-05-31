
ETCDIR=$(prefix)/etc/guibuilder/renderer

DataFiles=rdrlist

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)


SUBDIRS=classic fashion flat skin

EXTRA_DIST=$(DataFiles)

