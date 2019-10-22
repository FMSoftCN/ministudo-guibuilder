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

#include "log.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"

#include "ui-event-id.h"
#include "panel.h"

#include "editable-listview.h"
#include "fieldpanel.h"

#include "defundo-redo-observer.h"
#include "edituipanel.h"

#include "navigator-panel.h"

extern string getConfigFile(const char* cfgFileName);

NavigatorPanel::NavigatorPanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
	flags = 0;
	//memset(&bmpClosed, 0, sizeof(bmpClosed));

	//try to load the bitmap of closed
	//LoadBitmapFromFile(HDC_SCREEN, &bmpClosed, g_env->getConfigFile("uieditor/closed-ui.png").c_str());

}

NavigatorPanel::~NavigatorPanel() {
    if (hIconView)
        SetWindowAdditionalData(hIconView, 0);
	//UnloadBitmap(&bmpClosed);
}

HWND NavigatorPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);
    int size = 128;

	hIconView = CreateWindow(CTRL_ICONVIEW, "", WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_CHILD|IVS_LOOP, 3,
			0, 0, RECTW(rt), RECTH(rt), hParent, 0);

    ::SendMessage(hIconView, IVM_SETITEMSIZE, size, size);
    ::SendMessage(hIconView, SVM_SETSCROLLVAL, size, size);

	SetWindowAdditionalData(hIconView,(DWORD)this);
	oldIconViewProc = SetWindowCallbackProc(hIconView, NavigatorPanel::_new_icon_view_proc);
	SetNotificationCallback(hIconView, NavigatorPanel::_iconview_notifi);

	return hIconView;
}

void NavigatorPanel::insertEditUIPanel(const char* strName, EditUIPanel* eui)
{
//	int idx = -1;
	if(strName == NULL && eui == NULL)
		return ;

	/*PanelInfo * pi = getPanelInfo(strName,idx);

	if(pi)
	{
		if(pi->isObject())
			return ;
		pi->setPanel(eui);
		GHANDLE handle = iconview_get_item(hIconView,idx);
		iconview_set_item_bitmap(handle, eui->getOutlineBmp());
		::SendMessage(hIconView, IVM_REFRESHITEM, idx, 0);
		return ;
	}

	char szName[256];
	parserName(strName, szName);*/

	if(findByEditUIPanel(eui) != -1)
		return ;

	IVITEMINFO ivItem;
	memset(&ivItem, 0, sizeof(ivItem));
	ivItem.nItem = ::SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0);
	ivItem.bmp = /*eui?*/eui->getOutlineBmp();//:&bmpClosed;
	ivItem.label = (char*)eui->getFileName();//szName;
	ivItem.addData = (DWORD)eui;//(DWORD) new PanelInfo(strName, eui);

	::SendMessage(hIconView, IVM_ADDITEM, 0, (LPARAM)&ivItem);
}

/*NavigatorPanel::PanelInfo* NavigatorPanel::getPanelInfo(const char* strName, int& idx)
{
	if(strName == NULL)
		return NULL;

	for(int i=0; i<SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0); i++)
	{
		PanelInfo * pi = (PanelInfo*)::SendMessage(hIconView, IVM_GETITEMADDDATA, i, 0);
		if(pi && pi->equal(strName)){
			idx = i;
			return pi;
		}
	}

	return NULL;

}*/

void NavigatorPanel::closePanel(EditUIPanel *eui)
{
	if(eui == NULL)
		return ;

	int idx = findByEditUIPanel(eui);

	if(idx < 0)
		return ;

	/*PanelInfo * pi = (PanelInfo*)::SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	pi->closePanel();

	GHANDLE hiv = iconview_get_item(hIconView,idx);
	if(hiv)
	{
		iconview_set_item_bitmap(hiv, &bmpClosed);
	}

	::SendMessage(hIconView, IVM_REFRESHITEM, idx, 0);*/

	//delete
	::SendMessage(hIconView, IVM_DELITEM, idx, 0);

}

void NavigatorPanel::removeEditUIPanel(EditUIPanel *eui)
{
	int idx = findByEditUIPanel(eui);
	if(idx < 0)
		return;

	/*PanelInfo * pi = (PanelInfo*)::SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	if(pi)
		delete pi;
*/
	::SendMessage(hIconView,IVM_DELITEM, idx, 0);
}

void NavigatorPanel::setCurPanel(EditUIPanel* eui)
{
	int idx = findByEditUIPanel(eui);
	if(idx < 0)
		return;

	flags = 1;
	::SendMessage(hIconView, IVM_SETCURSEL, idx, 1);
	flags = 0;
}

/*const char* NavigatorPanel::parserName(const char* strName, char* szBuff)
{
	if(strName == NULL || szBuff == NULL)
		return NULL;

	const char* str = strrchr(strName, '/');

	if(str == NULL)
		str = strrchr(strName, '\\');
	if(str == NULL)
		str = strName;
	else
		str ++;

	strcpy(szBuff, str);
	return szBuff;
}*/

int NavigatorPanel::findByEditUIPanel(EditUIPanel* eui)
{
	if(eui == NULL)
		return -1;
	for(int i=0; i<(int)SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0); i++)
	{
		/*PanelInfo * pi = (PanelInfo*)::SendMessage(hIconView, IVM_GETITEMADDDATA, i, 0);
		if(pi && pi->equal(eui))
			return i;*/
		EditUIPanel* euitmp = (EditUIPanel*)::SendMessage(hIconView, IVM_GETITEMADDDATA, i, 0);
		if(euitmp == eui)
			return i;
	}

	return -1;
}

void NavigatorPanel::updateEditUIPanel(EditUIPanel* eui)
{
	if(eui == NULL)
		return ;
	int idx = findByEditUIPanel(eui);

	if(idx < 0)
		return ;

	::SendMessage(hIconView, IVM_REFRESHITEM, idx, 0);
}

void NavigatorPanel::showPanel()
{
	LRESULT idx = ::SendMessage(hIconView,IVM_GETCURSEL, 0, 0);
	if(idx < 0)
		return;

	/*PanelInfo * pi = (PanelInfo*)::SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	if(pi && !pi->isObject())
	{
		//send Event
		sendEvent(NAVIGATOR_OPENPANEL,(DWORD)pi->getName(),0);
	}*/

	EditUIPanel* eup = (EditUIPanel*)::SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	if(eup && eup->isHidden())
	{
		sendEvent(NAVIGATOR_SHOWPANEL,(DWORD)eup,0);
	}

}

void NavigatorPanel::_iconview_notifi(HWND hIconView, LINT id, int nc, DWORD add_data)
{
	NavigatorPanel * np = (NavigatorPanel*) GetWindowAdditionalData(hIconView);

	if(np)
	{
		if(nc == IVN_SELCHANGED)
		{
			if(np->flags)
				return;
			LRESULT curSel = SendMessage(hIconView, IVM_GETCURSEL, 0, 0);
			/*PanelInfo* pi = (PanelInfo*)SendMessage(hIconView,
					IVM_GETITEMADDDATA,	curSel, 0);
			if (pi)
				np->sendEvent(NAVIGATOR_SELCHANGE, (DWORD)(pi->infos.eui), 0);*/
			EditUIPanel* eui = (EditUIPanel*)SendMessage(hIconView,
					IVM_GETITEMADDDATA,	curSel, 0);
			if(eui)
				np->sendEvent(NAVIGATOR_SELCHANGE, (DWORD)eui, 0);
		}
	}
}

LRESULT NavigatorPanel::_new_icon_view_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NavigatorPanel * np = (NavigatorPanel*) GetWindowAdditionalData(hwnd);

	if(!np)
		return DefaultControlProc(hwnd, message, wParam, lParam);

/*	if(MSG_DESTROY == message)
	{
		for(int i=0; i < SendMessage(hwnd, IVM_GETITEMCOUNT, 0, 0); i++)
		{
			//PanelInfo* pi = (PanelInfo*)SendMessage(hwnd, IVM_GETITEMADDDATA, i, 0);
			if(pi)
				delete pi;
		}
	}
	else*/ if(MSG_LBUTTONDBLCLK == message)
	{
		/*int oldIdx = SendMessage(hwnd, IVM_GETCURSEL, 0, 0);
		int ret = np->oldIconViewProc(hwnd, message, wParam, lParam);
		int newIdx = SendMessage(hwnd, IVM_GETCURSEL, 0, 0);
		if(oldIdx != newIdx  && newIdx >= 0)
			*/np->showPanel();
		//return ret;
	}
	else if(MSG_KEYDOWN == message && wParam == SCANCODE_ENTER)
	{
		np->showPanel();
	}

	return np->oldIconViewProc(hwnd, message, wParam, lParam);
}
