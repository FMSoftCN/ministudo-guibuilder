/*
 * textDlg.c
 *
 *  Created on: 2009-4-29
 *      Author: chp
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
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

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

#include "editable-listview.h"
#include "texteditor.h"
#include "textlistpanel.h"
#include "text-dlg.h"

#include "dlgtmpls.h"

#define IDC_SLANG      2000
#define IDC_CLANG      2100
#define IDC_SCHARSET   2200
#define IDC_CCHARSET   2300

#if 0
static CTRLDATA _add_newitem_ctrltmpl[] =
{
	{ CTRL_STATIC,
		WS_CHILD | WS_VISIBLE | SS_LEFT,
		30, 10, 150, 20,
		IDC_SLANG,
		"Select Language:", 0,
		WS_EX_TRANSPARENT | WS_EX_USEPARENTRDR
	},
	{CTRL_COMBOBOX,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST
		| CBS_NOTIFY | CBS_READONLY | WS_BORDER | CBS_EDITNOBORDER,
		30, 35, 150, 20,
		IDC_CLANG,
		"",  0,
		WS_EX_USEPARENTRDR
	},
    {
		CTRL_BUTTON,
		WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
		30, 70, 70, 25,
		IDOK,
		"OK",
		0
    },
	{
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		150, 70, 70, 25,
		IDCANCEL,
		"Cancel",
		0
	},
};

static DLGTEMPLATE _add_newitem_dlgtmpl =
{
	WS_BORDER | WS_CAPTION,
	WS_EX_NONE,
	120, 120, 240, 140,
	"Add Language",
	0, 0,
	TABLESIZE(_add_newitem_ctrltmpl),
	_add_newitem_ctrltmpl,
	0
};
#endif

BOOL NewItemDialog::onInitDialog(HWND hFocus, LPARAM lParam)
{
	int idx;
	HWND hLang = GetChild(IDC_CLANG);
	char szPath[1024];

	sprintf(szPath, "%s", g_env->getConfigFile("language.cfg").c_str());

	GHANDLE hEtc = LoadEtcFile(szPath);

	if(hEtc == 0)
		return FALSE;

	int count = 0;
	if(GetIntValueFromEtc(hEtc, "number","total", &count) != ETC_OK)
		goto FAILED;

	if(count <= 0)
		goto FAILED;

	for(int i=0; i<count; i++)
	{
		char szKeyOne[50];
		sprintf(szKeyOne,"language%d",i);
		char szLanguage[256] = "\0";
		if(GetValueFromEtc(hEtc,"language", szKeyOne, szLanguage, sizeof(szLanguage)-1)!=ETC_OK)
			continue;

		char szKeyTwo[10];
		sprintf(szKeyTwo,"lang%d",i);
		char szLang[10] = "\0";
		if(GetValueFromEtc(hEtc,"lang", szKeyTwo, szLang, sizeof(szLang)-1)!=ETC_OK)
			continue;

		char* langName = strdup(szLang);

		idx = ::SendMessage(hLang, CB_ADDSTRING, 0, (LPARAM)szLanguage);
		if(idx >= 0)
		{
			::SendMessage(hLang, CB_SETITEMADDDATA, idx, (LPARAM)langName);
		}
	}
	::SendMessage(hLang, CB_SETCURSEL, 0, 0);

	UnloadEtcFile(hEtc);
	return TRUE;
FAILED:
	UnloadEtcFile(hEtc);
	return FALSE;

}

void NewItemDialog::onDestroy()
{
	HWND hwnd = GetChild(IDC_CLANG);
	int count = ::SendMessage(hwnd, CB_GETCOUNT, 0, 0);

	for(int i=0; i< count; i++)
	{
		char* str = (char*)::SendMessage(hwnd, CB_GETITEMADDDATA, i, 0);
		if(str)
			free(str);
	}
}

void NewItemDialog::onOK()
{
	int sel;
	HWND hLang = GetChild(IDC_CLANG);

	if(0 <= (sel = ::SendMessage(hLang, CB_GETCURSEL, 0 ,0)))
	{
		lang_country = (const char *)::SendMessage(hLang, CB_GETITEMADDDATA, sel, 0);
	}
	EndDialog(IDOK);
}

void NewItemDialog::onCancel()
{
	EndDialog(IDCANCEL);
}

BEGIN_MSG_MAP(NewItemDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
	MAP_DESTROY(onDestroy)
END_MSG_MAP


NewItemDialog::NewItemDialog(HWND hParent, list<TeNode*> *pConfigList)
{
	int x,y;
	RECT rp,rs;

	this->pConfigList = pConfigList;

#if 0
	Create(hParent, &_add_newitem_dlgtmpl);
#else
	Create(hParent, GetDlgTemplate(ID_ADDLANG));
#endif

	GetWindowRect(&rs);
	::GetWindowRect(hParent, &rp);
	x = rp.left + (RECTW(rp) - RECTW(rs))/2;
	y = rp.top + (RECTH(rp) - RECTH(rs))/2;
	if(x<0) x = 0;
	if(y<0) y = 0;
	MoveWindow(x,y,RECTW(rs),RECTH(rs),TRUE);
}

NewItemDialog::~NewItemDialog()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////

#define IDC_IDLISTVIEW  200
#define IDADD           300
#define IDDEL           400
#define IDSETDEF        500
#define IDSETCUR        600
#define MIN_WIDTH       490
#define OTHERS_WIDTH    330
#define COL_COUNT       4

#if 0
static CTRLDATA _add_text_ctrltmpl[] =
{
    {
		CTRL_BUTTON,
		WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP | WS_GROUP,
		190, 210, 100, 30,
		IDOK,
		"OK",
		0
    },
    {
    	CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		300, 210, 100, 30,
		IDCANCEL,
		"Cancel",
		0
    },
    {
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		320, 18, 90, 30,
		IDADD,
		"Add",
		0
	},
	{
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		320, 55, 90, 30,
		IDDEL,
		"Delete",
		0
	},
	{
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		320, 92, 90, 30,
		IDSETDEF,
		"As Default",
		0
	},
	{
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		320, 129, 90, 30,
		IDSETCUR,
		"As Current",
		0
	},
};

static DLGTEMPLATE _add_text_dlgtmpl =
{
	WS_BORDER | WS_CAPTION,
	WS_EX_NONE,
	100, 100, 440, 285,
	"Profile ...",
	0, 0,
	TABLESIZE(_add_text_ctrltmpl),
	_add_text_ctrltmpl,
	0
};
#endif

BOOL AddTextDialog::onInitDialog(HWND hFocus, LPARAM lParam)
{
    return 1;
}

void AddTextDialog::onOK()
{
	EndDialog(IDOK);
}

void AddTextDialog::onCancel()
{
	EndDialog(IDCANCEL);
}

void AddTextDialog::appendText(const char* plang_country)
{
	BOOL hasCurLang = FALSE;
	char sLang[20] = "";
	char *pCountry = NULL;
	char *pTmp = NULL;
	list<TeNode*>::iterator listIter;

	if(!plang_country || 0 == strlen(plang_country))
		return ;

	strcpy(sLang, plang_country);
	pTmp = strchr(sLang, '_');
	if(NULL == pTmp)
		return ;

	pCountry = pTmp + 1;
	*pTmp = '\0';

	for(listIter = pConfigList->begin(); listIter != pConfigList->end(); ++listIter)
	{
		TeNode *txtNode = *listIter;
		if(strcmp(txtNode->langType, sLang) == 0
				&& strcmp(txtNode->country, pCountry) == 0)
		{
			MessageBox("", "This language exist !", MB_OK);
			return;
		}
		if (txtNode->status == TeNode::CUR_LANG)
			hasCurLang = TRUE;
	}

	int count = listview.GetItemCount();
	HLVITEM hAdd = listview.AddItem(0, count, 28, 0, 0);
	listview.FillSubitem(hAdd, 0, count, 0, sLang, PIXEL_black, 0);
	listview.FillSubitem(hAdd, 0, count, 1, "", PIXEL_black, 0);
	listview.SelectItem(count,0);

	TeNode * txtNode = new TeNode;
	strcpy(txtNode->country, pCountry);
	strcpy(txtNode->langType, sLang);
	if (!hasCurLang)
		txtNode->status = TeNode::CUR_LANG;

	pConfigList->push_back(txtNode);
}

void AddTextDialog::onAdd()
{
	NewItemDialog newItemDialog (m_hWnd , pConfigList);

	if (IDOK == newItemDialog.DoMode())
	{
		appendText(newItemDialog.lang_country.c_str());
	}

	updateText();
}

void AddTextDialog::onDel()
{
	HLVITEM hSelItem = listview.GetSelectedItem();
	TeNode *pNode = (TeNode *)listview.GetItemData(hSelItem);

	if (pNode == NULL)
		return;

	listview.DeleteItem(0, hSelItem);
	pConfigList->remove(pNode);

	updateText();
}

BEGIN_MSG_MAP(AddTextDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		MAP_COMMAND(IDADD, onAdd)
		MAP_COMMAND(IDDEL, onDel)
		MAP_COMMAND(IDSETDEF, onSetDef)
		MAP_COMMAND(IDSETCUR, onSetCur)
	END_COMMAND_MAP
END_MSG_MAP

void AddTextDialog::InitListView()
{
	listview.Attach(m_hWnd, 100);
	listview.AddColumn(0, 140, "language",  0, NULL, LVCF_LEFTALIGN);
	listview.AddColumn(1, 140, "status",  0, NULL, LVCF_LEFTALIGN);

	updateText();
}

void AddTextDialog::updateText()
{
	list<TeNode*>::iterator listIter;
	HLVITEM hlv = 0;
	TeNode *pNode = NULL;
	int i = 0;
	char buff[64] = {0};

	listview.clearAll();

	for(listIter = pConfigList->begin(), i = 0; listIter != pConfigList->end(); ++listIter, ++i)
	{
		pNode = *listIter;
		hlv = listview.AddItem(0, i, 28, (DWORD)pNode, 0);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%s_%s", pNode->langType, pNode->country);
		listview.FillSubitem(hlv, 0, i, 0, buff, PIXEL_black, 0);

		if(0 != (pNode->status & TeNode::DEF_LANG)){
			listview.FillSubitem(hlv, 0, i, 1, "default", PIXEL_black, 0);
		}else if(0 != (pNode->status & TeNode::CUR_LANG)){
			listview.FillSubitem(hlv, 0, i, 1, "current", PIXEL_black, 0);
		}
	}
}

void AddTextDialog::onSetDef()
{
	HLVITEM hlv = 0;
	TeNode *pNode = NULL;
	list<TeNode*>::iterator listIter;
	list<TeNode*>::iterator insideIter;

	if (HWND_NULL == (hlv = listview.GetSelectedItem()))
		return;

	if(NULL == (pNode = (TeNode *)listview.GetItemAddData(hlv,0)))
		return;

	for(listIter = pConfigList->begin(); listIter != pConfigList->end(); ++listIter)
	{
		TeNode *txtNode = *listIter;
		if((txtNode->status & TeNode::DEF_LANG) != 0)
		{
			txtNode->status &= ~TeNode::DEF_LANG;
		}
		if(((txtNode->status & TeNode::CUR_LANG) != 0) && (txtNode == pNode))
		{
			txtNode->status &= ~TeNode::CUR_LANG;
			for(insideIter = pConfigList->begin(); insideIter != pConfigList->end(); ++insideIter)
			{
				TeNode *insideNode = *insideIter;
				if((0 == insideNode->status) && insideNode != pNode)
				{
					insideNode->status = TeNode::CUR_LANG;
					break;
				}
			}
		}
	}
	pNode->status = TeNode::DEF_LANG;

	updateText();
}

void AddTextDialog::onSetCur()
{
	HLVITEM hlv = 0;
	TeNode *pNode = NULL;
	char buf[255] = {0};
	list<TeNode*>::iterator listIter;
	list<TeNode*>::iterator insideIter;

	hlv = listview.GetSelectedItem();

	if (HWND_NULL == (hlv = listview.GetSelectedItem()))
		return;

	if(NULL == (pNode = (TeNode *)listview.GetItemAddData(hlv,0)))
		return;

	listview.GetSubitemText(hlv, 0, buf, 254);

	for(listIter = pConfigList->begin(); listIter != pConfigList->end(); ++listIter)
	{
		TeNode *txtNode = *listIter;
		if((txtNode->status & TeNode::CUR_LANG) != 0)
		{
			txtNode->status &= ~TeNode::CUR_LANG;
		}
		if(((txtNode->status & TeNode::DEF_LANG) != 0) && (txtNode == pNode))
		{
			txtNode->status &= ~TeNode::DEF_LANG;
			for(insideIter = pConfigList->begin(); insideIter != pConfigList->end(); ++insideIter)
			{
				TeNode *insideNode = *insideIter;
				if((0 == insideNode->status) && insideNode != pNode)
				{
					insideNode->status = TeNode::DEF_LANG;
					break;
				}
			}
		}
	}

	pNode->status = TeNode::CUR_LANG;

	updateText();
}

AddTextDialog::AddTextDialog(HWND hParent, list<TeNode*> &configList)
{
	pConfigList = &configList;

#if 0
	Create(hParent, &_add_text_dlgtmpl);
#else
	Create(hParent, GetDlgTemplate(ID_TEXTPROFILE));
#endif

	InitListView();

	CenterWindow();
}

AddTextDialog::~AddTextDialog()
{
	Destroy();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

static CTRLDATA _prog_ctrltmpl[] =
{
	{CTRL_STATIC,
		WS_VISIBLE,
		15, 10, 140, 20,
		IDTITLE,
		"",  0,
		WS_EX_USEPARENTRDR
	},
	{CTRL_PROGRESSBAR,
		WS_VISIBLE,
		15, 35, 210, 25,
		IDCPROG,
		"",  0,
		WS_EX_USEPARENTRDR
	},
	{
		CTRL_BUTTON,
		WS_VISIBLE | BS_PUSHBUTTON,
		70, 70, 100, 25,
		IDCANCEL,
		"Cancel",0
	},
};

static DLGTEMPLATE _prog_dlgtmpl =
{
	WS_BORDER | WS_CAPTION | WS_VISIBLE,
	WS_EX_NONE,
	120, 120, 240, 130,
	"Translater",
	0, 0,
	TABLESIZE(_prog_ctrltmpl),
	_prog_ctrltmpl,
	0
};

BEGIN_MSG_MAP(ProgressDialog)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
END_MSG_MAP

void ProgressDialog ::onCancel()
{
	EndDialog(IDCANCEL);
}

int ProgressDialog ::onInitDialog(HWND hFocus, DWORD lParam)
{
	return 0;
}

ProgressDialog :: ProgressDialog(HWND hParent, const char* name, Translater *ts)
{
	int x, y;
	RECT rs, rp;

	Create(hParent, &_prog_dlgtmpl);

	GetWindowRect(&rs);
	::GetWindowRect(hParent, &rp);
	x = rp.left + (RECTW(rp) - RECTW(rs))/2;
	y = rp.top + (RECTH(rp) - RECTH(rs))/2;
	if(x<0) x = 0;
	if(y<0) y = 0;
	MoveWindow(x,y,RECTW(rs),RECTH(rs),TRUE);

	h_prog = GetDlgItem(m_hWnd, IDCPROG);
	trans = ts;
}

ProgressDialog :: ~ProgressDialog()
{
	Destroy();
}

int ProgressDialog :: setProg(int prog)
{
  ::SendMessage (h_prog, PBM_SETPOS, prog, 0);
  if (prog == total)
	   EndDialog(IDOK);
  return 0;
}

void ProgressDialog :: setTitle(const char * title)
{
	::SetWindowText (GetChild(IDTITLE), title);
}

void ProgressDialog :: setTotal(int to)
{
	total = to;
	 ::SendMessage (h_prog, PBM_SETRANGE, 0, total);
}


