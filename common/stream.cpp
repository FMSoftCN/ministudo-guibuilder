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
#ifndef WIN32
#include <stdint.h>
#else
#include "func-win.h"
#endif
#include <map>

#include "mgheads.h"

using namespace std;

#include <string>
#include "stream.h"

//////////////////////
//TextStream
void TextStream::vprintf(const char* format, va_list args)
{
	if(storage == NULL || format == NULL)
		return;

	int prelen = strlen(prefix);
	if(bNewLine){
		bNewLine = false;
		storage->write(prelen,prefix);
	}

	//char strp[1024*4];
	char *strp = NULL;

	vasprintf(&strp, format, args);
	if(!strp)
		return;

	int i, begin=0;
	for(i=0; strp[i]; i++)
	{
		if(strp[i] == '\r' || strp[i]=='\n')
		{
			bNewLine = true;
			storage->write(i-begin+1, strp+begin);
			begin = i+1;
		}
		else
		{
			if(bNewLine){
				bNewLine = false;
				storage->write(prelen, prefix);
			}
		}
	}
	if(begin < i)
	{
		storage->write(i-begin, strp+begin);
	}
	free(strp);
}


#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
BOOL isFileExist(const char* file)
{
	struct stat buf;
	if(file == NULL)
		return FALSE;

	return stat(file,&buf) == 0;
}

#if 0
//////////////////////////////
//StrPoolBinStream

StrPoolBinStream::~StrPoolBinStream()
{
	flush();
	while(pools){
		StrPool *pool = pools;
		pools = pools->next;
		delete pool;
	}
	pools = NULL;
}

void StrPoolBinStream::flush()
{
	if(storage == NULL)
		return ;

	BinStream::flush();
	StrPool * pool = pools;
	while(pool)
	{
		storage->write(pool->size, pool->pool);
		pool = pool->next;
	}
}

void StrPoolBinStream::saveStr(const char* str, int size)
{
	int len;
	int offset;
	if(str == NULL || size <= 0 || (len = strlen(str))==0)
		return ;


	DWORD key = Str2Key(str);

	if(strKeyInfo.count(key) == 0) //not in the class
	{
		offset = totalOffset;
		//insert into the pool
		if(curPool == NULL){
			pools = new StrPool;
			pools->next = NULL;
			pools->size = 0;
			curPool = pools;
		}
		//try to insert
		int len = strlen(str)+1;
		int size = (len+3) & 0xFFFFFFFC; //alignment at 4

		totalOffset += size;

		while(1){
			int copyed = sizeof(curPool->pool) - curPool->size;
			if(copyed > size)
				copyed = size;
			memcpy(curPool->pool+curPool->size, str, len);
			if(copyed > len)
				memset(curPool->pool+curPool->size + len, 0, copyed-len);

			curPool->size += copyed;
			size -= copyed;
			if(size > 0){
				//create new pool
				curPool->next = new StrPool;
				curPool = curPool->next;
				curPool->size = 0;
				curPool->next = NULL;
			}
		}
		//save offset
		strKeyInfo[key] = offset;
	}
	else
	{
		offset = strKeyInfo[key];
	}

	//save the offset
	save32(offset);
}
#endif

#define SET_ENTITY_REFERENCE(er) \
{ strcpy(buff + idx, er); idx += (sizeof(er) - 1); }
std::string EntityReferenceTranslate(const char* str)
{
	char buff[1024];
	if(str == NULL)
		return std::string("");

	std::string xstr = "";
	
	int i;
	int idx = 0;
	for(i = 0; str[i]; i ++)
	{
		if(str[i] == '&')
			SET_ENTITY_REFERENCE("&amp;")
		else if(str[i] == '\'')
			SET_ENTITY_REFERENCE("&apos;")
		else if(str[i] == '\"')
			SET_ENTITY_REFERENCE("&quot;")
		else if(str[i] == '<')
			SET_ENTITY_REFERENCE("&lt;")
		else if(str[i] == '>')
			SET_ENTITY_REFERENCE("&gt;")
		else
			buff[idx++] = str[i];
		if(idx >= sizeof(buff)-1)
		{
			buff[idx] = 0;
			xstr += buff;
			idx = 0;
		}
	}

	buff[idx] = 0;
	xstr += buff;

	return xstr;
}


