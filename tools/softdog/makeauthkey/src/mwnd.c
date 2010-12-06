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

struct FMSOFT_AUTH_INFO
{
	char sign[8];
	int clientID;
	time_t validDate;
	time_t curDate;
	time_t expiredDate;
}__attribute__((__packed__));

//$func @3825180672 onPushed -- Need by merge, don't modify
static void Button2_onPushed (mButton* self, int id, int nc)
{

	//TODO:
	DestroyMainWindow (GetParent(self->hwnd));
}

//$handle @3825180672 -- Need by merge, don't modify
static NCS_EVENT_HANDLER Button2_handlers [] = {
	{NCS_NOTIFY_CODE(NCSN_BUTTON_PUSHED), Button2_onPushed},
//$user -- TODO add your handlers hear
	{-1,NULL}
};

//$func @1801961472 onPushed -- Need by merge, don't modify
static void Button1_onPushed (mButton* self, int id, int nc)
{

	//TODO:
	struct FMSOFT_AUTH_INFO info;
	struct tm time_str_start;
	struct tm time_str_end;
	time_t start_time;
	time_t end_time;
	char mark[] = "FMSoft";
	FILE *fp;

	char buff[12];
	mSlEdit *edit_1 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT1 );
	mSlEdit *edit_2 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT2 );
	mSlEdit *edit_3 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT3 );
	mSlEdit *edit_4 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT4 );
	mSlEdit *edit_5 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT5 );

	//_c(edit_1)->getContent(edit_1, buff, 12, 0, -1);
	_M(edit_1, getContent, buff, 12, 0, -1);
	time_str_start.tm_year = strtol(buff, 0, 0) - 1900;
	_M(edit_2, getContent, buff, 12, 0, -1);
	time_str_start.tm_mon  = strtol(buff, 0, 0) - 1;
	_M(edit_3, getContent, buff, 12, 0, -1);
	time_str_start.tm_mday = strtol(buff, 0, 0);
	_M(edit_4, getContent, buff, 12, 0, -1);
	time_str_start.tm_hour = strtol(buff, 0, 0);
	_M(edit_5, getContent, buff, 12, 0, -1);
	time_str_start.tm_min = strtol(buff, 0, 0);
	time_str_start.tm_sec = 0;
	time_str_start.tm_isdst = -1;

	mSlEdit *edit_6 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT11 );
	mSlEdit *edit_7 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT12 );
	mSlEdit *edit_8 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT13 );
	mSlEdit *edit_9 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT14 );
	mSlEdit *edit_10 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT15 );

	_M(edit_6, getContent, buff, 12, 0, -1);
	time_str_end.tm_year = strtol(buff, 0, 0) - 1900;
	_M(edit_7, getContent, buff, 12, 0, -1);
	time_str_end.tm_mon  = strtol(buff, 0, 0) - 1;
	_M(edit_8, getContent, buff, 12, 0, -1);
	time_str_end.tm_mday = strtol(buff, 0, 0);
	_M(edit_9, getContent, buff, 12, 0, -1);
	time_str_end.tm_hour = strtol(buff, 0, 0);
	_M(edit_10, getContent, buff, 12, 0, -1);
	time_str_end.tm_min = strtol(buff, 0, 0);
	time_str_end.tm_sec = 0;
	time_str_end.tm_isdst = -1;

	fprintf(stderr, "year is %d, mon is %d, day is %d, hour is %d, min is %d\n",
				time_str_start.tm_year, time_str_start.tm_mon, time_str_start.tm_mday,
				time_str_start.tm_hour, time_str_start.tm_min);

	start_time = mktime(&time_str_start);
	end_time = mktime(&time_str_end);

	info.validDate = start_time;
	info.curDate = 0;
	info.expiredDate = end_time;

	mSlEdit *edit_11 = ncsGetChildObj(GetParent(self->hwnd), ID_SLEDIT16 );
	_M(edit_11, getContent, buff, 12, 0, -1);
	info.clientID = strtol(buff, 0, 0);

	strcpy(info.sign, mark);

	fprintf(stderr, "year is %d, mon is %d, day is %d, hour is %d, min is %d, start_time is"
						 " %d, end_time is %d, sign is %s, id is %d\n",
				time_str_end.tm_year, time_str_end.tm_mon, time_str_end.tm_mday,
				time_str_end.tm_hour, time_str_end.tm_min, info.validDate, info.expiredDate, info.sign,
				info.clientID);

	if ((fp = fopen("auth.bin", "wb")) == NULL)
	{
		fprintf(stderr, "err open file\n");
	}

	fprintf(stderr, "size is %d\n", sizeof(struct FMSOFT_AUTH_INFO));

	if ((fwrite(&info, sizeof(struct FMSOFT_AUTH_INFO), 1, fp)) == 0)
		fprintf(stderr, "err write file\n");

	if ((fclose(fp)) != 0)
		fprintf(stderr, "fclose err\n");
}

//$handle @1801961472 -- Need by merge, don't modify
static NCS_EVENT_HANDLER Button1_handlers [] = {
	{NCS_NOTIFY_CODE(NCSN_BUTTON_PUSHED), Button1_onPushed},
//$user -- TODO add your handlers hear
	{-1,NULL}
};

//$mainhandle -- Need by merge, don't modify
NCS_EVENT_HANDLER_INFO mainwnd_Mainwnd1_handlers [] = {
	{ID_BUTTON1, Button1_handlers},
	{ID_BUTTON2, Button2_handlers},
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
		NULL,
		user_data);
}

