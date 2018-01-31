/*
 * component-instance.cpp
 *
 *  Created on: 2009-3-27
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include <mgncs/mgncs.h>

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"

ComponentInstance::ComponentInstance(Class* cls)
:Instance(cls)
{
	// TODO Auto-generated constructor stub
	hwnd = HWND_INVALID;
	parent = children = prev = next = NULL;

	prevWndState = 0;

}

ComponentInstance::~ComponentInstance() {
	destroyPreviewWindow();

	//delete all the children
	ComponentInstance* child = children;
	while(child)
	{
		ComponentInstance * in = child;
		child=child->next;
		in->release();
	}
}


void ComponentInstance::setLocation(int x, int y)
{
	DWORD mask = getBoundMask();
	if(mask&BOUND_MASK_X)
		setField(PropX, (Value)x);
	if(mask&BOUND_MASK_Y)
		setField(PropY, (Value)y);
}

void ComponentInstance::getLocation(int&x, int &y)
{
	x = (int)getField(PropX);
	y = (int)getField(PropY);
}

void ComponentInstance::setSize(int cx, int cy)
{
	DWORD mask = getBoundMask();
	if(mask&BOUND_MASK_WIDTH)
		setField(PropWidth, (Value)cx);
	if(mask&BOUND_MASK_HEIGHT)
		setField(PropHeight, (Value)cy);
}

void ComponentInstance::getSize(int &cx, int &cy)
{
	cx = (int)getField(PropWidth);
	cy = (int)getField(PropHeight);
}

DWORD ComponentInstance::getBoundMask()
{
	DWORD boundMask = 0;
	if(!(getFieldAttr(PropX) &(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE)))
		boundMask |= BOUND_MASK_X;

	if(!(getFieldAttr(PropY) &(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE)))
		boundMask |= BOUND_MASK_Y;

	if(!(getFieldAttr(PropWidth) &(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE)))
		boundMask |= BOUND_MASK_WIDTH;

	if(!(getFieldAttr(PropHeight) &(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE)))
		boundMask |= BOUND_MASK_HEIGHT;

	return boundMask;
}

BOOL ComponentInstance::insert(ComponentInstance* insert, BOOL bAutoCreate)
{
	if(insert == NULL || !isContainer())
		return FALSE;

	//try to find it in the instance?
	for(ComponentInstance* in = children; in; in = in->next)
	{
		if(in == insert)
			return TRUE;
	}
#if 0
	//insert at begin
	insert->next = children;
	insert->prev = NULL;
	insert->parent = this;
	if(children)
		children->prev = insert;
	children = insert;
#else
	//for tab position, should insert at end of list
	insert->next = NULL;
	insert->prev = NULL;
	insert->parent = this;

	if (children == NULL)
		children = insert;
	else {
		//get end of instance
		ComponentInstance* in = children;

		while(in->next) {
			in = in->next;
		}
		insert->prev = in;
		in->next = insert;
	}

#endif
	//insert->addRef();
	//insert
	if(bAutoCreate && IsWindow(hwnd))
		insert->createPreviewWindow();

	notifyInsertInstance(insert);
	return TRUE;
}

BOOL ComponentInstance::remove(ComponentInstance* remove, BOOL bAutoDestroy)
{
	if(remove == NULL || !isContainer())
		return FALSE;

	if(remove->parent == this){
		if(remove->prev)
			remove->prev->next = remove->next;
		if(remove->next)
			remove->next->prev = remove->prev;
		if(remove == children)
			children = remove->next;

		//destroy window
		if(bAutoDestroy)
			remove->destroyPreviewWindow();

		notifyRemoveInstance(remove);
		remove->notifyRemoveFromParent();
		return TRUE;
	}

	return FALSE;
}

void ComponentInstance::destroyPreviewWindow()
{
	for(ComponentInstance* inst = children; inst ; inst = inst->next)
	{
		inst->destroyPreviewWindow();
	}
}

void ComponentInstance::resetPreviewHandler()
{
	hwnd = HWND_INVALID;
	for(ComponentInstance* inst = children; inst ; inst = inst->next)
	{
		inst->resetPreviewHandler();
	}
}

int ComponentInstance::hittest(int x, int y)
{
	if(IsWindow(hwnd) && IsWindowVisible(hwnd))
	{
		//RECT rc;
	//	GetWindowRect(hwnd, &rc);
	//	if(!PtInRect(&rc, x, y))
	//		return OUT;
	LRESULT hitcode = ::SendMessage(hwnd, MSG_HITTEST, x,y);
	if(hitcode == HT_UNKNOWN || hitcode == HT_OUT || hitcode == HT_TRANSPARENT)
		return OUT;

		if(isContainer())
		{
/*			x -= rc.left;
			y -= rc.top;
			WindowToClient(hwnd, &x, &y);
			GetClientRect(hwnd, &rc);
			if(PtInRect(&rc, x, y))*/
				return CONTAINER;
//			return REQ_MOUSE_AREA;
		}
		return IN;
	}

	return OUT;
}

LRESULT ComponentInstance::processMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(hwnd, message, wParam, lParam);
}

ComponentInstance* ComponentInstance::cloneChildren(ComponentInstance * newParent) const
{
	//copy children
	ComponentInstance* newChildren = NULL;
	ComponentInstance * myinst = NULL;
	for(ComponentInstance * chilinst = children; chilinst; chilinst = chilinst->next)
	{
		if(myinst == NULL){
			myinst = static_cast<ComponentInstance*>(chilinst->clone());
			newChildren = myinst;
		}
		else{
			myinst->next = static_cast<ComponentInstance*>(chilinst->clone());
			myinst->next->prev = myinst;
			myinst = myinst->next;
		}

		myinst->parent = newParent;
	}
	return newChildren;
}

void ComponentInstance::updatePrevWindow(BOOL updateFlag)
{
	if(!IsWindow(hwnd))
		return;

	setPreviewWindowUpdateFlag(TRUE);

/*	MSG Msg;
	if (updateFlag)
		//::UpdateWindow(hwnd, TRUE);
		InvalidateRect(hwnd,NULL, TRUE);

	while(PeekMessageEx(&Msg, hwnd, MSG_PAINT, MSG_PAINT+1, FALSE, PM_REMOVE)){
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
*/
	::UpdateWindow(hwnd, TRUE);
	//update children
	for(ComponentInstance * child=children; child; child = child->getNext())
		child->updatePrevWindow(updateFlag);

	setPreviewWindowUpdateFlag(FALSE);
}


#include "window-instance.h"
#include "page-window-instance.h"
#include "invisible-component.h"

ComponentInstance* ComponentInstance::createFromClassName(const char* typeName, const char* className)
{
	char szTypeName[100];

	if(className == NULL)
		return NULL;

	if(typeName == NULL){
		typeName = strchr(className, ':');
		if(typeName){
			strncpy(szTypeName, className, typeName - className);
			szTypeName[typeName - className] = '\0';
			while(*typeName==':') typeName ++;
			className = typeName;
		}
		else{
			strcpy(szTypeName, "window");
		}
	}
	else
	{
		strcpy(szTypeName, typeName);
	}

	if(strcasecmp(szTypeName,"window") == 0)
		return createInstanceByName<WindowInstance>("window", className);

	if(strcasecmp(szTypeName,"paged-window") == 0)
		return createInstanceByName<PageWindowInstance>("paged-window",className);

	if(strcasecmp(szTypeName, "component") == 0)
		return createInstanceByName<InvisibleComponent>("component",className);

	return NULL;
}

ComponentInstance* ComponentInstance::createFromXmlNode(xmlNodePtr node)
{
	xmlChar* xclass = xmlGetProp(node, (const xmlChar*)"class");

	if(xclass == NULL)
		return NULL;

	ComponentInstance * instance = createFromClassName((const char*)node->name, (const char*)xclass);

	xmlFree(xclass);

	if(!instance)
		return NULL;

	if(!instance->loadFromXML(node)){
		delete instance;
		return NULL;
	}

	return instance;
}

void ComponentInstance::switchComponentInstances(ComponentInstance* cinst1, ComponentInstance* cinst2)
{
	if(cinst1 == NULL || cinst2 == NULL || cinst1 == cinst2)
		return;

	ComponentInstance* cinst1_prev, *cinst1_next, *cinst2_prev, *cinst2_next;

	if(cinst2->next == cinst1){
		ComponentInstance* t = cinst1;
		cinst1 = cinst2;
		cinst2 = t;
	}

	if (cinst1->next == cinst2 )
	{
		cinst1_prev = cinst1->prev;
		cinst2_next = cinst2->next;

		if(cinst1_prev)
			cinst1_prev->next = cinst2;
		cinst2->prev = cinst1_prev;
		cinst2->next = cinst1;

		cinst1->prev = cinst2;
		cinst1->next = cinst2_next;

		if(cinst2_next)
			cinst2_next->prev = cinst1;
	}
	else {
		cinst1_prev = cinst1->prev;
		cinst1_next = cinst1->next;
		cinst2_prev = cinst2->prev;
		cinst2_next = cinst2->next;

		cinst1->prev = cinst2_prev;
		cinst1->next = cinst2_next;
		if(cinst2_prev)
			cinst2_prev->next = cinst1;
		if(cinst2_next)
			cinst2_next->prev = cinst1;

		cinst2->prev = cinst1_prev;
		cinst2->next = cinst1_next;
		if(cinst1_prev)
			cinst1_prev->next = cinst2;
		if(cinst1_next)
			cinst1_next->prev = cinst2;

	}

	if (cinst1 == children)
		children = cinst2;
	else if (cinst2 == children)
		children = cinst1;

}

string ComponentInstance::newName()
{
	ResManager * resMgr = g_env->getResManager(NCSRT_UI | NCSRT_CONTRL);
	string name = Instance::newName();
	while(resMgr && !(resMgr->isValidName(name.c_str())))
	{
		name += "_COPY";
	}

	return name;
}


BOOL ComponentInstance::setDefRenderer(const char* defRdrName)
{
	for(ComponentInstance* child = getChildren();
		child; child = child->getNext())
	{
		child->setDefRenderer(defRdrName);
	}
	return TRUE;
}


////////////////////////////////////////
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

static const char* inst_getID(Instance *inst, int type)
{
	static char szText[100];
	ResManager * resMgr = g_env->getResManager(type);
	const char* strName = NULL;

	if(resMgr){
		strName = resMgr->idToName(inst->getID());
	}

	if(strName == NULL)
	{
		sprintf(szText, "%d", inst->getID());
		return szText;
	}

	return strName;
}

static string inst_getComponentInstanceName(Instance* inst, int type)
{
	ResManager * resMgr = g_env->getResManager(type);
	const char* strName = NULL;

	if(resMgr){
		strName = resMgr->idToName(inst->getID());
	}

	if(strName == NULL){
		const char* clsName = inst->getClass()->getClassName();
		if(clsName)
		{
			const char* str = strchr(clsName,':');
			if(str)
				clsName = str + 2;

			char szText[100];
			sprintf(szText, "%s%ld", clsName, inst->getSerialNumber());
			return string(szText);
		}
	}

	if(strName)
		return string(strName);
	return string("");
}

static int make_lua_inst(lua_State* L, Instance* inst, int res_type)
{
    if(inst == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
	int inst_table = lua_gettop(L); //get the index of table on the stack

    //set instance
    SET_TABLE(L, inst_table, __inst, lua_pushinteger(L, (int)inst));

	//set instance id
	SET_TABLE(L, inst_table, id, lua_pushstring(L, inst_getID(inst, res_type)));
	//set instance name
	SET_TABLE(L, inst_table, name,
					lua_pushstring(L,inst_getComponentInstanceName(inst, res_type).c_str()));

	//set serial number
	SET_TABLE(L, inst_table, serial, lua_pushinteger(L,inst->getSerialNumber()));

    return 1;
}

static Instance* instance_from_lua(lua_State* L, int idx)
{
    lua_getfield(L, idx, "__inst");
    Instance* inst = (Instance*) lua_tointeger(L,-1);
    lua_pop(L,1);
    return inst;
}


static int compinst_getNextInstance(lua_State *L)
{
	//get instance pointer from L
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L, 1);
	if(cinst)
		cinst = cinst->getNext();
    
    return make_lua_inst(L, (Instance*)cinst,NCSRT_UI|NCSRT_CONTRL);
}

static int compinst_getInstChildren(lua_State *L)
{
	//get instance pointer from L
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L, 1);
	if(cinst)
		cinst = cinst->getChildren();

    return make_lua_inst(L, (Instance*)cinst, NCSRT_UI|NCSRT_CONTRL);

}


#define PROP_KEY_INDEX 0
#define PROP_KEY_ID    1
#define PROP_KEY_NAME  2
static void inst_getProperties(lua_State* L,Instance* inst, int res_type, int prop_begin, int prop_end, int prop_key)
{
	//create table
	lua_newtable(L);
	int inst_table = lua_gettop(L); //get the index of table on the stack
	int count = 0;

	const mapex<int, Field*>& fields = inst->getFields();
	for(mapex<int,Field*>::const_iterator it = fields.lower_bound(prop_begin);
		it != fields.end() && it->first <= prop_end; ++it)
	{
		FieldType* ft = inst->getClass()->getFieldType(it->first);
		if(!ft)
			continue;

		//push key
		if(prop_key == PROP_KEY_ID)
			lua_pushinteger(L, it->first);
		else if(prop_key == PROP_KEY_NAME)
			lua_pushstring(L, ft->name.c_str());
		else
			lua_pushinteger(L,count);
		//-- prop-table = {}
		lua_newtable(L);
		int prop_table = lua_gettop(L);
		//set id
		SET_TABLE(L,prop_table,id, lua_pushinteger(L, it->first));
		//set Name
		SET_TABLE(L,prop_table, name, lua_pushstring(L, ft->name.c_str()));
		//set Value(to binery)
		SET_TABLE(L, prop_table, value,
				do{
					if(!Instance::inst_push_value_type(L, prop_table, it->second->value, ft->vtype))
						lua_pushinteger(L, 0);
				}while(0)
		);
		//inst_table[count] = prop_table
		lua_settable(L, inst_table);
		count ++;
	}
	SET_TABLE(L, inst_table, propCount, lua_pushinteger(L, count));
}

static int compinst_getInstanceEvents(lua_State* L)
{
	//get first event
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L,1);

	if(cinst)
	{
		/**
		 *  --the equivalent lua code
		 * handlers = { } --create a handlers table
		 *  event = {} -- event is a table
		 *  event.name = name -- set event name
		 *  event.funcName = funcName -- set event funcName
		 *  event.prototype = eventPrototype -- set the prototype of function
		 *  event.code =  code
		 *  handlers[1] = event;
		 *    --- .....
		 */
		//create table
		lua_newtable(L);
		int handlers = lua_gettop(L); //get the index of table on the stack

        int count = 0;
		//set event
		const mapex<int, Field*>& fields = cinst->getFields();
		for(mapex<int,Field*>::const_iterator it = fields.lower_bound(ComponentInstance::PropEventBegin);
			it != fields.end() && it->first <= ComponentInstance::PropEventEnd; ++it)
		{
			Value value = it->second->value;
			if(value != 0){
				FieldType *ft = cinst->getClass()->getFieldType(it->first);
				EventValueType * mvt = (EventValueType*)ft->vtype;
				//-- event = {}
				lua_newtable(L);
				int event = lua_gettop(L);
				//event.name = name
				SET_TABLE(L, event, name,lua_pushstring(L, ft->name.c_str()));
				//event.funcName = funcName
				SET_TABLE(L, event, funcName,
						lua_pushstring(L, (const char*)value));
				//event.prototype = prototype
				SET_TABLE(L, event, prototype, lua_pushstring(L,mvt->getPrototype()));
				//event.code = code
				SET_TABLE(L, event, code, lua_pushstring(L, mvt->getCode()));
				//event.content = content
				SET_TABLE(L, event, content, lua_pushstring(L, mvt->getContent()));
				//handler[count] = event
                lua_rawseti(L, handlers, ++ count);
			}
		}
	}
	return 1;
}

static int compinst_getProperties(lua_State* L)
{
	//get instance pointer from L
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L, 1);
    int key_type = lua_tointeger(L, 2);
	if(cinst)
	{
		inst_getProperties(L,(Instance*)cinst, NCSRT_UI|NCSRT_CONTRL,0, ComponentInstance::PropEventBegin-1,key_type);
	}
	return 1;
}

static void compinst_setListens(ComponentInstance *inst, lua_State*L, int table, int &idx, ResManager *resMgr)
{
	int id = inst->getID();
	list<ListenEntry*>& lle = inst->getListens();
	char szText[100];
	char szSenderText[100];
	const char* strListenId = resMgr->idToName(id);
	if(strListenId == NULL){
		strListenId = szText;
		sprintf(szText,"%d",id);
	}

	for(list<ListenEntry*>::iterator it = lle.begin(); it != lle.end(); ++it)
	{
		ListenEntry* le = *it;
		if(!le)
			continue;
		ComponentInstance *sender = dynamic_cast<ComponentInstance*>((Instance*)resMgr->getRes(le->sender_id));
		EventValueType * evt ;
		if(!sender
				|| !(evt = dynamic_cast<EventValueType*>
							(sender->getClass()->getFieldValueType(le->event_id))))
			continue;

		const char * strSenderId = resMgr->idToName(le->sender_id);
		if(strSenderId == NULL){
			strSenderId = szSenderText;
			sprintf(szSenderText,"%d",le->sender_id);
		}

		int t = (lua_newtable(L), lua_gettop(L));
		SET_TABLE(L, t, listener, lua_pushstring(L,strListenId));
		SET_TABLE(L, t, listenType, lua_pushstring(L,inst->getClass()->getClassName()));
		SET_TABLE(L, t, listenSerial, lua_pushinteger(L, inst->getSerialNumber()));
		SET_TABLE(L, t, sender, lua_pushstring(L,strSenderId));
		SET_TABLE(L, t, sendType, lua_pushstring(L, sender->getClass()->getClassName()));
		SET_TABLE(L, t, sendSerial, lua_pushinteger(L, sender->getSerialNumber()));
		SET_TABLE(L, t, event, lua_pushstring(L,evt->getCode()));
		SET_TABLE(L, t, prototype, lua_pushstring(L,le->prototype.c_str()));
        lua_rawseti(L, table, ++idx);
	}
}

static int compinst_getInstListens(lua_State *L)
{
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L, 1);
	ResManager *resMgr = g_env->getResManager(NCSRT_UI);

	if(cinst)
	{
		/*
		 *  listens = {
		 *  {listener=id, listenType, listenSerial, sender=id, senderType, senderSerial, event=code, prototype=""},
		 *        ...
		 *        }
		 *
		 */
		//create table
		lua_newtable(L);
		int listens = lua_gettop(L); //get the index of table on the stack
        int idx = 0;
		compinst_setListens(cinst,L,listens, idx,resMgr);
	}

	return 1;
}

static int compinst_getTemplateData(lua_State* L)
{
	ComponentInstance* cinst = (ComponentInstance*) instance_from_lua(L, 1);
	if(cinst)
	{
		/*
		 * template_table = {}
		 * template_table.ctrlClass =
		 * template_table.x =
		 * template_table.y =
		 * template_table.w =
		 * template_table.h =
		 * template_table.style =
		 * template_table.exstyle =
		 * template_table.caption =
		 * template_table.props = { {id, value},{id,value}, {...} }
		 * template_table.rdr_info = { gbl_rdr = "classic", ctrl_rdr="classic", {id, value}, {id,value}}
		 * template_table.bk_color =
		 */
		//create table
		lua_newtable(L);
		int inst_table = lua_gettop(L); //get the index of table on the stack


		cinst->toLuaTable((void*)L, inst_table);
	}
	return 1;
}

static const luaL_reg lua_compinst_libs[] = {
		{"getNextInstance", compinst_getNextInstance},
		{"getInstanceEvents", compinst_getInstanceEvents},
		{"getInstChildren", compinst_getInstChildren},
		{"getProperties", compinst_getProperties },
		{"getInstListens", compinst_getInstListens},
		{"getCompTemplateData", compinst_getTemplateData},
		{NULL, NULL}
};

extern "C" void luaL_openlibs (lua_State *L);

static void comp_inst_call_lua(ComponentInstance* cinst, const char* scriptFile, const char* fileName, const char* script_func, const char* args=NULL)
{
	if(fileName == NULL
			|| cinst == NULL
			|| scriptFile == NULL)
		return;

	if(script_func == NULL)
		script_func = "main";

	if(args == NULL)
		args = "";
	//open lib
	lua_State* L = luaL_newstate();

	//open standare lib
	luaL_openlibs(L);

	//open compinst libs
	luaL_register(L, "compinst", lua_compinst_libs);

	//run file
	luaL_dofile(L, scriptFile);

	//get global func scriptFunc
	lua_getglobal(L, script_func);

	//push value
	lua_pushstring(L,fileName);
    make_lua_inst(L, (Instance*)cinst, NCSRT_CONTRL|NCSRT_UI);
	lua_pushstring(L,args);
	//call func
	lua_pcall(L, 3, 0, 0);

	lua_close(L);
}

void ComponentInstance::saveSrouce(const char* fileName, ComponentInstance *cinst, const char* scriptFile, const char* args)
{
	comp_inst_call_lua(cinst, scriptFile, fileName,"trans_main", args);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//

void ComponentInstance::saveTemplates(const char* fileName, ComponentInstance* cinst, const char* scriptFile ,const char* args)
{
	comp_inst_call_lua(cinst, scriptFile, fileName,"main", args);
}

/////////////////////////////////////////////////////////////
//////////////////////////////////
//ComponentInstanceUndoRedoCommand
ComponentInstanceUndoRedoCommand::ComponentInstanceUndoRedoCommand(ComponentInstance *parent, int count, int type)
{
	if(parent == NULL)
		throw("ComponentInstanceUndoRedoCommand cannot recieve a NULL parent param");

	if(type == ADD)
		this->type = ADD;
	else
		this->type = DELETE;

	this->parent = parent;
	this->count = count;
	insts = NULL;
	if(count > 0){
		insts = new ComponentInstance* [count];
		memset(insts, 0, sizeof(ComponentInstance*)*count);
	}
}

ComponentInstanceUndoRedoCommand::~ComponentInstanceUndoRedoCommand()
{
	for(int i=0; i<count; i++)
	{
		if(insts[i])
			insts[i]->release();
	}
	delete[] insts;
}

void ComponentInstanceUndoRedoCommand::setInstance(int idx, ComponentInstance *inst)
{
	if(idx < 0 || idx > count || !inst)
		return;

	if(insts[idx])
		insts[idx]->release();
	insts[idx] = inst;
	inst->addRef();
	inst->autoStoreIdName();
}


void ComponentInstanceUndoRedoCommand::execute()
{
	if(type == ADD) //undo, to delete it
	{
		for(int i=0; i<count;i++)
		{
			if(insts[i]){
				parent->remove(insts[i], TRUE);
				insts[i]->decUseOfRefReses();
				insts[i]->release();
			}
		}
	}
	else
	{
		for(int i=0; i<count; i++){
			if(insts[i]){
				insts[i]->incUseOfRefReses();
				insts[i]->autoRegisterID();
				insts[i]->addRef();
				parent->insert(insts[i], TRUE);
			}
		}
	}

	type = (type==ADD)?DELETE:ADD;
}

//end of ComponentInstanceUndoRedoCommand
/////////////////////////////////
///ComponentInstanceBoundUndoRedoCommand
ComponentInstanceLocationUndoRedoCommand::ComponentInstanceLocationUndoRedoCommand(ComponentInstance *oldContainer, int count)
{
	boundInfos = NULL;
	this->count = count;
	if(count > 0){
		boundInfos = new BoundInfo[count];
		memset(boundInfos, 0, sizeof(BoundInfo)*count);
	}

	this->old_container = oldContainer;
}
ComponentInstanceLocationUndoRedoCommand::~ComponentInstanceLocationUndoRedoCommand()
{
    delete[] boundInfos;
}

void ComponentInstanceLocationUndoRedoCommand::setInstance(int idx, ComponentInstance *inst, int v1, int v2)
{
	if(idx < 0 || idx > count  || !inst)
		return;
	boundInfos[idx].compinst = inst;
	boundInfos[idx].x = v1;
	boundInfos[idx].y = v2;
}

void ComponentInstanceLocationUndoRedoCommand::execute()
{
	if(!boundInfos)
		return;
	ComponentInstance *cur_container = boundInfos[0].compinst->getParent();
	for(int i=0; i< count; i++)
	{
		if(!boundInfos[i].compinst)
			continue;
		int x,y;
		ComponentInstance *cinst = boundInfos[i].compinst;
		cinst->getLocation(x, y);
		if(old_container && cur_container && cur_container != old_container){
			cur_container->remove(cinst, TRUE);
			cinst->setLocation(boundInfos[i].x, boundInfos[i].y);
			old_container->insert(cinst, TRUE);
		}
		else
		{
			cinst->setLocation(boundInfos[i].x, boundInfos[i].y);
		}
		//set Rect
		RECT rc;
		GetWindowRect(cinst->getPreviewHandler(), &rc);
		MoveWindow(cinst->getPreviewHandler(),boundInfos[i].x, boundInfos[i].y, RECTW(rc), RECTH(rc),FALSE);
		boundInfos[i].x = x;
		boundInfos[i].y = y;
	}
	old_container = cur_container;
}

/////////////////////////////////////
ComponentInstanceBoundUndoRedoCommand::ComponentInstanceBoundUndoRedoCommand(int count)
{
	bounds = NULL;
	this->count = count;
	if(count > 0)
	{
		bounds = new Bound[count];
		memset(bounds, 0, sizeof(Bound)*count);
	}
}

ComponentInstanceBoundUndoRedoCommand::~ComponentInstanceBoundUndoRedoCommand()
{
    delete[] bounds;
}

void ComponentInstanceBoundUndoRedoCommand::execute()
{
	if(!bounds)
		return;

	for(int i=0; i<count; i++)
	{
		ComponentInstance *cinst = bounds[i].cinst;
		if(!cinst)
			continue;
		HWND hwnd = cinst->getPreviewHandler();
		RECT rc;
		GetWindowRect(hwnd,&rc);
		//set old
		MoveWindow(hwnd,bounds[i].rc.left, bounds[i].rc.top, RECTW(bounds[i].rc),RECTH(bounds[i].rc),FALSE);
		//set inst
		if(cinst->getParent()) //donnot modify the top most instance
			cinst->setLocation(bounds[i].rc.left, bounds[i].rc.top);
		cinst->setSize(RECTW(bounds[i].rc),RECTH(bounds[i].rc));
		bounds[i].rc = rc;
	}
}

void ComponentInstanceBoundUndoRedoCommand::setInstance(int idx, ComponentInstance *cinst)
{
	if(!cinst)
		return;
	RECT rc;
	GetWindowRect(cinst->getPreviewHandler(), &rc);
	setInstance(idx, cinst, &rc);
}

void ComponentInstanceBoundUndoRedoCommand::setInstance(int idx, ComponentInstance *cinst, const RECT *rc)
{
	if(idx < 0 || idx >= count || !cinst)
		return ;
	bounds[idx].cinst = cinst;
	bounds[idx].rc    = *rc;
}

//////////////////
ComponentInstanceTextUndoRedoCommand::ComponentInstanceTextUndoRedoCommand(ComponentInstance *cinst)
{
	if(!cinst)
		throw("ComponentInstanceTextUndoRedoCommand cannot accept cinst as NULL");

	this->cinst = cinst;
	oldstr = (char*)0xFFFFFFFF;
	if(cinst->isSettedField(ComponentInstance::PropText))
	{
		ValueType *vtype = cinst->getClass()->getFieldValueType(ComponentInstance::PropText);
		const char* str = (const char*)vtype->toBinary(cinst->getField(ComponentInstance::PropText));
		oldstr = str?strdup(str):NULL;
	}
}

ComponentInstanceTextUndoRedoCommand::~ComponentInstanceTextUndoRedoCommand()
{
	if(oldstr && oldstr!=(char*)0xFFFFFFFF)
	{
		free(oldstr);
	}
}

void ComponentInstanceTextUndoRedoCommand::execute()
{
	if(!cinst)
		return;

	ValueType *vtype = cinst->getClass()->getFieldValueType(ComponentInstance::PropText);
	int text_id = (int)cinst->getField(ComponentInstance::PropText);

	char * str = cinst->isSettedField(ComponentInstance::PropText)
			 ? (char*)vtype->toBinary(text_id)
		  :(char*)0xFFFFFFFF;
	if(str!=NULL && str != (char*)0xFFFFFFFF)
		str = strdup(str);

	if(oldstr == (char*)0xFFFFFFFF)
	{
		cinst->cleanField(ComponentInstance::PropText);
	}
	else
	{
		((TextValueType*)vtype)->saveToRes(text_id, (DWORD)oldstr);
	}

	if(oldstr && oldstr!=(char*)0xFFFFFFFF)
		free(oldstr);

	oldstr = str;
}
