#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include "mapex.h"
#include <vector>

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

#include "newidrange.h"
#include "dlgtmpls.h"

#define DEFAULT_SIZE        300
#define ID_SLE_MIN			0x26
#define ID_SLE_MAX			0x27
#define ID_LV_FREERANGE			0x18
#define ID_CB_USER			0x21
#define ID_LBL_TIP			0x19
#define ID_LEARNMORE			0x24

NewIDRange::NewIDRange(HWND hParent, IDRangeManager* idrm,BOOL bEnableCancel)
{
	if(idrm == NULL)
	{
		throw("IDRangeManager cannot be NULL");
	}

	this->idrm = idrm;

	Create(hParent, GetDlgTemplate(ID_NEWIDRANGE));
	if(!bEnableCancel)
		::IncludeWindowExStyle(m_hWnd,WS_EX_NOCLOSEBOX);
	EnableChild(IDCANCEL, bEnableCancel);
	CenterWindow(TRUE);
	ShowWindow(SW_SHOW);
}

BOOL NewIDRange::onInitDialog(HWND hFocus, LPARAM lParam)
{
	IDRangeInfoList freeList;

	MGListView lv (GetChild(ID_LV_FREERANGE));

	lv.AddColumn(0, 80, "Min", 0, NULL, 0);
	lv.AddColumn(1, 80, "Max", 0, NULL, 0);

	if(!idrm->getFreeIDRanges(freeList))
		return TRUE;

	int count = 0;

	for(IDRangeInfoList::iterator it = freeList.begin();
			it != freeList.end(); ++it)
	{
		char szBuff[100];
		HLVITEM hlvItem = lv.AddItem(0, count ++);
		sprintf(szBuff, "%d", it->min);
		lv.SetSubitemText(hlvItem, 0, szBuff);
		sprintf(szBuff, "%d", it->max);
		lv.SetSubitemText(hlvItem, 1, szBuff);
	}
	lv.SelectItem(0, NULL);
	onFreeRangeChanged(lv.GetHandle());

	//fill owners
	IDRangeOwnerManager* om = g_env->getIDRangeOwnerManager();
	if(om)
	{
		MGComboBox cbUser(GetChild(ID_CB_USER));
		IDRangeOwner** owners ;
		int count = om->getAllOwners(&owners);
		int cur_idx = 0;
		IDRangeOwner* cur_owner = om->getCurOwner();
		for(int i=0 ; i<count; i++)
		{
			cbUser.AddString(owners[i]->name.c_str());
			if(owners[i] == cur_owner)
				cur_idx = i;
		}
		cbUser.SetCurSel(cur_idx);
		delete[] owners;
	}

	char szText[256];
	sprintf(szText,_("Usable range (for type : %s %s)"), idrm->getTypeName(), idrm->getName());
	SetChildText(ID_LBL_TIP, szText);

	SetFocus(GetChild(ID_SLE_MIN));

	return TRUE;
}

void NewIDRange::onFreeRangeChanged(HWND hlv)
{
	MGListView lv(hlv);
	HLVITEM hlvItem = lv.GetSelectedItem();
	int min, max;
	if(hlvItem == 0)
		return;

	char szBuff[100];
	lv.GetSubitemText(hlvItem, 0, szBuff, 99);
	SetChildText(ID_SLE_MIN, szBuff);
	min = strtol(szBuff, NULL, 0);
	lv.GetSubitemText(hlvItem, 1, szBuff, 99);
	max = strtol(szBuff, NULL, 0);
	min += DEFAULT_SIZE;
	if(max > min){
		max = min;
		sprintf(szBuff, "%d", max);
	}
	SetChildText(ID_SLE_MAX, szBuff);
}

void NewIDRange::onOK()
{
	int min, max;
	IDRangeOwner* owner;
	IDRangeOwnerManager * om = g_env->getIDRangeOwnerManager();

	min = GetChildInt(ID_SLE_MIN);
	max = GetChildInt(ID_SLE_MAX);

	if(min >= max)
	{
		InfoBox(_("Error"), _("Min(%d) must < Max(%d), please reinput them"), min,max);
		SetFocus(GetChild(ID_SLE_MIN));
		return ;
	}

	//get owner
	char szBuff[256];
	GetChildText(ID_CB_USER, szBuff, sizeof(szBuff)-1);

	owner = om->getOwner(szBuff);

	if(owner == NULL)
	{
		if(YesNoBox(_("Question"), _("user \"%s\" does not exist, create it automatically?"), szBuff) != IDYES)
			return ;

		owner = om->newOwner(szBuff);
	}

	if(min < idrm->getLimitMin() || max > idrm->getLimitMax())
	{
		InfoBox(_("Error"), _("The value of Min and max must be in the range (%d, %d]"), idrm->getLimitMin(), idrm->getLimitMax());
		SetFocus(GetChild(ID_SLE_MIN));
		return ;
	}

	IDRange* idrange = idrm->addIDRange(min, max, owner);


	if(idrange == NULL)
	{
		InfoBox(_("Error"), _("ID Range %d ~ %d is invalid"), min, max);
		return ;
	}


	idrm->updateNewIDRanges();

	EndDialog(1);
}

void NewIDRange::onCancel()
{
    /*if(YesNoBox(_("Question"), _("Do you make sure input the right range?")) == IDYES)
    {
        onOK();
    }
    else
    {
        SetFocus(GetChild(ID_SLE_MIN));
    }*/
    EndDialog(0);
}

#define id_range_doc _( \
"Since miniStudio is not a collaborative development platform, it is very easy to " \
"cause ID resource conflict when developers work together to develop a project " \
"by using miniStudio and SVN/CVS or other version control tools. And it is hard to " \
"solve this problem by manual work.\n" \
" \n" \
"To avoid this, miniStudio asks each developer to specify an ID range only for itself." \
"The different developer uses different ID range.\n" \
" \n" \
"If miniStudio doesn't find any ID range for current developmer, it will ask the" \
"developer to specify an ID range.\n" \
" \n" \
"Every resource need a Range for its ID: \n" \
" UI:  for MainWindow; \n" \
" Control: for Controls; \n" \
" Image : for Images; \n" \
" Renderer: for Renderer Resource; \n" \
" Renderer-Set: for Renderer Set Resource; \n" \
" Text: for the Text Resource with a name; \n" \
" Anonymous Text: for the Anonymous Text Resource; " \
)

void NewIDRange::onLearnMore()
{
	::MessageBox(m_hWnd, id_range_doc, _("ID Range Help"), 0);
}

BEGIN_MSG_MAP(NewIDRange)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
		MAP_COMMAND(ID_LEARNMORE, onLearnMore)
		BEGIN_NOTIFY_MAP(ID_LV_FREERANGE)
			MAP_CTRL_NOTIFY(LVN_SELCHANGE, onFreeRangeChanged)
		END_NOTIFY_MAP
	END_COMMAND_MAP
END_MSG_MAP


