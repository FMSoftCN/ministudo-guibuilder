list(APPEND texteditor_srcs
    GTranslater.cpp 
	text-dlg.cpp 
	texteditor.cpp 
	textlistpanel.cpp 
	translater.cpp 
	GTranslater.h 
	text-dlg.h 
	texteditor.h 
	textlistpanel.h 
	translater.h
    )


mg_add_source_files(${texteditor_srcs})

mg_add_sub_source_dirs(json)

mg_commit_source_files()

