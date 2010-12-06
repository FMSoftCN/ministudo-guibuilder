list (APPEND resideditor_srcs
    resid-editor.cpp 
	resid-list-panel.cpp 
	resid-editor.h 
	resid-list-panel.h
    )


mg_add_source_files(${resideditor_srcs})
mg_commit_source_files()

