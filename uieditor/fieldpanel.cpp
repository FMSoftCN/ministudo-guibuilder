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
#include "msd_intl.h"
using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"

#include "ui-event-id.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "editable-listview.h"
#include "fieldpanel.h"
#include "component-instance.h"

FieldPanel::FieldPanel(PanelEventHandler* handler)
:Panel(handler)
,EditableListView(this)
{
	// TODO Auto-generated constructor stub
	instance = NULL;
}

FieldPanel::~FieldPanel() {
	// TODO Auto-generated destructor stub
}

HWND FieldPanel::createPanel(HWND hParent)
{
    RECT rt;
	::GetClientRect(hParent,&rt);

	Create(hParent, 0, 0,RECTW(rt), RECTH(rt), WS_CHILD|WS_VISIBLE|LVCF_TREEVIEW|WS_HSCROLL|WS_VSCROLL,0);
	AddColumn(0, 134, _("Name"),  0, NULL, LVHF_CENTERALIGN);
	AddColumn(1, 110, _("Value"),  0, NULL, LVHF_CENTERALIGN);

	return getHandler();
}

void FieldPanel::setInstance(Instance *instance, UndoRedoObserver *observer)
{
	setUndoRedoObserver(observer);
	if(instance == this->instance)
		return;

	//clear all items
	clearAll();

	this->instance = instance;

	//insert items
	if(this->instance)
	{
		Class *cls = this->instance->getClass();
		ValueType * vtype = getIDValueType();
		if(vtype){
			//insert id
			int inst_id = this->instance->getID();
			appendField(0, "ID", inst_id,vtype);
		}
		//insert other filed
		if(cls)
		{
			int at = 1;
			cls->enumFields(this, TRUE,(DWORD)&at);
			//sort the item
			PFNLVCOMPARE sortFunc = getSortFunc();
			if(sortFunc){
				SortItems(NULL,sortFunc);
			}//end osrtFunc
		}//end cls
	}//end if this->instance
}

//implement EditableListViewHandler
HWND FieldPanel::getEditor(HWND hParent, DWORD add_data, int sub_index)
{
	if(sub_index != 1)
		return HWND_INVALID;

	Value value;
	if(!instance)
		return HWND_INVALID;

	int id = (int)add_data;
	ValueType *vtype;
	if(id == 0) //its ID
	{
		vtype = getIDValueType();
		value = (Value)instance->getID();
	}
	else
	{
		if(instance->getFieldAttr(id) != 0) //cannot be changed
			return HWND_INVALID;
		vtype = getFieldValueType(id);
		value = instance->getField(id);
	}

	return vtype?vtype->retrieveEditor(hParent,value,this,(DWORD)instance):HWND_INVALID;
}
//implement InstanceNotificationHandler
Value FieldPanel::updateValue(Value value, ValueType *vtype,DWORD mask)
{
	//HLVITEM hlv;
	if(instance && vtype)
	{
		//get current id
		int id;
		if(!getCurData((DWORD*)&id))
				return 0;
		ValueType *tvtype;

		if(id == 0){
			//tvtype = ValueType::getValueType("int");
			//InstancePropertyUndoRedoCommand *cmd =
			//	new InstancePropertyUndoRedoCommand(instance,(int)id);

			//pushUndoRedoCommand(cmd);
			instance->setID((int)value);
		}
		else
		{
			tvtype = getFieldValueType(id);
			if(tvtype != vtype)
				return 0;

			pushUndoRedoCommand(new InstancePropertyUndoRedoCommand(instance,(int)id,instance->getField(id), !instance->isSettedField((int)id)));
			instance->setField(id, value,TRUE);
		}


		//TODO update to parent
		sendEvent(FIELDPANEL_INSTANCE_FIELD_CHANGED, (DWORD)instance, (DWORD)id);

		return value;

	}
	return 0;
}

BOOL FieldPanel::updatingValue(Value old_value, Value new_value,ValueType *vtype,DWORD mask)
{
	if(old_value != new_value)
	{
		DWORD id;
		//new InstancePropertyUndoRedoCommand
		if(!getCurData(&id) || id <= 0)
			return TRUE;

		return TRUE;
	}
	return FALSE;
}

void FieldPanel::updateFieldItem(int id, ValueType *vtype/*=NULL*/)
{
	if(instance == NULL)
		return ;

	HLVITEM hlv = findByAddData(id);
	if(hlv == (HLVITEM)0)
		return ;

	if(vtype == NULL)
		vtype = getFieldValueType(id);

	if(vtype == NULL)
		return ;

	Value value = instance->getField(id);

	SetSubitemText(hlv, 1, vtype->toString(value).c_str());

}
//implement FieldEnumHandler
BOOL FieldPanel::setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user)
{
	if(name){
		if(instance->getFieldAttr(id) != 0)
			return TRUE;

		int *pat = (int*)user;
		appendField(id, name, instance->getField(id) , vtype, *pat);
		(*pat) ++;
	}
	return TRUE;
}

void FieldPanel::appendField(int id, const char* name,Value value,ValueType* vtype, int nItem)
{
	if(name == NULL || vtype == NULL)
		return;
	//add item
	HLVITEM hlv = AddItem(0, nItem, 26, (DWORD)id,0);

	if(hlv == 0)
		return;

	//insert name
	SetSubitemText(hlv,0,name);
	SetSubitemText(hlv,1,vtype->toString(value).c_str());
}

void FieldPanel::updateEditingField()
{
	HWND hEditor = getCurrentEditor(1);
	int id;
	if(!getCurData((DWORD*)&id))
		return ;
	if(::IsWindow(hEditor))
	{
		if(id == 0) //instance Id
		{
			::SendMessage(hEditor,VTM_RESETVALUE, 0, (LPARAM)instance->getID());
		}
		else
		{
			Value value = instance->getField(id);
			::SendMessage(hEditor, VTM_RESETVALUE, 0, (LPARAM)value);
		}
	}
	else
	{
		HLVITEM hlv = hOldSelItem;
		if(hlv)
		{
			ValueType * vt = instance->getClass()->getFieldValueType(id);
			Value value = instance->getField(id);
			if(vt == NULL)
				return;

			//reset data
			SetSubitemText(hlv,1,vt->toString(value).c_str());
		}
	}
}

void FieldPanel::refreshFields(Instance *inst, int *ele_ids)
{
	if(inst == NULL || inst != instance)
		return;

	//refresh all 
    if (ele_ids == NULL) {
        LVITEM lvitem;
        int id = 0, idx = 0, itemCount = 0;

        itemCount = GetItemCount();

        while (idx < itemCount) {
            lvitem.nItem = idx;
            if (GetItem(0, &lvitem) == LV_OKAY) {
                id = lvitem.itemData;
                if (id > 0) {
                    Value value = instance->getField(id);
                    ValueType *vtype = instance->getClass()->getFieldValueType(id);
                    if(vtype) {
                        SetSubitemText(idx, 1, vtype->toString(value).c_str());
                    }
                }
            }
            idx ++;
        }
        updateEditingField();
    }
    else {
        //refresh specified items
        while (*ele_ids) {
            refreshField(inst, *ele_ids);
            ele_ids++;
        }
    }
}

void FieldPanel::refreshField(Instance* inst, int id)
{
	if(inst == NULL || inst != instance)
		return ;

	int field_id = -1;
	getCurData((DWORD*)&field_id);

	if(field_id == id)
	{
		updateEditingField();
		return ;
	}

	HLVITEM hlv = findByAddData((DWORD)id,0);
	if(hlv)
	{
		ValueType * vt = instance->getClass()->getFieldValueType(id);
		Value value = instance->getField(id);
		if(vt == NULL)
			return;

		//reset data
		SetSubitemText(hlv,1,vt->toString(value).c_str());
	}
}

void FieldPanel::refreshInstanceIdName(int inst_id, const char* name)
{
	if(instance && instance->getID() == inst_id)
	{
		int field_id = -1;
		getCurData((DWORD*)&field_id);
		if(field_id == 0) //id
			updateEditingField();
		else
		{
			HLVITEM hlv = findByAddData(0,0);
			if(hlv)
			{
				SetSubitemText(hlv,1,name);
			}
		}
	}
}

void FieldPanel::onRButtonUp(int x, int y)
{
    if (instance) {
        mapex<int, int> idsets;
					DWORD id;
					if(!getCurData(&id))
						return ;
					if(id == 0 || !instance->enableClean((int)id)){
						idsets[ResEditor::UI_MENUCMD_SETDEFVALUE] = MAP_MASK_STATE(MIIM_STATE,MFS_DISABLED);
						idsets[ResEditor::UI_MENUCMD_SETDEFALL] = 0;
					}

        HMENU hMenu = g_env->createPopMenuFromConfig(ResEditor::UI_SET_POPMENU, idsets);

        ClientToScreen(&x, &y);
        TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_DESTROY, x, y,m_hWnd);
    }
}

void FieldPanel::setDefValue()
{
	//get current id
	int id;
	if(!instance)
		return;

	if(getCurData((DWORD*)&id))
	{
		if(id <= 0 || !instance->isFieldExist(id))
			return;

		//undo redo support
		InstancePropertyUndoRedoCommand *cmd =
			new InstancePropertyUndoRedoCommand(instance,(int)id, instance->getField((int)id),FALSE);

		pushUndoRedoCommand(cmd);
		/////

		instance->cleanField(id);
		updateEditingField();
		sendEvent(FIELDPANEL_INSTANCE_FIELD_CHANGED, (DWORD)instance, (DWORD)id);
	}
}

void FieldPanel::setAllDefValue()
{
	if(!instance)
		return;

    int size = instance->getFieldsSize();
    if (size <= 0)
        return;

	if(YesNoBox(_("Information"), _("All the properties would be unrecoverable after erasing. Are you sure you want to erase them?"))==IDNO)
		return;

    mapex<int, Field*> fields = instance->getFields();
    int *ids, i = 0;
	ids = new int(size + 1);

    for (mapex<int, Field*>::iterator it = fields.begin();
            it != fields.end(); ++it, ++i) {
        ids[i] = it->first;
    }
    ids[size] = 0;

	cleanAll();

	//refresh all items on panel
	LVITEM lvitem;
	int id = 0, idx = 0, itemCount = 0;

	itemCount = GetItemCount();

	while (idx < itemCount) {
		lvitem.nItem = idx;
		if (GetItem(0, &lvitem) == LV_OKAY) {
			id = lvitem.itemData;
			if (id > 0) {
				Value value = instance->getField(id);
				ValueType *vtype = instance->getClass()->getFieldValueType(id);
				if(vtype) {
					SetSubitemText(idx, 1, vtype->toString(value).c_str());
				}
			}
		}
		idx ++;
	}
	updateEditingField();
	sendEvent(FIELDPANEL_INSTANCE_FIELD_RESET, (DWORD)instance, (DWORD)ids);
	delete[] ids;
}

BOOL FieldPanel::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{

	if(iMsg == MSG_RBUTTONUP)
	{
		BOOL bret = EditableListView::WndProc(iMsg, wParam, lParam, pret);
		onRButtonUp(LOSWORD(lParam), HISWORD(lParam));
		return bret;
	}
	else if(iMsg == MSG_COMMAND)
	{
		switch(LOWORD(wParam))
		{
        case ResEditor::UI_MENUCMD_SETDEFVALUE:
			setDefValue();
			break;
        case ResEditor::UI_MENUCMD_SETDEFALL:
			setAllDefValue();
			break;
		}
	}
    else if(iMsg == MSG_SHOWPAGE) {
        updateEditors();
        SetFocus(getCurrentEditor(1));
    }

	return EditableListView::WndProc(iMsg, wParam, lParam, pret);
}
