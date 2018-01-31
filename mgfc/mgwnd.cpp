/*
 * file mgwnd.cpp the window classes of MGFC
 *
 * MGFC -MiniGUI Foundation Classes, The C++ Classes based on MiniGUI
 * CopyRight (c) 2007
 * Author: vecodo doon. (vecodo@sohu.com)
 * http://www.mgfc.org.cn  http://www.minigui.org
 * Any question e-mail: mgfc888@sohu.com, mgfc666@sina.com
 *
 * GPL Stander
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mgwnd.h"

MGWnd * MGWnd::WndFromHandle(HWND hWnd)
{
	if(::IsWindow(hWnd))
	{
		return (MGWnd*)GetWindowAdditionalData(hWnd);
	}
	return NULL;
}


int MGLoadControls(HWND hWnd,PCTRLDATA pCtrlData,int controls)
{
	int i;
	int num = 0;
	HWND hCtrl;
	if(!IsWindow(hWnd) || controls<=0 ||  pCtrlData==NULL)
	{
		return num;
	}

	for (i = 0; i < controls; i++)
	{
	  if (pCtrlData[i].class_name)
	  {
	      hCtrl = CreateWindowEx (pCtrlData[i].class_name,
	                    pCtrlData[i].caption,
	                    pCtrlData[i].dwStyle | WS_CHILD,
	                    pCtrlData[i].dwExStyle,
	                    pCtrlData[i].id,
	                    pCtrlData[i].x,
	                    pCtrlData[i].y,
	                    pCtrlData[i].w,
	                    pCtrlData[i].h,
	                    hWnd,
	                    pCtrlData[i].dwAddData);
	      if(hCtrl!=HWND_INVALID)
	      	num ++;
	    }
	}
	return num;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
MGDialog::MGDialog()
	:MGMainWnd()
{
}

MGDialog::~MGDialog()
{
}

BOOL MGDialog::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{
	if (iMsg == MSG_CLOSE) {
		EndDialog(IDCANCEL);
		return FALSE;
	}
	return FALSE;
}

MGMainWnd::MGMainWnd()
	:MGWnd()
{
}

MGMainWnd::~MGMainWnd()
{
	//Destroy();
	if(IsMainWindow())
	{
		SetWindowAdditionalData(m_hWnd, 0);
		m_hWnd = HWND_INVALID;
	}
}

BOOL MGMainWnd::Create(DWORD dwStyle,
					const char* spCaption,
					int x,int y,int rx,int by,
					HWND host/*=HWND_DESKTOP*/,
					HCURSOR hCursor/*=(HCURSOR)0*/,
					HMENU hMenu/*=(HMENU)0*/,
					HICON hIcon/*=(HICON)0*/,
					int iBackColor/*=COLOR_lightwhite*/,
					DWORD dwStyleEx/*=WS_EX_NONE*/)
{
	MAINWINCREATE CreateInfo;

	if(hCursor==(HCURSOR)0)
	{
		hCursor = GetSystemCursor(0);
	}

	host = GetMainWindowHandle(host);

	CreateInfo.dwStyle   = dwStyle;
	CreateInfo.dwExStyle = dwStyleEx;
	CreateInfo.spCaption = spCaption;
	CreateInfo.hMenu     = hMenu;
	CreateInfo.hCursor   = hCursor;
	CreateInfo.hIcon     = hIcon;
	CreateInfo.MainWindowProc = MGMainWnd::_mainWndProc;
	CreateInfo.lx        = x;
	CreateInfo.ty        = y;
	CreateInfo.rx        = rx;
	CreateInfo.by        = by;
	CreateInfo.iBkColor  = iBackColor;
	CreateInfo.dwAddData = (DWORD)this;
	CreateInfo.hHosting  = host;
	CreateInfo.dwReserved= 0;

	m_hWnd = CreateMainWindow(&CreateInfo);

	if(!IsMainWindow())
	{
		m_hWnd = HWND_INVALID;
		return FALSE;
	}

	if(dwStyle&WS_VISIBLE)
	{
		ShowWindow();
	}
	return TRUE;
}

BOOL MGMainWnd::Create(HWND hWndParent,PDLGTEMPLATE pDlgTemplate, LPARAM lParam/*=0*/)
{
	MAINWINCREATE CreateInfo;
	HWND hFocus;

	if (pDlgTemplate->controlnr > 0 && !pDlgTemplate->controls)
        return FALSE;

	hWndParent = GetMainWindowHandle(hWndParent);

	CreateInfo.dwStyle        = pDlgTemplate->dwStyle;
	CreateInfo.dwExStyle      = pDlgTemplate->dwExStyle;
	CreateInfo.spCaption      = pDlgTemplate->caption;
	CreateInfo.hMenu          = pDlgTemplate->hMenu;
	CreateInfo.hCursor        = GetSystemCursor (IDC_ARROW);
	CreateInfo.hIcon          = pDlgTemplate->hIcon;
	CreateInfo.MainWindowProc = MGMainWnd::_mainWndProc;
	CreateInfo.lx             = pDlgTemplate->x;
	CreateInfo.ty             = pDlgTemplate->y;
	CreateInfo.rx             = pDlgTemplate->x + pDlgTemplate->w;
	CreateInfo.by             = pDlgTemplate->y + pDlgTemplate->h;
	CreateInfo.iBkColor       =
#if MG_VER >= MG_VER_3_0_x
		GetWindowElementPixel (hWndParent, WE_MAINC_THREED_BODY);
#else
		 GetWindowElementColor (BKC_DIALOG);
#endif
	CreateInfo.dwAddData      = (DWORD)this;
	CreateInfo.hHosting       = hWndParent;

	m_hWnd = CreateMainWindow (&CreateInfo);
	if(m_hWnd==HWND_INVALID)
		return FALSE;

	LoadControls(pDlgTemplate->controls,pDlgTemplate->controlnr);

	hFocus = GetNextTabItem((HWND)0);
	//send initdialog msg
	if(!SendMessage(MSG_INITDIALOG,(WPARAM)hFocus, lParam))
	{
		return FALSE;
	}

	//here clear the wnd;

	return TRUE;
}

DWORD MGMainWnd::DoMode()
{
	HWND hWndOwner, hFocus;
	MSG  msg;
	DWORD dwRetCode = 0;

	if(!IsMainWindow())
		return 0;

	SetWindowAdditionalData2(m_hWnd,(DWORD)(&dwRetCode));

	/*if(hWndOwner == HWND_INVALID)
		hWndOwner = GetParent();
	hWndOwner =:: GetMainWindowHandle(hWndOwner);*/
	hWndOwner = GetHosting();
	if(hWndOwner && hWndOwner != HWND_DESKTOP)
	{
		if(::IsWindowEnabled(hWndOwner))
		{
			::EnableWindow(hWndOwner,FALSE);
			::IncludeWindowExStyle (hWndOwner,/* WS_EX_MODALDISABLED*/0x10000000L);
		}
		while (::PeekPostMessage (&msg, hWndOwner, MSG_KEYDOWN, MSG_KEYUP, PM_REMOVE));
	}

	hFocus = GetNextTabItem((HWND)0);
	if(hFocus)
	{
		SetFocus(hFocus);
	}

	ShowWindow(SW_SHOWNORMAL);

	while( GetMessage (&msg, m_hWnd) ) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
		msg.message = 0;
	}

	if(msg.message == MSG_QUIT) {
		DestroyMainWindow(m_hWnd);
	}

   ::MainWindowCleanup (m_hWnd);

  //ThrowAwayMessages(m_hWnd);

 /* isActive = (GetActiveWindow() == m_hWnd);

  if(hWndParent)
	{
		::EnableWindow(hWndParent,TRUE);
		if(isActive)
		{
			::ShowWindow(hWndParent,SW_SHOWNORMAL);
			::SetActiveWindow(hWndParent);
		}
	}
*/
	return dwRetCode;
}


LRESULT MGMainWnd::_mainWndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	MGMainWnd *pThis;
	pThis = (MGMainWnd*)GetWindowAdditionalData(hWnd);
	if(pThis)
	{
		int ret=0;
		if(iMsg == MSG_CREATE){
			pThis->m_hWnd = hWnd;
			pThis->WndProc(iMsg, wParam, lParam, &ret);
		}
		else if(iMsg == MSG_DESTROY)
		{
			pThis->WndProc(iMsg, wParam, lParam, &ret);
			pThis->Destroy();
		}
		else if(pThis->WndProc(iMsg,wParam,lParam,&ret))
			return ret;
	}

	return DefaultMainWinProc(hWnd,iMsg,wParam,lParam);
}


void MGMainWnd::Destroy()
{
	if(IsMainWindow())
	{
		//MiniGUI can destroy children
		//DestroyAllControls(m_hWnd);
		//DestroyMainWindow(m_hWnd);
		SetWindowAdditionalData(m_hWnd, 0);
		m_hWnd = HWND_INVALID;
	}
}

//////////////////////////////////////////////////////////////////////

BOOL MGMenu::LoadMenuBar(PMENUBARTEMPLATE pMenuBarTemplate)
{
	if(!pMenuBarTemplate)
		return FALSE;
	if(!CreateMenu())
		return FALSE;
	for(int i=0;i<pMenuBarTemplate->MenuItemCnt;i++)
	{
		LoadMenuItem(m_hMenu,i,pMenuBarTemplate->pMenuItems+i,TRUE);
	}
	return TRUE;
}
BOOL MGMenu::LoadPopupMenu(PPOPMENUTEMPLATE pPopMenuTemplate)
{
	m_hMenu = ::LoadPopMenuItem(pPopMenuTemplate);
	return m_hMenu!=(HMENU)NULL;
}

BOOL LoadMenuItem(HMENU hMenu,int iPos,PMUENUITEMTEMPLATE pMenuItemTempl,BOOL fStripPopMenu/*=FALSE*/)
{
	MENUITEMINFO mii;
	if(pMenuItemTempl==NULL)
		return FALSE;
	memset(&mii,0,sizeof(mii));

	mii.mask = MIIM_TYPE;
	if(pMenuItemTempl->Id==-1)
	{
		mii.type = MFT_SEPARATOR;
	}
	else
	{
		mii.type = MFT_STRING;
		mii.id = pMenuItemTempl->Id;
		mii.mask |= (MIIM_ID|MIIM_STATE);
	}
	mii.state = pMenuItemTempl->initState;
	mii.typedata = (DWORD)pMenuItemTempl->strCaption;
	if(pMenuItemTempl->strCaption)
		mii.mask |= MIIM_STRING;
	if(pMenuItemTempl->subMenu)
	{
		mii.hsubmenu = LoadPopMenuItem(pMenuItemTempl->subMenu,fStripPopMenu);
		if(mii.hsubmenu)
			mii.mask |= MIIM_SUBMENU;
	}
	return InsertMenuItem(hMenu,iPos,MF_BYPOSITION,&mii)==0;
}

HMENU LoadPopMenuItem(PPOPMENUTEMPLATE pPopMenuTemplate,BOOL fStripPopMenu/*=FALSE*/)
{
	int i;
	MENUITEMINFO mii;
	HMENU hMenu;
	if(!pPopMenuTemplate)
	{
		return FALSE;
	}
	memset(&mii,0,sizeof(mii));
	mii.type = MFT_STRING;
	mii.typedata = (DWORD)pPopMenuTemplate->strCaption;
	hMenu = CreatePopupMenu(&mii);
	if(hMenu==(HMENU)NULL)
		return FALSE;
	for(i=0;i<pPopMenuTemplate->menuItemCnt;i++)
	{
		LoadMenuItem(hMenu,i,pPopMenuTemplate->pMenuItems+i,TRUE);
	}
	if(fStripPopMenu)
		hMenu = StripPopupHead(hMenu);
	return hMenu;
}


#define INIT_SCROLLINFO(si)  do{ \
	memset(&si, 0, sizeof(si)); \
	si.cbSize = sizeof(si); \
	si.fMask = SIF_ALL; \
}while(0)

int GetWindowMetrics(HWND hwnd, int which){
	const WINDOW_ELEMENT_RENDERER* rdr;
	const WINDOWINFO* wi = GetWindowInfo(hwnd);
	if(wi == NULL ||(rdr = wi->we_rdr) == NULL){
		return 0;
	}

	return (*rdr->calc_we_metrics)(hwnd, NULL, which);
}

static inline void set_scroll_info(HWND hwnd, int iSbar, SCROLLINFO *psi, int doc_size, int view_size, BOOL boldShowed)
{
	GetScrollInfo(hwnd, iSbar, psi);
	psi->nMin = 0;
	psi->nMax = doc_size;
	psi->nPage = view_size;
	if(psi->nPos < psi->nMin)
		psi->nPos = psi->nMin;
	else if(psi->nPos > (int)(psi->nMax - psi->nPage))
		psi->nPos = psi->nMax - psi->nPage;

	if(!boldShowed) //show scroll
	{
		IncludeWindowStyle(hwnd, iSbar==SB_HORZ?WS_HSCROLL:WS_VSCROLL);
		SetScrollInfo(hwnd, iSbar, psi,FALSE);
		EnableScrollBar(hwnd, iSbar, TRUE);
		ShowScrollBar(hwnd, iSbar, TRUE);
	}
	else
		SetScrollInfo(hwnd, iSbar, psi,FALSE);
}

BOOL UpdateHVScroll(HWND hwnd,
	const RECT* rtdoc,
	const RECT* rtview,
	int *x_offset,
	int *y_offset,
	BOOL bAutoUpdate)
{
	SCROLLINFO hsi, vsi;
	DWORD dwStyle, dwOldStyle;
	int scrollsize ;
	int h_view_width;
	int h_doc_width;
	int v_view_height;
	int v_doc_height;
	int hscroll_height;
	int vscroll_width;
	int h_diff;
	int v_diff;

	if(!IsWindow(hwnd) || rtdoc == NULL)
		return FALSE;

	h_doc_width = RECTWP(rtdoc);
	v_doc_height = RECTHP(rtdoc);

	if(rtview == NULL)
	{
		RECT rt;
		GetClientRect(hwnd, &rt);
		h_view_width = RECTW(rt);
		v_view_height = RECTH(rt);
	}
	else{
		h_view_width = RECTWP(rtview);
		v_view_height = RECTHP(rtview);
	}

	dwOldStyle = dwStyle = GetWindowStyle(hwnd);

	scrollsize = GetWindowElementAttr(HWND_NULL,WE_METRICS_SCROLLBAR);

	vscroll_width = GetWindowMetrics(hwnd,LFRDR_METRICS_VSCROLL_W);
	hscroll_height = GetWindowMetrics(hwnd,LFRDR_METRICS_HSCROLL_H);

	if(vscroll_width == 0)
		h_view_width -= scrollsize;
	if(hscroll_height == 0)
		v_view_height -= scrollsize;

	h_diff = h_doc_width - h_view_width;
	v_diff = v_doc_height - v_view_height;

	INIT_SCROLLINFO(hsi);
	INIT_SCROLLINFO(vsi);

	if(h_diff <= scrollsize && v_diff <= scrollsize) //hide HScroll and VScroll
	{
		dwStyle &= ~(WS_HSCROLL|WS_VSCROLL);
		if(dwStyle != dwOldStyle){
			SetScrollInfo(hwnd, SB_HORZ, &hsi, FALSE);
			SetScrollInfo(hwnd, SB_VERT, &hsi, FALSE);
			ExcludeWindowStyle(hwnd, WS_HSCROLL|WS_VSCROLL);
			if(x_offset)
				*x_offset = 0;
			if(y_offset)
				*y_offset = 0;
			if(bAutoUpdate)
				UpdateWindow(hwnd,TRUE);
			return TRUE;
		}
		else{
			if(bAutoUpdate)
				InvalidateRect(hwnd, NULL, TRUE);
		}
		return FALSE;
	}
	else if(h_diff <= 0 && v_diff > scrollsize) //hide  HSCROLL, show VScroll
	{
		if(hscroll_height == 0)
			v_view_height += scrollsize;
		dwStyle &= ~WS_HSCROLL;
		dwStyle |= WS_VSCROLL;
	}
	else if(v_diff <= 0 && h_diff > scrollsize) // hide VSCROLL, show HScroll
	{
		if(vscroll_width == 0)
			h_view_width += scrollsize;
		dwStyle &= ~WS_VSCROLL;
		dwStyle |= WS_HSCROLL;
	}
	else //show VScroll and HScroll
	{
		dwStyle |= (WS_VSCROLL|WS_HSCROLL);
	}

	if(dwStyle & WS_HSCROLL){
		set_scroll_info(hwnd, SB_HORZ, &hsi, h_doc_width, h_view_width, dwOldStyle&WS_HSCROLL);

	}
	else
	{
		if(dwOldStyle & WS_HSCROLL) //hide
		{
			SetScrollInfo(hwnd, SB_HORZ, &hsi,FALSE);
			EnableScrollBar(hwnd, SB_HORZ, FALSE);
			ShowScrollBar(hwnd, SB_HORZ, FALSE);
			ExcludeWindowStyle(hwnd, WS_HSCROLL);
		}
	}

	if(dwStyle & WS_VSCROLL){
		set_scroll_info(hwnd, SB_VERT, &vsi, v_doc_height, v_view_height, dwOldStyle&WS_VSCROLL);
	}
	else
	{
		if(dwOldStyle & WS_VSCROLL) //hide
		{
			SetScrollInfo(hwnd, SB_VERT, &vsi,FALSE);
			EnableScrollBar(hwnd, SB_VERT, FALSE);
			ShowScrollBar(hwnd, SB_VERT, FALSE);
			ExcludeWindowStyle(hwnd, WS_VSCROLL);
		}
	}

	if(x_offset)
		*x_offset = -hsi.nPos;
	if(y_offset)
		*y_offset = -vsi.nPos;


	if(bAutoUpdate){
		UpdateWindow(hwnd,TRUE);
	}

	return TRUE;
}

int ProcessScrollMessage(HWND hwnd, int iSBar, int nc,int pos, int line, BOOL bRedraw)
{
	SCROLLINFO si;
	int nPos;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;

	if(!GetScrollInfo(hwnd, iSBar, &si))
		return -1;
	nPos = si.nPos;

	switch(nc)
	{
	case SB_LINEUP:
	case SB_LINELEFT:
		nPos -= line;
		break;
	case SB_LINEDOWN:
	case SB_LINERIGHT:
		nPos += line;
		break;
	case SB_PAGEUP:
	case SB_PAGELEFT:
		nPos -= si.nPage;
		break;
	case SB_PAGEDOWN:
	case SB_PAGERIGHT:
		nPos += si.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = pos;
		break;
	case SB_ENDSCROLL:
		return nPos;
	default:
		return -1;
	}

	if(nPos < si.nMin)
		nPos = si.nMin;
	else if(nPos >(int) (si.nMax - si.nPage))
		nPos = si.nMax - si.nPage + 1 ;

	si.nPos = nPos;
	SetScrollInfo(hwnd, iSBar, &si, bRedraw);
	return nPos;
}

