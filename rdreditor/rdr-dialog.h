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

#ifndef _RDR_NEWDIALOG_H_
#define _RDR_NEWDIALOG_H_

#define MAX_NAME_LEN 128

class NewRdrSetDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onKeyDown(int scancode, DWORD key_status);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

    string rdrName;
    string idName;

	DECLARE_MSG_MAP;

public:
	NewRdrSetDialog(HWND hParent);
	~NewRdrSetDialog(){}

	const char* getRdrName() {return rdrName.c_str();}
	const char* getIdName() {return idName.c_str();}
};

class NewRdrDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onKeyDown(int scancode, DWORD key_status);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
    void onRdrSelChange (int code, HWND hWnd);

    string rdrName;
    string clsName;
    string idName;
    BOOL visibleCls;

    BOOL isRdrSetDlg;
    BOOL isDisabledCls;

	DECLARE_MSG_MAP;

public:
	NewRdrDialog(HWND hParent, const char* rdrname, 
            const char* clsname, BOOL visibleCls);

	~NewRdrDialog();

	const char* getRdrName() {return rdrName.c_str();}
	const char* getClsName() {return clsName.c_str();}
	const char* getIdName() {return idName.c_str();}
};


class AddRdrDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	BOOL onKeyDown(int scancode, DWORD key_status);

    string rdrName;
	set<int> idList;
	DWORD addData;
	DECLARE_MSG_MAP;

public:
	//addData is the id of RendererSet. Not used, please use 0.
	AddRdrDialog(HWND hParent, const char* rdrName, DWORD addData);
	~AddRdrDialog();

	set<int> getIDList() {return idList; }
};
#endif /*_RDR_NEWDIALOG_H_*/
