/*
 * log.cpp
 *
 *  Created on: 2009-3-30
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

static FILE * logfile = stderr;

class Log
{
	FILE * fplog;
public:
	Log(const char* logfile=NULL){
		fplog = NULL;
		if(logfile)
			fplog = fopen(logfile,"wt");
		if(fplog == NULL)
			fplog = stderr;
	}

	~Log(){
		if(fplog && fplog != stderr)
			fclose(fplog);
	}

	int printf(const char* head,const char* format, va_list args)
	{
		int i = fprintf(fplog, "%s", head);
		return i + vfprintf(fplog, format, args);
	}
};

#define LOGFILE  ".guibuilder.log"
Log log(LOGFILE);

void log_warning(const char* format, ...)
{
	if(format == NULL)
		return;

	va_list args;
	va_start(args, format);
	log.printf("WARNING>> ", format, args);
}

void log_debug(const char* format, ...)
{
	if(format == NULL)
		return;

	va_list args;
	va_start(args, format);
	log.printf("DEBUG>> ", format, args);
}

void log_dead(const char* format, ...)
{
	if(format == NULL)
		return;

	va_list args;
	va_start(args, format);
	log.printf("DEAD>> ", format, args);
	throw("log dead");
}

