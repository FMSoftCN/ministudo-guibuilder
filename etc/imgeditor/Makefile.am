ETCDIR=$(prefix)/etc/guibuilder/imgeditor

DataFiles=default_img.png

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi

	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)

.NOEXPORT:

EXTRA_DIST=$(DataFiles)

