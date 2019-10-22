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

#ifndef RESENV_H_
#define RESENV_H_

//#include "gbconfig.h"
#include "guibuilder.h"

#define DEF_TEMPL_PATH ("uieditor/templates")
#define USR_TEMPL_PATH ("user-templates")

#define ID2TYPE(id)  ((int)(((id)&0xFFFF0000)>>16))
#define VALIDID(id,type)  ((id)>0 && (ID2TYPE(id)==type))
#define NCSRT_CONTRL 0x4000

//menu command macro
#define EDITOR_MENUCMD_BASE          100
#define EDITOR_MENUCMD_MAX           999

#define MSG_GUIBUILDER_BEGIN   MSG_USER + 7000
#define MSG_GUIBUILDER_END     MSG_GUIBUILDER_BEGIN + 500
#define MSG_NEED_SYS_IME MSG_GUIBUILDER_BEGIN + 1
//return 0, 1

class IDRangeManager;

class ResManager;

class IDRangeOwnerManager;

class ResEnumHandler
{
public:
	virtual ~ResEnumHandler(){}
	virtual void setRes(ResManager* resMgr,int type, int id, const char* name, DWORD res) = 0;
};

class ResManager
{
public:
	virtual ~ResManager(){}

	//test the ResManager support the type or not
	virtual int getTypeMask() = 0;

	virtual const char* getTypeName(int type) = 0;

	//create a Res
	virtual int createRes(int type, const char* name, int id, const char* source, DWORD init_res) = 0;
	//get res
	virtual DWORD getRes(int id) = 0;
	//set Res
	virtual BOOL setRes(int id, DWORD res) = 0;
	//add the use ref
	virtual int use(int id, BOOL auto_ref = FALSE) = 0;
	//unuse the res
	virtual int unuse(int id, BOOL bauto_ref=FALSE) = 0;
	//reset resName
	virtual int setResName(int id, const char* name, DWORD res = 0) = 0;
	//reset res id
	virtual BOOL setResId(int oldId, int newId, DWORD res = 0) = 0;
	//remove Res
	virtual BOOL removeRes(int id, DWORD res = 0) = 0;
	//call specall function
	//format : strName = funcName, [[ret], [arg1,[arg2, [...]]]
	virtual BOOL callSpecial(const char* strName, ...) = 0;
	//enum res
	virtual void enumRes(ResEnumHandler *handler) = 0;
	//id to name
	//name to id
	virtual const char* idToName(int id) = 0;
	virtual int nameToId(const char* name) = 0;
	virtual BOOL isValidName(const char* name) = 0;

	virtual int getAllIDRangeManagers(vector<IDRangeManager*> & mngrlist) = 0;
	virtual BOOL updateIDRangeManager(IDRangeManager* idrm) = 0;
};

class ResEditorEnv
{
public:
	virtual ~ResEditorEnv(){}

	virtual ResManager* getResManager(int type) = 0;

    virtual const char* getVersion() = 0;

	virtual int getAllResManager(ResManager*** resManagers) = 0;

	virtual const char* getResourcePath() = 0;
	virtual const char* getHeaderFile() = 0;
	virtual const char* getSourcePath() = 0;
	virtual const char* getProjectPath() = 0;
    virtual const char* getDefRdrName() = 0;
    virtual BOOL setDefRdrName(const char* rdrName) = 0;

	//update ResInfo
	virtual void updateResName(int id, const char* newName) = 0;
	virtual void updateResId(int oldId, int newId) = 0;
	virtual void updateResValue(int id) = 0;
	virtual void updateAllRes(int type) = 0;
	virtual void deleteRefRes(int id) = 0;
	//string pool
	virtual const char* getString(int id) = 0;
	virtual int stringToId(const char* str) = 0;
	virtual int addString(const char*str) = 0;
	virtual int removeString(int id) = 0;

	virtual int selectID(int type) = 0;

	//system config
	virtual const char* getSysConfig(const char* configName, const char* defaultName=NULL) = 0;
	virtual void saveAll() = 0;
	//endian config
	virtual int getEndian() = 0;

	virtual string getConfigFile(const char* strCfgName) = 0;

	//get menu , mapex(id, state)
	virtual HMENU createPopMenuFromConfig(int popmenu_id, mapex<int, int> id_state) = 0;

	virtual void setMainCaption(const char* caption) = 0;

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    virtual BOOL checkSoftDog(int* remainDay) = 0;
    virtual BOOL checkLimit() = 0;
    virtual BOOL isAuthMode() = 0;
    virtual string checkSNInfo() = 0;
#endif

	virtual BOOL newIDRange(IDRangeManager* idrm) = 0;

	virtual IDRangeOwnerManager* getIDRangeOwnerManager() = 0;
};
extern ResEditorEnv * g_env;

extern BOOL isFileExist(const char* file);

static inline BOOL GetAllIDRangeManagers(vector<IDRangeManager*> &list)
{
	ResManager** managers;
	int count;
	if((count = g_env->getAllResManager(&managers)) <= 0)
		return FALSE;

	for(int i =0 ; i < count; i ++)
	{
		managers[i]->getAllIDRangeManagers(list);
	}

	return TRUE;
}

#define MAP_MASK_STATE(mask, state) ((mask)<<16 | (state))
#define GET_MASK(value) (((value) & 0xFFFF0000) >> 16)
#define GET_STATE(value) ((value) & 0x0000FFFF)


#define GET_ARG(TArg, arg, va) TArg arg = va_arg(va, TArg)
#define GET_RET(TRet,va)  GET_ARG(TRet*, ret, va)
//void call
#define ARGS1(TArg1,va) GET_ARG(TArg1, arg1, va);
#define VCALL1(Func,TArg1, va) do { \
		ARGS1(TArg1, va) \
		Func(arg1); \
}while(0)

#define ARGS2(TArg1, TArg2, va) \
		ARGS1(TArg1, va) \
		GET_ARG(TArg2,arg2, va);
#define VCALL2(Func, TArg1, TArg2, va) do { \
		ARGS2(TArg1, TArg2,va) \
		Func(arg1, arg2); \
}while(0)

#define ARGS3(TArg1, TArg2, TArg3, va) \
	ARGS2(TArg1, TArg2, va) \
	GET_ARG(TArg3, arg3, va);
#define VCALL3(Func, TArg1, TArg2, TArg3, va) do { \
	ARGS3(TArg1, TArg2, TArg3, va) \
	Func(arg1, arg2, arg3); \
}while(0)

#define ARGS4(TArg1, TArg2, TArg3, TArg4, va) \
	ARGS3(TArg1, TArg2, TArg3, va) \
	GET_ARG(TArg4,arg4, va);
#define VCALL4(Func, TArg1, TArg2, TArg3, TArg4, va) do { \
	ARGS4(TArg1, TArg2, TArg3, TArg4, va) \
	Func(arg1, arg2, arg3, arg4); \
}while(0)

#define ARGS5(TArg1, TArg2, TArg3, TArg4, TArg5, va) \
	ARGS4(TArg1, TArg2, TArg3, TArg4, va) \
	GET_ARG(TArg5,arg5, va);
#define VCALL5(Func, TArg1, TArg2, TArg3, TArg4, TArg5, va) do { \
	ARGS5(TArg1, TArg2, TArg3, TArg4, TArg5, va) \
	Func(arg1, arg2, arg3, arg4, arg5); \
}while(0)

#define RCALL0(TFunc, TRet, va) do{ \
	GET_RET(TRet, va); \
	*ret = TFunc(); \
}while(0)

#define RCALL1(TFunc, TRet, TArg1, va) do { \
	GET_RET(TRet, va); \
	ARGS1(TArg1, va) \
	*ret = TFunc(arg1); \
}while(0)

#define RCALL2(TFunc, TRet, TArg1, TArg2, va) do { \
	GET_RET(TRet, va); \
	ARGS2(TArg1, TArg2, va); \
	*ret = TFunc(arg1, arg2); \
}while(0)

#define RCALL3(TFunc, TRet, TArg1, TArg2, TArg3, va) do { \
	GET_RET(TRet, va); \
	ARGS3(TArg1, TArg2, TArg3, va); \
	*ret = TFunc(arg1, arg2, arg3); \
}while(0)

#define RCALL4(TFunc, TRet, TArg1, TArg2,TArg3, TArg4, va) do { \
	GET_RET(TRet, va); \
	ARGS4(TArg1, TArg2, TArg3, TArg4, va); \
	*ret = TFunc(arg1, arg2, arg3, arg4); \
}while(0)

#define RCALL5(TFunc, TRet, TArg1, TArg2, TArg3, TArg4, TArg5, va) do { \
	GET_RET(TRet, va); \
	ARGS5(TArg1, TArg2, TArg3, TArg4, TArg5, va); \
	*ret = TFunc(arg1, arg2, arg3, arg4, arg5); \
}while(0)

#define CALL_FUNCTION(szName, func_name, CALL) \
	if(strcmp(szName, func_name) == 0){ \
		CALL; \
		return TRUE;\
	}


///////////////////////////////////////////////////////////
// id range manager

class IDRangeOwner;

class IDRange
{
protected:
	enum {
		COMMON = 0x1,
		NEWIDRANGE = 0x02
	};
	unsigned short used;
	unsigned short flags;

public:
	unsigned short min, max;

	IDRange(int min, int max);

	~IDRange(){ }

	BOOL isCommon(){  return (flags&COMMON) == COMMON; }
	void operator++() { if(used == 0xFFFF)used =0;  used ++; }
	void operator--() { if(used == 0)used = 0xFFFF; used --; }

	BOOL isNewIDRange(){ return flags&NEWIDRANGE; }
	void setNewIDRange(BOOL isNew){ if(isNew) flags |= NEWIDRANGE; else flags &= ~NEWIDRANGE;}

	BOOL extend(int min, int max);

	void setUsed(int used){
		this->used = used;
		setNewIDRange(FALSE);
	}

	int getUsed(){ return used; }

    int unusedCount() { return used - (max - min); }

	BOOL isInRange(int id)
	{
		return id > min && id <= max;
	}

public:
	IDRange * next, *prev; 
	IDRange * next_slibing, *prev_slibing;
	IDRangeOwner * owner;
	IDRangeManager * manager;
};

class IDRangeOwner
{
public:
	IDRangeOwner();

	string    name;
	IDRange * header;

	BOOL isInRange(int id);

	void addIDRange(IDRange* idrange);
};

class IDRangeOwnerManager
{
public:
	virtual ~IDRangeOwnerManager() { }

	virtual IDRangeOwner * getOwner(const char* name) = 0;
	virtual IDRangeOwner * getCurOwner() = 0;
	virtual IDRangeOwner * newOwner(const char* name) = 0;
	virtual int getAllOwners(IDRangeOwner***) = 0;
};

class IDRangeInfo
{
public:
	unsigned short min, max;
	IDRangeInfo(int min, int max){
		this->min = min&0xFFFF;
		this->max = max&0xFFFF;
	}

	IDRangeInfo(const IDRangeInfo & iri){
		this->min = iri.min;
		this->max = iri.max;
	}
};

typedef vector<IDRangeInfo> IDRangeInfoList;

class IDRangeManager
{
protected:
	string name;
	int    type;
	unsigned short limit_min, limit_max;
	IDRange * header;

	int next_id;
	IDRange* cur_idrange;

	ResManager * resManager;

	static void includeIDRange(IDRangeInfoList& list, int min, int max);
	static void excludeIDRange(IDRangeInfoList& list, int min, int max);

	IDRange * getIDRangeById(int id);
public:
	IDRangeManager(ResManager* resManager, int type, const char* name=NULL, int limit_min=0, int limit_max=65535);
	~IDRangeManager();

	BOOL getFreeIDRanges(IDRangeInfoList & list);
	static BOOL andIDRanges(IDRangeInfoList& target, IDRangeInfoList& list1, IDRangeInfoList& list2);

	BOOL useId(int id);
	BOOL unuseId(int id);

	const char* getName(){
		return name.c_str();
	}

	const char* getTypeName(){
		return resManager?resManager->getTypeName(type):NULL;
	}

	void setName(const char* name){
		if(name)
			this->name = name;
	}

	int getType(){ return type; }
	void setType(int type) { this->type = type; }

	IDRange* getHeader(){ return header; }

	int nextId(IDRangeOwner* owner);

	int getLimitMin(){ return limit_min; }
	int getLimitMax(){ return limit_max; }

	IDRange* addIDRange(int min, int max, IDRangeOwner* owner);

	BOOL updateNewIDRanges()
	{
		return resManager && resManager->updateIDRangeManager(this);
	}

	class IDNewRangeIterator {
		friend class IDRangeManager;

		IDRange* current;

		void findNewIDrange(){
			while(current)
			{
				if(current->isNewIDRange())
					break;
				current = current->next;
			}
		}

		IDNewRangeIterator(IDRange* header){
			this->current = header;
			findNewIDrange();
		}


	public:
		IDNewRangeIterator(const IDNewRangeIterator & idnri){
			current = idnri.current;
			findNewIDrange();
		}

		IDNewRangeIterator& operator=(const IDNewRangeIterator& idnri){
			current = idnri.current;
			findNewIDrange();
            return *this;
		}

		BOOL isEnd(){ return current == NULL ; }

		BOOL operator!(){
			return current == NULL;
		}

		IDNewRangeIterator& operator++(){
			current = current->next;
			findNewIDrange();
			return *this;
		}

		IDRange* operator->(){
			return current;
		}

		IDRange& operator*(){
			if(current == NULL)
				throw("IDRange is NULL");
			return *current;
		}
	};

	IDNewRangeIterator getNewIDRanges(){
		return IDNewRangeIterator(header);
	}
};

#endif /* RESENV_H_ */
