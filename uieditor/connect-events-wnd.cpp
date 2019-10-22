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
#include <set>
#include <queue>
#include "mapex.h"

#include "log.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

#include <mgncs/mgncs.h>
#include "msd_intl.h"

using namespace std;

#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"
#include "window-instance.h"
#include "page-window-instance.h"

#include "ui-event-id.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "connect-events-wnd.h"

//////////////////////////////////
string MakeInstanceName(ComponentInstance* inst)
{
	if(!inst)
		return "";

	ResManager* resMgr = g_env->getResManager(NCSRT_UI);

	const char* name = resMgr->idToName(inst->getID());
	if(name)
		return name;

	char szName[100];
	sprintf(szName, "%s(%d)", inst->getClass()->getClassName(), inst->getID());
	return szName;
}

////////////////////////////////////////////////
// Event Name Editor
static char*  IDName2FuncName(const char* szID, char *buff)
{
	if(!szID)
		return buff;
	if(!buff)
		return NULL;

	if(szID[0] == 'I' && szID[1]== 'D' && szID[2] !='\0'){
		const char *oldId = szID+2;
		for(szID=szID+2; *szID && *szID !='_'; szID++);
		if(*szID == '_')
			szID ++;
		else if(*szID=='\0')
			szID = oldId;
	}

	while(*szID){
		*buff = tolower(*szID);
		buff ++;
		szID ++;
	}

	return buff;
}

class EventNameEditor : public TMGStaticSubclass<MGEdit>
{
public:

	void AutoSetEventName(const char* szSender, const char* szListener, const char* event_name)
	{
		char szName[256];
		char *str = szName;

		str = IDName2FuncName(szListener, str);
		strcpy(str , "_on_");
		str += 4;
		str = IDName2FuncName(szSender, str);
		*str = '_';
		str++;
		if(event_name)
		{
			if(event_name[0] == 'o' && event_name[1] == 'n') //on
			{
				event_name += 2;
				while(event_name[0]){
					*str = tolower(event_name[0]);
					str ++;
					event_name ++;
				}
			}
		}
		*str = '\0';

		SetWindowText(szName);
	}

	BOOL getEventName(char *szText, int nMax){
		if(GetWindowText(szText, nMax)<=0
				|| (szText[0] >='0' && szText[0]<='9'))
		{
			InfoBox(_("Error"), _("Please input a valid event name.\nIt must be a C language function name."));
			return FALSE;
		}
		return TRUE;
	}

	DECLARE_MSG_MAP;

protected:
	BOOL onChar(int ch, DWORD key_flag){
		if((ch >= '0' && ch <= '9')
				|| (ch>='A' && ch<='Z')
				|| (ch>='a' && ch<='z')
				|| (ch=='_')
				|| (ch==127))
			return FALSE; //continue call default proc
		return TRUE;//break;
	}
};

BEGIN_MSG_MAP(EventNameEditor)
	MAP_CHAR(onChar)
END_MSG_MAP
///////////////////////////////////////////////

#define ID_MW_CONNECT_EVENTS		100
#define ID_MW_INPUT_EVENT_NAME		101
#define ID_SL_SENDER		102
#define ID_SL_LISTENER		103
#define ID_LB_LISTENER		104
#define ID_LB_SENDER		105
#define ID_DELETE			106
#define ID_ADD				107
#define ID_SLE_EVENT		108
#define ID_BT_SHOWWND       109
#define ID_BT_LINSTENER_MORE 110
#define ID_SLE_INPUTEVENT	120
#define ID_TV_WINDOW        200

#define ID_STATIC1		109
#define ID_STATIC3		110
#define ID_STATIC4		111
#define ID_STATIC5		112
#define ID_LV_LISTEN	115
#define ID_MODIFY      	116
#define ID_SKIP      	117

#include "dlgtmpls.h"
///////////////////////////////////////////////
//class input event-name

class ModifyEventWnd : public MGMainWnd
{
	string eventName;
	EventNameEditor eventNameEditor;
public:
	ModifyEventWnd(HWND hParent, const char* name)
	{
		if(name)
			eventName = name;

		Create(hParent, GetDlgTemplate(ID_INPUTEVENTNAME));

		CenterWindow(TRUE);
		ShowWindow(SW_SHOW);
	}

	const char* getEventName(){ return eventName.c_str();}

	DECLARE_MSG_MAP;

	BOOL onInitDialog(HWND hFocus, LPARAM lParam)
	{
		eventNameEditor.Attach(m_hWnd, ID_SLE_INPUTEVENT);

		eventNameEditor.SetWindowText(eventName.c_str());
		eventNameEditor.SelectAll();
		SetFocus(eventNameEditor.GetHandle());
		//TODO
		return TRUE;
	}

	void onOK()
	{
		char szText[256];
		if(!eventNameEditor.getEventName(szText, sizeof(szText)))
			return;

		eventName = szText;
		EndDialog(1);
	}

	void onCancel()
	{
		EndDialog(0);
	}
};

BEGIN_MSG_MAP(ModifyEventWnd)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
END_MSG_MAP

BOOL WindowTreeView::Create(HWND hParent, int width, int height, ComponentInstance *baseinstance, int id)
{
	MGTreeView::Create(NULL, hParent,0, 0, width, height, WS_BORDER | WS_CHILD | TVS_NOTIFY | WS_HSCROLL | WS_VSCROLL, 0x20000000L, id);
	DeleteTree(GetRoot());
	Subclass();
	setBaseInstance(baseinstance);
	return TRUE;
}

BOOL WindowTreeView::Show(ComponentInstance* sel_instance, int x, int y)
{
	GHANDLE hitem = findItemByInstance(sel_instance, 0);
	//select the hitem
	if(hitem != 0)
		SetSelItem(hitem);
	RECT rc;
	GetWindowRect(&rc);
	MoveWindow(x, y, RECTW(rc), RECTH(rc), FALSE);
	BOOL bret = ShowWindow(SW_SHOW);
	SetFocus(m_hWnd);
	return bret;
}

void WindowTreeView::insertInstance(ComponentInstance* instance, GHANDLE parent, ResManager *resMgr)
{
	if(!instance)
		return;

	if(!resMgr)
		resMgr = g_env->getResManager(NCSRT_UI);

	char szName[100];
	const char* name = NULL;

	int id = instance->getID();
	name = resMgr->idToName(id);

	if(!name){
		sprintf(szName, "%s(%d)", instance->getClass()->getClassName(), id);
	}
	else
		strcpy(szName, name);

	GHANDLE hitem = AddItem(parent, szName, 0, (DWORD)instance);

	for(ComponentInstance *child = instance->getChildren(); child; child = child->getNext())
		insertInstance(child, hitem, resMgr);
	
}

GHANDLE WindowTreeView::findItemByInstance(ComponentInstance* instance, GHANDLE item)
{
	if(instance == NULL)
		return 0;
	if(item == 0)
		item = GetRoot();

	for(GHANDLE child = GetFirstChild(item); child; child = GetNextSibling(child))
	{
		if(GetItemAddData(child) == (DWORD) instance)
			return child;
		GHANDLE hitem = findItemByInstance(instance, child);
		if(hitem != 0)
			return hitem;
	}
	return 0;
}

void WindowTreeView::onKillFocus() {
	Hide();
}

void WindowTreeView::onLButtonDblClk(int x, int y, DWORD key_status) {
	Hide();
}

BOOL WindowTreeView::onKeyDown(int scancode, DWORD key_status) {
	if(scancode == SCANCODE_ENTER)
	{
		Hide();
		return TRUE;
	}
	return FALSE;
}


///////////////////////////////////////

BEGIN_MSG_MAP(WindowTreeView)
	MAP_KILLFOCUS(onKillFocus)
	MAP_LBUTTONDBLCLK(onLButtonDblClk)
	MAP_KEYDOWN(onKeyDown)
END_MSG_MAP


class SelectEventWnd : public MGMainWnd, FieldEnumHandler
{
	ComponentInstance *cinst;
	ComponentInstance *listener;
	EventNameEditor eventNameEditor;
	int event_id;
	ComponentInstance *sender;
	string eventName;
	WindowTreeView  tvWindows;
public:
	SelectEventWnd(HWND hParent, ComponentInstance *cinst, ComponentInstance *cinst_listener)
	{
		event_id = 0;
		sender = NULL;
		this->cinst = cinst;
		listener = cinst_listener;

		Create(hParent, GetDlgTemplate(ID_SELECTEVENT));

		CenterWindow(TRUE);
		ShowWindow(SW_SHOW);
	}

	int getEvent(){ return event_id;}
	ComponentInstance* getSender(){ return sender; }
	const char* getEventName(){ return eventName.c_str();}

	DECLARE_MSG_MAP;

	BOOL onInitDialog(HWND hFocus, LPARAM lParam)
	{
		eventNameEditor.Attach(m_hWnd, ID_SLE_EVENT);
		//init dialog
		HWND hsender = GetChild(ID_SL_SENDER);

		ResManager* resMgr = g_env->getResManager(NCSRT_CONTRL);
		if(!resMgr)
			return FALSE;


		RECT rc;
		::GetWindowRect(hsender, &rc);
		tvWindows.Create(m_hWnd, RECTW(rc), 150, cinst, ID_TV_WINDOW);

		onSenderSelChanged(hsender);

		return TRUE;
	}

	void onWindowTVSelChanged(HWND hCtrl)
	{
		ComponentInstance* inst = tvWindows.GetSelInstance();
		if(!inst || inst == listener)
			return ;
		if(inst == sender)
			return ;

		sender = inst;

		SetChildText(ID_SL_SENDER, MakeInstanceName(sender).c_str());
	}

	void onSenderSelChanged(HWND hCtrl)
	{
		if(!sender)
			return;
		HWND lb = GetChild(ID_LB_SENDER);

		::SendMessage(lb, LB_RESETCONTENT, 0, 0);

		sender->getClass()->enumFields(this, TRUE,(DWORD)lb);
		eventNameEditor.SetWindowText("");
	}

	BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user)
	{
		if(id >= ComponentInstance::PropEventBegin+50)
		{
			//insert name and id
			LRESULT idx = ::SendMessage((HWND)user, LB_ADDSTRING, 0, (LPARAM)name);
			::SendMessage((HWND)user, LB_SETITEMADDDATA, idx, (LPARAM)id);
		}
		else if(id > ComponentInstance::PropEventEnd)
			return FALSE;

		return TRUE;
	}

	ComponentInstance * getSenderFromControl()
	{
		HWND hctrl = GetChild(ID_SL_SENDER);
		LRESULT idx = ::SendMessage(hctrl, CB_GETCURSEL, 0, 0);
		if(idx < 0)
			return NULL;
		return (ComponentInstance*)
			::SendMessage(hctrl, CB_GETITEMADDDATA, idx	, 0);
	}

	void onSenderLBSelChange(HWND hCtrl)
	{
		HWND hlb;
		if(!sender)
			return ;
		int sender_id = sender->getID();

		hlb = GetChild(ID_LB_SENDER);

		LRESULT idx  = ::SendMessage(hlb, LB_GETCURSEL, 0, 0);
		if(idx < 0)
			return ;

		event_id = ::SendMessage(hlb, LB_GETITEMADDDATA, idx, 0);

		const char* prototype = listener->getListenPrototype(sender_id, event_id);
		if(prototype)
		{
			//Set Window Text
			eventNameEditor.SetWindowText(prototype);
			eventNameEditor.SelectAll();
		}
		else
		{
			ResManager *resMgr = g_env->getResManager(NCSRT_CONTRL);
			if(!resMgr)
				return;

			EventValueType *vt = dynamic_cast<EventValueType*>(sender->getClass()->getFieldValueType(event_id));
			if(!vt)
				return;
			eventNameEditor.AutoSetEventName(resMgr->idToName(sender->getID()),
							resMgr->idToName(listener->getID()),
							vt->getName());
		}
	}

	void onOK()
	{

		if(sender == NULL)
		{
			InfoBox(_("Error"), _("Please select a sender"));
			return;
		}

		if(event_id <= 0)
		{
			InfoBox(_("Error"), _("Please select an event"));
			return ;
		}

		//get event
		char szText[256];
		if(!eventNameEditor.getEventName(szText, sizeof(szText)-1))
			return;

		eventName = szText;

		EndDialog(1);
	}

	void onCancel()
	{
		EndDialog(0);
	}
	
	void onShowWindow() {
		RECT rc;
		HWND hwnd = GetChild(ID_SL_SENDER);
		::GetWindowRect(hwnd, &rc);
		tvWindows.Show(sender, rc.left, rc.bottom + 1);
	}
};

BEGIN_MSG_MAP(SelectEventWnd)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		MAP_COMMAND(ID_BT_SHOWWND, onShowWindow)
		BEGIN_NOTIFY_MAP(ID_TV_WINDOW)
			MAP_CTRL_NOTIFY(TVN_SELCHANGE, onWindowTVSelChanged)
			MAP_CTRL_NOTIFY(WindowTreeView::WTVN_HIDE, onSenderSelChanged)
		END_NOTIFY_MAP
		BEGIN_NOTIFY_MAP(ID_LB_SENDER)
			MAP_CTRL_NOTIFY(LBN_SELCHANGE, onSenderLBSelChange)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP


/////////////////////////////////////////////////

ConnectEventsWnd::ConnectEventsWnd(HWND hParent,ComponentInstance *baseInstance, ComponentInstance *inst, int default_id/*=-1*/) {
	// TODO Auto-generated constructor stub

    bSkipCode = FALSE;
	bChanged = FALSE;
	if(inst == NULL || baseInstance == NULL)
		throw("Cannot get events from this instance");

	this->inst = inst;
	this->baseInstance = baseInstance;


	if(!Create(hParent, GetDlgTemplate(ID_CONNECTEVENT), default_id))
		throw("Cannot create the connect event window");

	RECT rc;
	::GetWindowRect(GetChild(ID_SL_LISTENER), &rc);
	tvWindows.Create(m_hWnd, RECTW(rc), 150, baseInstance, ID_TV_WINDOW);
	
	CenterWindow(TRUE);
	ShowWindow(SW_SHOW);
}

ConnectEventsWnd::~ConnectEventsWnd() {
	// TODO Auto-generated destructor stub
}

BEGIN_MSG_MAP(ConnectEventsWnd)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(ID_ADD, onAddEvent)
		MAP_COMMAND(ID_DELETE, onDeleteEvent)
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		MAP_COMMAND(ID_MODIFY, onModify)
		MAP_COMMAND(ID_SKIP, onSkip)
		MAP_COMMAND(ID_BT_LINSTENER_MORE, onShowListeners)
		BEGIN_NOTIFY_MAP(ID_TV_WINDOW)
			MAP_CTRL_NOTIFY(TVN_SELCHANGE, onListenerSelChanged)
			MAP_CTRL_NOTIFY(WindowTreeView::WTVN_HIDE, onListenerSelected)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP


BOOL ConnectEventsWnd::onInitDialog(HWND hFocus,LPARAM lParam)
{
	HWND hslListener;
	ResManager *resMgr;
	int idx;
	if(!inst)
		return FALSE;

	int default_id = (int)lParam;
	int sel_idx = 0;

	resMgr = g_env->getResManager(NCSRT_CONTRL);
	if(!resMgr)
		return FALSE;

	hslListener = GetChild(ID_SL_LISTENER);

	::SetWindowText(hslListener, MakeInstanceName(inst).c_str());

	//insert column
	HWND hlv = GetChild(ID_LV_LISTEN);

	LVCOLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.colFlags = LVHF_CENTERALIGN;
	lvCol.width = 100;
	char szText[100];
	lvCol.pszHeadText = szText;

	strcpy(szText,"Sender");
	::SendMessage(hlv, LVM_ADDCOLUMN, 0, (LPARAM)&lvCol);
	strcpy(szText,"Event");
	lvCol.nCols ++;
	::SendMessage(hlv, LVM_ADDCOLUMN, 0, (LPARAM)&lvCol);
	lvCol.width = 200;
	lvCol.nCols ++;
	strcpy(szText,"Prototype");
	::SendMessage(hlv, LVM_ADDCOLUMN, 0, (LPARAM)&lvCol);
	//add item
	onListenerSelected(HWND_NULL);

	return TRUE;
}

void ConnectEventsWnd::onAddEvent()
{
	ComponentInstance *listener = getListener();
	if(!listener)
		return;

	SelectEventWnd sew(m_hWnd, baseInstance,listener);

	if(sew.DoMode())
	{
		//TODO
		ListenEntry* le = listener->addListen(
				sew.getSender()->getID(),
				sew.getEvent(),
				sew.getEventName());

		if(le)
		{
			//add into list item
			addListenEntry(0,NULL,le,listener);
			bChanged = TRUE;
		}
	}
}

void ConnectEventsWnd::onDeleteEvent()
{

	//Delete
	ComponentInstance *listener = getListener();
	if(!listener)
		return;

	HWND hlv = GetChild(ID_LV_LISTEN);
	HLVITEM hlvItem = (HLVITEM)::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
	if(hlvItem == 0)
		return ;

	ListenEntry* le =	(ListenEntry*)::SendMessage(hlv, LVM_GETITEMADDDATA,0, (LPARAM)hlvItem);
	if(!le)
		return;

	if(listener->removeListen(le->sender_id, le->event_id))
	{
		//delete the listview
		::SendMessage(hlv,LVM_DELITEM, 0, (LPARAM)hlvItem);
		bChanged = TRUE;
	}
}

void ConnectEventsWnd::onOK()
{
	EndDialog(bChanged);
}

void ConnectEventsWnd::onCancel()
{
	EndDialog(bChanged);
}

void ConnectEventsWnd::onSkip()
{
	ListenEntry *le = getCurListen();

    if (le) {
        eventName = le->prototype;
        bSkipCode = TRUE;
        LOG_WARNING("want to skip to code. \n");
        onOK();
    }
}

void ConnectEventsWnd::onModify()
{
	ListenEntry *le = getCurListen();
	if(!le)
		return;
	ModifyEventWnd mew(m_hWnd,le->prototype.c_str());
	if(mew.DoMode())
	{
		le->prototype = mew.getEventName();
		//update list view
		HWND hlv = GetChild(ID_LV_LISTEN);
		HLVITEM hlvItem = (HLVITEM)::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
		if(hlvItem == 0)
			return;
		LVSUBITEM lvSubItem;
		memset(&lvSubItem, 0, sizeof(lvSubItem));
		lvSubItem.subItem = 2;
		lvSubItem.pszText = (char*)le->prototype.c_str();
		::SendMessage(hlv,LVM_SETSUBITEMTEXT, (WPARAM)hlvItem, (LPARAM)&lvSubItem);
		bChanged = TRUE;
	}
}

void ConnectEventsWnd::addListenEntry(HWND hlv, ResManager *resMgr, ListenEntry* le, ComponentInstance *listener, int idx)
{
	LVITEM    lvItem;
	LVSUBITEM lvSubItem;

	if(hlv == 0)
		hlv = GetChild(ID_LV_LISTEN);

	if(resMgr == NULL)
		resMgr = g_env->getResManager(NCSRT_CONTRL);

	memset(&lvItem, 0, sizeof(lvItem));
	memset(&lvSubItem, 0, sizeof(lvSubItem));
	lvItem.nItemHeight = 25;

	if(idx < 0)
		lvItem.nItem = ::SendMessage(hlv, LVM_GETITEMCOUNT, 0, 0);
	else
		lvItem.nItem = idx;

	//add into listview
	lvItem.itemData = (DWORD)le;
	HLVITEM hlvItem = (HLVITEM)::SendMessage(hlv, LVM_ADDITEM, 0, (LPARAM)&lvItem);

	//insert sender
	lvSubItem.nItem = lvItem.nItem;
	lvSubItem.subItem = 0;
	lvSubItem.pszText = (char*)resMgr->idToName(le->sender_id);
	::SendMessage(hlv,LVM_SETSUBITEMTEXT, (WPARAM)hlvItem, (LPARAM)&lvSubItem);

	ComponentInstance *sender = dynamic_cast<ComponentInstance*>((Instance*)resMgr->getRes(le->sender_id));

	//insert event
	EventValueType* vt;
	if(sender
			&& (vt = dynamic_cast<EventValueType*>(
							sender->getClass()->getFieldValueType(le->event_id))))
	{
		lvSubItem.subItem = 1;//event name
		lvSubItem.pszText = (char*)vt->getName();
		::SendMessage(hlv,LVM_SETSUBITEMTEXT, (WPARAM)hlvItem, (LPARAM)&lvSubItem);
	}

	//insert prototype
	lvSubItem.subItem = 2;
	lvSubItem.pszText = (char*)le->prototype.c_str();
	::SendMessage(hlv,LVM_SETSUBITEMTEXT, (WPARAM)hlvItem, (LPARAM)&lvSubItem);
}

void ConnectEventsWnd::onListenerSelChanged(HWND hCtrl)
{
	inst = tvWindows.GetSelInstance();
	SetChildText(ID_SL_LISTENER, MakeInstanceName(inst).c_str());	
}

void ConnectEventsWnd::onListenerSelected(HWND hCtrl)
{
	HWND hlv = GetChild(ID_LV_LISTEN);

	ResManager *resMgr;

	resMgr = g_env->getResManager(NCSRT_CONTRL);
	if(!resMgr)
		return ;

	::SendMessage(hlv, LVM_DELALLITEM, 0, 0);

	//list all the events
	ComponentInstance * listner = getListener();
	if(!listner)
		return;

	list<ListenEntry*> &lle = listner->getListens();

	int idx = 0;

	for(list<ListenEntry*>::iterator it = lle.begin(); it != lle.end(); ++it)
	{
		ListenEntry *le = *it;
		if(!le)
			continue;
		ComponentInstance* cinst = dynamic_cast<ComponentInstance *>((Instance*)(resMgr->getRes(le->sender_id)));
		if(!cinst)
			continue;

		addListenEntry(hlv, resMgr, le, cinst, idx++);
	}
}

void ConnectEventsWnd::onShowListeners() {
	RECT rc;
	::GetWindowRect(GetChild(ID_SL_LISTENER), &rc);
	tvWindows.Show(inst, rc.left, rc.bottom + 1);
}


ComponentInstance * ConnectEventsWnd::getListener()
{
	return inst;
}

ListenEntry* ConnectEventsWnd::getCurListen()
{
	HWND hlv = GetChild(ID_LV_LISTEN);
	HLVITEM hlvItem = (HLVITEM)::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
	if(hlvItem == 0)
		return NULL;

	return (ListenEntry*)::SendMessage(hlv, LVM_GETITEMADDDATA,0, (LPARAM)hlvItem);

}
