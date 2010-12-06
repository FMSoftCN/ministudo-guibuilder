/*
 * clipboard.h
 *
 *  Created on: 2009-5-4
 *      Author: dongjunjie
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
