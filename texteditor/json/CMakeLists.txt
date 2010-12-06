list (APPEND json_srcs
     arraylist.c 
	arraylist.h 
	bits.h 
	config.h 
	debug.c 
	debug.h 
	json.h 
	json_object.c 
	json_object.h 
	json_object_private.h 
	json_tokener.c 
	json_tokener.h 
	json_util.c 
	json_util.h 
	linkhash.c 
	linkhash.h 
	printbuf.c 
	printbuf.h
    )

mg_add_source_files(${json_srcs})
mg_commit_source_files()
