
ETCDIR=$(prefix)/etc/guibuilder

DataFiles = mainframe.cfg MiniGUI.cfg guibuilder.cfg.sh \
			imgeditor.layout rdreditor.layout resid.layout \
			texteditor.layout uieditor.layout language.cfg \
			lang.cfg.sh


MV=mv

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)
	chmod a+x $(ETCDIR)/guibuilder.cfg.sh
	chmod a+x $(ETCDIR)/lang.cfg.sh
	$(ETCDIR)/guibuilder.cfg.sh $(ETCDIR) > $(ETCDIR)/guibuilder.cfg
	$(ETCDIR)/lang.cfg.sh $(ETCDIR) > $(ETCDIR)/lang.cfg
	$(RM) $(ETCDIR)/guibuilder.cfg.sh
	$(RM) $(ETCDIR)/lang.cfg.sh
	chmod a+rw $(ETCDIR)/user-templates

SUBDIRS=Emulator icon msd renderer uieditor user-templates lang


EXTRA_DIST= $(DataFiles)

