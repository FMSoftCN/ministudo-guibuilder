/*
 * dirtreepanel.cpp
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <list>
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "panel.h"

#include "dirtreepanel.h"
#include "img-event-id.h"

#ifdef WIN32
#include "func-win.h"
#endif

#define ID_DIRTREE		1001

static int GetRealPath (MGTreeView *tree, GHANDLE item, char *realpath, int max)
{
	char szName[MAX_NAME+1];
	int len=0;
	int i;
	int nlen;

	realpath[len] = 0;

	tree->GetItemText(item,szName);

	while(item && item!=tree->GetRoot() && len < max)
	{
		nlen = tree->GetItemText(item,szName);
		if(nlen <= 0)
			break;

		for(i = len-1; i >= 0; i--){
			realpath[i+nlen+1]=realpath[i];
		}
#ifdef WIN32
		realpath[nlen] = '\\';
#else
		realpath[nlen] = '/';
#endif
		for(i = 0; i < nlen; i++)
			realpath[i] = szName[i];

		len += (nlen+1);
		
		item = tree->GetParentItem(item);
	}
#ifndef WIN32    // on windows ignore the root path...
	nlen = tree->GetItemText(item, szName);

	for(i = len-1; i >= 0; i--){
		realpath[i+nlen] = realpath[i];
	}

	for(i = 0; i < nlen; i++)
		realpath[i] = szName[i];

	len += nlen;
#endif
	realpath[len] = '\0';

	return len;
}

static int ItemListDir(MGTreeView *tree, GHANDLE parent, int depth)
{
#ifdef WIN32
	int num_child = 0;
	GHANDLE add;
	char dir[MAX_PATH];
	TVITEMINFO tvItemInfo;

	if (parent == tree->GetRoot())
	{
		// TODO, add the logical devices (C: D: E: ....)
		int i;
		char buff[512];
		int dv_num = win_get_drives(512-1, buff); //GetLogicalDriveStrings
		
		for (i = 0; i < dv_num; i += 4)
		{
			char szTemp[3] = "x:";
			szTemp[0] = buff[i];
			tvItemInfo.text = szTemp;
			tvItemInfo.dwFlags = TVIF_FOLD;
			tvItemInfo.dwAddData = 0;

			add = tree->AddItem(parent, &tvItemInfo);

			if (depth > 0)
				 tree->SetItemAddData(add, ItemListDir (tree, add, depth-1));

			num_child++;
		}
	} 
	else
	{
		void *ffd;
		int hFind;

		if(0 >= GetRealPath(tree, parent, dir, MAX_PATH-1))
		  return 0;
		
		strcat (dir, "\\*");
		ffd = calloc(1, get_file_data_size());

		hFind = win_find_first_file(dir, ffd);

		if (get_invalid_handle() == hFind) 
			return 0;

		do{
			if (win_is_dir(ffd))
			{
				tvItemInfo.text = win_get_file_name(ffd);
				if(strcmp(tvItemInfo.text, ".") == 0
					|| strcmp(tvItemInfo.text,"..") == 0)
					continue;
				tvItemInfo.dwFlags = TVIF_FOLD;
				tvItemInfo.dwAddData = 0;

				add = tree->AddItem(parent, &tvItemInfo);

				if (depth > 0)
					 tree->SetItemAddData(add, ItemListDir (tree, add, depth-1));

				num_child++;
			}
		} while (win_find_next_file(hFind, ffd) != 0);
		
		free(ffd);
		win_close_find(hFind);
	}

	return num_child;
#else
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	TVITEMINFO tvItemInfo;
	GHANDLE add;
	char dir[MAX_PATH];
	char full_name[MAX_PATH];
	int num_child = 0;

	if(0 >= GetRealPath(tree, parent, dir, MAX_PATH-1))
	  return 0;

	if((dp = opendir(dir)) == NULL)
	  return 0;

	while((entry = readdir(dp)) != NULL)
	{
		strcpy(full_name, dir);
		strcat(full_name, entry->d_name);
		stat(full_name, &statbuf);

		if(S_ISDIR(statbuf.st_mode)
				&& strncmp(".",entry->d_name, 1) != 0	
				&& access(full_name, R_OK | X_OK) == 0
				)
		{
			 tvItemInfo.text = entry->d_name;
			 tvItemInfo.dwFlags = TVIF_FOLD;
			 tvItemInfo.dwAddData = 0;
			 add = tree->AddItem(parent, &tvItemInfo);

			 if (depth > 0)
				 tree->SetItemAddData(add, ItemListDir (tree, add, depth-1));

			 num_child ++;
		}
	}

	closedir(dp);

	return num_child;
#endif
}

DirTreePanel::DirTreePanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
}

DirTreePanel::~DirTreePanel() {
	// TODO Auto-generated destructor stub
}

HWND DirTreePanel::createPanel(HWND hParent)
{
	RECT rt;
	GHANDLE root;
#ifdef WIN32
	char tmp[] = "computer";
#else
	char tmp[] = "/";
#endif 
	TVITEMINFO info ={tmp, 0, 0};

	::GetClientRect(hParent, &rt);

	Create("", hParent, 0, 0, RECTW(rt), RECTH(rt),
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_NOTIFY, WS_EX_NONE,
				ID_DIRTREE, (DWORD)&info);

	root = GetRoot();
	SetItemAddData(root, ItemListDir (this, root, 1));
	SetNotification(this);

	return getHandler();
}

void DirTreePanel::OnCtrlNotified(MGWnd* wnd, int id, int code, DWORD add_data)
{
	char dir[MAX_PATH];
	GHANDLE sel;

	if (code == TVN_SELCHANGE)
	{
		HCURSOR old = SetCursor(GetSystemCursor(IDC_BUSY));

		sel = GetSelItem();

		loadSubDirs(sel);

		GetRealPath(this, sel, dir, MAX_PATH-1);
		sendEvent(DIR_TREE_PANEL_DIR_CHANGED, (DWORD)dir);

		SetCursor(old);
	}
}

void DirTreePanel::loadSubDirs(GHANDLE hitem)
{
	if (hitem && hitem != GetRoot())
	{
		GHANDLE child = GetFirstChild(hitem);
		while (child)
		{
			if (GetItemAddData(child) == 0)
			{
				::ShowWindow(getHandler(), SW_HIDE);
				SetItemAddData(child, ItemListDir (this, child, 0));
				::ShowWindow(getHandler(), SW_SHOW);
			}
			child = GetNextSibling(child);
		}
	}
}

void DirTreePanel::setPath(const char* path)
{
	const char* dir = path;
	if(path == NULL || *path != '/')
		return ;

	GHANDLE hitem = GetRoot();

	while(*path && (*path == '/' || *path == '\\')) path++;

	while(*path){
		char szName[MAX_NAME+1];
		int i;
		for(i=0 ; *path && *path != '/' && *path != '\\'; i++, path++ ){
			szName[i] = *path;
		}
		szName[i] = 0;
		hitem = FindChild(hitem, szName);
		if(!hitem)
			return ;
		loadSubDirs(hitem);
		while(*path && (*path == '/' || *path == '\\')) path++;
	}

	if(hitem){
		//Unfold this item
		SetItemFold(hitem, FALSE);
		SetSelItem(hitem);

		sendEvent(DIR_TREE_PANEL_DIR_CHANGED, (DWORD)dir);
	}

}

int DirTreePanel::getCurPath(char* path, int nMax)
{
	GHANDLE sel = GetSelItem();
	if(sel == 0)
		return 0;
	return GetRealPath(this, sel, path, nMax);
}

BEGIN_MSG_MAP(DirTreePanel)
END_MSG_MAP
