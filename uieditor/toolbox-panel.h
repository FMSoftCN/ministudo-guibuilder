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

#ifndef TOOLBOXPANEL_H_
#define TOOLBOXPANEL_H_


class ToolboxPanel: public Panel, MGCtrlNotification {

	MGListView listview;

	BOOL loadConfig(const char* strConfig);
	BOOL loadControl(xmlNodePtr node, HLVITEM hlvItem, int nItem);
	BOOL loadControls(xmlNodePtr node, HLVITEM hlvParent, int nItem);

	void OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data);

	DWORD sendBySystem;

	string ctrlistName;
    set<PBITMAP> bmps;
    set<char*> items;

public:
	ToolboxPanel(PanelEventHandler *handler, const mapex<string,string>*params);
	virtual ~ToolboxPanel();

	HWND createPanel(HWND hPrent);
	virtual HWND getHandler(){
		return listview.GetHandle();
	}

	void cancelSelect();

};

#endif /* TOOLBOXPANEL_H_ */
