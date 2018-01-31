/*
 * editable-listview.cpp
 *
 *  Created on: 2009-3-23
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <string>

#include "mgheads.h"
#include "mgfcheads.h"

#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "editable-listview.h"

#define _CUSTOM_DRAWHDR 1
EditableListView::EditableListView(EditableListViewHandler * handler)
{

	this->handler = handler;
	count = 0;
	editors = NULL;
	hOldSelItem = (HLVITEM) 0 ;
}

EditableListView::~EditableListView() {
    delete[] editors;
}

void EditableListView::editable_listview_notifi(HWND hwnd, int id, int nc, DWORD add_data)
{
	EditableListView * _this = (EditableListView*)MGWnd::WndFromHandle(hwnd);
	if(_this == NULL)
		return ;

	if(nc == LVN_SELCHANGE){
		_this->updateEditors();
	}
}

#ifdef _CUSTOM_DRAWHDR
static void my_draw_hdr_bk (HWND hWnd, HLVHDR hlvhdr, HDC hdc, RECT *rcDraw)
{
    gal_pixel oldClr;
    DWORD color = GetWindowElementAttr(hWnd, WE_BGC_HIGHLIGHT_ITEM);

    oldClr = SetBrushColor (hdc, DWORD2Pixel(hdc, color));
    if (rcDraw->left == 0)
        rcDraw->left += 1;

    if (rcDraw->right > rcDraw->left)
        FillBox (hdc, rcDraw->left, rcDraw->top, RECTWP(rcDraw), RECTHP(rcDraw));
    SetBrushColor(hdc, oldClr);
}

static void my_draw_hdr_item (HWND hWnd, int idx, HDC hdc, RECT *rcDraw)
{

    gal_pixel penColor = GetWindowElementPixelEx(hWnd, hdc, WE_BGC_WINDOW);
    gal_pixel oldPenColor = SetPenColor(hdc, penColor);
    Rectangle (hdc, rcDraw->left, rcDraw->top, rcDraw->right, rcDraw->bottom);
    SetPenColor(hdc, oldPenColor);

    gal_pixel textColor = GetWindowElementPixelEx(hWnd, hdc, WE_FGC_HIGHLIGHT_ITEM);
    gal_pixel oldTextColor = SetTextColor(hdc, textColor);
    LVCOLUMN clmInfo;
    UINT text_format;

    clmInfo.nTextMax = 31;
    clmInfo.pszHeadText = (char*)calloc(1, sizeof(char)*32);
    ::SendMessage(hWnd, LVM_GETCOLUMN, idx, (LPARAM)&clmInfo);

    if (clmInfo.colFlags & LVHF_RIGHTALIGN)
        text_format = DT_SINGLELINE | DT_RIGHT | DT_VCENTER;
    else if (clmInfo.colFlags & LVHF_CENTERALIGN)
        text_format = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
    else
        text_format = DT_SINGLELINE | DT_LEFT | DT_VCENTER;

    ::SetBkMode (hdc, BM_TRANSPARENT);
    DrawText(hdc, clmInfo.pszHeadText, -1, rcDraw, 
            text_format);

    free(clmInfo.pszHeadText);
    SetTextColor(hdc, oldTextColor);
}
#endif

BOOL EditableListView::Create(HWND hParent, int x, int y, int w, int h, DWORD dwStyle, DWORD dwExStyle)
{
	if(!TMGStaticSubclass<MGListView>::Create(hParent, x, y, w, h, dwStyle | LVS_WITHGRID, dwExStyle))
		return FALSE;

	hOldSelItem = 0;

	SetStrCmpFunc(my_strcmp);

#ifdef _CUSTOM_DRAWHDR
    LVCUSTOMDRAWFUNCS myfuncs = {my_draw_hdr_bk, my_draw_hdr_item};
    ::SendMessage (m_hWnd, LVM_SETCUSTOMDRAW, 0, (LPARAM)&myfuncs);
#endif
    ::SendMessage (m_hWnd, LVM_SETHEADHEIGHT, 26, 0);

	SetNotificationCallback(m_hWnd, editable_listview_notifi);

	Subclass();

	return TRUE;

}

void EditableListView::clearAll()
{
	hideAllEditors(FALSE);
	DeleteAllItem();
	hOldSelItem = 0;
}

#define VTM_SAVEVALUE MSG_USER+555
void EditableListView::hideAllEditors(BOOL updateItem)
{
	if(hOldSelItem != (HLVITEM)0)
	{
		for(int i=0; i<count; i++)
		{
			//IMPORTANT : avoid the hideAllEditors recalled
			HWND editor = editors[i];
			if(editor == (HWND) 0 )
				continue;

			//save value
			if(!::SendMessage(editor,VTM_SAVEVALUE, 0, 0))
				continue;

			if(updateItem){
				char szText[512];
				::GetWindowText(editor, szText, sizeof(szText)-1);
				SetSubitemText(hOldSelItem, i, szText);
			}
			//hide editor
			::ShowWindow(editor, SW_HIDE);
			//remove editor
			editors[i] = (HWND) 0;
		}
	}
}

void EditableListView::updateEditors()
{
	checkEditorArray();

	hideAllEditors();

	hOldSelItem = GetSelectedItem();
	DWORD add_data = GetItemData(hOldSelItem);
	LRESULT cx = SendMessage(SVM_GETCONTENTX);
	RECT rt;

	if(!GetSelectedItemRect(&rt))
		return ;

	for(int i=0; i<count; i++){
		int colwidth;
		colwidth = GetColumnWidth(i);
		editors[i] = getEditor(add_data, i);
		if(::IsControl(editors[i]))
		{
			::ThrowAwayMessages(editors[i]);
			::IncludeWindowStyle(editors[i], WS_BORDER);
			::MoveWindow(editors[i], cx, rt.top, colwidth, RECTH(rt),FALSE);
			::ShowWindow(editors[i], SW_SHOW);
		}
		else
			editors[i] = (HWND) 0;
		cx += colwidth;
	}

}

void EditableListView::showEditors(int iCmdShow)
{
	for(int i=0; i<count; i++){
		if(editors[i] == (HWND) 0 )
			continue;
		::ShowWindow(editors[i], iCmdShow);
	}
}

HLVITEM EditableListView::findByAddData(DWORD data, HLVITEM pi/*=HLVITEM(0)*/)
{
	LVFINDINFO lvfindInfo;
	memset(&lvfindInfo, 0, sizeof(lvfindInfo));

	lvfindInfo.flags = LVFF_ADDDATA;
	lvfindInfo.addData = data;

	return FindItem(pi, &lvfindInfo);
}

void EditableListView::offsetEditors(int xoff, int yoff, BOOL bUpdate)
{
	RECT rt;
	for(int i=0; i<count; i++)
	{
		if(editors[i] == (HWND) 0 )
				continue;
		::GetWindowRect(editors[i], &rt);
		::MoveWindow(editors[i], rt.left+xoff, rt.top+yoff, RECTW(rt), RECTH(rt), bUpdate);
	}
}

void EditableListView::deleteItem(HLVITEM hlv, BOOL autoSelect)
{
	if(hlv == 0)
		hlv = GetSelectedItem();
	if(hlv == 0)
		return;

	if(hlv == hOldSelItem)
	{
		hideAllEditors();
		hOldSelItem = 0;
		if(autoSelect)
		{
			HLVITEM hlvRelated = GetRelatedItem(hlv, TVIR_NEXTSIBLING);
			if(hlvRelated == 0)
				hlvRelated = GetRelatedItem(hlv, TVIR_PREVSIBLING);
			if(hlvRelated != 0)
			{
				SelectItem(0,hlvRelated);
				updateEditors();
			}
		}
	}

	DeleteItem(0, hlv);

}

BOOL EditableListView::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{
	//printf( "message = 0x%08X\n", iMsg);
	HWND hactive;
    if(iMsg >= MSG_FIRSTMOUSEMSG && iMsg <= MSG_LASTMOUSEMSG && ::IsControl(hactive=GetFocusChild(m_hWnd)))
	{
		HWND hwnd = (HWND) 0;
		while(::IsControl(hwnd=::GetNextChild(hactive,hwnd)))
		{
			if(::IsWindowVisible(hwnd) &&  ::GetWindowExStyle(hwnd) & 0x20000000L ){ //WS_EX_CTRLASMAINWIN
				int x = LOSWORD(lParam);
				int y = HISWORD(lParam);
				ClientToScreen(&x, &y);
				::ScreenToClient(hactive, &x, &y);
				RECT rt;
				::GetWindowRect(hwnd, &rt);
				//printf("x=%d,y=%d, rt=%d,%d,%d,%d\n", x, y, rt.left, rt.top, rt.right, rt.bottom);
				if(PtInRect(&rt, x, y)){
					*pret = DefaultControlProc(m_hWnd, iMsg, wParam, lParam);
					return TRUE;
				}
			}
		}
	}//end if iMsg
	else if(MSG_PAINT == iMsg)
	{
		if(hOldSelItem != (HLVITEM)0){
			RECT rt;
			//LRESULT cx = -SendMessage(SVM_GETCONTENTX);
			if(!GetSelectedItemRect(&rt))
				return FALSE;

			int cx = rt.left;

			for(int i=0; i<count; i++){
				int colwidth = GetColumnWidth(i);
				if(::IsWindow(editors[i]) && ::IsWindowVisible(editors[i]))
				{
					::MoveWindow(editors[i], cx, rt.top, colwidth+1, RECTH(rt), FALSE);
				}
				cx += colwidth;
			}
			*pret = CallOldProc(iMsg, wParam, lParam);
			for(int i=0; i<count; i++){
				if(::IsWindow(editors[i]) && ::IsWindowVisible(editors[i]))
				{
					//::SendMessage(editors[i], MSG_ERASEBKGND, 0, 0);
					//::SendMessage(editors[i], MSG_PAINT, 0, 0);
					//::UpdateWindow(editors[i],TRUE);
					::InvalidateRect(editors[i],NULL, TRUE);
				}
			}
			HDC hdc = BeginPaint();
			EndPaint(hdc);
			return TRUE;
		}
	}
	else if(iMsg == MSG_KILLFOCUS)
	{
		hideAllEditors();
	}
	else if(iMsg == MSG_SETFOCUS)
	{
	/*  for(int i=0; i<count; i++){
			if(editors[i])
				return FALSE;
		}*/
		updateEditors();
		*pret = CallOldProc(iMsg, wParam, lParam);

		if(!::IsWindow(GetFocus(m_hWnd)))
		{
			for(int i = 0; i < count; i++)
			{
				if(::IsWindow(editors[i]))
				{
					SetFocus(editors[i]);
					break;
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}

