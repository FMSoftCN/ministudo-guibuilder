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
