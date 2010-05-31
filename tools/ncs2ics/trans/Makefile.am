
ETCDIR=$(prefix)/etc/guibuilder/tools/ncs2ics/trans

DataFiles = button.xml \
	checkbutton.xml \
	combobox.xml \
	edit.xml \
	groupbox.xml \
	hprogressbar.xml \
	hscrollbar.xml \
	hspinner.xml \
	htrackbar.xml \
	iconview.xml \
	image.xml \
	label.xml \
	listbox.xml \
	listview.xml \
	mainwnd.xml \
	menubutton.xml \
	mledit.xml \
	monthcal.xml \
	progressbar.xml \
	propsheet.xml \
	radiobutton.xml \
	scrollbar.xml \
	sledit.xml \
	spinbox.xml \
	spinner.xml \
	trackbar.xml \
	vprogressbar.xml \
	vscrollbar.xml \
	vspinner.xml \
	vtrackbar.xml \
	widget.xml


install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST=$(DataFiles)



