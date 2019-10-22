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

#ifndef CLIPBOARD_H_
#define CLIPBOARD_H_

#define DEFAULT_SIZE 1024
class ClipBoard
{
protected:
	struct DataInfo{
		size_t size; //by bytes
		unsigned char data[1];
	};

	struct ClipMemInfo{
		size_t buffer_size;
		int top;
		unsigned char buffer[1];
	};

	ClipMemInfo* mem;

	pthread_mutex_t mutex;

	void lock(){
		//TODO lock memory
		pthread_mutex_lock(&mutex);
	}
	void unlock(){
		//TODO unlock memory
		pthread_mutex_unlock(&mutex);
	}

public:
	ClipBoard(size_t size=DEFAULT_SIZE);
	virtual ~ClipBoard();

	size_t push(void* data, size_t size);
	void pop(int idx=0);
	size_t top(void* data=NULL,int idx=0);

};

template<class TPtr>
class ObjectClipBoard
{
protected:
	TPtr *objects;
	int max_count;
	int obj_top;
public:

	ObjectClipBoard(int max_count = 32){
		if(max_count < 8)
			max_count = 8;
		this->max_count = max_count;
		obj_top = max_count;
		objects = new TPtr[max_count];
		memset(objects, 0, max_count*sizeof(TPtr));
	}
	~ObjectClipBoard(){
		if(objects){
			for(int i=0; i<max_count; i++)
			{
				if(objects[i])
					objects[i]->release();
			}
			delete[] objects;
		}
	}

	template<class TPtrA>
	class TArray{
		friend class ObjectClipBoard<TPtrA>;
		TPtrA *ptrs;
		int  max;
		int  top;
		TArray(TPtrA *ptrs, int max, int top){
			this->ptrs = ptrs;
			this->max = max;
			this->top = top;
		}
	public:
		TArray() {
			ptrs = NULL;
			max = 0;
			top = 0;
		}
		TArray(const TArray<TPtrA>& a) {
			ptrs = a.ptrs;
			max = a.max;
			top = a.top;
		}

		bool operator!() {
			return ptrs == NULL;
		}

		operator bool(){
			return ptrs != NULL;
		}

		TPtrA operator[](int idx) {
			int i = (top + idx) % max;
			return ptrs[i];
		}
	};

	typedef TArray<TPtr> Array;

	Array top(int idx=0){
		if(idx < 0)
			idx = 0;
		if(obj_top == max_count)
			return Array();

		int i = obj_top;

		while(idx > 0)
		{
			if(objects[i] == NULL)
				idx --;
			i ++;
			if(i>=max_count)
				i = 0;
			if(i == obj_top)
				break;
		}

		if(idx != 0 || objects[i] == NULL)
			return Array();

		return Array(objects, max_count, i);
	}

	int push(TPtr* objs, int count)
	{
		int i;
		if(objs == NULL || count <= 0)
			return 0;

		if(count >= max_count-1)
			return 0; //out of memory

		i = 0;
		obj_top = obj_top - 1;
		if(obj_top <= 0)
			obj_top = max_count - 1;
		while(i<=count){
			if(objects[obj_top])
				objects[obj_top]->release();
			objects[obj_top] = NULL;
			obj_top --;
			if(obj_top < 0)
				obj_top = max_count - 1;
			i ++;
		}

		int top = obj_top;
		for(int i=0; i<count; i++)
		{
			objects[top++] = objs[i]->clone();
			if(top >= max_count)
				top = 0;
		}

		return count;

	}

	void pop()
	{
		if(obj_top == max_count || obj_top < 0)
			return ;

		do{
			if(!objects[obj_top])
				break;
			objects[obj_top]->release();
			objects[obj_top] = NULL;
			obj_top ++;
			if(obj_top >= max_count)
				obj_top = 0;
		}while(1);
	}
};

#endif /* CLIPBOARD_H_ */
