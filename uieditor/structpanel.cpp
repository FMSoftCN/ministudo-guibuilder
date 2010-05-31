/*
 * structpanel.cpp
 *
 *  Created on: 2009-3-22
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

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"

#include "ui-event-id.h"
#include "panel.h"
#include "component-instance.h"
#include "panellayout.h"
#include "reseditor.h"

#include "structpanel.h"

extern ResEditorEnv * g_env ;

StructPanel::StructPanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
	flags = 0;
	hStartWndItem = 0;
}

StructPanel::~StructPanel() {
	// TODO Auto-generated destructor stub
}

HWND StructPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);
	Create("", hParent, 0, 0, RECTW(rt), RECTH(rt), WS_CHILD| WS_VISIBLE | TVS_NOTIFY | WS_HSCROLL |WS_VSCROLL, 0, 2);
	SetNotification(this);
	DeleteTree(GetRoot());

	Subclass();

	return getHandler();
}

void StructPanel::insertInstance(ComponentInstance *instance, BOOL bhide/* = FALSE*/, BOOL bstart_wnd /*=FALSE*/)
{
	if(instance == NULL)
		return;

	if(findItemByInstance(instance, (GHANDLE)0) != (GHANDLE)0)
		return ;

	ComponentInstance * parent = instance->getParent();

	GHANDLE hitem = findItemByInstance(parent, (GHANDLE)0);

	insertInstance(instance, hitem, bhide, bstart_wnd);

}

void StructPanel::removeInstance(ComponentInstance *instance)
{
	GHANDLE hitem = findItemByInstance(instance, (GHANDLE)0);

	if(!hitem)
	{
		return;
	}

	if(hitem == hStartWndItem)
	{
		hStartWndItem = 0;
	}

	DeleteTree(hitem);

}

void StructPanel::selectInstance(ComponentInstance *instance)
{
	GHANDLE item = findItemByInstance(instance, (GHANDLE)0);

	flags = 1;
	SetSelItem(item);
	flags = 0;
}

void StructPanel::changeParent(const vector<HWND> *sellist, ComponentInstance * old_parent)
{
	if(sellist == NULL || old_parent == NULL)
		return ;

	if(sellist->size() <= 0)
		return;

	ComponentInstance * cinst = ComponentInstance::FromHandler(sellist->front());

	if(cinst->getParent() == old_parent )
		return ;

	flags = 1;

	GHANDLE item = findItemByInstance(old_parent, GetRoot());
	GHANDLE newitem = findItemByInstance(cinst->getParent(), GetRoot());
	if(item && newitem)
	{
		for(int i=0; i<(int)sellist->size(); i++){
			HWND hwnd = (*sellist)[i];
			ComponentInstance *cinst = ComponentInstance::FromHandler(hwnd);
			if(cinst == NULL)
				continue;
			GHANDLE hchild = findItemByInstance(cinst, item);
			if(hchild){
				DeleteTree(hchild);
			}

			insertInstance(cinst, newitem);
		}
	}
	flags = 0;
}

GHANDLE StructPanel::findItemByInstance(ComponentInstance* instance, GHANDLE item)
{
	if(instance == NULL)
		return (GHANDLE)0;

	if(item == (GHANDLE)0)
		item = GetRoot();

	// find in the children of item
	for(GHANDLE child = GetFirstChild(item); child; child = GetNextSibling(child))
	{
		if( GetItemAddData(child) == (DWORD) instance)
			return child;

		//try to find in the children
		GHANDLE hitem = findItemByInstance(instance, child);
		if(hitem != (GHANDLE)0)
			return hitem;
	}

	return (GHANDLE)0;
}

GHANDLE StructPanel::insertInstance(ComponentInstance *instance, GHANDLE parent, BOOL bhide /*= FALSE*/,BOOL bstart_wnd/*=FALSE*/)
{
	if(instance == NULL)
		return parent;

	int id = instance->getID();
	char szName[100];
	const char* name = NULL;

	ResManager * resMgr = g_env->getResManager(NCSRT_UI);

	if(resMgr){
		name = resMgr->idToName(id);
	}

	if(name == NULL){
		sprintf(szName, "%s(%d)", instance->getClass()->getClassName(),id);
	}
	else
		strcpy(szName, name);

	if(bstart_wnd)
	{
		strcat(szName,"[Start Winodw]");
	}

	if(bhide)
	{
		strcat(szName, "[Hide]");
	}


	GHANDLE ghandle = AddItem(parent, szName, 0, (DWORD)instance);

	if(!bhide)
	{
		for(ComponentInstance* child = instance->getChildren(); child; child = child->getNext())
		{
			insertInstance(child, ghandle);
		}
	}

	return ghandle;
}

void StructPanel::OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data)
{
	if(code == TVN_SELCHANGE)
	{
		if(flags)
			return ;

		DWORD dwAdd = GetItemAddData(GetSelItem());
		//send event
		sendEvent(STRUCT_SELCHANGE, dwAdd, 0);
	}
}

void StructPanel::setStartWnd(ComponentInstance *instance)
{
	setStartWndText(hStartWndItem, FALSE);

	hStartWndItem = 0;

	if(instance == NULL){
		return;
	}

	GHANDLE hitem = findItemByInstance(instance,GetRoot());
	if(hitem && hitem != hStartWndItem)
	{
		setStartWndText(hitem, TRUE);
		hStartWndItem = hitem;
	}
}

void StructPanel::setStartWndText(GHANDLE hitem, BOOL bStartWnd)
{
	if(hitem)
	{
		const char *name;
		char szName[100];
		ResManager * res = g_env->getResManager(NCSRT_UI|NCSRT_CONTRL);
		ComponentInstance* cinst = (ComponentInstance*)GetItemAddData(hitem);
		if(!cinst)
			return ;

		if(res){
			name = res->idToName(cinst->getID());
		}

		if(name == NULL){
			name = szName;
			sprintf(szName, "%s(%d)", cinst->getClass()->getClassName(),cinst->getID());
		}

		char szText[100];
		if(bStartWnd)
			sprintf(szText,"%s[Start window]", name);
		else
			strcpy(szText, name);

		SetItemText(hitem, szText);
	}
}

void StructPanel::refreshInstance(ComponentInstance* instance, BOOL update_children)
{
	if(!instance )
		return;

	if(update_children)
	{
		//remove the instance
		removeInstance(instance);
		//insert the instance too
		insertInstance(instance);
	}
	else
	{
		GHANDLE item = findItemByInstance(instance, GetRoot());
		if(!item)
			return;

		const char *name;
		char szName[100];
		ResManager * res = g_env->getResManager(NCSRT_UI|NCSRT_CONTRL);

		if(res){
			name = res->idToName(instance->getID());
		}

		if(name == NULL){
			name = szName;
			sprintf(szName, "%s(%d)", instance->getClass()->getClassName(),instance->getID());
		}

		SetItemText(item, name);
	}
}

void StructPanel::onRButtonDown(int x, int y, DWORD flags)
{
	GHANDLE hItem = GetSelItem();
	if(hItem == 0)
		return;

	ComponentInstance * inst = (ComponentInstance*)GetItemAddData(hItem);
	if(inst == NULL || inst->getParent() == NULL)
		return ;

 mapex<int, int>idsets;
 HMENU hMenu = g_env->createPopMenuFromConfig(ResEditor::UI_TAB_POPMENU, idsets);
	ClientToScreen(&x, &y);
	TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_DESTROY, x, y,m_hWnd);
}

void StructPanel::onLButtonDBLClick(int x, int y, DWORD flags)
{
	GHANDLE hitem = GetSelItem();
	if(!hitem)
		return;

	sendEvent(STRUCT_INSTANCE_DBLCLKED, (DWORD)GetItemAddData(hitem));
}

void StructPanel::onMoveItems(int id)
{
	GHANDLE hItem1 = GetSelItem();
	if(hItem1 == 0)
		return;

	GHANDLE hItem2 = this->GetRelatedItem(hItem1,id==ResEditor::UI_MENUCMD_MOVEUP?TVIR_PREVSIBLING:TVIR_NEXTSIBLING);
	if(hItem2 == 0)
		return;

	switchItem(hItem1, hItem2);

	SetSelItem(hItem2);
}

ComponentInstance* StructPanel::getInstanceFromItem(GHANDLE hItem)
{
	TVITEMINFO tvi;
	memset(&tvi, 0, sizeof(tvi));
	if(GetItemInfo(hItem, &tvi) == 0)
	{
		return (ComponentInstance*)tvi.dwAddData;
	}
	return NULL;
}

void StructPanel::switchItem(GHANDLE hItem1, GHANDLE hItem2)
{
	if(hItem1 == 0 || hItem2 == 0)
		return;

	char szText1[256];
	char szText2[256];
	TVITEMINFO tvi1, tvi2;
	tvi1.text = szText1;
	tvi2.text = szText2;
	GetItemInfo(hItem1, &tvi1);
	GetItemInfo(hItem2, &tvi2);
	ComponentInstance * cinst1, *cinst2;
	cinst1 = (ComponentInstance*)tvi1.dwAddData;
	cinst2 = (ComponentInstance*)tvi2.dwAddData;
	SetItemInfo(hItem1, &tvi2);
	//reset children
	resetChildren(hItem1, cinst2);

	SetItemInfo(hItem2,&tvi1);
	//reset children
	resetChildren(hItem2, cinst1);

	int idx = ::GetWindowZOrder(cinst2->getPreviewHandler());
	if(idx >= 0){
		HWND hwnd1 = cinst1->getPreviewHandler();
		//HWND hwnd2 = cinst2->getPreviewHandler();
		//HWND hParent = ::GetParent(hwnd1);
		::SetWindowZOrder(hwnd1, idx);
		/*RECT rt;
		::GetWindowRect(hwnd1, &rt);
		::InvalidateRect(hParent, &rt, TRUE);
		::GetWindowRect(hwnd2, &rt);
		::InvalidateRect(hParent, &rt, TRUE);*/
	}

	//Switch Instance1 and Instance2
  sendEvent(STRUCT_SWITCHINSTANCE, (DWORD)cinst1, (DWORD)cinst2);
}

void StructPanel::resetChildren(GHANDLE hItem, ComponentInstance * inst)
{
	//delete All children
	for(GHANDLE hchild = GetRelatedItem(hItem,TVIR_FIRSTCHILD);
		hchild;
		hchild = GetRelatedItem(hchild, TVIR_NEXTSIBLING))
	{
		DeleteTree(hchild);
	}

	for(ComponentInstance* child = inst->getChildren(); child; child = child->getNext())
	{
		insertInstance(child, hItem);
	}

}

void StructPanel::setInstanceHidden(ComponentInstance *instance, BOOL bhide /*= TRUE*/,BOOL bstart_wnd/*=FALSE*/)
{
	if(instance == NULL)
		return ;

	//find item handler
	GHANDLE hitem = findItemByInstance(instance,0);
	if(hitem == 0)
		return ;

	//delete item
	for(GHANDLE hchild = GetRelatedItem(hitem,TVIR_FIRSTCHILD); hchild;
		hchild = GetRelatedItem(hchild, TVIR_NEXTSIBLING))
	{
		DeleteTree(hchild);
	}

	//reset name
	const char *name;
	char szName[100];
	ResManager * res = g_env->getResManager(NCSRT_UI|NCSRT_CONTRL);

	if(res){
		name = res->idToName(instance->getID());
	}

	if(name == NULL){
		sprintf(szName, "%s(%d)", instance->getClass()->getClassName(),instance->getID());
	}
	else
	{
		strcpy(szName, name);
	}

	//is start window
	if(bstart_wnd)
		strcat(szName,"[Start Window]");

	if(bhide)
		strcat(szName,"[Hide]");

	SetItemText(hitem, szName);

	if(!bhide)
	{
		//insert instance
		for(ComponentInstance* child = instance->getChildren(); child; child = child->getNext())
		{
			insertInstance(child, hitem);
		}
	}
}

///////////////////////////////////////////////////////////////////
#ifndef WIN32
BEGIN_MSG_MAP(StructPanel)
	MAP_RBUTTONDOWN(onRButtonDown)
	MAP_LBUTTONDBLCLK(onLButtonDBLClick)
	BEGIN_COMMAND_MAP
		MAP_COMMAND_RANGE(ResEditor::UI_MENUCMD_MOVEUP, ResEditor::UI_MENUCMD_MOVEDOWN, onMoveItems)
	END_COMMAND_MAP
END_MSG_MAP
#else
BOOL StructPanel::WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret) 
{
	if(iMsg == MSG_RBUTTONDOWN)
	{
		onRButtonDown(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam);
		RETURN(0);
	} else if (iMsg == MSG_LBUTTONDBLCLK)
	{
		onLButtonDBLClick(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam);
		RETURN(0);
	} else if (iMsg == MSG_COMMAND)
	{
		int ctrlId = LOWORD(wParam);
		if (ctrlId >= ResEditor::UI_MENUCMD_MOVEUP && ctrlId <= ResEditor::UI_MENUCMD_MOVEDOWN)
		{
			onMoveItems(ctrlId);
		}
	}

	return FALSE;
}
#endif
