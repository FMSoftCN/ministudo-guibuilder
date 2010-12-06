list(APPEND imgeditor_srcs
    dirimgview.cpp 
	dirtreepanel.cpp 
	imageview.cpp 
	imgeditor.cpp 
	imgres-listpanel.cpp 
	resdescpanel.cpp 
	resimgview.cpp 
	dirimgview.h 
	dirtreepanel.h 
	imageview.h 
	imgeditor.h 
	img-event-id.h 
	imgres-listpanel.h 
	resdescpanel.h 
	resimgview.h
    )

mg_add_source_files(${imgeditor_srcs})
mg_commit_source_files()

