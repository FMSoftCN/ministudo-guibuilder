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

#include "extendidrange.h"
#include "dlgtmpls.h"

#define ID_LBL_MIN			0x2d
#define ID_LBL_MAX			0x2e
#define ID_SLE_MIN			0x26
#define ID_SLE_MAX			0x27

ExtendIDRange::ExtendIDRange(HWND hParent, IDRange* idrange)
{
	if(idrange == NULL)
	{
		throw("ExtendIDRange:(Constructor) param idrange is null");
	}

	this->idrange = idrange;

	Create(hParent, GetDlgTemplate(ID_EXTENDIDRANGE));
	CenterWindow(TRUE);
}


BOOL ExtendIDRange::onInitDialog(HWND hFocus, LPARAM lParam)
{
	SetFocus(GetChild(ID_SLE_MIN));

	char szBuff[100];
	sprintf(szBuff, "%d <=", idrange->prev?idrange->prev->max:idrange->manager->getLimitMin());
	SetChildText(ID_LBL_MIN, szBuff);
	sprintf(szBuff, "<= %d", idrange->next?idrange->next->min:idrange->manager->getLimitMax());
	SetChildText(ID_LBL_MAX, szBuff);

	sprintf(szBuff, "%d", idrange->min);
	SetChildText(ID_SLE_MIN, szBuff);
	sprintf(szBuff, "%d", idrange->max);
	SetChildText(ID_SLE_MAX, szBuff);

	return TRUE;
}

void ExtendIDRange::onOK()
{
	int min, max;
	min = GetChildInt(ID_SLE_MIN);
	max = GetChildInt(ID_SLE_MAX);

	if(min >= max)
	{
		InfoBox(_("Error"), _("Please input the valid range ( min < max)"));
		SetFocus(GetChild(ID_SLE_MIN));
		return ;
	}

	if(min < (idrange->prev?idrange->prev->max:idrange->manager->getLimitMin()))
	{
		InfoBox(_("Error"), _("The Min Value (%d) must be greater then or equal %d"), 
				min, idrange->prev ? idrange->prev->max : idrange->manager->getLimitMin());
		SetFocus(GetChild(ID_SLE_MIN));
		return ;
	}

	if(max > (idrange->next?idrange->next->min:idrange->manager->getLimitMax()))
	{
		InfoBox(_("Error"), _("The Max Value  (%d) must be less then or equal %d"), 
				max, idrange->next ? idrange->next->min : idrange->manager->getLimitMax());
		SetFocus(GetChild(ID_SLE_MAX));
		return ;
	}

	if(! idrange->extend(min, max))
	{
		InfoBox(_("Error"), _("The range (%d,%d] is unacceptable, please input the valid values"), min, max);
		SetFocus(GetChild(ID_SLE_MIN));
		return ;
	}

	idrange->manager->updateNewIDRanges();

	EndDialog(1);
}

void ExtendIDRange::onCancel()
{
  /* if(YesNoBox(_("Question"), _("Do you make sure input the right range?")) == IDYES)
    {
        onOK();
    }
    else
    {
        SetFocus(GetChild(ID_SLE_MIN));
    }*/
	EndDialog(0);
}

BEGIN_MSG_MAP(ExtendIDRange)
	MAP_INITDIALOG(onInitDialog)
	MAP_CLOSE(onCancel)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
END_MSG_MAP

