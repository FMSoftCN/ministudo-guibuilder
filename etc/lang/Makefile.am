ETCDIR=$(prefix)/etc/guibuilder/lang

DataFiles=*.mo

EXTRA_DIST=$(DataFiles)

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi

	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)

