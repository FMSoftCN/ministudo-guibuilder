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
#include <mgutils/mgutils.h>

#include "resource.h"
#include "ncs-windows.h"


//$func #746252288 NCSN_WIDGET_CLICKED_2113349632_4228349952 -- Need by merge, don't modify
static BOOL sledit2_on_button3_clicked (mSlEdit *self, mButton* sender, int id, DWORD param)
{

	//TODO:
	FILEDLGDATA fdd;
	memset(&fdd, 0, sizeof(fdd));

	strcpy(fdd.filter, "BIN File(*.bin)");

	getcwd(fdd.filepath, sizeof(fdd.filepath));

	if(FileOpenSaveDialog(NULL, self->hwnd, NULL, &fdd))
	{
		SetWindowText(self->hwnd, fdd.filefullname);
	}

	return TRUE; /* allow the event to go next */
}

//$func #746252288 NCSN_WIDGET_CLICKED_724303872_787445760 -- Need by merge, don't modify
static BOOL sledit1_on_button1_clicked (mSlEdit *self, mButton* sender, int id, DWORD param)
{

	//TODO:
	FILEDLGDATA fdd;
	memset(&fdd, 0, sizeof(fdd));

	strcpy(fdd.filter, "BIN File(*.bin)");

	getcwd(fdd.filepath, sizeof(fdd.filepath));

	if(FileOpenSaveDialog(NULL, self->hwnd, NULL, &fdd))
	{
		SetWindowText(self->hwnd, fdd.filefullname);
	}

	return TRUE; /* allow the event to go next */
}

//$func #746252288 NCSN_WIDGET_CLICKED_2619203584_746252288
static BOOL mainwnd1_on_button4_clicked (mMainWnd *self, mButton* sender, int id, DWORD param)
{

	//TODO:
	char key_file[256];
	char exe_file[256];

	mSlEdit *edit_1 = (mSlEdit*)_c(self)->getChild(self, ID_SLEDIT1);
	mSlEdit *edit_2 = (mSlEdit*)_c(self)->getChild(self, ID_SLEDIT2);

	GetWindowText(edit_1->hwnd, key_file, sizeof(key_file));
	GetWindowText(edit_2->hwnd, exe_file, sizeof(exe_file));

	fprintf(stderr, "key_file : %s, exe_file: %s\n", key_file, exe_file);\

	char *cmd[] = { "writehex", exe_file, "ef01", key_file, "a001", (char*)0};

	if ( -1 == execv("./writehex", cmd))
	{
			perror("execv");
			DestroyMainWindow (self->hwnd);
			exit(EXIT_FAILURE);
	}

	DestroyMainWindow (self->hwnd);
	exit(EXIT_SUCCESS);

	return TRUE; /* allow the event to go next */
}

//$func #746252288 NCSN_WIDGET_CLICKED_1958737152_746252288
static BOOL mainwnd1_on_button5_clicked (mMainWnd *self, mButton* sender, int id, DWORD param)
{

	//TODO:
	DestroyMainWindow (self->hwnd);
	return TRUE; /* allow the event to go next */
}

//$connect #746252288 -- Need by merge, don't modify
static NCS_EVENT_CONNECT_INFO Mainwnd1_connects [] = {
	{ID_BUTTON4, ID_MAINWND1, NCSN_WIDGET_CLICKED, (NCS_CB_ONOBJEVENT)mainwnd1_on_button4_clicked},
	{ID_BUTTON3, ID_SLEDIT2, NCSN_WIDGET_CLICKED, (NCS_CB_ONOBJEVENT)sledit2_on_button3_clicked},
	{ID_BUTTON1, ID_SLEDIT1, NCSN_WIDGET_CLICKED, (NCS_CB_ONOBJEVENT)sledit1_on_button1_clicked},
	{ID_BUTTON5, ID_MAINWND1, NCSN_WIDGET_CLICKED, (NCS_CB_ONOBJEVENT)mainwnd1_on_button5_clicked},
//$user -- TODO add your handlers hear
	{-1, -1, 0, NULL}
};

//$func @2619203584 onClicked -- Need by merge, don't modify
static void Button4_onClicked (mButton* self, int id, int nc)
{

	//TODO:

}

//$handle @2619203584 -- Need by merge, don't modify
static NCS_EVENT_HANDLER Button4_handlers [] = {
	{NCS_NOTIFY_CODE(NCSN_WIDGET_CLICKED), Button4_onClicked},
//$user -- TODO add your handlers hear
	{-1,NULL}
};

//$func @1958737152 onClicked -- Need by merge, don't modify
static void Button5_onClicked (mButton* self, int id, int nc)
{

	//TODO:

}

//$handle @1958737152 -- Need by merge, don't modify
static NCS_EVENT_HANDLER Button5_handlers [] = {
	{NCS_NOTIFY_CODE(NCSN_WIDGET_CLICKED), Button5_onClicked},
//$user -- TODO add your handlers hear
	{-1,NULL}
};

//$mainhandle -- Need by merge, don't modify
NCS_EVENT_HANDLER_INFO mainwnd_Mainwnd1_handlers [] = {
//$user --TODO: Add your handlers here
	{-1, NULL}
};

//$mainwnd_entry -- don't modify
NCS_WND_EXPORT mMainWnd* ntCreateMainwnd1Ex(HPACKAGE package, HWND hParent, HICON h_icon, HMENU h_menu, DWORD user_data)
{
	// please don't change me
	return ncsCreateMainWindowIndirectFromID(package,
		ID_MAINWND1, /* window id */
		hParent,
		h_icon,
		h_menu,
		mainwnd_Mainwnd1_handlers,
		Mainwnd1_connects,
		user_data);
}

