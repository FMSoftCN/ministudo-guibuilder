/*
 * dirimgview.cpp
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <map>
#include "mapex.h"
#include <list>
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "stream.h"
#include "resenv.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "dirtreepanel.h"
#include "imgres-listpanel.h"
#include "imageview.h"
#include "dirimgview.h"
#include "resimgview.h"
#include "resdescpanel.h"
#include "imgeditor.h"
#include "img-event-id.h"


DirImageView::DirImageView(PanelEventHandler* handler)
:ImageView(handler)
{
	//EnableMenuItem(hPopupMenu, ImageEditor::IMG_MENUCMD_REMOVE, MF_BYCOMMAND | MF_DISABLED);
}

DirImageView::~DirImageView()
{

}

BOOL DirImageView::SetPath(const char *newpath)
{
	path = (char *)newpath;

	return TRUE;
}

void DirImageView::Refresh()
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char full_name[MAX_PATH];
	char *fileName;

	Reset();

	if((dp = opendir(path)) == NULL)
	  return;

	strcpy(full_name, path);
	fileName = full_name + strlen(full_name);
	if(fileName[-1] != '/' && fileName[-1] != '\\')
	{
	 fileName[0] = '/';
	 fileName ++;
	}

	while((entry = readdir(dp)) != NULL)
	{
		strcpy(fileName, entry->d_name);
		stat(full_name,&statbuf);

		if(S_ISREG(statbuf.st_mode))
		{
			AddFile(full_name, 0);
		}
	}

	closedir(dp);
}
void DirImageView::onRBtnUp(int x, int y, DWORD key_flag)
{
	mapex<int, int>idsets;

	//idsets[ImageEditor::IMG_MENUCMD_SORT] = MAP_MASK_STATE(0, 0);
	//idsets[ImageEditor::IMG_MENUCMD_CLEAN] = MAP_MASK_STATE(0, 0);
	idsets[ImageEditor::IMG_MENUCMD_IMPORT] = MAP_MASK_STATE(0, 0);
	idsets[ImageEditor::IMG_MENUCMD_IMPORTALL] = MAP_MASK_STATE(0, 0);

	hPopupMenu = g_env->createPopMenuFromConfig(ImageEditor::IMG_POPMENU, idsets);
	if(hPopupMenu != 0 && -1 != GetSelectIdx())
	{
		TrackPopupMenu (hPopupMenu, TPM_DEFAULT, x, y, getHandler());
	}
}

HWND DirImageView::_hIconView;
