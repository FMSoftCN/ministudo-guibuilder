
ETCDIR=$(prefix)/etc/guibuilder/uieditor/trans

DataFiles=dlgtmpl.lua ncs-ctmpl.lua ncs-merge.lua ncs-wndhead.lua minigui-cfg.lua

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST =$(DataFiles) 
