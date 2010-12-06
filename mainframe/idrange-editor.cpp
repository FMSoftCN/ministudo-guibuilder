
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
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

#include "idrange-editor.h"

#include "newidrange.h"

#include "extendidrange.h"

#include "dlgtmpls.h"

#define ID_CB_RESTYPE			0xa
#define ID_LV_USEDRANGE			0xb
#define ID_BTN_NEWRANGE			0xc
#define ID_BTN_EXIT			0xd
#define ID_BTN_EXTEND			0xe


IDRangeEditor::IDRangeEditor(HWND hParent)
{
	bChanged = 0;
	//create
	Create(hParent, GetDlgTemplate(ID_IDRANGEEDITOR));

	CenterWindow();
	ShowWindow(SW_SHOW);
}

BOOL IDRangeEditor::onInitDialog(HWND hFocus, LPARAM lParam)
{

	//intialize the header of list view
	MGListView lvUsed(GetChild(ID_LV_USEDRANGE));
	lvUsed.AddColumn(0,80, "Min",0, NULL, 0);
	lvUsed.AddColumn(1,80, "Max",0, NULL, 0);
	lvUsed.AddColumn(2,150, "ResType",0, NULL, 0);
	lvUsed.AddColumn(3,80,  "Used",0, NULL, 0);
	lvUsed.AddColumn(4,200, "Owner",0, NULL, 0);

	//fill the resource type
	MGComboBox cbResType(GetChild(ID_CB_RESTYPE));

	cbResType.AddString("All");
	vector<IDRangeManager*> mngrlist;
	GetAllIDRangeManagers(mngrlist);

	for(vector<IDRangeManager*>::iterator it = mngrlist.begin();
			it != mngrlist.end(); ++it)
	{
		IDRangeManager* idrm = *it;
		char szText[256];
		sprintf(szText,"%s:%s", idrm->getTypeName(), idrm->getName());
		int cur = cbResType.AddString(szText);
		cbResType.SetItemAddData(cur, (DWORD)(idrm));
	}

	cbResType.SetCurSel(0);
	onResTypeChanged(cbResType.GetHandle());

	return TRUE;
}

void IDRangeEditor::onNewRange()
{
	MGComboBox cbResType(GetChild(ID_CB_RESTYPE));

	IDRangeManager* idrm = (IDRangeManager*)cbResType.GetItemAddData(cbResType.GetCurSel());
	if(idrm == NULL)
	{
		InfoBox(_("Error"), _("Please select the correct resource type first."));
		SetFocus(cbResType.GetHandle());
		return ;
	}

	NewIDRange newIdRange(m_hWnd, idrm, TRUE);

	if(newIdRange.DoMode())
	{
		onResTypeChanged(cbResType.GetHandle());
		bChanged = 1;
	}
}

void IDRangeEditor::onExtendRange()
{
	MGListView lv(GetChild(ID_LV_USEDRANGE));
	HLVITEM hItem = lv.GetSelectedItem();
	if(hItem == 0)
	{
		InfoBox(_("Error"), _("Please select an ID range first"));
		SetFocus(lv.GetHandle());
		return ;
	}

	IDRange *idrange = (IDRange*)lv.GetItemData(hItem);
	if(idrange == NULL)
	{
		InfoBox(_("Error"), _("The current ID range is Null, cannot extend it"));
		SetFocus(lv.GetHandle());
		return ;
	}

	ExtendIDRange extendIdRange(m_hWnd, idrange);
	if(extendIdRange.DoMode())
	{
		onResTypeChanged(GetChild(ID_CB_RESTYPE));
		bChanged = 1;
	}
}

void IDRangeEditor::onExit()
{
	EndDialog(bChanged);
}


static void fill_used_idrange(MGListView& lv, IDRangeManager* idrm)
{
	if(!idrm)
		return;

	//min, max, restype, used, owner
	ResManager *resManager = g_env->getResManager(idrm->getType());

	if(!resManager)
		return;

	const char* restype = resManager->getTypeName(idrm->getType());

	IDRange* idrange = idrm->getHeader();

	int pos = lv.GetItemCount();

	while(idrange)
	{
		char szBuff[100];
		HLVITEM hlvItem = lv.AddItem(0, pos++, 0, (DWORD)idrange);

		sprintf(szBuff, "%d", idrange->min);
		lv.SetSubitemText(hlvItem, 0, szBuff);
		sprintf(szBuff, "%d", idrange->max);
		lv.SetSubitemText(hlvItem, 1, szBuff);
		lv.SetSubitemText(hlvItem, 2, restype);
		sprintf(szBuff, "%d", idrange->getUsed());
		lv.SetSubitemText(hlvItem, 3, szBuff);
		if(idrange->owner)
			lv.SetSubitemText(hlvItem,4, idrange->owner->name.c_str());
		idrange = idrange->next;
	}

}

void IDRangeEditor::onResTypeChanged(HWND hCBResType)
{
	MGListView lv(GetChild(ID_LV_USEDRANGE));
	MGComboBox cb(hCBResType);

	lv.DeleteAllItem();

	IDRangeManager* idrm = (IDRangeManager*)cb.GetItemAddData(cb.GetCurSel());
	if(!idrm)
	{
		vector<IDRangeManager*> mngrlist;
		GetAllIDRangeManagers(mngrlist);

		for(vector<IDRangeManager*>::iterator it = mngrlist.begin();
				it != mngrlist.end(); ++it)
		{
			fill_used_idrange(lv, *it);
		}
	}
	else
	{
		fill_used_idrange(lv, idrm);
	}
}



BEGIN_MSG_MAP(IDRangeEditor)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onExit)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(ID_BTN_NEWRANGE, onNewRange)
		MAP_COMMAND(ID_BTN_EXTEND, onExtendRange)
		MAP_COMMAND(ID_BTN_EXIT, onExit)
		BEGIN_NOTIFY_MAP(ID_CB_RESTYPE)
			MAP_CTRL_NOTIFY(CBN_SELCHANGE, onResTypeChanged)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP



