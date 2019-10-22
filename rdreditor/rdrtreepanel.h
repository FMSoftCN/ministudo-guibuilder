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

#ifndef RDRTREEPANEL_H_
#define RDRTREEPANEL_H_

class RendererTreePanel: public Panel, public MGCtrlNotification, public TMGStaticSubclass<MGTreeView> {

protected:
	DECLARE_MSG_MAP;

	void onRButtonUp(int x, int y, DWORD key_flag);
	void onPopupMenuCmd(int id);
	BOOL getIDName (int id, char* idName);

public:
	RendererTreePanel(PanelEventHandler* handler);
	virtual ~RendererTreePanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return GetHandle();
	}

	BOOL insertItem(int id, GHANDLE parent);
//	BOOL removeItem(int id, GHANDLE parent);
	BOOL removeItem(int id, GHANDLE parent, BOOL delAllRef=FALSE);
	BOOL selectItem(int id, GHANDLE parent);
	//find first occur item
	GHANDLE findItem(int id, GHANDLE parent);

	GHANDLE getSelItem();
	DWORD getSelItemAddData();
	DWORD getItemAddData(GHANDLE hitem);
	BOOL updateIDName (int id, const char* name);
	BOOL updateIDValue (int oldId, int newId);

	void updateInstance();

	void OnCtrlNotified(MGWnd* ctrl, int id, int code, DWORD add_data);
	BOOL isValidIdHandle(GHANDLE handle);
	GHANDLE getParentHandle(GHANDLE handle);
	GHANDLE setSelItem(GHANDLE selHandle);
};

#endif /* RDRTREEPANEL_H_ */
