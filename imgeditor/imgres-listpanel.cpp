/*
 * imgres-listpanel.cpp
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <map>
#include "mapex.h"
#include <vector>
#include <list>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "resenv.h"
#include "panel.h"
#include "imgres-listpanel.h"
#include "img-event-id.h"
//#include "imgeditor.h"

ImageResListPanel::ImageResListPanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub

}

ImageResListPanel::~ImageResListPanel() {
	// TODO Auto-generated destructor stub
}

HWND ImageResListPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);

	listbox.Create("", hParent,
					0, 0,RECTW(rt), RECTH(rt),
					WS_CHILD| WS_VISIBLE| WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
					0,1);
	listbox.SetNotification(this);

	return getHandler();
}

void ImageResListPanel::OnCtrlNotified(MGWnd* ctrl, int id, int code, DWORD add_data)
{
	if (code == LBN_SELCHANGE){
		int sel = listbox.GetCurSel();
		if(sel < 0)
			return ;

		sendEvent(IMGRES_LISTPANEL_SEL_CHANGE, (DWORD)listbox.GetItemAddData(sel));
	}
}

BOOL ImageResListPanel::AddRes(const char *idName, int id)
{
	int add;

	if ((add = GetIndex(id)) >= 0 )
	{
		listbox.SetItemText(add,idName);
	}
	else
		add = listbox.AddString(idName);

	listbox.SetItemAddData(add, (DWORD)id);

	sendEvent(IMGRES_LISTPANEL_ADD_IMAGE, (DWORD)id);

	return TRUE;
}

BOOL ImageResListPanel::RemoveRes(int id)
{
	int index = GetIndex(id);
	//sendEvent(REM_RES_IMG, id);
	listbox.DeleteItem(index);
	return TRUE;
}

BOOL ImageResListPanel::SelectRes(int id)
{
	listbox.SetCurSel(GetIndex(id));
	return TRUE;
}

int ImageResListPanel::GetSelect()
{
	int sel = listbox.GetCurSel();
	if(sel < 0)
		return -1;
	return listbox.GetItemAddData(sel);
}

int ImageResListPanel::GetIndex(int id)
{
	for(int i=0; i < listbox.GetCount(); i++){
		if(listbox.GetItemAddData(i) == (DWORD)id)
			return i;
	}
	return -1;
}

int ImageResListPanel::GetCount()
{
	return listbox.GetCount();
}

BOOL ImageResListPanel::updateIDName(int newid, int oldid){

	int type = ID2TYPE(newid);

	const char* newName;

	if(type != NCSRT_IMAGE)
		return FALSE;

	if(newid != oldid)
		updateIDValue(oldid, newid);

	ResManager* resMgr = g_env->getResManager(NCSRT_IMAGE);

	newName = resMgr->idToName(newid);

	if(newName)
	{
		int index = GetIndex(newid);
		listbox.SetItemText(index, newName);
	}
	return TRUE;
}

BOOL ImageResListPanel::updateIDValue(int oldId, int newId){
	int idx = GetIndex(oldId);
	listbox.SetItemAddData(idx, (DWORD)newId);
	return TRUE;
}
