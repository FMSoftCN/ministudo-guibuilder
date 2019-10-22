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
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <dirent.h>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include "msd_intl.h"

using namespace std;

#include "log.h"
#include "undo-redo.h"

#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "class-instance.h"
#include "../uieditor/component-instance.h"
#include "../uieditor/window-instance.h"
#include "rdr-instance.h"

#include "rdrtreepanel.h"
#include "rdr-preview-panel.h"
#include "editable-listview.h"
#include "uieditor/fieldpanel.h"
#include "uieditor/rdrpanel.h"
#include "rdreditor.h"

#include "rdr-dialog.h"
#include "dlgtmpls.h"

#define IDC_RENDERER    100
#define IDC_CTRLCLASS   110
#define IDC_INPUT       120

#define ID_STATIC_RDRTYPE       16
#define ID_STATIC_CTRLTYPE      17
#define ID_STATIC_ID            18
#define ID_EDIT_ID              120
#define ID_COMB_RDR             100
#define ID_COMB_CTRL            110


//for renderer set
#define ID_RDRSET_EDIT_ID       120
#define ID_RDRSET_COMB_RDR      100

BEGIN_MSG_MAP(NewRdrSetDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_KEYDOWN(onKeyDown)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
END_MSG_MAP_PARENT_PROC(MGDialog)

BOOL NewRdrSetDialog::onKeyDown(int scancode, DWORD key_status)
{
    switch (scancode)
    {
        case SCANCODE_KEYPADENTER:
        case SCANCODE_ENTER:
            onOK();
            break;
    }

    return FALSE;
}

void NewRdrSetDialog::onOK()
{
	char name[MAX_NAME_LEN];
	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    GetChildText(ID_RDRSET_COMB_RDR, name, MAX_NAME_LEN);
    rdrName = name;

	GetChildText(ID_RDRSET_EDIT_ID, name, MAX_NAME_LEN);

	if (resMgr->isValidName(name)) {
		idName = name;
		EndDialog(IDOK);
	}
	else {
		MessageBox(_("Warning"), _("The ID name is invalid or already used.\n Please input another name."), MB_OK);
	}
}

void NewRdrSetDialog::onCancel()
{
	EndDialog(IDCANCEL);
}

BOOL NewRdrSetDialog::onInitDialog(HWND hFocus, LPARAM lParam)
{
    set <string> rdrList;
    set <string>::iterator it;
    HWND hRdrWnd, hEditWnd;
    LRESULT idx;
	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    //limit edit maximum length
    hEditWnd = GetChild(ID_RDRSET_EDIT_ID);
    ::SendMessage(hEditWnd, EM_LIMITTEXT, 64, 0);


    resMgr->getRendererList(NULL, &rdrList);
    hRdrWnd = GetChild(ID_RDRSET_COMB_RDR);

    for (it = rdrList.begin(); it != rdrList.end(); ++it) {
        ::SendMessage(hRdrWnd, CB_ADDSTRING, 0, (LPARAM)it->c_str());
    }
    idx = ::SendMessage(hRdrWnd, CB_FINDSTRING, 0, (LPARAM)rdrName.c_str());
    ::SendMessage(hRdrWnd, CB_SETCURSEL, idx, 0);

	return TRUE;
}

NewRdrSetDialog::NewRdrSetDialog(HWND hParent)
{
    PDLGTEMPLATE pDlgTmpl = GetDlgTemplate(ID_NEWRENDERERSET);
    pDlgTmpl->dwAddData = (DWORD)this;
	Create(hParent, pDlgTmpl);
	CenterWindow();
}


/*=====================================================*/
BEGIN_MSG_MAP(NewRdrDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_KEYDOWN(onKeyDown)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
        MAP_COMMANDEX(ID_COMB_RDR, onRdrSelChange)
	END_COMMAND_MAP
END_MSG_MAP_PARENT_PROC(MGDialog)

void NewRdrDialog::onRdrSelChange (int code, HWND hWnd)
{
    if (visibleCls && code == CBN_SELCHANGE) {
        RendererEditor* resMgr = 
            (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
        if (!resMgr)
            return;

        //change class combobox's content
        HWND hClsWnd = GetChild(ID_COMB_CTRL);
        char rdrName[MAX_NAME_LEN], clsName[MAX_NAME_LEN];
        set <string> clsList;
        set <string>::iterator it;
        LRESULT clsSelIdx;

        GetChildText(ID_COMB_RDR, rdrName, MAX_NAME_LEN);
        GetChildText(ID_COMB_CTRL, clsName, MAX_NAME_LEN);

        resMgr->getClassList(rdrName, clsList);
        ::SendMessage(hClsWnd, CB_RESETCONTENT, 0, 0);
	    for (it = clsList.begin(); it != clsList.end(); ++it) {
			::SendMessage(hClsWnd, CB_ADDSTRING, 0, (LPARAM)it->c_str());
	    }

        clsSelIdx = ::SendMessage(hClsWnd, CB_FINDSTRING, 0, (LPARAM)clsName);
	    ::SendMessage(hClsWnd, CB_SETCURSEL, clsSelIdx, 0);
    }
}

BOOL NewRdrDialog::onKeyDown(int scancode, DWORD key_status)
{
    switch (scancode)
    {
        case SCANCODE_KEYPADENTER:
        case SCANCODE_ENTER:
            onOK();
            break;
    }

    return FALSE;
}

void NewRdrDialog::onOK()
{
	char name[MAX_NAME_LEN];
	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    GetChildText(ID_COMB_RDR, name, MAX_NAME_LEN);
    rdrName = name;

	if (visibleCls) {
		GetChildText(ID_COMB_CTRL, name, MAX_NAME_LEN);
		clsName = name;
	}


	GetChildText(ID_EDIT_ID, name, MAX_NAME_LEN);
	if (resMgr->isValidName(name)) {
		idName = name;
		EndDialog(IDOK);
	}
	else {
		MessageBox(_("Warning"), _("The ID name is invalid or already used.\n Please input another name."), MB_OK);
	}
}

void NewRdrDialog::onCancel()
{
	EndDialog(IDCANCEL);
}

BOOL NewRdrDialog::onInitDialog(HWND hFocus, LPARAM lParam)
{
    set <string> clsList;
    set <string> rdrList;
    set <string>::iterator it;
    HWND hRdrWnd, hClsWnd, hEditWnd;
    LRESULT idx;
	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    //set renderer list
    if (strcmp(clsName.c_str(), "") == 0)
        resMgr->getRendererList(NULL, &rdrList);
    else
        resMgr->getRendererList(clsName.c_str(), &rdrList);

    //limit edit maximum length
    hEditWnd = GetChild(ID_EDIT_ID);
    ::SendMessage(hEditWnd, EM_LIMITTEXT, 64, 0);

    hRdrWnd = GetChild(ID_COMB_RDR);

    for (it = rdrList.begin(); it != rdrList.end(); ++it) {
        ::SendMessage(hRdrWnd, CB_ADDSTRING, 0, (LPARAM)it->c_str());
    }
    idx = ::SendMessage(hRdrWnd, CB_FINDSTRING, 0, (LPARAM)rdrName.c_str());
    ::SendMessage(hRdrWnd, CB_SETCURSEL, idx, 0);

	hClsWnd = GetChild(ID_COMB_CTRL);

	if (!visibleCls) {
		if (strcmp(clsName.c_str(), "") != 0)
			::SendMessage(hClsWnd, CB_ADDSTRING, 0, (LPARAM)clsName.c_str());
	}
	else {
	    //set class list
		if (strcmp(rdrName.c_str(), "") == 0)
			resMgr->getClassList(NULL, clsList);
		else
			resMgr->getClassList(rdrName.c_str(), clsList);

	    for (it = clsList.begin(); it != clsList.end(); ++it) {
			::SendMessage(hClsWnd, CB_ADDSTRING, 0, (LPARAM)it->c_str());
	    }
	    idx = ::SendMessage(hClsWnd, CB_FINDSTRING, 0, (LPARAM)clsName.c_str());
	    ::SendMessage(hClsWnd, CB_SETCURSEL, idx, 0);
	}

	return TRUE;
}

NewRdrDialog::NewRdrDialog(HWND hParent, const char* rdrname,
        const char* clsname, BOOL visibleCls)
{
    isRdrSetDlg = FALSE;
    isDisabledCls = FALSE;

    if (!visibleCls) {
        if (clsname == NULL) {
            isRdrSetDlg = TRUE;
        }
        else {
            isDisabledCls = TRUE;
        }
    }

	if (rdrname)
		this->rdrName = rdrname;

	if (clsname)
		this->clsName = clsname;

	this->visibleCls = visibleCls;

    PDLGTEMPLATE pDlgTmpl;
    PCTRLDATA pCtrlCont, pCtrlType;
    pDlgTmpl = GetDlgTemplate(ID_NEWRENDERER);

    pCtrlType = GetControlData(pDlgTmpl, ID_STATIC_CTRLTYPE);
    pCtrlCont = GetControlData(pDlgTmpl, ID_COMB_CTRL);
    if (isDisabledCls) {
        pCtrlType->dwStyle |= WS_DISABLED;
        pCtrlCont->dwStyle |= WS_DISABLED;
        pCtrlCont->caption = clsname;
    }
    else {
        pCtrlType->dwStyle &= ~WS_DISABLED;
        pCtrlCont->dwStyle &= ~WS_DISABLED;
    }

    pDlgTmpl->dwAddData = (DWORD)this;
	Create(hParent, pDlgTmpl);
	CenterWindow();
}

NewRdrDialog::~NewRdrDialog()
{

}

/*=====================================================*/
#define IDC_IDLIST      100
BOOL AddRdrDialog::onKeyDown(int scancode, DWORD key_status)
{
    switch (scancode)
    {
        case SCANCODE_KEYPADENTER:
        case SCANCODE_ENTER:
            onOK();
            break;
    }

    return FALSE;
}

BOOL AddRdrDialog::onInitDialog(HWND hFocus, LPARAM lParam)
{
	set<int> List;
    HWND hWnd;
    set <int>::iterator it;
    RendererSet *inst = NULL;

	RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

	resMgr->getIDListByRdrName(rdrName.c_str(), List, RendererEditor::IDLSIT_RDR);
    hWnd = GetChild(IDC_IDLIST);

    if (addData)
    	inst = (RendererSet *)(resMgr->getRes(addData));

    for (it = List.begin(); it != List.end(); ++it) {
    	if (inst && !(inst->isExist(*it)) )
    		::SendMessage(hWnd, LB_ADDSTRING, 0,
        		(LPARAM)(resMgr->idToName(*it)));
    }
    ::SendMessage(hWnd, LB_SETSEL, 1, 0);

    return 1;
}

void AddRdrDialog::onOK()
{
	RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    HWND hWnd = GetChild(IDC_IDLIST);
	LRESULT count = ::SendMessage(hWnd, LB_GETSELCOUNT, 0, 0);

	if (count > 0) {
		char name [2*MAX_NAME_LEN];
		int id;
		int *items = (int *)calloc(count, sizeof(int));

		count = ::SendMessage(hWnd, LB_GETSELITEMS, count, (LPARAM)items);

		for (int i = 0; i < count; i++) {
			if (LB_OKAY == ::SendMessage(hWnd, LB_GETTEXT, items[i], (LPARAM)name)) {
				id = resMgr->nameToId(name);
				if (id != -1)
					idList.insert(id);
			}
		}
		free(items);
	}
	EndDialog(IDOK);
}

void AddRdrDialog::onCancel()
{
	EndDialog(IDCANCEL);
}

BEGIN_MSG_MAP(AddRdrDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_KEYDOWN(onKeyDown)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
//END_MSG_MAP
END_MSG_MAP_PARENT_PROC(MGDialog)

AddRdrDialog::AddRdrDialog(HWND hParent, const char* rdrName, DWORD addData)
{
	if (!rdrName)
		return;

	//valid rdrName

	this->rdrName = rdrName;
	this->addData = addData;

    PDLGTEMPLATE pDlgTmpl = GetDlgTemplate(ID_ADDRENDERER);
	Create(hParent, pDlgTmpl);
	CenterWindow();
}

AddRdrDialog::~AddRdrDialog()
{
}

