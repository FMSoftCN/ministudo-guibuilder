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

#ifndef STRUCTPANEL_H_
#define STRUCTPANEL_H_


class StructPanel: public Panel, MGCtrlNotification, TMGStaticSubclass<MGTreeView>
{
public:
	StructPanel(PanelEventHandler* handler);
	virtual ~StructPanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return GetHandle();
	}

	void insertInstance(ComponentInstance *instance, BOOL bhide = FALSE, BOOL bstart_wnd=FALSE);

	void removeInstance(ComponentInstance *instance);

	void selectInstance(ComponentInstance *instance);

	void changeParent(const vector<HWND>* sellist, ComponentInstance * old_parent);

	void refreshInstance(ComponentInstance* instance, BOOL update_children = TRUE);

	void setStartWnd(ComponentInstance *instance);

	void setInstanceHidden(ComponentInstance *instance, BOOL bhide = TRUE, BOOL bstart_wnd=FALSE);

protected:

	DWORD flags ;

	GHANDLE hStartWndItem;

	GHANDLE findItemByInstance(ComponentInstance* instance, GHANDLE item);

	GHANDLE insertInstance(ComponentInstance *instance, GHANDLE parent, BOOL bhide = FALSE,BOOL bstart_wnd=FALSE);

	void OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data);

	void setStartWndText(GHANDLE hitem, BOOL bStartWnd = TRUE);

	DECLARE_MSG_MAP;

	void onRButtonDown(int x, int y, DWORD flags);
	void onMoveItems(int id);
	void onLButtonDBLClick(int x, int y, DWORD flags);

	void switchItem(GHANDLE hItem1, GHANDLE hItem2);

	void resetChildren(GHANDLE hItem, ComponentInstance * inst);

	ComponentInstance* getInstanceFromItem(GHANDLE hItem);

};

#endif /* STRUCTPANEL_H_ */
