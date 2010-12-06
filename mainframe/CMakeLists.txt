list (APPEND mainframe_srcs
    mainframe.cpp 
	menu-manager.cpp 
	select-project-type-dlg.cpp  
	mainframe.h 
	menu-manager.h 
	select-project-type-dlg.h 
	customial.c  
	idrange-editor.h idrange-editor.cpp 
	newidrange.h newidrange.cpp 
	extendidrange.h extendidrange.cpp
    dlgtmpls.c
    )

mg_add_source_files(${mainframe_srcs})

mg_commit_source_files()
