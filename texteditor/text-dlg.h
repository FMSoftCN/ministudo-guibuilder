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

#ifndef TEXTDLG_H_
#define TEXTDLG_H_

#include "translater.h"

#define MAX_NAME_LEN 128

class NewItemDialog: public MGMainWnd {
private:
	list<TeNode*> *pConfigList;

	void onOK();
	void onCancel();
	void onDestroy();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	DECLARE_MSG_MAP;

public:
	string lang_country;

	NewItemDialog(HWND hParent, list<TeNode*> *pConfigList);
	~NewItemDialog();
};


class AddTextDialog: public MGMainWnd, EditableListViewHandler {
private:
	EditableListView listview;
	list<TeNode*> *pConfigList;

	void onOK();
	void onCancel();
	void onAdd();
	void onDel();
	void onSetDef();
	void onSetCur();
	void appendText(const char* plang_coutry);
	void updateText();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	void InitListView();

	DECLARE_MSG_MAP;

public:
	AddTextDialog(HWND hParent, list<TeNode*> &configList);
	~AddTextDialog();

	HWND getEditor(HWND hParent, DWORD add_data, int sub_index){return 0;}
};


#define IDCPROG 111
#define IDTITLE 112

class ProgressDialog: public MGMainWnd {
protected:
	int total;
	Translater *trans;
	HWND h_prog;
	void onCancel(void);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	DECLARE_MSG_MAP;

public:

	ProgressDialog(HWND hParent, const char* name, Translater *ts);
	~ProgressDialog();

	int setProg(int prog);
	void setTitle(const char *title);
	void setTotal(int to);
};

#endif /* TEXTDLG_H_ */
