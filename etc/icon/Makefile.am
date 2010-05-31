
ETCDIR=$(prefix)/etc/guibuilder/icon

SUBDIRS=uieditor

DataFiles=animate.png \
	buttongroup.png \
	button.png \
	checkbutton.png \
	colorbutton.png \
	combobox.png \
	panel.png \
	cursor.png \
	dialog.png \
	frame.png \
	groupbox.png \
	hprogressbar.png \
	hscrollbar.png \
	hseparator.png \
	hspinner.png \
	htrackbar.png \
	iconview.png \
	image.png \
	ledlabel.png \
	listbox.png \
	listview.png \
	menubutton.png \
	mledit.png \
	monthcalender.png \
	container.png \
	progressbar.png \
	propsheet.png \
	radiobutton.png \
	rect.png \
	spinbox.png \
	static.png \
	textedit.png \
	timer.png \
	toolbar_ids.png \
	toolbar_image.png \
	toolbar_rdr.png \
	toolbar_text.png \
	toolbar_ui.png \
	treeview.png \
	vprogressbar.png \
	vscrollbar.png \
	vseparator.png \
	vspinner.png \
	vtrackbar.png \
	selector.png \
	gblogo.png \
	left_top.png

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST= $(DataFiles)
