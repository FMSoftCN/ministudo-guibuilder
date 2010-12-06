list(APPEND rdreditor_srcs
    rdr-dialog.cpp 
	rdreditor.cpp 
	rdr-instance.cpp 
	rdr-preview-panel.cpp 
	rdrtreepanel.cpp 
	rdr-dialog.h 
	rdreditor.h 
	rdr-event-id.h 
	rdr-instance.h 
	rdr-preview-panel.h 
	rdrtreepanel.h 
    )

mg_add_source_files(${rdreditor_srcs})
mg_commit_source_files()

