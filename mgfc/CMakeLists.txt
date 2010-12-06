
list (APPEND mgfc_srcs 
    fixedlayout.c 
	gridlayout.c 
	mgapp.cpp 
	mgctrl.cpp 
	mgwnd.cpp 
	fixedlayout.h 
	gridlayout.h 
	mgapp.h 
	mgcomm.h 
	mgctrl.h 
	mgwnd.h
    )

mg_add_source_files(${mgfc_srcs})

mg_commit_source_files()


