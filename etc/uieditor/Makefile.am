
ETCDIR=$(prefix)/etc/guibuilder/uieditor

DataFiles=compund-type.def control.cfg mobile.cfg ctrllist

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)


SUBDIRS=defines templates trans

EXTRA_DIST=$(DataFiles)

