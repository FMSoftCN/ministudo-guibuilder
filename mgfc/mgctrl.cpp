/*
 * file mgctrl.cpp - the minigui control of MGFC
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
#include "mgctrl.h"

static int KMPMatch_Forward(const char *t,int n,const char* p,int m);
static int KMPMatch_Backward(const char* t,int n,const char* p,int m);
static int KMPMatch_Forward_NoCaps(const char *t,int n,const char* p,int m);
static int KMPMatch_Backward_NoCaps(const char* t,int n,const char* p,int m);


//MGUserCtrl
BOOL MGUserCtrl::Register(const char* spClass,DWORD opMask, DWORD dwStyle/* = WS_CHILD*/, DWORD dwStyleEx/* = WS_EX_NONE*/, HCURSOR hCursor /*=(HCURSOR)0*/, int iBkColor/* = COLOR_lightgray*/)
{
	WNDCLASS wndCls;
	wndCls.dwAddData = 0;
	wndCls.dwExStyle = dwStyleEx;
	wndCls.dwStyle = dwStyle;
	wndCls.hCursor = hCursor;
	wndCls.iBkColor = iBkColor;
	wndCls.opMask = opMask;
	wndCls.spClassName = (char*) spClass;
	wndCls.WinProc = (WNDPROC) MGUserCtrl::_WndProc;
	return RegisterWindowClass(&wndCls)==ERR_OK;
}

int MGUserCtrl::_WndProc(HWND hWnd, int iMsg, WPARAM wParam, LPARAM lParam)
{
	MGUserCtrl* pThis = (MGUserCtrl*)GetWindowAdditionalData(hWnd);
	if(pThis)
	{
		int ret;
		if(pThis->WndProc(iMsg,wParam,lParam,&ret))
			return ret;
	}
	return DefaultControlProc(hWnd,iMsg,wParam,lParam);
}

///////////////////////////////////////////////////////////////////////
// Edit
//WNDPROC TMGStaticSubclass<MGEdit>::_oldProc = NULL;
void TransPos(const char* pstr,char plinesep,int ipos,int *pline_pos,int *pchar_pos)
{
	int line_pos = 0;
	int char_pos = 0;
	int i = 0;
	while(i<ipos)
	{
		if(pstr[i]==plinesep)
		{
			line_pos ++;
			char_pos = 0;
		}
		else
		{
			char_pos ++;
		}
		i ++;
	}
	if(pline_pos)
		*pline_pos = line_pos;
	if(pchar_pos)
		*pchar_pos = char_pos;
}

int MGEdit::Find(const char* pFind,BOOL fDown/*=TRUE*/,BOOL fCaps/*=FALSE*/)
{
	int len;
	int editLen;
	int old_caret_line_pos;
	int old_caret_char_pos;
	int old_sel_line_pos;
	int old_sel_char_pos;
	int caret_line_pos;
	int caret_char_pos;
	int sel_line_pos;
	int sel_char_pos;
	char *ptext=NULL;
	int textLen;
	int ipos;
	int ret;

	if(pFind == NULL)
		return -1;
	len = strlen(pFind);
	if(len<=0)
		return -1;

	editLen = GetWindowTextLength();
	//printf("editLen:%d\n",editLen);
	if(editLen<len)
		return -1;

	ret = GetCaretPos(&old_caret_line_pos,&old_caret_char_pos);
	ret = GetSelPos(&old_sel_line_pos,&old_sel_char_pos);
	if(ret<0)
	{
		old_sel_line_pos = old_caret_line_pos;
		old_sel_char_pos = old_caret_char_pos;
	}

	if(fDown)
	{
		caret_line_pos = max(old_caret_line_pos,old_sel_line_pos);
		caret_char_pos = max(old_caret_char_pos,old_sel_char_pos);
		sel_line_pos = GetLineCount()-1;
		sel_char_pos = 0;//editLen;
		//textLen = editLen-sel_char_pos + 1;
	}
	else
	{
		caret_line_pos = 0;
		caret_char_pos = 0;
		sel_line_pos = min(old_caret_line_pos,old_sel_line_pos);
		sel_char_pos = min(old_caret_char_pos,old_sel_char_pos);

		//textLen = sel_char_pos;
	}

	//printf("caret_line_pos=%d,caet_char_pos=%d\n",caret_line_pos,caret_char_pos);
	//printf("sel_line_pos=%d,sel_char_pos=%d\n",sel_line_pos,sel_char_pos);

	SetCaretPos(caret_line_pos,caret_char_pos);

	ptext = (char*)malloc(editLen);
	if(ptext==NULL)
		goto FAIL;

	SetSel(sel_line_pos,sel_char_pos);

	textLen = GetSelText(ptext,editLen);

	//printf("----------------%d:%s\n",textLen,ptext);

	//return -1;

	if(textLen<len)
	{
		ipos = -1;
		goto FAIL;
	}

	if(fDown)
	{
		if(fCaps)
			ipos = KMPMatch_Forward(ptext,textLen,pFind,len);
		else
			ipos = KMPMatch_Forward_NoCaps(ptext,textLen,pFind,len);
	}
	else
	{
		if(fCaps)
			ipos = KMPMatch_Backward(ptext,textLen,pFind,len);
		else
			ipos = KMPMatch_Backward_NoCaps(ptext,textLen,pFind,len);
	}

	//printf("ipos:%d\n",ipos);

	if(ipos<0)
		goto FAIL;

	//find the pos of line_pos and char_pos
	{
		int line_pos,char_pos;
		TransPos(ptext,'\n',ipos,&line_pos,&char_pos);
		if(fDown)
		{
			sel_line_pos = line_pos + caret_line_pos;
			sel_char_pos = char_pos;
			if(line_pos==0)
			{
				sel_char_pos += caret_char_pos;
			}
		}
		else
		{
			sel_line_pos = line_pos;
			sel_char_pos = char_pos;
		}
	}

	//printf("sel_line_pos=%d,sel_char_pos=%d\n",sel_line_pos,sel_char_pos);

	free(ptext);

	SetCaretPos(sel_line_pos,sel_char_pos);
	SetSel(sel_line_pos,sel_char_pos+len);
	return ipos;

FAIL:
    free(ptext);
	SetCaretPos(old_caret_line_pos,old_caret_char_pos);
	SetSel(old_sel_line_pos,old_sel_char_pos);
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////
// MGSheetPage
BOOL MGSheetPage::OnInitPage(DLGTEMPLATE *pdlg_temp)
{
	return TRUE;
}

BOOL MGSheetPage::OnShowPage(HWND focus_hwnd,int show_cmd)
{
	return TRUE;
}

int MGSheetPage::OnSheetCmd(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

BOOL MGSheetPage::WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{
	switch(iMsg){
	case MSG_INITPAGE:
		OnInitPage((DLGTEMPLATE*)lParam);
		break;
	case MSG_SHOWPAGE:
		OnShowPage((HWND)wParam,(int)lParam);
		break;
	case MSG_SHEETCMD:
		switch(wParam){
		case IDOK:
			OnOK(lParam);
			break;
		case IDCANCEL:
			OnCancel(lParam);
			break;
		case IDAPPLY:
			OnApply(lParam);
			break;
		default:
			OnSheetCmd(wParam,lParam);
			break;
		}
	}
	*pret = DefaultPageProc(m_hWnd,iMsg,wParam,lParam);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// kmp string search
// forward search
static unsigned char* KMPFailureFunc_Forward(const char* p,int m)
{
	int i=1,j=0;
	unsigned char* tbl = (unsigned char*)malloc(m);
	tbl[0] = 0;
	while(i<m)
	{
		if(p[j]==p[i])
		{
			tbl[i] = j+1;
			i++;
			j++;
		}
		else if(j>0)
		{
			j = tbl[j-1];
		}
		else
		{
			tbl[i]=0;
			i++;
		}
	}
	return tbl;
}

static int KMPMatch_Forward(const char *t,int n,const char* p,int m)
{
	int i=0;
	int j=0;
	int ipos = -1;
	unsigned char* tbl = KMPFailureFunc_Forward(p,m);

	while(i<n)
	{
		if(p[j]==t[i])
		{
			if(j==m-1)
			{
				ipos = i-m+1;
				goto RETURN;
			}
			i++;
			j++;
		}
		else if(j>0)
		{
			j = tbl[j-1];
		}
		else
		{
			i++;
		}
	}
RETURN:
	free(tbl);
	return ipos;
}


//backward search
static unsigned char * KMPFailureFunc_Backward(const char* p,int m)
{
	int i=m-2,j=m-1;
	unsigned char* tbl = (unsigned char*)malloc(m);
	tbl[j] = m-1;

	while(i>=0)
	{
		if(p[j]==p[i])
		{
			tbl[i] = m-j-1;
			i--;
			j--;
		}
		else if(j<m-1)
		{
			j = tbl[j+1];
		}
		else
		{
			tbl[i]=m-1;
			i--;
		}
	}
	return tbl;
}

static int KMPMatch_Backward(const char* t,int n,const char* p,int m)
{
	int i=n-1;
	int j=m-1;
	int ipos = -1;
	unsigned char* tbl = KMPFailureFunc_Backward(p,m);

	while(i>=0)
	{
		if(p[j]==t[i])
		{
			if(j==0)
			{
				ipos = i;
				goto RETURN;
			}
			i--;
			j--;
		}
		else if(j<m-1)
		{
			j = tbl[j+1];
		}
		else
		{
			i--;
		}
	}
RETURN:
	free(tbl);
	return ipos;
}


/////////////////////////////////////
// with upper, no caps
static inline char upper(char ch)  { return ch>'a'&&ch<'z'?(ch+'A'-'a'):ch; }
// forward search
static unsigned char* KMPFailureFunc_Forward_NoCaps(const char* p,int m)
{
	int i=1,j=0;
	unsigned char* tbl = (unsigned char*)malloc(m);
	tbl[0] = 0;
	while(i<m)
	{
		if(upper(p[j])==upper(p[i]))
		{
			tbl[i] = j+1;
			i++;
			j++;
		}
		else if(j>0)
		{
			j = tbl[j-1];
		}
		else
		{
			tbl[i]=0;
			i++;
		}
	}
	return tbl;
}

static int KMPMatch_Forward_NoCaps(const char *t,int n,const char* p,int m)
{
	int i=0;
	int j=0;
	int ipos = -1;
	unsigned char* tbl = KMPFailureFunc_Forward_NoCaps(p,m);

	while(i<n)
	{
		if(upper(p[j])==upper(t[i]))
		{
			if(j==m-1)
			{
				ipos = i-m+1;
				goto RETURN;
			}
			i++;
			j++;
		}
		else if(j>0)
		{
			j = tbl[j-1];
		}
		else
		{
			i++;
		}
	}
RETURN:
	free(tbl);
	return ipos;
}


//backward search
static unsigned char * KMPFailureFunc_Backward_NoCaps(const char* p,int m)
{
	int i=m-2,j=m-1;
	unsigned char* tbl = (unsigned char*)malloc(m);
	tbl[j] = m-1;

	while(i>=0)
	{
		if(upper(p[j])==upper(p[i]))
		{
			tbl[i] = m-j-1;
			i--;
			j--;
		}
		else if(j<m-1)
		{
			j = tbl[j+1];
		}
		else
		{
			tbl[i]=m-1;
			i--;
		}
	}
	return tbl;
}

static int KMPMatch_Backward_NoCaps(const char* t,int n,const char* p,int m)
{
	int i=n-1;
	int j=m-1;
	int ipos = -1;
	unsigned char* tbl = KMPFailureFunc_Backward_NoCaps(p,m);

	while(i>=0)
	{
		if(upper(p[j])==upper(t[i]))
		{
			if(j==0)
			{
				ipos = i;
				goto RETURN;
			}
			i--;
			j--;
		}
		else if(j<m-1)
		{
			j = tbl[j+1];
		}
		else
		{
			i--;
		}
	}
RETURN:
	free(tbl);
	return ipos;
}

