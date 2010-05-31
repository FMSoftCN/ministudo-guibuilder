/*
 * stream.cpp
 *
 *  Created on: 2009-3-26
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef WIN32
#include <stdint.h>
#endif
#include <map>

#include "mgheads.h"

using namespace std;

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

	char szText[1024];
	vsprintf(szText, format, args);
	int i, begin=0;
	for(i=0; szText[i]; i++)
	{
		if(szText[i] == '\r' || szText[i]=='\n')
		{
			bNewLine = true;
			storage->write(i-begin+1, szText+begin);
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
		storage->write(i-begin, szText+begin);
	}
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
