DataFiles= animate.xml \
	buttongroup.xml \
	button.xml \
	checkbutton.xml \
	colorbutton.xml \
	combo.xml \
	component.xml \
	panel.xml \
	dialog.xml \
	edit.xml \
	groupbox.xml \
	hprogressbar.xml \
	hscrollbar.xml \
	hseparator.xml \
	hspinner.xml \
	htrackbar.xml \
	iconview.xml \
	image.xml \
	itemview.xml \
	ledlabel.xml \
	listbox.xml \
	listview.xml \
	mainwnd.xml \
	menubutton.xml \
	mledit.xml \
	monthcalendar.xml \
	page.xml \
	container.xml \
	progressbar.xml \
	propsheet.xml \
	radiobutton.xml \
	rect.xml \
	scrollbar.xml \
	scrollview.xml \
	scrollwidget.xml \
	separator.xml \
	sledit.xml \
	slider.xml \
	spinbox.xml \
	spinner.xml \
	static.xml \
	timer.xml \
	trackbar.xml \
	vprogressbar.xml \
	vscrollbar.xml \
	vseparator.xml \
	vspinner.xml \
	vtrackbar.xml \
	texteditor.xml \
	abstractlist.xml \
	list.xml \
	widget.xml  \
	imwordsel.xml

ETCDIR=$(prefix)/etc/guibuilder/renderer/skin

install-data-local:
	if [ ! -d $(ETCDIR) ]; then $(INSTALL) -d $(ETCDIR); fi


	$(INSTALL_DATA) $(DataFiles) $(ETCDIR)



EXTRA_DIST=$(DataFiles)

