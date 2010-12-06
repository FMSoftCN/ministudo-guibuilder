
list (APPEND include_hdrs
	class-instance.h 
	clipboard.h 
	codecombin.h 
	defundo-redo-observer.h 
	editable-listview.h 
	log.h 
	luahlp.h 
	mapex.h 
	mgfcheads.h 
	mgheads.h 
	panel.h 
	panellayout.h 
	refpointer.h 
	reseditor.h 
	resenv.h 
	socket-client.h 
	stream.h 
	undo-redo.h 
	valuetype.h 
	xmlheads.h 
	xmlhelp.h 
	msd_intl.h
    )

mg_add_source_files(${include_hdrs})

mg_commit_source_files()

