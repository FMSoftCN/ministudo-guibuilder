/*
 * connect-events-wnd.cpp
 *
 *  Created on: 2009-7-1
 *      Author: dongjunjie
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
			InfoBox(_("Error"), _("Please input a valid event name.\nIt must be a C function name."));
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
#define ID_CB_SENDER		102
#define ID_CB_LISTENER		103
#define ID_LB_LISTENER		104
#define ID_LB_SENDER		105
#define ID_DELETE			106
#define ID_ADD				107
#define ID_SLE_EVENT		108
#define ID_SLE_INPUTEVENT	120

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
#if 0
static CTRLDATA _ctrl_MwInputEventName [] = {
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		80,148,80,30, /*x, y, w, h*/
		IDOK, /*id*/
		"OK", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		196,148,80,30, /*x, y, w, h*/
		IDCANCEL, /*id*/
		"Cancel", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT,
		12,8,172,28, /*x, y, w, h*/
		ID_STATIC4, /*id*/
		"Input the Event Name:", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"sledit",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT, /*dwstyle*/
		12,48,264,28, /*x, y, w, h*/
		ID_SLE_INPUTEVENT, /*id*/
		"SLEdit1", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT,
		12,100,264,32, /*x, y, w, h*/
		ID_STATIC5, /*id*/
		"Note: The event name must be a valid C function name.", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},

};
static DLGTEMPLATE _MwInputEventName_templ = {
	WS_CAPTION|WS_BORDER|WS_DLGFRAME, /*dwStyle*/
	0x0, /*dwExStyle*/
	0, 0, 300, 220, /*x, y, w, h*/
	"Input Event Name", /*caption*/
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ctrl_MwInputEventName)/sizeof(CTRLDATA),/*controlnr*/
	_ctrl_MwInputEventName,/*controls*/
	0
};
#endif

class ModifyEventWnd : public MGMainWnd
{
	string eventName;
	EventNameEditor eventNameEditor;
public:
	ModifyEventWnd(HWND hParent, const char* name)
	{
		if(name)
			eventName = name;
#if 0
		Create(hParent, &_MwInputEventName_templ);
#else
		Create(hParent, GetDlgTemplate(ID_INPUTEVENTNAME));
#endif

		CenterWindow(TRUE);
		ShowWindow(SW_SHOW);
	}

	const char* getEventName(){ return eventName.c_str();}

	DECLARE_MSG_MAP;

	BOOL onInitDialog(WPARAM wParam, LPARAM lParam)
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

///////////////////////////////////////
#if 0
static CTRLDATA _ctrl__MwSelectEvent [] = {
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		120,296,80,30, /*x, y, w, h*/
		IDOK, /*id*/
		"OK", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		212,296,80,30, /*x, y, w, h*/
		IDCANCEL, /*id*/
		"Cancel", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"combobox",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY,
		20,24,264,24, /*x, y, w, h*/
		ID_CB_SENDER, /*id*/
		"Combobox1", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"listbox",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP |LBS_NOTIFY|WS_VSCROLL, /*dwstyle*/
		20,56,264,112, /*x, y, w, h*/
		ID_LB_SENDER, /*id*/
		"ListBox1", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT, /*dwstyle*/
		20,0,264,20, /*x, y, w, h*/
		ID_STATIC1, /*id*/
		"Step 1: Select An Event", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"sledit",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP |WS_BORDER | ES_LEFT, /*dwstyle*/
		20,204,264,28, /*x, y, w, h*/
		ID_SLE_EVENT, /*id*/
		"SLEdit1", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT, /*dwstyle*/
		20,176,264,28, /*x, y, w, h*/
		ID_STATIC1, /*id*/
		"Step 2: Input the Event Name:", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT, /*dwstyle*/
		20,244,264,32, /*x, y, w, h*/
		ID_STATIC1, /*id*/
		"Note: The event name must be a valid C function name.", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},

};
static DLGTEMPLATE __MwSelectEvent_templ = {
		WS_CAPTION|WS_BORDER|WS_DLGFRAME, /*dwStyle*/
	0x0, /*dwExStyle*/
	0, 0, 332, 372, /*x, y, w, h*/
	"New Connect", /*caption*/
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ctrl__MwSelectEvent)/sizeof(CTRLDATA),/*controlnr*/
	_ctrl__MwSelectEvent,/*controls*/
	0
};
#endif

class SelectEventWnd : public MGMainWnd, FieldEnumHandler
{
	ComponentInstance *cinst;
	ComponentInstance *listener;
	EventNameEditor eventNameEditor;
	int event_id;
	ComponentInstance *sender;
	string eventName;
public:
	SelectEventWnd(HWND hParent, ComponentInstance *cinst, ComponentInstance *cinst_listener)
	{
		event_id = 0;
		sender = NULL;
		this->cinst = cinst;
		listener = cinst_listener;

#if 0
		Create(hParent, &__MwSelectEvent_templ);
#else
		Create(hParent, GetDlgTemplate(ID_SELECTEVENT));
#endif

		CenterWindow(TRUE);
		ShowWindow(SW_SHOW);
	}

	int getEvent(){ return event_id;}
	ComponentInstance* getSender(){ return sender; }
	const char* getEventName(){ return eventName.c_str();}

	DECLARE_MSG_MAP;

	BOOL onInitDialog(WPARAM wParam, LPARAM lParam)
	{
		eventNameEditor.Attach(m_hWnd, ID_SLE_EVENT);
		//init dialog
		HWND hsender = GetChild(ID_CB_SENDER);

		ResManager* resMgr = g_env->getResManager(NCSRT_CONTRL);
		if(!resMgr)
			return FALSE;

		int idx;
		const char* strName;

		if(listener != cinst)
		{
			strName = resMgr->idToName(cinst->getID());

			idx = ::SendMessage(hsender, CB_ADDSTRING, 0, (LPARAM)strName);
			::SendMessage(hsender, CB_SETITEMADDDATA, idx, (LPARAM)cinst);
		}

		//insert the childrens
		for(ComponentInstance * child = cinst->getChildren(); child; child = child->getNext())
		{
			strName = resMgr->idToName(child->getID());
			if(!strName && child == listener)
				continue;
			idx = ::SendMessage(hsender, CB_ADDSTRING, 0, (LPARAM)strName);
			::SendMessage(hsender, CB_SETITEMADDDATA, idx, (LPARAM)child);
		}

		::SendMessage(hsender, CB_SETCURSEL, 0, 0);
		onSenderSelChanged(hsender);

		return TRUE;
	}

	void onSenderSelChanged(HWND hCtrl)
	{
		//add the events
		HWND lb = GetChild(ID_LB_SENDER);
		::SendMessage(lb, LB_RESETCONTENT, 0, 0);

		int sel_idx = ::SendMessage(hCtrl, CB_GETCURSEL, 0, 0);

		//get the curent sender
		ComponentInstance * child = (ComponentInstance*)::SendMessage(hCtrl, CB_GETITEMADDDATA,sel_idx, 0);
		if(!child)
			return;

		child->getClass()->enumFields(this, TRUE,(DWORD)lb);
		eventNameEditor.SetWindowText("");
	}

	BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user)
	{
		if(id >= ComponentInstance::PropEventBegin+50)
		{
			//insert name and id
			int idx = ::SendMessage((HWND)user, LB_ADDSTRING, 0, (LPARAM)name);
			::SendMessage((HWND)user, LB_SETITEMADDDATA, idx, (LPARAM)id);
		}
		else if(id > ComponentInstance::PropEventEnd)
			return FALSE;

		return TRUE;
	}

	ComponentInstance * getSenderFromControl()
	{
		HWND hctrl = GetChild(ID_CB_SENDER);
		int idx = ::SendMessage(hctrl, CB_GETCURSEL, 0, 0);
		if(idx < 0)
			return NULL;
		return (ComponentInstance*)
			::SendMessage(hctrl, CB_GETITEMADDDATA, idx	, 0);
	}

	void onSenderLBSelChange(HWND hCtrl)
	{
		int idx = ::SendMessage(hCtrl, LB_GETCURSEL, 0, 0);
		if(idx < 0)
			return;

		event_id = ::SendMessage(hCtrl, LB_GETITEMADDDATA,idx, 0);
		sender = getSenderFromControl();
		if(!sender)
			return ;
		int sender_id = sender->getID();

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
};

BEGIN_MSG_MAP(SelectEventWnd)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		BEGIN_NOTIFY_MAP(ID_CB_SENDER)
			MAP_CTRL_NOTIFY(CBN_SELECTOK, onSenderSelChanged)
		END_NOTIFY_MAP
		BEGIN_NOTIFY_MAP(ID_LB_SENDER)
			MAP_CTRL_NOTIFY(LBN_SELCHANGE, onSenderLBSelChange)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP


/////////////////////////////////////////////////

#if 0
static CTRLDATA _ctrl_MwConnectEvents [] = {
	{
		"combobox",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_NOTIFY,
		20,36,264,24, /*x, y, w, h*/
		ID_CB_LISTENER, /*id*/
		"", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,36,80,30, /*x, y, w, h*/
		ID_ADD, /*id*/
		"Add", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,75,80,30, /*x, y, w, h*/
		ID_DELETE, /*id*/
		"Delete", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,115,80,30, /*x, y, w, h*/
		ID_MODIFY, /*id*/
		"Modify", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,155,80,30, /*x, y, w, h*/
		ID_SKIP, /*id*/
		"Source", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,195,80,30, /*x, y, w, h*/
		IDOK, /*id*/
		"OK", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"button",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		296,235,80,30, /*x, y, w, h*/
		IDCANCEL, /*id*/
		"Cancel", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"static",/*ctrl class*/
		WS_VISIBLE | SS_LEFT, /*dwstyle*/
		20,12,160,20, /*x, y, w, h*/
		ID_STATIC1, /*id*/
		"Event Listener:", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
	{
		"listview",/*ctrl class*/
		WS_VISIBLE | WS_TABSTOP | WS_BORDER |LVS_NOTIFY|WS_VSCROLL|WS_HSCROLL, /*dwstyle*/
		20,68,264,197, /*x, y, w, h*/
		ID_LV_LISTEN, /*id*/
		"ListView1", /*caption*/
		0, /*dwAddData*/
		0x0, /*exstyle*/
		NULL,
		NULL
	},
};

static DLGTEMPLATE _MwConnectEvents_templ = {
		WS_CAPTION|WS_BORDER|WS_DLGFRAME, /*dwStyle*/
	0x0, /*dwExStyle*/
	0, 0, 404, 320, /*x, y, w, h*/
	"Connect Events", /*caption*/
	0, /* hIcon */
	0, /* hMenu */
	sizeof(_ctrl_MwConnectEvents)/sizeof(CTRLDATA),/*controlnr*/
	_ctrl_MwConnectEvents,/*controls*/
	0
};
#endif



ConnectEventsWnd::ConnectEventsWnd(HWND hParent, ComponentInstance *inst, int default_id/*=-1*/) {
	// TODO Auto-generated constructor stub

    bSkipCode = FALSE;
	bChanged = FALSE;
	if(inst == NULL || inst->getChildren()==NULL)
		throw("Cannot get events from this instance");

	this->inst = inst;

#if 0
	if(!Create(hParent,&_MwConnectEvents_templ,default_id))
#else
	if(!Create(hParent, GetDlgTemplate(ID_CONNECTEVENT), default_id))
#endif
		throw("Cannot create the connect event window");

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
		BEGIN_NOTIFY_MAP(ID_CB_LISTENER)
			MAP_CTRL_NOTIFY(CBN_SELECTOK, onListenerSelChanged)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP


BOOL ConnectEventsWnd::onInitDialog(WPARAM wParam,LPARAM lParam)
{
	HWND hcbListener;
	ResManager *resMgr;
	int idx;
	if(!inst)
		return FALSE;

	int default_id = (int)lParam;
	int sel_idx = 0;

	resMgr = g_env->getResManager(NCSRT_CONTRL);
	if(!resMgr)
		return FALSE;

	hcbListener = GetChild(ID_CB_LISTENER);

	const char* strName = resMgr->idToName(inst->getID());
	if(strName){
		idx = ::SendMessage(hcbListener, CB_ADDSTRING, 0, (LPARAM)strName);
		::SendMessage(hcbListener, CB_SETITEMADDDATA, idx, (LPARAM)inst);
	}

	//insert the children
	for(ComponentInstance * child = inst->getChildren(); child; child = child->getNext())
	{
		strName = resMgr->idToName(child->getID());
		if(!strName)
			continue;
		idx = ::SendMessage(hcbListener, CB_ADDSTRING, 0, (LPARAM)strName);
		::SendMessage(hcbListener, CB_SETITEMADDDATA, idx, (LPARAM)child);
		if(default_id == child->getID())
			sel_idx = idx;
	}
	::SendMessage(hcbListener, CB_SETCURSEL, sel_idx, 0);

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
	onListenerSelChanged(HWND_NULL);

	return TRUE;
}

void ConnectEventsWnd::onAddEvent()
{
	ComponentInstance *listener = getListener();
	if(!listener)
		return;

	SelectEventWnd sew(m_hWnd, inst,listener);

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
	HLVITEM hlvItem = ::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
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
		HLVITEM hlvItem = ::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
		if(hlvItem == 0)
			return;
		LVSUBITEM lvSubItem;
		memset(&lvSubItem, 0, sizeof(lvSubItem));
		lvSubItem.subItem = 2;
		lvSubItem.pszText = (char*)le->prototype.c_str();
		::SendMessage(hlv,LVM_SETSUBITEMTEXT, hlvItem, (LPARAM)&lvSubItem);
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
	HLVITEM hlvItem = ::SendMessage(hlv, LVM_ADDITEM, 0, (LPARAM)&lvItem);

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


ComponentInstance * ConnectEventsWnd::getListener()
{
	HWND hctrl = GetChild(ID_CB_LISTENER);
	int idx = ::SendMessage(hctrl, CB_GETCURSEL, 0, 0);
	if(idx < 0)
		return NULL;
	return (ComponentInstance*)
		::SendMessage(hctrl, CB_GETITEMADDDATA, idx	, 0);
}

ListenEntry* ConnectEventsWnd::getCurListen()
{
	HWND hlv = GetChild(ID_LV_LISTEN);
	HLVITEM hlvItem = ::SendMessage(hlv, LVM_GETSELECTEDITEM, 0, 0);
	if(hlvItem == 0)
		return NULL;

	return (ListenEntry*)::SendMessage(hlv, LVM_GETITEMADDDATA,0, (LPARAM)hlvItem);

}
