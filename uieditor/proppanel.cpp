/*
 * proppanel.cpp
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <vector>
#include <map>
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
#include "editable-listview.h"
#include "fieldpanel.h"

#include "proppanel.h"
#include "ui-event-id.h"

#include "component-instance.h"

PropertyPanel::PropertyPanel(PanelEventHandler* handler)
:FieldPanel(handler)
{

}

PropertyPanel::~PropertyPanel() {
	// TODO Auto-generated destructor stub
}

void PropertyPanel::changeBounds(Instance *instance, DWORD bound)
{
	if(bound == 0 || instance == NULL || instance != this->instance)
		return;

	ValueType * vtype = getFieldValueType(ComponentInstance::PropX);
	if(vtype == NULL)
		return;

	if(bound & ComponentInstance::BOUND_MASK_X){
		updateFieldItem(ComponentInstance::PropX,vtype);
		//fix x<->width and y<->height sync bug
		updateFieldItem(ComponentInstance::PropWidth,vtype);
	}

	if(bound & ComponentInstance::BOUND_MASK_Y){
		updateFieldItem(ComponentInstance::PropY, vtype);
		//fix x<->width and y<->height sync bug
		updateFieldItem(ComponentInstance::PropHeight,vtype);
	}

	if(bound & ComponentInstance::BOUND_MASK_WIDTH){
		updateFieldItem(ComponentInstance::PropWidth,vtype);
	}

	if(bound & ComponentInstance::BOUND_MASK_HEIGHT){
		updateFieldItem(ComponentInstance::PropHeight,vtype);
	}

	updateEditingField();
}

BOOL PropertyPanel::setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user)
{
	if(id >= ComponentInstance::PropEventBegin && !IS_EXTEND_PROP(id))
		return FALSE;

	return FieldPanel::setField(cls, id, name, vtype, user);
}

static int getClassId(int id)
{
	if(id == 0)
		return 0;
	if(id >= ComponentInstance::PropClass && id <= ComponentInstance::PropMax)
		return 1;
	if(id >= ComponentInstance::PropStyleBegin && id <= ComponentInstance::PropStyleEnd)
		return 2;
	if(id >= ComponentInstance::PropExStyleBegin && id <= ComponentInstance::PropExStyleEnd)
		return 4;
	if(IS_EXTEND_PROP(id))
		return 10;

	return 3;
}

BOOL PropertyPanel::updatingValue(Value old_value, Value new_value,ValueType *vtype, DWORD mask)
{
	DWORD id;

	if(!getCurData(&id))
		return FALSE;

	if(id == 0 )
	{
		if(new_value<=0)
			return TRUE;

		if(NCSRT_CONTRL == ID2TYPE(instance->getID()))
		{
			//IDSTATIC can be repeate
			if((new_value&0xFFFF) == 0)
				return TRUE;
			//same id in the instance?
			ComponentInstance *prev = ((ComponentInstance*)instance)->getPrev();
			while(prev)
			{
				if(prev->getID() == (int)new_value){
					InfoBox(_("Error"), _("This ID has exist in the window!"));
					return FALSE;
				}
				prev = prev->getPrev();
			}
			ComponentInstance *next = ((ComponentInstance*)instance)->getNext();
			while(next)
			{
				if(next->getID() == (int)new_value)
				{
					InfoBox(_("Error"), _("This ID has exist in the window!"));
					return FALSE;
				}
				next = next->getNext();
			}
		}
		return TRUE;
	}
	else
	{
		if(vtype->getType() == VT_TEXT)
		{
			ComponentInstanceTextUndoRedoCommand *cmd =
				new ComponentInstanceTextUndoRedoCommand((ComponentInstance*)instance);

			pushUndoRedoCommand(cmd);

			return TRUE;
		}
		else if(id == ComponentInstance::PropWidth){
			if(new_value > RECTW(g_rcScr))
			{
				InfoBox(_("Error"),_("The Window Width must smaller than Screen Width(%d)"),RECTW(g_rcScr));
				return FALSE;
			}
		}
		else if(id == ComponentInstance::PropHeight){
			if(new_value > RECTH(g_rcScr))
			{
				InfoBox(_("Error"),_("The Window Height must smaller than Screen Height(%d)"),RECTH(g_rcScr));
				return FALSE;
			}
		}

		if(!FieldPanel::updatingValue(old_value, new_value, vtype, mask))
			return FALSE;

		if(vtype->getType() == VT_RDR)
		{
			if((int)new_value == -1) //new dialog
			{
				ResManager* resMgr = g_env->getResManager(NCSRT_RDR);
				if(!resMgr)
					return FALSE;

				const char* strClassName = instance->getClass()->getClassName();
				if(!strClassName)
					return FALSE;
				const char* str = strstr(strClassName, "::");
				if(str)
					strClassName = str;

				char szIdName[256] = "\0";
				//int newRdr(HWND hParent, const char* strClassName, char* strIdName, BOOL visibleCls, GHANDLE parent)
				int rdr_res_id;
				if(!resMgr->callSpecial("newRdr",&rdr_res_id,::GetMainWindowHandle(getHandler()),strClassName,szIdName, FALSE, 0))
					return FALSE;

				if(!VALIDID(rdr_res_id,NCSRT_RDR))
					return FALSE;

				if(!szIdName[0]){
					sprintf(szIdName,"%d",rdr_res_id);
				}
				//insert a new one
				int idx = ::SendMessage(hrdr_editor, CB_ADDSTRING,0, (LPARAM)szIdName);
				::SendMessage(hrdr_editor, CB_SETITEMADDDATA, idx, (LPARAM)rdr_res_id);
				//set selected
				::SendMessage(hrdr_editor, CB_SETCURSEL,idx, 0);
				::SendMessage(hrdr_editor,VTM_RESETVALUE, 0, (LPARAM)rdr_res_id);

				//update event
				instance->setField(ComponentInstance::PropRenderer, (Value)rdr_res_id, TRUE);
				sendEvent(FIELDPANEL_INSTANCE_FIELD_CHANGED, (DWORD)instance, (DWORD)ComponentInstance::PropRenderer);

				return FALSE;
			}
		}
		return TRUE;
	}
}

Value PropertyPanel::updateValue(Value value, ValueType *vtype,DWORD mask)
{
	DWORD id;
	if(!getCurData(&id) || id < 0)
		return 0;

	//ID has set, don't call instance->setID
	if(id == 0){
		sendEvent(FIELDPANEL_INSTANCE_FIELD_CHANGED, (DWORD)instance, (DWORD)id);
		return id;
	}
	else if(vtype->getType() == VT_INT)
	{
		BOOL bIsDefault = !instance->isSettedField(id);
		Value old_value = 0;
		if(!bIsDefault)
			old_value = instance->getField(id);
		instance->setField(id, value, TRUE);
		int ret = sendEvent(FIELDPANEL_INSTANCE_FIELD_CHANGED, (DWORD)instance, (DWORD)id);

		if(ret != 0)
		{
			if(bIsDefault)
				instance->cleanField(id,TRUE);
			else
				instance->setField(id, old_value, TRUE);
			updateEditingField();
			return old_value;
		}
		else
		{
			pushUndoRedoCommand(new InstancePropertyUndoRedoCommand(instance,(int)id,old_value, bIsDefault));
			return instance->getField(id);
		}
	}
	else
		return FieldPanel::updateValue(value, vtype, mask);
}

BOOL PropertyPanel::updateEditorContent(HWND hEditor, ValueType *vtype, Value value, DWORD mask)
{
	if(vtype->getType() == VT_RDR)
	{
		this->hrdr_editor = hEditor;

		::SendMessage(hEditor, CB_RESETCONTENT, 0, 0);

		//init value type
		//get RDR Editor
		ResManager* resMgr = g_env->getResManager(NCSRT_RDR);
		if(!resMgr)
			return FALSE;

		const char* strClassName = instance->getClass()->getClassName();
		if(!strClassName)
			return FALSE;
		const char* str = strstr(strClassName, "::");
		if(str)
			strClassName = str;

		map<int, string> rdrs;
		//void getRdrByClassName(const char* strClassName, map<int,string>*prdrs);
		if(!resMgr->callSpecial("getRdrByClassName", strClassName, &rdrs))
			return FALSE;

		//add the rdrs
		int sel_idx = -1;
		int idx;
		for(map<int,string>::iterator it = rdrs.begin(); it != rdrs.end(); ++it)
		{
			idx = ::SendMessage(hEditor, CB_ADDSTRING,0, (LPARAM)it->second.c_str());
			::SendMessage(hEditor, CB_SETITEMADDDATA, idx, (LPARAM)it->first);
			if((int)value == it->first)
				sel_idx = idx;
		}
		//add new open
		idx = ::SendMessage(hEditor, CB_ADDSTRING, 0, (LPARAM)"[New Renderer ...]");
		::SendMessage(hEditor, CB_SETITEMADDDATA, idx, (LPARAM)-1);
		if(sel_idx >= 0)
			::SendMessage(hEditor, CB_SETCURSEL,sel_idx, 0);

		return TRUE;
	}
    else if(vtype->getType() == VT_GROUP)
	{
		this->hrdr_editor = hEditor;

		::SendMessage(hEditor, CB_RESETCONTENT, 0, 0);

		ResManager* resMgr = g_env->getResManager(NCSRT_UI|NCSRT_CONTRL);
		if(!resMgr)
			return FALSE;

		map<int, string> group;
		if(!resMgr->callSpecial("getGroupList", &group))
			return FALSE;

		//add the group
		int sel_idx = -1;
		int idx;
		for(map<int,string>::iterator it = group.begin(); it != group.end(); ++it)
		{
			idx = ::SendMessage(hEditor, CB_ADDSTRING,0, (LPARAM)it->second.c_str());
			::SendMessage(hEditor, CB_SETITEMADDDATA, idx, (LPARAM)it->first);
			if((int)value == it->first)
				sel_idx = idx;
		}
		if(sel_idx >= 0)
			::SendMessage(hEditor, CB_SETCURSEL,sel_idx, 0);

		return TRUE;
	}
	return FALSE;
}



int PropertyPanel::_prop_cmp(HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortData)
{
	// 0 is ID
	// PropClass ~ PropMax < PropStyle < new control set ids < PropExStyle
	int id1 = ::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem1);
	int id2 = ::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem2);

	int clsid1 = getClassId(id1);
	int clsid2 = getClassId(id2);

	if(clsid1 == clsid2)
		return id1 - id2;
	else if(clsid1 < clsid2)
		return -1;
	else
		return 1;
}

void PropertyPanel::cleanAll()
{
	if(!instance)
		return;

	//undo redo
	InstanceAllDefPropertiesRedoUndoCommand *cmd =
		new InstanceAllDefPropertiesRedoUndoCommand(instance,0, ComponentInstance::PropEventBegin - 1);

	pushUndoRedoCommand(cmd);
	//end undo redo
	instance->cleanAll(0,ComponentInstance::PropClass-1);
	instance->cleanField(ComponentInstance::PropRenderer,FALSE);
	instance->cleanAll(ComponentInstance::PropText+1, ComponentInstance::PropEventBegin - 1);
	instance->cleanAll(ComponentInstance::PropEventBegin+1, ComponentInstance::PropExStyleEnd -1);
}

IDValueType PropertyPanel::_idValueType(NCSRT_UI|NCSRT_CONTRL);
