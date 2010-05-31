/*
 * ResIDListPanel.cpp
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <string>
#include <map>
#include <list>
#include "mapex.h"
#include <vector>

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

#include "editable-listview.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"
#include "resid-list-panel.h"

#define VALUEMASK 0xffff

ResIdListPanel::ResIdListPanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
	pIntValueType = (IntValueType*)ValueType::getValueType("int");

}

ResIdListPanel::~ResIdListPanel() {
	// TODO Auto-generated destructor stub
}

void ResIdListPanel::setRes(ResManager* resMgr, int type, int id, const char* name, DWORD res)
{
	char szId[32];
	int nItem = listview.GetItemCount();
	sprintf(szId,"%d",id&VALUEMASK);
	GHANDLE item = listview.AddItem(0, nItem, 26, id, 0);

	if(!item)
		return ;

	listview.SetSubitemText(item, 0, name);
	listview.SetSubitemText(item, 1, resMgr->getTypeName(type));
	listview.SetSubitemText(item, 2, szId);

}

HWND ResIdListPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent,&rt);
	listview.Create(hParent, 0, 0,RECTW(rt), RECTH(rt), WS_CHILD|WS_VISIBLE|LVCF_TREEVIEW|WS_HSCROLL|WS_VSCROLL,0);
	listview.AddColumn(0, 505, _("Name"),  0, NULL, LVHF_CENTERALIGN);
	listview.AddColumn(1, 210, _("Type"),  0, NULL, LVHF_CENTERALIGN);
	listview.AddColumn(2, 210, _("Value"),  0, NULL, LVHF_CENTERALIGN);
	listview.setHandler(this);

	return getHandler();
}

void ResIdListPanel::setContents()
{

	int m;
	int count;
	ResManager ** resMgrs;

	count = g_env->getAllResManager(&resMgrs);

	ShowWindow(listview.GetHandle(), SW_HIDE);
	listview.clearAll();

	for(m = 0; m < count; m++)
	{
		resMgrs[m]->enumRes(this);
	}
	ShowWindow(listview.GetHandle(), SW_SHOW);
}

HWND ResIdListPanel::getEditor(HWND hParent, DWORD add_data, int sub_index)
{
	int cur_id;
	if(!listview.getCurData((DWORD*)&cur_id))
		return HWND_INVALID;

	ValueType * vt = NULL;
	Value value = 0;
	IDValueType _idValueType(ID2TYPE(cur_id));

	switch(sub_index)
	{
		case 0: //name
			return HWND_INVALID;
			break;

		case 2://id
			vt = pIntValueType;
	        value = (Value)(cur_id & 0x0000FFFF);
			break;

		default:
			return HWND_INVALID;
	}

	if(vt == NULL)
		return HWND_INVALID;

	return vt?vt->retrieveEditor(hParent,value,this,sub_index):HWND_INVALID;
}

Value ResIdListPanel::updateValue(Value value, ValueType *vtype,DWORD mask)
{
	int cur_id;
	ResManager * resMgr;

	if(!listview.getCurData((DWORD*)&cur_id))
		return 0;

	resMgr = g_env->getResManager(ID2TYPE(cur_id));
	if(resMgr == NULL)
		return 0;

    switch (mask) {
        case 0: //Id Name
			g_env->updateResName(cur_id, (const char*)value);
            break;

        case 2: //Id Value
			listview.setCurData((int)value | (ID2TYPE(cur_id) <<16));
			g_env->updateResId(cur_id, (int)value | (ID2TYPE(cur_id) <<16));
            break;

        default:
            break;
    }

    return value;
}

BOOL ResIdListPanel::updatingValue(Value old_value, Value new_value, ValueType *vtype,DWORD mask)
{
	int cur_id;
	ResManager * resMgr;

	if(!listview.getCurData((DWORD*)&cur_id))
		return FALSE;

	resMgr = g_env->getResManager(ID2TYPE(cur_id));
	if(resMgr == NULL)
		return FALSE;

    switch (mask) {
        case 0:
            return TRUE;
        case 2: //Id Value
        {
            int new_id = (int)new_value|(ID2TYPE(cur_id) <<16);
			IDRangeOwnerManager * om = g_env->getIDRangeOwnerManager();
			if(om)
			{
				IDRangeOwner* owner = om->getCurOwner();
				if(owner && !owner->isInRange(new_id))
				{
					char szText[256];
					sprintf(szText, _("The id(%d) is out of the current user \"%s\", do you want to set the id too?"), (int)new_value, owner->name.c_str());
					if(::MessageBox(getHandler(), szText, "Query", MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1) !=IDYES)
						return FALSE;
				}
			}
            return resMgr->setResId(cur_id, new_id); //accepted by the res manager
        }

        default:
            break;
    }

	return FALSE;
}
