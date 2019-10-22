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
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

#include "clipboard.h"

#define MIN_SIZE  32

#define IPC_MALLOC(size) calloc(1,size)
#define IPC_FREE(p) free(p)

ClipBoard::ClipBoard(size_t size/*=DEFAULT_SIZE*/)
{
	//create memoery
	if(size < MIN_SIZE)
		size = MIN_SIZE;

	//TODO IPC Malloc
	mem = (ClipMemInfo*)IPC_MALLOC(size+sizeof(ClipMemInfo));
	mem->buffer_size = size;
	mem->top = mem->buffer_size - 1;
	pthread_mutex_init(&mutex,NULL);
}

ClipBoard::~ClipBoard()
{
	if(mem)
		IPC_FREE(mem);
	pthread_mutex_destroy(&mutex);
}

size_t ClipBoard::push(void* data, size_t size)
{
	if(!mem || !data || size < 0)
		return 0;

	//get free data from
	if(size + sizeof(size_t) > mem->buffer_size)
		return 0; //out of memory

	lock();

	mem->top = (mem->top - size - sizeof(size_t) + mem->buffer_size) % mem->buffer_size;

	DataInfo* di = (DataInfo*)(mem->buffer + mem->top);

	di->size = size;

	//copy data
	size_t cpy_size = mem->buffer_size - mem->top - sizeof(int);
	if(cpy_size > size)
		cpy_size = size;
	memcpy(di->data, data, cpy_size);

	int left_size = size - cpy_size;
	if(left_size > 0)
		memcpy(mem->buffer, (char*)data+cpy_size, left_size);

	unlock();

	return size;
}

void ClipBoard::pop(int idx/*=0*/)
{
	if(!mem)
		return;

	if(idx < 0)
		idx = 0;

	lock();

	unsigned char* buff = mem->buffer + mem->top;
	unsigned char* buff_prev = buff;

	int btest = 0;

	while(idx >= 0 && ((btest && buff < buff_prev) || !btest))
	{
		DataInfo *di = (DataInfo*)buff;
		if(di->size == 0)
			break;

		buff = buff + di->size + sizeof(int);

		if(buff >= mem->buffer + mem->buffer_size)
			 buff -= mem->buffer_size;

		di->size = 0;

		idx --;
	}

	mem->top = buff - mem->buffer;

	unlock();
}

size_t ClipBoard::top(void* data/*=NULL*/,int idx/*=0*/)
{
	if(!mem)
		return 0;

	if(idx < 0)
		idx = 0;

	lock();

	//find the check by idx
	unsigned char* buff = mem->buffer + mem->top;
	unsigned char* buff_prev = buff;

	int btest = 0;

	while(idx > 0 && ((btest && buff < buff_prev) || !btest))
	{
		DataInfo *di = (DataInfo*)buff;
		if(di->size == 0)
			break;

		buff = buff + di->size + sizeof(int);

		if(buff >= mem->buffer + mem->buffer_size){
			 buff -= mem->buffer_size;
			 btest = 1;
		}

		idx --;
	}

	DataInfo *di = (DataInfo*)buff;

	if(btest && (buff + di->size + sizeof(int)) > buff_prev)
	{
		unlock();
		return 0;
	}

	if(!data){
		unlock();
		return di->size;
	}

	size_t cpy_size = mem->buffer + mem->buffer_size - buff - sizeof(int);
	if(cpy_size > di->size)
		cpy_size = di->size;
	memcpy(data, di->data, cpy_size);
	if(cpy_size < di->size)
		memcpy((char*)data + cpy_size, mem->buffer, di->size - cpy_size);

	unlock();

	return di->size;
}

