/*
 * resdescpanel.cpp
 *
 *  Created on: 2009-4-7
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "panel.h"

#include "resdescpanel.h"

#ifdef WIN32
#include "func-win.h"
#endif

#define ID_STATIC  1002

ResDescPanel::ResDescPanel(PanelEventHandler* handler)
{
	//TODO
}

ResDescPanel::~ResDescPanel()
{
	//TODO
}

HWND ResDescPanel::createPanel(HWND hParent)
{
	RECT rt;

	GetClientRect (hParent, &rt);

	desc.Create("", hParent, 0, 0, RECTW(rt), RECTH(rt),
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER, WS_EX_NONE,
				ID_STATIC, 0);
	return getHandler();
}

void ResDescPanel::setText(const char *text)
{
#ifdef WIN32
	char szText[1024]="";
	asciitoutf8(text, szText, sizeof(szText));
	SetWindowText(getHandler(),szText);
#else
	SetWindowText(getHandler(),text);
#endif
}
