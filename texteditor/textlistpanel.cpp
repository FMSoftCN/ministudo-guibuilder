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

#include <string>
#include <map>
#include <list>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include "msd_intl.h"

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"

#include "panel.h"
#include "panellayout.h"
#include "editable-listview.h"
#include "reseditor.h"
#include "texteditor.h"
#include "textlistpanel.h"

TextListPanel::TextListPanel(PanelEventHandler* handler)
    :Panel(handler)
    ,EditableListView(this)
    ,pNameValueType(NULL)
    ,pdefText(NULL)
    ,pcurText(NULL)
    ,preses(NULL)
{
	pIdValueType = (IntValueType*)ValueType::getValueType("int");
}

TextListPanel::~TextListPanel()
{
    //before deleting defaultValueType and curValueType, should delete editor 
    //window to avoid coredump. see retrieveEditor implementation for detail.
    HWND child = GetChild((LINT)&defaultValueType);
    if (child)
        ::DestroyWindow(child);

    child = GetChild((LINT)&curValueType);
    if (child)
        ::DestroyWindow(child);
}

HWND TextListPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent,&rt);

	Create(hParent, 0, 0, RECTW(rt), RECTH(rt),
			WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 0);

	AddColumn(0, 200, _("Name"),  0, NULL, LVHF_CENTERALIGN);
	AddColumn(1, 105, _("Id"),  0, NULL, LVHF_CENTERALIGN);
	AddColumn(2, 310, _("Default Text"),  0, NULL, LVHF_CENTERALIGN);
	AddColumn(3, 310, _("Current Text"),  0, NULL, LVHF_CENTERALIGN);

	return getHandler();
}

void TextListPanel::setContents(map<int, ResEditor::Resource*> & reses, list<TeNode*> &configList)
{
	int i;
	list<TeNode*>::iterator listIter;
	TeNode *txtNode;
	map<int, ResEditor::Resource*>::iterator it;

	for(listIter = configList.begin(); listIter != configList.end(); listIter++)
	{
		txtNode = *listIter;
		if(0 != (txtNode->status & TeNode::DEF_LANG)){
			pdefText = &txtNode->langMap;
		}
		if(0 != (txtNode->status & TeNode::CUR_LANG)){
			pcurText = &txtNode->langMap;
		}
	}

	preses = &reses;

	for(it = reses.begin(), i = 0; it != reses.end(); it++, i++)
	{
		insertText(i, it->second);
	}
}

HWND TextListPanel::getEditor(HWND hParent, DWORD add_data, int sub_index)
{
	ValueType * vt = NULL;
	Value value = 0;
	ResManager * resMgr = NULL;
	int id = (int)add_data;

	if(id < DEFINABLE_ID_BASE && id != 0)
		return HWND_INVALID;

	resMgr = g_env->getResManager(NCSRT_TEXT);

	if(!resMgr)
		return HWND_INVALID;

	switch(sub_index)
	{
		case 0: //name
		{
			if(0 == id)
			{
				if(!pNameValueType)
					pNameValueType = (StringValueType*)ValueType::getValueType("string");
				vt = pNameValueType;
				value = pNameValueType->newValue("");
				break;
			}
			const char* strName = resMgr->idToName(id);
			if(strName == NULL || *strName == '\0')//anonymity
				return HWND_INVALID;

			if(!pNameValueType)
				pNameValueType = (StringValueType*)ValueType::getValueType("string");
			vt = pNameValueType;
			value = pNameValueType->newValue(strName);
			break;
		}
		case 1://id
		{
			//TODO FIXED ME, Check anonymity by Id Range
			const char* strName = resMgr->idToName(id);
			if((strName == NULL || *strName == '\0') && id != 0)//anonymity
				return HWND_INVALID;

			vt = pIdValueType;
			value = (int)(add_data&0xFFFF);
			break;
		}
		case 2://default
			if(pdefText)
			{
				if(0 == id)
				{
					vt = &defaultValueType;
					value = defaultValueType.newValue("");
					break;
				}
				map<int, string>::iterator it = pdefText->find(id);
				if(it != pdefText->end())
					value = defaultValueType.newValue(it->second.c_str());
				else
					value = defaultValueType.newValue("");
				vt = &defaultValueType;
			}
			break;
		case 3: //current
			if(pcurText)
			{
				if(0 == id)
				{
					vt = &curValueType;
					value = curValueType.newValue("");
					break;
				}

				map<int, string>::iterator it = pcurText->find(id);
				if(it != pcurText->end())
					value = curValueType.newValue(it->second.c_str());
				else
					value = curValueType.newValue("");
				vt = &curValueType;
			}
		/*	else if(pcurText == NULL)
			{
				vt = &curValueType;
				value = curValueType.newValue("");
			}*/
			break;
	}
	if(vt == NULL)
		return HWND_INVALID;

	return vt->retrieveEditor(hParent, value, this, sub_index);
}

Value TextListPanel::updateValue(Value value, ValueType *vtype, DWORD mask)
{
	int id;
	if(!getCurData((DWORD*)&id))
		return 0;

	DWORD param2 = id;

	switch(mask)
	{
	case 1://set Id
        setCurData((int)value | (ID2TYPE(id) <<16));
        g_env->updateResId(id, (int)value | (ID2TYPE(id) <<16));
		break;

	case 2://set def text
		if(pdefText){
			(*pdefText)[id] = (const char*)value;
			if(!pcurText)
				g_env->updateResValue(id);
		}
		break;
	case 3://set cur text
		if(pcurText){
			(*pcurText)[id] = (const char*)value;
			g_env->updateResValue(id);
		}
		break;
	}

	sendEvent(EVENT_UPDATE_VALUE, mask, param2);
	return 0;
}

BOOL TextListPanel::updatingValue(Value old_value, Value new_value, ValueType *vtype, DWORD mask)
{
	int id = -1;
	if(!getCurData((DWORD*)&id))
		return FALSE;

	ResManager *resMgr = g_env->getResManager(NCSRT_TEXT);
	if(!resMgr)
		return FALSE;

	switch(mask)
	{
		case 0://set name
			if(resMgr->setResName(id, (const char*)new_value) == -1)
				return FALSE;
			break;
		case 1://set Id
			if(!resMgr->setResId(id, (int)(new_value|(NCSRT_TEXT<<16))))
				return FALSE;
			break;
		case 2://set def text
			if(!pdefText)
				return FALSE;
			break;
		case 3://set cur text
			if(!pcurText)
				return FALSE;
			break;
	}

	return TRUE;
}

void TextListPanel::setSubText(TeNode *st_TeNode)
{
	map<int, string>::iterator mapIter;
	map<int, string>::iterator iter;
	int i = 0;
	int id = 0, tmpid = 0;
	char sId[MAX_ID_LEN] = {0};

	for(i = 0; i < GetItemCount(); ++i)
	{
		GetSubitemText(i, 1, sId, MAX_STRING_LEN);
		tmpid = atoi(sId);
		id = (2<<16) + tmpid;

		SetSubitemText(i, 3, "");
		iter = st_TeNode->langMap.find(id);
		if(iter != st_TeNode->langMap.end()){
			SetSubitemText(i, 3, iter->second.c_str());
		}
	}
	pcurText = &(st_TeNode->langMap);

}

void TextListPanel::insertText(int item, ResEditor::Resource* res)
{
	HLVITEM hlv;

	if(res == NULL)
		return;

	hlv = AddItem(0, item, 26, res->id, 0);
	updateText(hlv, item, res);
}

void TextListPanel::updateText(HLVITEM hlvItem, int item, ResEditor::Resource* res)
{
	char szTemp[20];
	map<int,string>::iterator mapIter;

	::ShowWindow(GetHandle(), SW_HIDE);

	//name
	FillSubitem(hlvItem, 0, item, 0, res->name.c_str(), PIXEL_black, 0);
	//id
	sprintf(szTemp, "%d", res->id&0xFFFF);
	FillSubitem(hlvItem, 0, item, 1, szTemp, PIXEL_black, 0);
	//default text
	if(pdefText){
		mapIter = pdefText->find(res->id);
		if(mapIter != pdefText->end()){
			FillSubitem(hlvItem, 0, item, 2, mapIter->second.c_str(), PIXEL_black, 0);
		}
	}
	//current text
	if(pcurText){
		mapIter = pcurText->find(res->id);
		if(mapIter != pcurText->end()){
			FillSubitem(hlvItem, 0, item, 3, mapIter->second.c_str(), PIXEL_black, 0);
		}
	}

	::ShowWindow(GetHandle(), SW_SHOW);
}

void TextListPanel::updateList(list<TeNode*> &configList, map<int, ResEditor::Resource*> & reses)
{
	list<TeNode*>::iterator listIter;
	map<int, string>::iterator iter;
	TeNode *pNode = NULL;
	int i, id, cnt, column;
	char sId[MAX_STRING_LEN] = "";
	int curExist = 0;

	for(listIter = configList.begin(); listIter != configList.end(); ++listIter)
	{
		pNode = *listIter;

		if(pNode->status & TeNode::DEF_LANG){
			i = 0;
			column = 2;
			pdefText = &pNode->langMap;
		} else if(pNode->status & TeNode::CUR_LANG){
			i = 0;
			column = 3;
			pcurText = &pNode->langMap;
			curExist = 1;
		} else {
			continue;
		}
		for(cnt = 0; cnt < GetItemCount(); cnt++, i++)
		{
			GetSubitemText(i, 1, sId, MAX_STRING_LEN);
			id = (2<<16) | atoi(sId);

			SetSubitemText(i, column, "");
			iter = pNode->langMap.find(id);
			if(iter != pNode->langMap.end())
				SetSubitemText(i, column, iter->second.c_str());
		}
	}

	if (curExist == 0) {
		i = 0;
		column = 3;
		for (cnt = 0; cnt < GetItemCount(); cnt++, i++) {
				SetSubitemText(i, column, "");
		}
		pcurText = NULL;
	}
}

void TextListPanel::updateHead(TeNode *def, TeNode *cur)
{
	LVCOLUMN col;
	col.pszHeadText = (char *)calloc(1, 256*sizeof(char));
	col.nTextMax = 255;

	if (def) {
		col.nCols = 2;
		sprintf(col.pszHeadText, _("Default Text (%s_%s)"), def->langType,
				def->country);
		ModifyHead(&col);
	}
	if (cur) {
		col.nCols = 3;
		sprintf(col.pszHeadText, _("Current Text (%s_%s)"), cur->langType,
				cur->country);
		ModifyHead(&col);
	}
	else if (cur == NULL)
	{
		col.nCols = 3;
		strcpy(col.pszHeadText, _("Current Text (not set)"));
		ModifyHead(&col);
	}
	free(col.pszHeadText);
}

void TextListPanel::onNewTextInserted(int id)
{
	if(id <= 0 || ID2TYPE(id) != NCSRT_TEXT)
		return;

	if(preses == NULL)
		return ;

	//insert new id
	int i;
	if(findByAddData(id, 0))
		return ;

	for(i=0; i<GetItemCount(); i++)
	{
		int insertId = (int)GetItemAddData(0,i);
		if(insertId > id){
			insertText(i==0 ? 0 : (i-1), (*preses)[id]);
			return;
		}
	}
	insertText(i,(*preses)[id]);
}

void TextListPanel::onTextUpdated(int id)
{
	if(id <= 0 || ID2TYPE(id) != NCSRT_TEXT)
		return;

	//insert new id
	for(int i=0; i<GetItemCount(); i++)
	{
		int tId = (int)GetItemAddData(0,i);
		if(tId == id){
			updateText(0, i, (*preses)[id]);
			return;
		}
	}
}

void TextListPanel::updateId(int oldid, int newid)
{
    HLVITEM p = 0;
    LVFINDINFO info;

    info.flags = LVFF_ADDDATA;
    info.iStart = 0;
    info.addData = oldid;
    p = FindItem (0, &info);

    if (p) {
        char szTemp[20];
        SetItemAddData(p, newid);
        sprintf(szTemp, "%d", newid&0xFFFF);
        SetSubitemText(p, 1, szTemp);
    }
}

void TextListPanel::updateIdName(int id)
{
    HLVITEM p = 0;
    LVFINDINFO info;
	TextEditor* resMgr = (TextEditor*)(g_env->getResManager(NCSRT_TEXT));

    info.flags = LVFF_ADDDATA;
    info.iStart = 0;
    info.addData = id;
    p = FindItem (0, &info);

    if (p) {
        SetSubitemText(p, 0, resMgr->idToName(id));
    }
}

void TextListPanel::appendText(ResEditor::Resource* res)
{
	if(res == NULL)
		return;

	int item = GetItemCount();
	insertText(item,res);
	SelectItem(item, 0);
	updateEditors();
}

void TextListPanel::deleteText()
{
	deleteItem(0,TRUE);
}

int TextListPanel::getSelection(void)
{
	char txt_id [128];

	HLVITEM hlv = GetSelectedItem();
	GetSubitemText(hlv, 1, txt_id, 127);
	return strtol(txt_id, NULL, 0);
}

void TextListPanel::deleteItemById(int id)
{
	HLVITEM hitem = findByAddData((DWORD)id, 0);
	deleteItem(hitem, TRUE);
}

BOOL TextListPanel::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{

	if(iMsg == MSG_RBUTTONUP)
	{
		int x = LOSWORD(lParam);
		int y = HISWORD(lParam);
		::ClientToScreen(getHandler(), &x, &y);
		onRButtonUp(x, y, wParam);
	}
	else if(iMsg == MSG_COMMAND)
	{
		onPopupMenuCmd(LOWORD(wParam));
	}

	return EditableListView::WndProc(iMsg, wParam, lParam, pret);
}

//execute popup menu command
void TextListPanel::onPopupMenuCmd(int id)
{
	TextEditor* resMgr = (TextEditor*)(g_env->getResManager(NCSRT_TEXT));
	resMgr->executeCommand(id, 0, 0);
}

void TextListPanel::onRButtonUp(int x, int y, DWORD key_flag)
{
	mapex<int, int>idsets;

	idsets[TextEditor::TXT_MENUCMD_NEW] = 0;
	idsets[TextEditor::TXT_MENUCMD_DEL] = 0;
	idsets[TextEditor::TXT_MENUCMD_TRANS] = 0;
	idsets[TextEditor::TXT_MENUCMD_TRANSALL] = 0;

	HMENU hMenu = g_env->createPopMenuFromConfig(TextEditor::TXT_POPMENU, idsets);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_DESTROY, x, y, getHandler());
}
