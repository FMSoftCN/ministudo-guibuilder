/*
 * window-instance.cpp
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
#include "mapex.h"
#include <set>
#include <vector>

#include "mgheads.h"
//#include "mgfcheads.h"
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

#include "uiconfig.h"
#include "component-instance.h"
#include "window-instance.h"
#include "rdreditor/rdr-instance.h"
#include "page-window-instance.h"

#ifndef _MGNCS_CLASS_PREFIX
#define _MGNCS_CLASS_PREFIX ""
#endif

#ifndef _MGNCS_CLASS_SUFFIX
#define _MGNCS_CLASS_SUFFIX ""
#endif

WindowInstance::WindowInstance(Class* cls)
:ComponentInstance(cls)
{
	// TODO Auto-generated constructor stub
	old_win_rdr = NULL;

	old_proc = NULL;
}

WindowInstance::WindowInstance(const WindowInstance &winst)
:ComponentInstance(winst._class)
{
	hwnd = HWND_INVALID;
	parent = NULL;
	next = prev = NULL;
	notification = winst.notification;

	winst.copyFields(fields);
	ref_list = winst.copyRefFieldList(fields);


	children = winst.cloneChildren(this);

	old_proc = NULL;
	old_win_rdr = NULL;
}

WindowInstance::~WindowInstance() {
	// TODO Auto-generated destructor stub
	destroyPreviewWindow();
	//delete the text value
	/*int text_id = (int)getField(PropText);

	ResManager *resMgr = g_env->getResManager(NCSRT_TEXT);
	if(resMgr)
		resMgr->removeRes(text_id);
*/
}

BOOL WindowInstance::loadFromXML(xmlNodePtr node)
{
	if(node == NULL)
		return FALSE;

	loadSerial(node);

	//get property
	for(xmlNodePtr child = node->xmlChildrenNode; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;

/*		else if(xmlStrcmp(child->name, (const xmlChar*)"message") == 0){
			loadMessage(child);
		}
		else if(xmlStrcmp(child->name, (const xmlChar*)"notify") == 0){
			//TODO * loadNotify
			loadMessage(child);
		}
		else if(xmlStrcmp(child->name, (const xmlChar*)"user-message") == 0){
			//TODO * loadUserMessage
		}
	*/if(xmlStrcmp(child->name, (const xmlChar*)"window") == 0
						|| xmlStrcmp(child->name, (const xmlChar*)"paged-window") == 0
						|| xmlStrcmp(child->name, (const xmlChar*)"component") == 0){
			ComponentInstance *instance = ComponentInstance::createFromXmlNode(child);
			if(instance){
				insert(instance);
			}
		}
		else if(xmlStrcmp(child->name,(const xmlChar*)"listen")==0)
		{
			loadListen(child);
		}
		else{
			loadProperty(child);
		}
	}

	return TRUE;
}

void WindowInstance::saveXMLToStream(TextStream *stream, int saveFlag)
{
	if(stream == NULL)
		return ;

	ResManager *resMgr = NULL;
	if(saveFlag & WI_SAVE_INDEPEND)
		resMgr = g_env->getResManager(ID2TYPE(id));
	//save instance
	stream->printf("<%s class=\"%s::%s\"",getClassType(),getClassType(), _class->getClassName());
	if (!(saveFlag & WI_SAVE_TMPL))
		saveSerial(stream);
	stream->println(">");

	stream->indent();
	//print id
	if ((saveFlag & WI_SAVE_TMPL) && resMgr){
		if(strcasecmp(getControlClass(), "mainwnd") == 0)
			stream->println("<ID>-1</ID>");
		else
			stream->println("<ID name=\"%s\" type=\"%d\">0x%0x</ID>",resMgr->idToName(id), ID2TYPE(id), id);
	}else if((saveFlag & WI_SAVE_INDEPEND) && resMgr) {
		stream->println("<ID name=\"%s\">0x%0x</ID>",resMgr->idToName(id),id);
	}else{
		stream->println("<ID>0x%0x</ID>",id);
	}
	//print fileds
	for(map<int, Field*>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		Field* field = it->second;
		FieldType *ft;
		if(field && (ft = _class->getFieldType(it->first))!= NULL)
		{
			//save name
			stream->printf("<%s",ft->name.c_str());
			//save attribute
			if(field->attr & Class::FIELD_ATTR_FIXED)
				stream->printf(" readonly=\"true\"");
			if(field->attr & Class::FIELD_ATTR_HIDE)
				stream->printf(" hide=\"true\"");
			stream->printf(">");
			//save value
			if(saveFlag & WI_SAVE_INDEPEND){
				if(ft->vtype->getType() == VT_TEXT){
					stream->printf("%s",(const char*)(ft->vtype->toBinary(field->value)));
				} else if(ft->vtype->getType() == VT_IMAGE
						|| ft->vtype->getType() == VT_RDR)
				{
					stream->printf("0");
				} else {
					ft->vtype->saveXMLStream(field->value, stream);
				}
			} else {
				ft->vtype->saveXMLStream(field->value, stream);
			}
			stream->println("</%s>",ft->name.c_str());
		}
	}

	//save children
	ComponentInstance * cinstance = children;
	while(cinstance)
	{
		if (saveFlag & WI_SAVE_TMPL)
			cinstance->saveIndependXML(stream, TRUE);
		else if(saveFlag & WI_SAVE_INDEPEND)
			cinstance->saveIndependXML(stream, FALSE);
		else
			cinstance->saveXMLToStream(stream);
		cinstance = cinstance->getNext();
	}

	//save listens
	saveListens(stream);

	stream->unindent();
	stream->println("</%s>",getClassType());
}

int WindowInstance::saveBinToStream(BinStream *stream)
{
	if(stream == NULL)
		return 0;

	int size = 0;
	unsigned int id;
	FieldType * ft;
	int begin_pos;

	begin_pos = stream->tell();
	//save stream

	//1. struct NCSRM_WINHEADER

	//1). save class id
	if(useNewControlSet()){
		char szClassName[32];
		sprintf(szClassName, _MGNCS_CLASS_PREFIX "%s" _MGNCS_CLASS_SUFFIX, getControlClass());
		id = g_env->addString(szClassName);
	}
	else
		id = g_env->addString(getControlClass());
	stream->save32(id);

	//2). save window id
	if(ID2TYPE(this->id) == NCSRT_CONTRL)
		stream->save32((this->id)&0xFFFF);
	else
		stream->save32(this->id);

	//3). save caption id //text
	ft = _class->getFieldType(PropText);
	if(ft){
		stream->save32(ft->vtype->toRes(getField(PropText)));
	}
	else
		stream->save32(0);

	//4) save rdr id
	stream->save32(ft->vtype->toRes(getField(PropRenderer)));

	//5) save x, y, w, h
	stream->save32(getField(PropX));
	stream->save32(getField(PropY));
	stream->save32(getField(PropWidth));
	stream->save32(getField(PropHeight));
	//6) save style
	stream->save32(getStyle());

	//7) save exstyle
	stream->save32(getExStyle());
	//save bkcolor
	if(isFieldExist(PropBkColor))
	{
		stream->save32(getBkColor());
	}
	else
	{
		stream->save32(0);
	}
	//save font
	if(isFieldExist(PropFont)){
		id = g_env->addString(getFont());
		stream->save32(id);
	}else{
		stream->save32(0);
	}
	//8) save size, 0 , reset it at last
	stream->save32(0);
	//9) save offset_props, 0, reset it later
	stream->save32(0);
	//10) save offset_ctrls, 0, reset it later
	stream->save32(0);
	//11) save nr_props, 0, reset it later
	stream->save32(0);
	//12) save nr_ctrls, 0, reset it later
	stream->save32(0);
	//13) save offset_adddata, 0
	stream->save32(0);
	//14) save size_adddata, 0
	stream->save32(0);

	size += sizeof(NCSRM_WINHEADER);

	int count = 0;
	int cur_pos1 = stream->tell();
	//save props
	//from 0 ~ to 100
	//for(int i=0; i<100; i++)
	map<int,Field*>::iterator it = fields.lower_bound(1);
	while(it != fields.end())
	{
		ft = _class->getFieldType(it->first);
		if(ft)
		{
			count ++;
			Value val = getField(ft->id);
			//save type
			stream->save32(ft->vtype->getType());
			//save id
			stream->save32(ft->id);
			//save value
			stream->save32(ft->vtype->toRes(val));
		}
		if(it->first >= PropExtendEnd)
			break;

		if(it->first >= PropClass  && it->first <= PropExtendBegin)
			it = fields.lower_bound(PropExtendBegin);
		else
			++it;
	}

	int cur_pos2 = stream->tell();

	if(count > 0)
	{
		//goto offset_props
		stream->seek(begin_pos+ (int)&(((NCSRM_WINHEADER*)0)->offset_props), StreamStorage::seek_begin);
		stream->save32(cur_pos1-begin_pos);
		stream->seek(begin_pos+ (int)&(((NCSRM_WINHEADER*)0)->nr_props),
				StreamStorage::seek_begin);
		stream->save32(count);
		size += count * sizeof(Uint32) * 3;
	}

	stream->seek(cur_pos2, StreamStorage::seek_begin);

	count = 0;
	int ctrls_size = 0;
	cur_pos1 = stream->tell();
	//save controls
	for(ComponentInstance* cinst = children; cinst; cinst = cinst->getNext())
	{
		int ctrl_size = cinst->saveBinToStream(stream);
		if(ctrl_size > 0){
			ctrls_size += ctrl_size;
			count ++;
		}
	}

	cur_pos2 = stream->tell();

	if(count > 0) //save controls
	{
		//goto offset_ctrls
		stream->seek(begin_pos+ (int)&(((NCSRM_WINHEADER*)0)->offset_ctrls), StreamStorage::seek_begin);
		stream->save32(cur_pos1-begin_pos);
		//goto nr_ctrls
		stream->seek(begin_pos+ (int)&(((NCSRM_WINHEADER*)0)->nr_ctrls), StreamStorage::seek_begin);
		stream->save32(count);
	}

	//get size
	size += ctrls_size;
	//save size
	stream->seek(begin_pos + (int)&(((NCSRM_WINHEADER*)0)->size), StreamStorage::seek_begin);
	stream->save32(size);

	stream->seek(cur_pos2, StreamStorage::seek_begin);

	return size;

}

void WindowInstance::setPreviewWindowHandler(HWND hwnd)
{
	if(!::IsWindow(hwnd))
		return;

	this->hwnd = hwnd;

	SetWindowAdditionalData(hwnd, (DWORD)this);

	if(isFieldExist(PropBkColor))
	{
		::SetWindowBkColor(hwnd, color2Pixel(getBkColor()));
	}

	if(isContainer())
	{
		old_proc = SetWindowCallbackProc(hwnd, _main_window_proc);
	}

	//create children
	ComponentInstance * instance = children;
	while(instance)
	{
		instance->createPreviewWindow();
		instance = instance->getNext();
	}
}

//create WindowInstance PreviewWindow
HWND WindowInstance::createPreviewWindow(HWND hParent /*= HWND_INVALID*/)
{
	//get Parent
	BOOL from_out = TRUE;
	if(IsWindow(hwnd))
		return hwnd;

	if(!::IsWindow(hParent))
	{
		if(parent)
		{
			hParent = parent->getPreviewHandler();
			from_out = FALSE;
		}
	}

/*	if(!parent)
	{
		hParent = HWND_INVALID;
	}
*/
	//get sysconfig : "control-set", default = new control set
	if(useNewControlSet()){
		mContainer *container = from_out?NULL:((mContainer*)ncsSafeCast((mObject*)ncsObjFromHandle(hParent), (mObjectClass*)&g_stmContainerCls));
		hwnd = createNCSWindow(container?_c(container)->getPanel(container):hParent);
		if(container)
			_c(container)->adjustContent(container);
	}
	else
		hwnd = createOldWindow(hParent);

	if (isFieldExist(PropFont))
	{
		LOGFONT *font = (LOGFONT *)LoadResource(getFont(), RES_TYPE_FONT, 0L);
		::SetWindowFont(hwnd, font);
	}

	if(IsWindow(hwnd))
	{
		setPreviewWindowHandler(hwnd);
	}
	return hwnd;
}

HWND WindowInstance::recreatePreviewWindow(HWND hParent)
{
	ComponentInstance * parent = getParent();
	if(parent && dynamic_cast<PageWindowInstance*>(parent)!=NULL){
		((PageWindowInstance*)parent)->reAddPage(this);
		return getPreviewHandler();
	}
	else
		return ComponentInstance::recreatePreviewWindow(hParent);
}

void WindowInstance::destroyPreviewWindow()
{
	if(IsWindow(hwnd)){
		ComponentInstance::destroyPreviewWindow();
		if(IsMainWindow(hwnd))
			DestroyMainWindow(hwnd);
		else
			DestroyWindow(hwnd);
		hwnd = HWND_INVALID;
	}
}

void WindowInstance::resetNCSProps()
{
	if(!::IsWindow(hwnd)){
		return ;
	}

	mWidget * widget = ncsObjFromHandle(hwnd);
	if(!widget)
		return ;
	//set current props

	map<int, Field*> values;

	getRangedFields(1,PropClass-1,values);

	for(map<int, Field*>::iterator it = values.begin();
		it != values.end(); ++it)
	{
		Field* f = it->second;
		FieldType *ft = _class->getFieldType(f->id);
		if (ft->vtype->getType() == VT_IMAGE) {
			RegisterResFromFile(HDC_SCREEN,(const char*)(ft->vtype->toBinary(f->value)));
			DWORD value = Str2Key((const char*)(ft->vtype->toBinary(f->value)));
			_c(widget)->setProperty(widget, f->id, (DWORD)GetBitmapFromRes(value));
		} else {
			_c(widget)->setProperty(widget, f->id, ft->vtype->toBinary(f->value));
		}
	}

}

HWND WindowInstance::createNCSWindow(HWND hParent)
{
	mWidget * widget;
	int x, y, w, h;
	getLocation(x, y);
	getSize(w, h);
	RendererInfo rdrInfo ;
	rdrInfo.elements = NULL;
	rdrInfo.glb_rdr = g_env->getDefRdrName();
	rdrInfo.ctl_rdr = g_env->getDefRdrName();
	char szClass[100];
	sprintf(szClass, "%s%s%s",_MGNCS_CLASS_PREFIX, getControlClass(),_MGNCS_CLASS_SUFFIX);

	getRendererInfo(&rdrInfo);

	//if(ncsIsChildClass(szClass, NCSCTRL_MAINWND)) //create Main Window
	if(isInstance("MainWnd"))
	{
		widget = ncsCreateMainWindow(szClass,
				getCaption(),
				getStyle()&~WS_VISIBLE,
				getExStyle()|WS_EX_AUTOSECONDARYDC,
				id,
				x, y, w, h,
				HWND_DESKTOP,
				0, 0,
				NULL, //props,
				&rdrInfo,//renderer
				NULL, 0);
	}
	else{
		widget = ncsCreateWindow(szClass,
				getCaption(),
				getStyle(),
				getExStyle(),
				id,
				x, y, w, h,
				hParent,
				NULL,
				&rdrInfo,//renderer
				NULL,
				 0);
	}

	if(widget == NULL)
		return HWND_INVALID;

	hwnd = widget->hwnd;

	if(!IsWindow(hwnd))
		return hwnd;

	_c(widget)->setProperty(widget, NCSP_DEFAULT_CONTENT,0);

	resetNCSProps();

	//set Renderer
	//TODO * set Renderer

	return hwnd;
}

HWND WindowInstance::createOldWindow(HWND hParent)
{
	int x, y, w, h;
	RendererInfo rdrInfo;
	rdrInfo.glb_rdr = g_env->getDefRdrName();
	rdrInfo.ctl_rdr = rdrInfo.glb_rdr;

	getRendererInfo(&rdrInfo);

	getLocation(x, y);
	getSize(w, h);

	if(!IsWindow(hParent))//create a main window
	{
		MAINWINCREATE mainCreate = {
				getStyle()&~WS_VISIBLE,
				getExStyle()|WS_EX_AUTOSECONDARYDC,
				getCaption(),
				0, //HMENU
				0, //HCURSOR
				0, //HICON
				HWND_DESKTOP,
				DefaultMainWinProc,
				x,y, x+w, y+h,
				getField(_class->getFieldId("bkcolor")),
				0
			};

		hwnd = CreateMainWindowEx(&mainCreate,
				rdrInfo.glb_rdr, (const WINDOW_ELEMENT_ATTR*)rdrInfo.elements, NULL, NULL);
	}
	else
	{
		hwnd = CreateWindowEx2(
				getControlClass(),
				getCaption(),
				getStyle(),
				getExStyle(),
				id,
				x, y, w, h,
				hParent,
				rdrInfo.glb_rdr, (const WINDOW_ELEMENT_ATTR*)rdrInfo.elements,
				0);
	}

	//TODO : set renderer

	return hwnd;
}

void WindowInstance::setCaption(const char* strcaption)
{
	if(!strcaption)
		return;

	if(getFieldAttr(PropText)&(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE))
		return ;

	ValueType * vtype = _class->getFieldValueType(PropText);
	TextValueType * tvtype = dynamic_cast<TextValueType*>(vtype);
	if(!tvtype)
		return;

	Value v = getField(PropText);
	if(v == 0)
	{
		v = tvtype->newValue(strcaption);
		setField(PropText, v);
	}
	else
	{
		tvtype->saveToRes(v, (DWORD)strcaption);
	}

}

Instance* WindowInstance::clone()
{
	return new WindowInstance(*this);
}

DWORD WindowInstance::getCommStyle(int id_begin, int id_end){

	DWORD dw = 0;
	for(int i=id_begin; i<id_end; i++)
	{
		FieldType *ft = _class->getFieldType(i);
		if(ft == NULL)
			continue;

		Value v = getField(i);

		DWORD d = ft->vtype->toBinary(v);

		DPRINT("Field: name=%s binary=0x%08X",ft->name.c_str(), d);

		dw |= d;
	}

	return dw;
}

BOOL WindowInstance::getDlgTemplate(DLGTEMPLATE* pteml)
{
	if(!pteml)
		return FALSE;

	memset(pteml, 0, sizeof(DLGTEMPLATE));
	pteml->caption = getCaption();
	pteml->dwStyle = getStyle();
	pteml->dwExStyle = getExStyle();
	getLocation(pteml->x, pteml->y);
	getSize(pteml->w, pteml->h);

	return TRUE;
}

BOOL WindowInstance::getCtrlData(CTRLDATA *pctrl)
{
	if(!pctrl)
		return FALSE;

	memset(pctrl, 0, sizeof(CTRLDATA));
	pctrl->caption = getCaption();
	pctrl->class_name = getControlClass();
	pctrl->dwStyle = getStyle();
	pctrl->dwExStyle = getExStyle();
	getLocation(pctrl->x, pctrl->y);
	getSize(pctrl->w, pctrl->h);

	return TRUE;
}


Instance* WindowInstance::getRendererInstance()
{
	//get instance
	int rdrid = (int)getField(PropRenderer);

	if(rdrid <= 0 || ID2TYPE(rdrid) != NCSRT_RDR)
		return NULL;

	ResManager* resMgr = g_env->getResManager(NCSRT_RDR);
	if(resMgr)
	{
		return  (Instance*)(RendererInstance*)resMgr->getRes(rdrid);
	}
	return NULL;
}

NCS_RDR_INFO* WindowInstance::getRendererInfo(NCS_RDR_INFO *rdr_info)
{
	if(rdr_info == NULL)
		return NULL;

	RendererInstance* rdrInst = (RendererInstance*)getRendererInstance();

	if(rdrInst == NULL)
			return NULL;

	if(rdrInst->getNcsRdrInfo(*((RendererInfo*)rdr_info)))
		return rdr_info;

	return NULL;
}

BOOL WindowInstance::updateSpecialField(int field_id, DWORD param)
{
	BOOL bret;
	bret = ComponentInstance::updateSpecialField(field_id, param);

	if(field_id == PropRenderer && param!=0 && IsWindow(hwnd))
	{
		DWORD* params = (DWORD*)param;
		int element_id = (int)params[0];
		RendererInstance * rdrInst = (RendererInstance*)params[1];
		if(rdrInst == NULL)
			return bret;

		int rdr_res_id = (int)getField(field_id);
		if(rdr_res_id == rdrInst->getID())
		{
			rdrInst->updatePreviewWindow(element_id, hwnd);
			return TRUE;
		}
	}

	return bret;
}

int WindowInstance::syncPreviewWindow(int id)
{
	if(!IsWindow(hwnd))
		return SPWE_NOHANDLE;

	if(id >= PropClass && id < PropMax)
	{
		switch(id){
		case PropX:
		case PropY:
		case PropWidth:
		case PropHeight:
			{
				int x, y, w, h;
				getLocation(x, y);
				getSize(w, h);
				MoveWindow(hwnd, x, y, w, h, TRUE);
				return SPWE_OK;
			}
		case PropText:
			{
				SetWindowText(hwnd, getCaption());
				return SPWE_OK;
			}
		case PropRenderer:
			//reset the renderer
			resetRenderer();
			return SPWE_OK;
		case PropBkColor:
			SetWindowBkColor(hwnd, color2Pixel(getBkColor()));
			return SPWE_OK;
		case PropFont:
			{
				int font_id;
				PLOGFONT old = ::SetWindowFont(hwnd, CreateLogFontByName(getFont()));

				for(font_id = 0; font_id < NR_SYSLOGFONTS; font_id++)
				{
					if (old == ::GetSystemFont (font_id))
						return SPWE_OK;
				}
				::DestroyLogFont(old);
				return SPWE_OK;
			}
		}
	}
	else if(id >= PropStyleBegin && id < PropStyleEnd)
	{
		DWORD dwStyle = getStyle();
		DWORD oldStyle = GetWindowStyle(hwnd);
		ExcludeWindowStyle(hwnd, 0xFFFFFFFF);
		IncludeWindowStyle(hwnd, dwStyle);

		if(useNewControlSet())
		{
			if((dwStyle&0xFFFF) != (oldStyle&0xFFFF)) //control's update
			{
				mWidget * widget = ncsObjFromHandle(hwnd);
				if(widget){
					_c(widget)->refresh(widget);
					resetNCSProps();
				}
			}
		}

		if((dwStyle&WS_DISABLED) != (oldStyle&WS_DISABLED))
		{
			//send MSG_ENABLE
			::SendMessage(hwnd, MSG_ENABLE,dwStyle&WS_DISABLED?FALSE:TRUE,0);
		}

		//::UpdateWindow(hwnd, TRUE);
		InvalidateRect(hwnd, NULL,TRUE);
		return SPWE_OK;
	}
	else if(id >= PropExStyleBegin && id < PropExStyleEnd)
	{
		DWORD dwExStyle = getExStyle();
		ExcludeWindowExStyle(hwnd, 0xFFFFFFFF);
		IncludeWindowExStyle(hwnd, dwExStyle);
		//::UpdateWindow(hwnd, TRUE);
		InvalidateRect(hwnd,NULL, TRUE);
		return SPWE_OK;
	}
	else if(id >= PropEventBegin && id <= PropEventEnd)
	{
		return SPWE_IGNORED;
	}
	else
	{
		//property
		if(useNewControlSet())
		{
			Value value = getField(id);
			FieldType * ft = getClass()->getFieldType(id);
			if(ft == NULL)
				return SPWE_NOFIELD;

			if (ft->vtype->getType() == VT_IMAGE) {
				RegisterResFromFile(HDC_SCREEN,(const char*)(ft->vtype->toBinary(value)));
				ncsSetProperty(hwnd, id,
						(DWORD)GetBitmapFromRes(Str2Key((const char*)(ft->vtype->toBinary(value)))));
			}
			else
			{
				if(IS_EXTEND_PROP(id))
					{
						ncsSetProperty(hwnd, id, ft->vtype->toBinary(value));
						return SPWE_OK;
					}
				else if(ft->vtype->getType() == VT_INT){
					DWORD propvalue = ft->vtype->toBinary(value);
					DWORD newvalue;
					if(!ncsSetProperty(hwnd, id, propvalue))
						return SPWE_REJECT;

					if((newvalue=ncsGetProperty(hwnd, id)) != propvalue)
					{
						//invalidate value
						setField(id, newvalue);
						return SPWE_NEWVALUE;
					}
				}
				else
				{
					if(!ncsSetProperty(hwnd, id, ft->vtype->toBinary(value)))
						return SPWE_REJECT;
				}
			}
			return SPWE_OK;
		}
	}
	return SPWE_IGNORED;
}

void WindowInstance::resetRenderer()
{
	ComponentInstance * parent = getParent();
	if(parent && dynamic_cast<PageWindowInstance*>(parent)!=NULL){
		((PageWindowInstance*)parent)->reAddPage(this);
	}
	else{
		destroyPreviewWindow();
		createPreviewWindow();
	}
}

const char* WindowInstance::getControlClass()
{
	ValueType *vtype = StringValueType::getInstance();
	if(vtype)
	{
		return (const char*)vtype->toBinary(getField(PropClass));
	}
	return NULL;
}

const char* WindowInstance::getCaption()
{
	ValueType * vtype = _class->getFieldValueType(PropText);
	if(vtype)
	{
		const char* caption =
			(const char*)vtype->toBinary(getField(PropText));
		return caption?caption:strNull;
	}
	return strNull;
}

const char * WindowInstance::getFont()
{
	ValueType * vtype = _class->getFieldValueType(PropFont);
	if(vtype)
	{
		const char* font =
			(const char*)vtype->toBinary(getField(PropFont));
		return font?font:strNull;
	}
	return strNull;
}

#define RectTrans(hwnd, prt, Trans) do{ \
	Trans(hwnd, &((prt)->left), &((prt)->top)); \
	Trans(hwnd, &((prt)->right), &((prt)->bottom)); \
}while(0)

#define RectClientToScreen(hwnd, prt) RectTrans(hwnd, prt, ClientToScreen)

#define RectScreenToWindow(hwnd, prt) RectTrans(hwnd, prt, ScreenToWindow)

#define RectClientToWindow(hwnd, prt) RectTrans(hwnd, prt, ClientToWindow)

inline static void ControlToMainRect(RECT *prt, HWND hwnd, HWND hMain)
{
	if(hwnd == hMain){
		RectClientToWindow(hwnd, prt);
	}
	else{
		RectClientToScreen(hwnd, prt);
		RectScreenToWindow(hMain, prt);
	}
}

static void draw_gride(HWND hwnd, HDC hdc, int space){
	RECT rt;
	gal_pixel clr;

	GetClientRect(hwnd, &rt);

	//DP(0x20, "hwnd=%p,--------- draw -grid", hwnd);

	if(hdc == 0){
		HWND hMain = GetMainWindowHandle(hwnd);
		hdc = GetSecondaryDC(hMain);

		if(hwnd == hMain){
			ControlToMainRect(&rt, hwnd, hMain);
		}
		else{
			ControlToMainRect(&rt, hwnd, hMain);
			RECT rtParent;
			HWND hParent = GetParent(hwnd);
			GetClientRect(hParent, &rtParent);
			ControlToMainRect(&rtParent, hParent, hMain);
			IntersectRect(&rt, &rt, &rtParent);
		}
	}


	clr = ~GetWindowBkColor(hwnd);
	if(space < 5)
		space = 5;


	for(int x=rt.left; x<rt.right; x += space){
		for(int y=rt.top; y < rt.bottom; y += space){
			SetPixel(hdc, x, y, clr);
		}
	}
}

int WindowInstance::_main_window_proc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	WindowInstance* wi = (WindowInstance*)GetWindowAdditionalData(hwnd);

	if(!wi || (wi->prevWndState & ComponentInstance::PreWndPreviewing))
	{
		if(wi && wi->old_proc)
			return (wi->old_proc)(hwnd, message, wParam, lParam);
		else
			return DefaultMainWinProc(hwnd, message, wParam, lParam);
	}

	if( !(wi->prevWndState&ComponentInstance::PreWndUpdating)){
		if( message >= MSG_FIRSTPAINTMSG && message <= MSG_LASTPAINTMSG)
		{
			if(message == MSG_PAINT){
				HDC hdc = BeginPaint(hwnd);
				EndPaint(hwnd, hdc);
			}
			return 0;
		}
	}

	//DP(0x20,"----- message=0x%08X --- Begin", message);

	switch(message){
	case MSG_ERASEBKGND:
		{
/*			PBITMAP pbmp = NULL;
			int mode;
			if(wi){
				pbmp = (PBITMAP) GetResource(wi->m_imgBkId);
				mode = wi->m_imgMode;
			}
			if(pbmp){
				draw_bkimage(hwnd,(HDC)wParam, pbmp, mode);
			}
			else*/
			//TODO Set the back image

			if(wi->old_proc)
				(wi->old_proc)(hwnd, message, wParam, lParam);
			else
				DefaultMainWinProc(hwnd, message, wParam, lParam);
			if(wi->prevWndState&PreWndSnapGrid)
				draw_gride(hwnd, (HDC)wParam, 8);
			return TRUE;
		}
	case MSG_DESTROY:
		SetWindowAdditionalData(hwnd, 0);
		break;
	}

	if(wi->old_proc)
		return (wi->old_proc)(hwnd, message, wParam, lParam);

	return DefaultMainWinProc(hwnd, message, wParam, lParam);
}


////////////////////////////////
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

/*
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
void WindowInstance::toLuaTable(void *luaState, int table_idx)
{
	lua_State *L = (lua_State*)luaState;
	SET_TABLE(L, table_idx, ctrlClass, lua_pushstring(L, getControlClass()));
	int x, y, w, h;
	getLocation(x,y);
	getSize(w,h);
	SET_TABLE(L, table_idx, x, lua_pushinteger(L, x));
	SET_TABLE(L, table_idx, y, lua_pushinteger(L, y));
	SET_TABLE(L, table_idx, w, lua_pushinteger(L, w));
	SET_TABLE(L, table_idx, h, lua_pushinteger(L, h));
	SET_TABLE(L, table_idx, style, lua_pushinteger(L, getStyle()));
	SET_TABLE(L, table_idx, exstyle, lua_pushinteger(L, getExStyle()));
	SET_TABLE(L, table_idx, caption, lua_pushstring(L, getCaption()));
	SET_TABLE(L, table_idx, bk_color, lua_pushinteger(L, getBkColor()));

	//prop table
	SET_TABLE(L,table_idx, props,setPropsToLuaTable(L,1, PropClass-1));

	//set Renderer instance
	RendererInstance* rdrInst = (RendererInstance*)getRendererInstance();
	if(rdrInst)
		SET_TABLE(L, table_idx, rdr_info, rdrInst->toLuaTable(L));

}
BOOL WindowInstance::setDefRenderer(const char* defRdrName)
{
	BOOL bret = FALSE;
	map<int,Field*>::iterator it = fields.find(PropRenderer);
	if (it == fields.end()) {
		if (useNewControlSet())
			ncsSetProperty(getPreviewHandler(), NCSP_WIDGET_RDR, (DWORD)defRdrName);
		else
			SetWindowElementRenderer(getPreviewHandler(), defRdrName, NULL);
		bret = TRUE;
	}

	ComponentInstance::setDefRenderer(defRdrName);
	return bret;
}

void WindowInstance::previewWindow(BOOL bPreview)
{
	ComponentInstance::previewWindow(bPreview);
	if(!bPreview)//cancel preivew
	{
		SetNullFocus(hwnd); //cancel the focus
		//reset the content
		SetWindowText(hwnd,getCaption());
		resetNCSProps();
	}
}


