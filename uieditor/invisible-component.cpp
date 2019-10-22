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

using namespace std;

#include <mgncs/mgncs.h>

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"
#include "invisible-component.h"

InvisibleComponent::InvisibleComponent(Class *cls)
:ComponentInstance(cls)
{
	// TODO Auto-generated constructor stub
	pshow_bmp = getBitmapByInstance(cls);

}

InvisibleComponent::InvisibleComponent(const InvisibleComponent& invcomp)
:ComponentInstance(invcomp._class)
{
	pshow_bmp = invcomp.pshow_bmp;

	//copy maps
	invcomp.copyFields(fields);

	children = invcomp.cloneChildren(this);
}


InvisibleComponent::~InvisibleComponent() {
	// TODO Auto-generated destructor stub
}

HWND InvisibleComponent::createPreviewWindow(HWND hParent/* = HWND_INVALID*/)
{
	if(IsWindow(hwnd))
		return hwnd;
	//create special timer
	if(hParent == HWND_INVALID)
	{
		if(!parent)
			return HWND_INVALID;
		hParent = parent->getPreviewHandler();
	}

	int x, y, width, height;
	getLocation(x, y);
	getSize(width, height);
	//create a special window
	hwnd = CreateWindowEx2("static","", WS_VISIBLE|WS_BORDER|WS_THICKFRAME,0,-1,x, y, width, height, hParent, "classic", NULL,(DWORD)this);
	SetWindowCallbackProc(hwnd,InvisibleComponent::_invcomp_preview_proc);
	return hwnd;
}
void InvisibleComponent::destroyPreviewWindow()
{
	DestroyWindow(hwnd);
	hwnd = HWND_INVALID;
}
int InvisibleComponent::syncPreviewWindow(int field_id)
{
	if(field_id == PropX || field_id == PropY
			|| field_id == PropWidth || field_id == PropHeight)
	{
		int x, y, w, h;
		getLocation(x, y);
		getSize(w, h);
		MoveWindow(hwnd, x, y, w, h, TRUE);
	}
	return SPWE_OK; //directly return, the field Changed will not shown in the preview window
}

Instance * InvisibleComponent::clone()
{
	return new InvisibleComponent(*this);
}

BOOL InvisibleComponent::loadFromXML(xmlNodePtr node)
{
	if(node == NULL)
		return FALSE;

	loadSerial(node);

	//get property
	for(xmlNodePtr child = node->xmlChildrenNode; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;
		if(xhIsNode(child,"listen"))
			loadListen(child);
		else
			loadProperty(child);
	}
	return TRUE;
}

void InvisibleComponent::saveXMLToStream(TextStream *stream)
{
	if(stream == NULL)
			return ;

		char szClassName[256];
		//save instance
		stream->printf("<component class=\"%s\"",getClassName(szClassName));
		saveSerial(stream);
		stream->println(">");

		stream->indent();
		//print id
		stream->println("<ID>%d</ID>",id);
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
				ft->vtype->saveXMLStream(field->value, stream);
				stream->println("</%s>",ft->name.c_str());
			}
		}

		//save children
	/*	ComponentInstance * cinstance = children;
		while(cinstance)
		{
			cinstance->saveXMLToStream(stream);
			cinstance = cinstance->getNext();
		}*/

		//save listens
		saveListens(stream);

		stream->unindent();
		stream->println("</component>");
}

//return size of total window
int InvisibleComponent::saveBinToStream(BinStream *stream)
{
	if(stream == NULL)
		return 0;

	int size = 0;
	int id;
	FieldType * ft;
	long begin_pos;

	begin_pos = stream->tell();
	//save stream

	//1. struct NCSRM_WINHEADER

	//1). save class id
	char szClassName[32];
	sprintf(szClassName, "%s" _MGNCS_CLASS_SUFFIX, getControlClass());
	id = g_env->addString(szClassName);
	stream->save32(id);
	//2). save component id
	stream->save32(this->id&0xFFFF);

    //save the serial num
    stream->save32(getSerialNumber());

	//3). save caption id //text
	stream->save32(0);
	//4) save rdr id
	stream->save32(0);
	//5) save x, y, w, h
	stream->save32(getField(PropX));
	stream->save32(getField(PropY));
	stream->save32(0);
	stream->save32(0);
	//6) save style
	stream->save32(0);
	//7) save exstyle;
	stream->save32(0);
	//save bkcolor
	stream->save32(0);
	//save font
	stream->save32(0);
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
	for(map<int,Field*>::iterator it = fields.lower_bound(1);
		it != fields.end() && it->first < PropClass; ++it)
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
	}

	int cur_pos2 = stream->tell();

	if(count > 0)
	{
		//goto offset_props
		stream->seek(begin_pos+ (long)&(((NCSRM_WINHEADER*)0)->offset_props), StreamStorage::seek_begin);
		stream->save32(cur_pos1-begin_pos);
		stream->seek(begin_pos+ (long)&(((NCSRM_WINHEADER*)0)->nr_props),
				StreamStorage::seek_begin);
		stream->save32(count);
		size += count * sizeof(Uint32) * 3;
	}

	//save size
	stream->seek(begin_pos + (long)&(((NCSRM_WINHEADER*)0)->size), StreamStorage::seek_begin);
	stream->save32(size);

	stream->seek(cur_pos2, StreamStorage::seek_begin);

	return size;
}

char* InvisibleComponent::getClassName(char* szBuff)
{
	if(!szBuff)
		return NULL;

	const char* strName = _class->getClassName();
	if(strstr(strName,"::"))
		strcpy(szBuff, strName);
	else
		sprintf(szBuff,"component::%s", strName);

	return szBuff;
}

const char* InvisibleComponent::getControlClass()
{
	ValueType *vtype = StringValueType::getInstance();
	if(vtype)
	{
		return (const char*)vtype->toBinary(getField(PropClass));
	}
	return NULL;
}

/////////////////////////////////

LRESULT InvisibleComponent::_invcomp_preview_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	InvisibleComponent* pic = (InvisibleComponent*)GetWindowAdditionalData(hWnd);
	switch(message)
	{
	case MSG_PAINT:
		{

			HDC hdc = BeginPaint(hWnd);
			if(pic->pshow_bmp){
				RECT rc;
				GetClientRect(hWnd, &rc);
				int x = 0, y = 0, w = 0, h = 0;
				w = pic->pshow_bmp->bmHeight;
				h = pic->pshow_bmp->bmWidth;
				if(w > RECTW(rc))
				{
					w = RECTW(rc);
				}
				else
				{
					x = (RECTW(rc)-w)/2;
				}
				if(h > RECTH(rc))
				{
					h = RECTH(rc);
				}
				else
				{
					y = (RECTH(rc) - h)/2;
				}
				FillBoxWithBitmap(hdc, x,y, w, h, pic->pshow_bmp);
			}
			EndPaint(hWnd, hdc);
		}
		return 0;
	}

	return DefaultControlProc(hWnd, message, wParam, lParam);
}

PBITMAP InvisibleComponent::getBitmapByInstance(Class *cls)
{
	if(cls == NULL)
		return NULL;

	map<DWORD,BITMAP>::iterator it = _comp_bmps.find((DWORD)cls);
	if(it != _comp_bmps.end())
		return &it->second;

	//load from config file
	char szPath[1024];
	char szFile[256];
	const char* strName = cls->getClassName();
	if(strName == NULL)
		return NULL;

	if(strstr(strName,"::"))
	{
		int i;
		for(i=0; *strName; i++, strName ++)
		{
			if(strName[0] == ':' && strName[1] == ':')
				szFile[i] = '_';
			else
				szFile[i] = tolower(strName[0]);
		}
		szFile[i] = 0;
	}
	else
	{
		strcpy(szFile,"component_");
		int i = strlen(szFile);
		for(;*strName ; i++, strName ++)
			szFile[i] = tolower(strName[0]);
		szFile[i] = '\0';
	}
	//get path of icons
	sprintf(szPath,"%s/%s.png",g_env->getConfigFile("icon/uieditor/defines").c_str(),szFile);
	BITMAP &bmp = _comp_bmps[(DWORD)cls];
	// try to load from file
	if(LoadBitmapFromFile(HDC_SCREEN,&bmp,szPath) != 0)
	{
		_comp_bmps.erase((DWORD)cls);
		LOG_WARNING("Cannot Load Bitmap File In InvisibleInstance:\"%s\"", szPath);
		return NULL;
	}

	return &bmp;

}

map<DWORD, BITMAP> InvisibleComponent::_comp_bmps;

////////////////////////////////
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

/*
 * template_table.ctrlClass =
 * template_table.x = 0
 * template_table.y = 0
 * template_table.w = 0
 * template_table.h = 0
 * template_table.style =
 * template_table.exstyle =
 * template_table.caption =
 * template_table.props = { {id, value},{id,value}, {...} }
 * template_table.rdr_info = { gbl_rdr = "classic", ctrl_rdr="classic", {id, value}, {id,value}}
 * template_table.bk_color =
 */

void InvisibleComponent::toLuaTable(void *luaState, int table_idx)
{
	lua_State *L = (lua_State*)luaState;
	SET_TABLE(L, table_idx, ctrlClass, lua_pushstring(L, getControlClass()));

	//prop table
	SET_TABLE(L,table_idx, props,setPropsToLuaTable(L,1, PropClass-1));
}
