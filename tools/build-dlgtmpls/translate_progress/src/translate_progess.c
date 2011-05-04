/**************************************************************
*  This file is generated automatically, don't modify it.
*  Feynman software Technology Co.,Ltd
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <mgncs/mgncs.h>

#include "resource.h"
#include "ncs-windows.h"


//$mainhandle -- Need by merge, don't modify
NCS_EVENT_HANDLER_INFO mainwnd_TransProgress_handlers [] = {
	//$user --TODO Add your handlers here
	{ -1, NULL } 
};

//$mainwnd_entry -- don't modify
NCS_WND_EXPORT mMainWnd* ntCreateTransProgressEx(HPACKAGE package, HWND hParent, HICON h_icon, HMENU h_menu, DWORD user_data)
{
	// please don't change me
	return ncsCreateMainWindowIndirectFromID(package,
		ID_TRANS_PROGRESS, /* window id */
		hParent,
		h_icon,
		h_menu,
		mainwnd_TransProgress_handlers,
		NULL,
		user_data);
}

