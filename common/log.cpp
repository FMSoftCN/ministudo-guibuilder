/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

