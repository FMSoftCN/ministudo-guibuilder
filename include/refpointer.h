/*
 * refpointer.h
 *
 *  Created on: 2009-3-24
 *      Author: dongjunjie
 */

#ifndef REFPOINTER_H_
#define REFPOINTER_H_

template<typename T>
class TRefPointer
{
protected:
	T *ptr;

public:
	TRefPointer(T* ptr){
		this->ptr = ptr;
		addRef();
	}

	TRefPointer(TRefPointer& refp){
		ptr = refp->ptr;
		addRef();
	}

	static T* newObject(const T* pstr, int size){
		if(pstr == NULL || size<=0)
			return NULL;
		T* ptr = new T[size+1];
		ptr ++;
		ptr[-1] = 0;
		memcpy(ptr, pstr, sizeof(T)*size);
	}

	T* operator->(){
		return ptr;
	}

	T& operator*(){
		return *ptr;
	}

	operator T*(){
		return ptr;
	}

	int addRef(){
		if(!ptr)
			return 0;
		return ++ptr[-1];
	}

	int release(){
		if(!ptr)
			return 0;
		if(--ptr[-1] == 0)//release
			delete[] &ptr[-1];
	}
};


#endif /* REFPOINTER_H_ */
