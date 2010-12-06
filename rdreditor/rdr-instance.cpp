/**
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include <mgncs/mgncs.h>
#include <mgncs/mrdr.h>

using namespace std;

#ifndef _MGNCS_CLASS_PREFIX
#define _MGNCS_CLASS_PREFIX ""
#endif

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "../uieditor/component-instance.h"
#include "../uieditor/window-instance.h"
#include "rdr-instance.h"
#include "rdr-event-id.h"

#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "rdrtreepanel.h"
#include "rdr-preview-panel.h"
#include "editable-listview.h"
#include "uieditor/fieldpanel.h"
#include "uieditor/rdrpanel.h"

#include "rdreditor.h"
#include "clipboard.h"

RendererInstance::RendererInstance(Class* cls)
:Instance(cls)
{
	char rdrname[100], clsname[100], type[100];
	char szClass[100];
    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

	resMgr->getName(cls, rdrname, clsname, type);

	rdrName = rdrname;
	clsName = clsname;
    typeName = type;
#if 0
    fprintf (stderr, "rdrname:%s, clsname:%s, typename:%s \n",
    		getRdrName(), getClsName(), getTypeName());
#endif
    Class *win_class= _class;
	ValueType *vtype = StringValueType::getInstance();

    while(win_class)
    {
    	sprintf(szClass, "%s%s%s",_MGNCS_CLASS_PREFIX,
    			(char *)(vtype->toBinary(win_class->getFieldDefault(ComponentInstance::PropClass))),
    			_MGNCS_CLASS_SUFFIX);
    	rdr = ncsRetriveCtrlRDR (rdrName.c_str(), szClass);
    	if(rdr)
    		break;
    	win_class = win_class->getSuper();
    }
	hWnd = HWND_NULL;
	win_inst = NULL;
}

RendererInstance::RendererInstance(const RendererInstance &rdrst)
:Instance(rdrst._class)
{
	hWnd = HWND_NULL;
	rdrName = rdrst.rdrName;
	clsName = rdrst.clsName;
    typeName = rdrst.typeName;
    rdr =  rdrst.rdr;
    win_inst = NULL;

    rdrst.copyFields(fields);
}

static map<string, int> _rdr_map_ref;
string RendererInstance::newName()
{
	ResManager * resMgr = g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET);
	char szName[100];
	int ref;

	do {
		ref = _rdr_map_ref[rdrName];
		_rdr_map_ref[rdrName] ++;

		sprintf(szName, "IDRDR_%s_%s%d", getRdrName(), getClsName(), ref);

		for(int i=0; szName[i]; i++) {
			szName[i] = toupper(szName[i]);
        }
	} while(resMgr && !(resMgr->isValidName(szName)));

	return string(szName);
}

RendererInstance::~RendererInstance()
{
    delete win_inst;
    win_inst = NULL;
}

Instance* RendererInstance::clone()
{
	return new RendererInstance(*this);
}

Value RendererInstance::getField(int id) {
	if (isFieldExist(id)) {
		return Instance::getField(id);
	}
	else {
		const WINDOW_ELEMENT_RENDERER *renderer = GetWindowRendererFromName(getRdrName());
		if (renderer) {
			FieldType *ft = _class->getFieldType(id);
			if (ft && ft->vtype->getType() == VT_IMAGE)
				return 0;

			return ncsGetElementEx(NULL, getRdrName(), id);
		}
	}
	return -1;
}

const char* RendererInstance::getControlClass()
{
	ValueType *vtype = StringValueType::getInstance();
	if(vtype)
	{
		return (const char*)vtype->toBinary(getField(ComponentInstance::PropClass));
	}
	return NULL;
}


RendererInstance*
RendererInstance::createFromClassName(const char* typeName,
		const char* className)
{
	char szTypeName[100];

	if(className == NULL)
		return NULL;

	if(typeName == NULL){
		typeName = strchr(className, ':');
		if(typeName){
			strncpy(szTypeName, className, typeName - className);
			szTypeName[typeName - className] = '\0';
			//last ::
			typeName = strrchr(className, ':');
			while(*typeName==':') typeName ++;
			className = typeName;
		}
		else{
			strcpy(szTypeName, "renderer");
		}
	}
	else
	{
		strcpy(szTypeName, typeName);
	}

	if(strcmp(szTypeName,"renderer") == 0)
		return createInstanceByName<RendererInstance>("renderer", className);

	return NULL;
}

RendererInstance* RendererInstance::createFromXmlNode(xmlNodePtr node)
{
	xmlChar* xclass = xmlGetProp(node, (const xmlChar*)"class");
	char classname[100];

	if(xclass == NULL)
		return NULL;

	sprintf (classname, "renderer::%s", (const char*)xclass);
	RendererInstance * instance =
        createFromClassName((const char*)node->name, classname);

	xmlFree(xclass);

	if(!instance)
		return NULL;

	if(!instance->loadFromXML(node)){
		delete instance;
		return NULL;
	}
	return instance;
}

BOOL RendererInstance::loadFromXML(xmlNodePtr node)
{
	if(node == NULL)
		return FALSE;

	//get property
	for(xmlNodePtr child = node->xmlChildrenNode; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp(child->name,(const xmlChar*)"ID") == 0){
			xmlChar* xstr = xhGetNodeText(child);
			id = strtol((const char*)xstr, NULL, 0);
			xmlFree(xstr);
		}
		else{
			loadProperty(child);
		}
	}
	return TRUE;
}

void RendererInstance::saveXMLToStream(TextStream *stream)
{
	stream->println("<%s class=\"%s\">", getClassType(), _class->getClassName());

	stream->indent();
	stream->println("<ID>0x%0x</ID>", id);

	//print fields
	for(map<int, Field*>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		Field* field = it->second;
		FieldType *ft;
		if(field && (ft = _class->getFieldType(it->first))!= NULL)
		{
			//save name
			stream->printf("<%s>",ft->name.c_str());
			//save value
			ft->vtype->saveXMLStream(field->value, stream);
			stream->println("</%s>",ft->name.c_str());
		}
	}
	stream->unindent();
	stream->println("</%s>", getClassType());
}


int RendererInstance::saveBinToStream(BinStream *stream)
{
	int name_id;
	int count = 0;

	//save renderer name id
	name_id = g_env->addString(getRdrName());
	stream->save32(name_id);

	char szClass[100];
	const char *ctrlClsName = getControlClass();
	if (!ctrlClsName)
		return 0;
	sprintf(szClass, "%s%s%s",_MGNCS_CLASS_PREFIX, ctrlClsName, _MGNCS_CLASS_SUFFIX);

	//save class name id
	name_id = g_env->addString(szClass);
	stream->save32(name_id);
	count = 2*sizeof(Uint32);

	mapex<int, Field*> fields = getFields();
	mapex<int, Field*>::iterator iter;
	FieldType * ft;

	for (iter = fields.begin(); iter != fields.end(); ++iter) {
		iter->first;
		ft = _class->getFieldType(iter->first);
		if(ft)
		{
			Value val = getField(ft->id);
			//save id
			stream->save32(ft->id);
			//save value
			if (WE_ATTR_TYPE_FONT==(WE_ATTR_TYPE_MASK&ft->id)){
				int str_id = g_env->addString((const char *)val);
				stream->save32(str_id);
			} else {
				stream->save32(ft->vtype->toRes(val));
			}
			count += 2*sizeof(Uint32);
		}
	}
	//save end flag
	stream->save32(-1);
	stream->save32(0);
	count += 2*sizeof(Uint32);

	return count;
}

DWORD RendererInstance::getRdrElementValue(int id, Value val, ValueType *vtype)
{
	if(!isFieldExist(id))
	{
		return ncsGetElementEx(NULL, rdrName.c_str(), id);
	}
	else
	{
		DWORD dval = vtype->toBinary(val);
		if(vtype->getType() == VT_IMAGE)
		{
			const char* file = (const char*)dval;
			RegisterResFromFile(HDC_SCREEN,file);
			//FIXED ME : Special for skin
			if((WE_ATTR_TYPE_RDR==(WE_ATTR_TYPE_MASK&id))
					&& ((WE_ATTR_INDEX_MASK&id)>=1 && (WE_ATTR_INDEX_MASK&id)<WE_LFSKIN_NUMBER))
			{
				return dval;
			}
			else
				return Str2Key(file);
		}
		else if (vtype->getType() == VT_FONT)
		{
			//return (DWORD)CreateLogFontByName((const char*)dval);
			return (DWORD)LoadResource((const char*)dval, RES_TYPE_FONT, 0L);
		}

		return dval;
	}
}

BOOL RendererInstance::getNcsRdrInfo(RendererInfo &rdrInfo)
{
	int count = 0;
	FieldType * ft;

	rdrInfo.glb_rdr = rdrInfo.ctl_rdr = rdrName.c_str();

	mapex<int, Field*>::iterator iter;

	count = fields.size();

	if (count > 0) {
		int i = 0;
		rdrInfo.resize(count);
		for (iter = fields.begin(); iter != fields.end(); ++iter) {
			Field * f = iter->second;
			if(f && (ft = _class->getFieldType(f->id))){
				rdrInfo[i].id = f->id;
				//TODO: for test
				rdrInfo[i].value = getRdrElementValue(f->id, f->value, ft->vtype);
				i ++;
			}
		}
	}

	return TRUE;
}

void RendererInstance::destroyPreviewWindow()
{
	if (win_inst) {
        delete win_inst;
        hWnd = HWND_NULL;
        win_inst = NULL;
	}
}

HWND RendererInstance::createPreviewWindow(HWND hParent)
{
	if (win_inst == NULL)
	{
		win_inst = (WindowInstance*)ComponentInstance::createFromClassName(
					getTypeName(), getClsName());
		if (win_inst == NULL)
			return HWND_NULL;

		win_inst->setLocation(50, 50);
	}
	FieldType *ft = win_inst->getClass()->getFieldType("Border");
	if (ft)
		win_inst->setField(ft->id, WS_BORDER);

	win_inst->setField(ComponentInstance::PropRenderer, getID());
	hWnd = win_inst->createPreviewWindow(hParent);
	SetWindowText(hWnd, getClsName());
	return hWnd;
}

void RendererInstance::updatePreviewWindow(int* element_ids, HWND hwndToUp/*=HWND_INVALID*/)
{
	if (hwndToUp == HWND_NULL)
		return;

    mWidget *previewObj = NULL; 
	if(hwndToUp == HWND_INVALID) {
		hwndToUp = hWnd;
    }
    else {
        previewObj = ncsObjFromHandle(hWnd);
    }

    FieldType *ft;
    Value val;
    int id;
    mWidget *self = ncsObjFromHandle(hwndToUp);

    while (*element_ids != 0) {
        id = *element_ids;
        ft = _class->getFieldType(id);
        val = getField(id);

        if (id > 0) {
            ncsSetElement(self, id, getRdrElementValue(id, val, ft->vtype));
            if (previewObj)
                ncsSetElement(previewObj, id, getRdrElementValue(id, val, ft->vtype));
        }
        element_ids++;
    }

    SendMessage(hwndToUp, MSG_NCPAINT, 0, 0);
    InvalidateRect(hwndToUp, NULL, TRUE);

    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
    if (resMgr && previewObj) {
        SendMessage(hWnd, MSG_NCPAINT, 0, 0);
        InvalidateRect(hWnd, NULL, TRUE);
        resMgr->refreshRdrPanel(this);
    }
}

//////////////////////////////////////////
RendererSet::RendererSet(const char* clsName)
{
	rdrName = clsName;
	id = -1;
}

RendererSet::~RendererSet()
{
}

BOOL RendererSet::loadFromXML(xmlNodePtr node)
{
	if(node == NULL)
		return FALSE;

	//get property
	for(xmlNodePtr child = node->xmlChildrenNode; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp(child->name,(const xmlChar*)"ID") == 0){
			xmlChar* xstr = xhGetNodeText(child);
			id = strtol((const char*)xstr, NULL, 0);
			xmlFree(xstr);
		}
		else{
			if(xmlStrcmp(child->name, (const xmlChar*)"SubID") == 0){
				xmlChar* xstr = xhGetNodeText(child);
				int subId = strtol((const char*)xstr, NULL, 0);
				rdrIds.insert(subId);
				xmlFree(xstr);
			}
		}
	}
	return TRUE;
}

void RendererSet::saveXMLToStream(TextStream *stream)
{
	stream->println("<%s class=\"%s\">", getClassType(), getRdrName());

	stream->indent();
	stream->println("<ID>0x%0x</ID>", id);

	//print SubIDs
	for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
	{
        stream->printf("<SubID>");
        stream->printf("0x%0x", (*it)->getID());
        stream->println("</SubID>");
	}
	stream->unindent();
	stream->println("</%s>", getClassType());
}

int RendererSet::saveBinToStream(BinStream *stream)
{
	int count = 0;
	//save renderer name id
	int rdrname_id = g_env->addString(getRdrName());
	stream->save32(rdrname_id);

	count = sizeof(Uint32);
	//save subIDs
	for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
    {
		stream->save32((*it)->getID());
	}
	//save end flag
	stream->save32(-1);
	count += (insts.size() + 1)*sizeof(Uint32);

	return count;
}

RendererSet*
RendererSet::createFromClassName(const char* typeName,
		const char* className)
{
	if(className == NULL || typeName == NULL)
		return NULL;

	if(strcmp(typeName,"rdrset") == 0)
		return (new RendererSet(className));
	return NULL;
}

RendererSet* RendererSet::createFromXmlNode(xmlNodePtr node)
{
	xmlChar* xclass = xmlGetProp(node, (const xmlChar*)"class");

	if(xclass == NULL)
		return NULL;

	RendererSet * instance = createFromClassName((const char*)node->name, (const char*)xclass);

	xmlFree(xclass);

	if(!instance)
		return NULL;

	if(!instance->loadFromXML(node)){
		delete instance;
		return NULL;
	}

	return instance;
}

//please call it after loading rdr.xml
void RendererSet::initRdrSets()
{
    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    for(set<int>::iterator it = rdrIds.begin(); it != rdrIds.end(); ++it) {
        RendererInstance * inst =
        	(RendererInstance *)(resMgr->getRes(*it));
        if (inst) {
            insts.insert(inst);
            inst->addRef();
        }
    }
}
BOOL RendererSet::insertInstance(RendererInstance *inst) {
	if (accept(inst)) {
        insts.insert(inst);
        //add reference
        inst->addRef();
        return TRUE;
    }
	return FALSE;
}

BOOL RendererSet::hasRefInstance()
{
    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    if (!resMgr)
    	return TRUE;

	for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
	{
		RendererInstance * rdrInst = (RendererInstance *)(*it);
        int ref = resMgr->getUseRefValue(rdrInst->getID());
        if (ref > 0) {
        	if (rdrInst->getRef() <= 2) {//only exist here
        		return TRUE;
        	}
        }
    }

    return FALSE;
}

BOOL RendererSet::removeInstance(RendererInstance *inst)
{
    if (inst) {
        set <RendererInstance *>::iterator it = insts.find(inst);
        if (it != insts.end()) { //find
            insts.erase(it);
            inst->decRef();
            return TRUE;
        }
    }

    return FALSE;
}

int RendererSet::release()
{
    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    //release sub-ids
	for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
    {
        if (removeInstance (*it))
        	resMgr->removeRes((*it)->getID());
    }
	return 0;
}

BOOL RendererSet::accept(RendererInstance *inst)
{
    if (!inst)
        return FALSE;

    //not same renderer set
    if (strcmp(inst->getRdrName(), getRdrName()))
        return FALSE;

    set <RendererInstance *>::iterator it = insts.find(inst);
    if (it != insts.end()) //find
        return FALSE;

    const char* clsName = inst->getClsName();

	for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
    {
    	if (strcmp((*it)->getClsName(), clsName) == 0)
    		return FALSE;
    }

    return TRUE;
}

static map<string, int> _rdrset_map_ref;
string RendererSet::newName()
{
	ResManager * resMgr = g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET);
	char szName[100];
	int ref;

	do {
		ref = _rdrset_map_ref[rdrName];
		_rdrset_map_ref[rdrName] ++;

		sprintf(szName, "IDRDRS_%s%d", rdrName.c_str(), ref);

		for(int i=0; szName[i]; i++) {
			szName[i] = toupper(szName[i]);
        }
	} while(resMgr && !(resMgr->isValidName(szName)));

	return string(szName);
}
/////////////////////////////
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}
/*
 * rdr_info = {}
 * rdr_info.gbl_rdr = ""
 * rdr_info.ctrl_rdr= ""
 * rdr_info.ctrl_class = ""
 * rdr_info.elements={ id1=vale1, id2=value2 , ... }
 *  ....
 *
 */

void RendererInstance::toLuaTable(void *luaState)
{
	lua_State *L = (lua_State*)luaState;

	//rdr_info = {}
	lua_newtable(L);
	int rdr_info = lua_gettop(L);

	const char* rdr_name = getRdrName();
	SET_TABLE(L, rdr_info, gbl_rdr, lua_pushstring(L, rdr_name));
	SET_TABLE(L, rdr_info, ctrl_rdr, lua_pushstring(L, rdr_name));
	SET_TABLE(L, rdr_info, ctrl_class, lua_pushstring(L, getControlClass()));

	SET_TABLE(L, rdr_info, elements, setPropsToLuaTable(L,0,0x7FFFFFF));

}
