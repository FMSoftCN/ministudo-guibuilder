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

#ifndef CONNECTEVENTSWND_H_
#define CONNECTEVENTSWND_H_


class WindowTreeView : public TMGStaticSubclass<MGTreeView>
{
	ComponentInstance* baseInstance;
public:
	
	enum {
		WTVN_HIDE  = 20
	};

	BOOL Create(HWND hParent, int width, int height, ComponentInstance *baseinstance, int id);

	BOOL Show(ComponentInstance* sel_instance, int x, int y);
	
	BOOL Hide() {
		BOOL bret = ShowWindow(SW_HIDE);
		::NotifyParent(m_hWnd, GetID(), WTVN_HIDE);
		return bret;
	}

	ComponentInstance* GetSelInstance() {
		GHANDLE item = GetSelItem();
		return (ComponentInstance*)GetItemAddData(item);
	}

	DECLARE_MSG_MAP;

private:
	void setBaseInstance(ComponentInstance* baseInstance)
	{
		insertInstance(baseInstance, 0);
	}

	void insertInstance(ComponentInstance* instance, GHANDLE parent, ResManager *resMgr = NULL);
	
	GHANDLE findItemByInstance(ComponentInstance* instance, GHANDLE item);
	
	void onKillFocus();

	void onLButtonDblClk(int x, int y, DWORD key_status) ;

	BOOL onKeyDown(int scancode, DWORD key_status);
};

///////////////////////////////////////

class ConnectEventsWnd: public MGMainWnd{
public:
	ConnectEventsWnd(HWND hParent, ComponentInstance* baseInstance, ComponentInstance *inst, int default_id=-1);
	virtual ~ConnectEventsWnd();
    BOOL toSource() { return bSkipCode; }
    string getEventName() {return eventName; }

protected:
	DECLARE_MSG_MAP;
	ComponentInstance * inst;
	ComponentInstance * baseInstance;

	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	void onAddEvent();
	void onDeleteEvent();
	void onModify();
	void onSkip();
	void onOK();
	void onCancel();
	void onListenerSelChanged(HWND hCtrl);
	void onListenerSelected(HWND hCtrl);
	void onShowListeners();

	ComponentInstance * getListener();

	ListenEntry* getCurListen();

	void addListenEntry(HWND hlv, ResManager *resMgr,ListenEntry* le, ComponentInstance *listener, int idx=-1);

	BOOL bChanged;
	BOOL bSkipCode;
	string eventName;
	WindowTreeView tvWindows;
};

#endif /* CONNECTEVENTSWND_H_ */
