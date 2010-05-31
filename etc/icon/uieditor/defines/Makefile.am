
ETCDIR=$(prefix)/etc/guibuilder/icon/uieditor/defines

DataFiles=component_timer.png

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST=$(DataFiles)

