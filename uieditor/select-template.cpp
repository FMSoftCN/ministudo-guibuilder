/*
 * select-template.cpp
 *
 *  Created on: 2009-4-7
 *      Author: dongjunjie
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
#include "msd_intl.h"
using namespace std;

#include "stream.h"

#include "log.h"
#include "undo-redo.h"

#include "resenv.h"

#include "select-template.h"



#define MAX_FILE_LEN 50

#if 1
#include "dlgtmpls.h"
#else
static CTRLDATA _selected_templ_ctrls[] = {
	{
		CTRL_PROPSHEET,
		WS_VISIBLE|PSS_SIMPLE|PSS_SCROLLABLE,
		20, 20, 460, 250,
		100,
		"",
		0
	},
	{
		CTRL_STATIC,
		WS_VISIBLE,
		20, 288, 100, 25,
		-1,
		"Filename:",
		0
	},
	{
		CTRL_SLEDIT,
		WS_VISIBLE|ES_LEFT|WS_BORDER,
		120, 280, 360, 30,
		101,
		"",
		0
	},
	{
		CTRL_BUTTON,
		WS_VISIBLE|BS_AUTOCHECKBOX,
		20, 330, 200, 30,
		102,
		"Auto Over Write file",
		0
	},
	{
		"button",
		WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP,
		400, 330, 80, 30,
		IDOK,
		"OK",
		0
	},
	{
		"button",
		WS_VISIBLE|BS_PUSHBUTTON|WS_TABSTOP,
		300, 330, 80, 30,
		IDCANCEL,
		"Cancel",
		0
	}
};

static DLGTEMPLATE _selected_templ_tmpl = {
	WS_BORDER|WS_CAPTION|WS_DLGFRAME,
	WS_EX_NONE,
	0,0,500,400,
	"New File ....",
	0,0,
	sizeof(_selected_templ_ctrls)/sizeof(CTRLDATA),
	_selected_templ_ctrls,
	0
};
#endif

SelectTemplate::SelectTemplate(HWND hParent, const char* strTemplPaths) {
	// TODO Auto-generated constructor stub

	//create Window
#if 1
	Create(hParent, GetDlgTemplate(ID_NEWFILE));
#else
	Create(hParent, &_selected_templ_tmpl);
#endif

	hOwner = hParent;

	//load templates
	if(strTemplPaths == NULL){
		loadTemplFromDir(g_env->getConfigFile(DEF_TEMPL_PATH).c_str());
		loadTemplFromDir(g_env->getConfigFile(USR_TEMPL_PATH).c_str());
		string str_extend;
		//get resource
		ResManager *resMgr = g_env->getResManager(NCSRT_UI);
		if(resMgr && resMgr->callSpecial("getExtendTemplates", &str_extend))
		{
			loadTemplFromDir(str_extend.c_str());
		}
	}
	else
	{
		char szPath[512];
		while(1)
		{
			const char* str1 = strchr(strTemplPaths, ';');
			if(str1)
				strncpy(szPath, strTemplPaths, str1 - strTemplPaths);
			else
				strcpy(szPath, strTemplPaths);
			loadTemplFromDir(szPath);
			if(str1 == NULL)
				break;
			strTemplPaths = str1 + 1;
		}
	}

	CenterWindow();

}

SelectTemplate::~SelectTemplate() {
	// TODO Auto-generated destructor stub
}

void SelectTemplate::loadTemplFromDir(const char* path)
{
	HWND hIconView = (HWND)0;
	if(path == NULL)
		return;

	HWND hPropSheet = GetChild(100);

	DIR* dir = opendir(path);

	if(dir == NULL)
		return;

	struct dirent* dirent;
	char szFullName[512] = "\0";
	int len;

	strcpy(szFullName, path);
	len = strlen(szFullName);
	szFullName[len++] = '/';

	while((dirent = readdir(dir)))
	{
		if(strcmp(dirent->d_name, ".") == 0
				|| strcmp(dirent->d_name, "..") == 0)
			continue;

		//get filull name
		strcpy(szFullName+len, dirent->d_name);

		struct stat statbuf;
		if(stat(szFullName, &statbuf) == 0)
		{
			if(S_ISREG(statbuf.st_mode)) // is file
			{
				const char* strext = strrchr(dirent->d_name,'.');
				if(strext == NULL)
					continue;

				if(strcasecmp(strext+1, "tmpl") != 0)
					continue;

				//insert
				if(hIconView == (HWND)0)
					hIconView = insertNewPage(hPropSheet, path);
				//insert
				insertTemplate(szFullName, hIconView);

			}
			else if(S_ISDIR(statbuf.st_mode))
			{
				loadTemplFromDir(szFullName);
			}
		}
	}

	closedir(dir);

}


static LRESULT _template_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == MSG_SIZECHANGED)
	{
		DefaultControlProc(hwnd, message, wParam, lParam);
		HWND hchild = GetNextChild(hwnd,0);
		if(IsControl(hchild))
		{
			RECT rt;
			GetClientRect(hwnd, &rt);
			MoveWindow(hchild, 0, 0, RECTW(rt),RECTH(rt),FALSE);
		}
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

/* item struct */
struct _MgItem
{
    list_t    list;          // list pointer
    list_t    sel_list;      // selected list pointer
    DWORD     flags;         // item flags

    DWORD     addData;       // item additional data
};

static void onDelIconView(HWND hwnd, HSVITEM hsvi){
	if(hsvi){
		PIVITEMINFO pivitem = (PIVITEMINFO) &((struct _MgItem*)hsvi)->addData;
		if(pivitem->bmp){
			UnloadBitmap(pivitem->bmp);
			delete pivitem->bmp;
		}
        free((void*)pivitem->addData);
	}
}

HWND SelectTemplate::insertNewPage(HWND hPropSheet, const char* szPath)
{
	if(!::IsWindow(hPropSheet) || szPath == NULL)
		return HWND_INVALID;

	const char* strName = strrchr(szPath, '/');
	char szCaption[150];
	if(strName == NULL){
		strName = strrchr(szPath, '\\');
	}

	if(strName == NULL)
		strName = szPath;
	else
		strName ++;

	strncpy(szCaption, strName, sizeof(szCaption)-1);
	szCaption[sizeof(szCaption)-1] = 0;
	szCaption[0] = toupper(szCaption[0]);

	//insert a new page
	DLGTEMPLATE _template;

	memset(&_template, 0, sizeof(DLGTEMPLATE));
	_template.caption = szCaption;

	int idx = (int)::SendMessage(hPropSheet, PSM_ADDPAGE, (WPARAM)&_template, (LPARAM) _template_proc);

	if(idx!= PS_ERR){
		RECT rt;
		HWND hpage = (HWND)::SendMessage(hPropSheet, PSM_GETPAGE, (WPARAM)idx, 0);
		::GetClientRect(hpage, &rt);
		HWND hIconView = CreateWindow(CTRL_ICONVIEW, "", WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL,
					1, 0, 0, RECTW(rt), RECTH(rt), hpage, 0);

		::SendMessage(hIconView, IVM_SETITEMDESTROY, 0, (LPARAM)onDelIconView);
		::SendMessage(hIconView, IVM_SETITEMSIZE, 128, 128);

		return hIconView;
	}

	return HWND_INVALID;
}

/////////////////
// get file
PBITMAP getTemplOutline(const char* file)
{
	char szImageFile[512];
	strcpy(szImageFile, file);
	char* strext = (char*)strrchr(szImageFile,'.');
	strext ++;

	static const char* img_ext [] = {
		"png",
		"bmp",
		"jpg",
		"jpeg",
		"gif",
		NULL
	};

	PBITMAP pbmp = new BITMAP;

	for(int i=0; img_ext[i]; i++)
	{
		strcpy(strext, img_ext[i]);
		if(LoadBitmapFromFile(HDC_SCREEN, pbmp, szImageFile) == 0)
			return pbmp;
	}

	delete pbmp;

	return NULL;
}

void SelectTemplate::insertTemplate(const char* file, HWND hIconView)
{
	if(file == NULL || !::IsControl(hIconView))
		return;

	const char* strFileName = strrchr(file, '/');

	if(strFileName == NULL)
		strFileName = strrchr(file, '\\');

	if(strFileName == NULL)
		strFileName = file;
	else
		strFileName ++;

	char szFile[256];
	strcpy(szFile, strFileName);
	char* str = (char*)strrchr(szFile,'.');
	if(str)
		*str = 0;

	//get template files
	IVITEMINFO ivItem;
	memset(&ivItem, 0, sizeof(ivItem));
	ivItem.bmp = getTemplOutline(file);
	ivItem.label = szFile;
	ivItem.nItem = ::SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0);
	ivItem.addData = (DWORD)strdup(file);

	::SendMessage(hIconView, IVM_ADDITEM, 0, (LPARAM)&ivItem);

}

BEGIN_MSG_MAP(SelectTemplate)
	MAP_INITDIALOG(onInitDialog)
	//MAP_DESTROY(onDestroy)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
    MAP_CLOSE(onCancel)
END_MSG_MAP

void SelectTemplate::onOK()
{
	//get current idx
	HWND hPropSheet = GetChild(100);
	int idx = (int)::SendMessage(hPropSheet, PSM_GETACTIVEINDEX, 0, 0);
	HWND hPage = (HWND)::SendMessage(hPropSheet, PSM_GETPAGE, idx, 0);
	HWND hIconView = ::GetNextChild(hPage, 0);


	idx = (int)::SendMessage(hIconView, IVM_GETCURSEL, 0, 0);
	char* strfile = (char*)::SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);

	if(!strfile)
	{
		InfoBox(_("Error"), _("Please select a template first!"));
		::SetFocus(GetChild(100));
		return ;
	}

	if(!updateNewFile())
		return ;

	if(!copyfile(strfile, newFile.c_str())){
		InfoBox(_("Error"), _("Cannot create the new file \"%s\""), newFile.c_str());
		return;
	}

	EndDialog(1);
	return;
}

void SelectTemplate::onCancel()
{
	EndDialog(0);
}

BOOL SelectTemplate::onInitDialog(HWND hFocus, LPARAM lParam)
{
	::SendDlgItemMessage(m_hWnd, 101, EM_SETLIMITTEXT, MAX_FILE_LEN-1, 0);
	return TRUE;
}

BOOL SelectTemplate::updateNewFile()
{
	char szFileName[1024];
	char* file;
	int len;
	int dwTextLen;

	sprintf(szFileName,"%s/ui/",g_env->getResourcePath());
	len = strlen(szFileName);
	file = szFileName + len;

	if((dwTextLen = GetChildText(101, file, sizeof(szFileName) - len))<=0)
	{
		InfoBox(_("Error"), _("Please input filename!"));
		::SetFocus(GetChild(101));
		return FALSE;
	}

	if(dwTextLen >= MAX_FILE_LEN)
	{
		if(YesNoBox(_("Warning"), _("The filename is too long, and will be get cut off. Do you want to input another filename?")) == IDYES)
		{
			::SetFocus(GetChild(101));
			return FALSE;
		}
		file[MAX_FILE_LEN] = '\0';
	}

	const char* rextend = strrchr(file, '.');
	if(!rextend || strcmp(rextend, ".xml")!=0)
	{
		if(!rextend)
			strcat(file,".xml");
		else
			strcpy((char*)rextend, ".xml");
	}

	if(::SendMessage(hOwner,MSG_SLT_ISOPENED,0, (LPARAM)file)){
		InfoBox(_("Error"), _("This file \"%s\" has been opened, please select another file!"),file);
		::SetFocus(GetChild(101));
			return FALSE;
	}

	//test file is exist
	if(!IsDlgButtonChecked(102))
	{
		if(isFileExist(szFileName)
				&& YesNoBox(_("Warning"), _("The File \"%s\" already exists, do you want to overwrite it?"), szFileName) == IDNO)
		{
			return FALSE;
		}
	}

	newFile = szFileName;
	return TRUE;
}

BOOL copyfile(const char* filesrc, const char* filedst)
{
	BOOL bret = FALSE;
	FILE* fpsrc = fopen(filesrc,"rb");
	FILE* fpdst = fopen(filedst,"wb");

	if(fpsrc == NULL || fpdst == NULL)
		goto FAILED;

	while(!feof(fpsrc))
	{
		char szText[1024];
		size_t len = fread(szText,1, sizeof(szText), fpsrc);
		fwrite(szText, 1, len, fpdst);
	}
	bret = TRUE;

FAILED:
	if(fpsrc)
		fclose(fpsrc);
	if(fpdst)
		fclose(fpdst);
	return bret;
}
