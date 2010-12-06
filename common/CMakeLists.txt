list (APPEND common_srcs 
    class-instance.cpp 
	clipboard.cpp 
	codecombin.cpp 
	defundo-redo-observer.cpp 
	editable-listview.cpp 
	log.cpp 
	luahlp.c 
	panellayout.cpp 
	reseditor.cpp 
	socket-client.cpp 
	stream.cpp 
	valuetype.cpp 
	xmlhelp.cpp
    )

mg_add_source_files(${common_srcs})

mg_commit_source_files()

