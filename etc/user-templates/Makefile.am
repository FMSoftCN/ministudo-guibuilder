
ETCDIR=$(prefix)/etc/guibuilder/user-templates

DataFiles= file_select.bmp file_select.tmpl phone_app.bmp phone_app.tmpl

EXTRA_DATA=$(DataFiles)

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi

	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)

