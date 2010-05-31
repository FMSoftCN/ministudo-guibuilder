
ETCDIR=$(prefix)/etc/guibuilder/uieditor/templates

DataFiles= dialog2.png \
	dialog2.tmpl \
	dialog3.png \
	dialog3.tmpl \
	dialog.png \
	dialog.tmpl \
	empty_window.png \
	empty_window.tmpl \
	groupbox.png \
	groupbox.tmpl \
	propsheet.png \
	propsheet.tmpl 

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)


EXTRA_DIST=$(DataFiles)

