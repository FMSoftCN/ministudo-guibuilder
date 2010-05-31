/*
 * file mgwnd.h - The Window Class and Menu Class of MGFC declear
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
#ifndef MINIGUI_WND_H
#define MINIGUI_WND_H

#include "mgcomm.h"

/*
 * Load control into hWnd from Array of pCtrlData
 */
int MGLoadControls(HWND hWnd,PCTRLDATA pCtrlData,int controls);

/*
 * Class MGWnd, the base class of window
 */
class MGWnd
{
public:
	MGWnd()
	{
		m_hWnd = HWND_INVALID;
	}
	MGWnd(HWND hWnd)
	{
		m_hWnd = HWND_INVALID;
		Attach(hWnd);
	}
	virtual ~MGWnd(){
		if(IsWindow())
			Detach();
	}


	inline int SendMessage(int iMsg, WPARAM wParam=0, LPARAM lParam=0)
	{
		return ::SendMessage(m_hWnd,iMsg,wParam,lParam);
	}
	inline int PostMessage(int iMsg, WPARAM wParam=0, LPARAM lParam=0)
	{
		return ::PostMessage(m_hWnd,iMsg,wParam,lParam);
	}

	inline operator HWND ()
	{
		return m_hWnd;
	}

	inline HWND GetHandle(){ return m_hWnd; }

	inline HWND Attach(HWND hWndNew)
	{
		HWND hWnd = m_hWnd;
		if(IsWindow())
		{
			SetWindowAdditionalData(hWnd,0);
		}

		m_hWnd = hWndNew;

		if(IsWindow())
		{
			SetWindowAdditionalData(m_hWnd,(DWORD)this);
		}
		return hWnd;
	}

	//attention: hWnd Must Create By MGFC class or atteched by BGFC class
	static MGWnd *WndFromHandle(HWND hWnd);

	inline BOOL IsWindow()
	{
		return ::IsWindow(m_hWnd);
	}

	inline BOOL IsMainWindow()
	{
		return ::IsMainWindow(m_hWnd);
	}
	inline BOOL IsVisible()
	{
		return ::IsWindowVisible(m_hWnd);
	}

	inline HWND Detach()
	{
		HWND hWnd = m_hWnd;
		if(IsWindow())
		{
			SetWindowAdditionalData(m_hWnd,0);
		}
		m_hWnd = HWND_INVALID;
		return hWnd;
	}

	inline int GetWindowText(char* txtBuf,int nMaxLen)
	{
		return SendMessage(MSG_GETTEXT, (WPARAM)nMaxLen, (LPARAM)txtBuf);
	}
	inline int GetWindowTextLength()
	{
		return SendMessage(MSG_GETTEXTLENGTH);
	}
	/*
	 * Function GetWindowText
	 *   Alloc Memory to store the window text. use "delete" to free
	 */
	inline char* GetWindowText(int *ptxtlen=NULL)
	{
		int   txtLength;
		char *txt;

		txtLength = GetWindowTextLength();
		if(ptxtlen) *ptxtlen = txtLength;
		if(txtLength>0)
		{
			txt = new char[txtLength+1];
			if(!txt)
				return NULL;
			GetWindowText(txt,txtLength);
			return txt;
		}
		return NULL;
	}

	inline BOOL SetWindowText(const char* txt)
	{
		return (SendMessage(MSG_SETTEXT, 0 ,(LPARAM)txt) == 0);
	}
	inline BOOL MoveWindow(int x,int y,int w,int h,BOOL fPaint=TRUE)
	{
		return ::MoveWindow(m_hWnd,x,y,w,h,fPaint);
	}
	inline BOOL MoveWindow(const RECT *prt,BOOL fPaint=TRUE)
	{
		if(!prt) return FALSE;
		return ::MoveWindow(m_hWnd,
                        prt->left, prt->top, RECTWP(prt), RECTHP(prt), fPaint);
	}
	inline const char* GetCaption()
	{
		return GetWindowCaption(m_hWnd);
	}
	inline BOOL SetCaption(const char* spCaption)
	{
		return SetWindowCaption(m_hWnd,spCaption);
	}

	inline int GetID(){return GetDlgCtrlID(m_hWnd);}

	inline int LoadControls(PCTRLDATA pctrlData,int controls){ return ::MGLoadControls(m_hWnd,pctrlData,controls); }

	inline HWND GetChild(int nItemID){ return GetDlgItem(m_hWnd,nItemID);}

	inline int  GetChildInt(int nItemID) { return (int)GetDlgItemInt(m_hWnd,nItemID,NULL,TRUE); }
	inline UINT GetChildUINT(int nItemID) { return (UINT)GetDlgItemInt(m_hWnd,nItemID,NULL,FALSE); }
	inline BOOL SetChildInt(int nItemID,int iValue) { return SetDlgItemInt(m_hWnd,nItemID,(UINT)iValue,TRUE);}
	inline BOOL SetChildInt(int nItemID,UINT uValue) { return SetDlgItemInt(m_hWnd,nItemID,uValue,FALSE); }

	inline int GetChildText(int nItemID,char *txtBuf,int txtMax) { return GetDlgItemText(m_hWnd,nItemID,txtBuf,txtMax); }
	inline char* GetChildText(int nItemID,int *plen) { return GetDlgItemText2(m_hWnd,nItemID,plen); }
	inline BOOL SetChildText(int nItemID,const char* txt) { return SetDlgItemText(m_hWnd,nItemID,txt); }

	inline BOOL IsChildEnable(int nItemID)	{ return ::IsWindowEnabled(GetChild(nItemID)); }
	inline BOOL EnableChild(int nItemID,BOOL fEnable=TRUE){ return ::EnableWindow(GetChild(nItemID),fEnable); }

	inline BOOL IsChildVisible(int nItemID){ return ::IsWindowVisible(GetChild(nItemID)); }
	inline BOOL VisibleChild(int nItemID,BOOL fVisible=TRUE)
	{
		return ::ShowWindow(GetChild(nItemID),fVisible?SW_SHOWNORMAL:SW_HIDE);
	}

	inline HWND GetNextChild(HWND hchild=(HWND)0){ return ::GetNextChild(m_hWnd, hchild); }
	inline HWND GetFirstChild() { return GetNextChild(); }
	inline HWND GetNextSlibWnd(){
		if(!IsMainWindow())
			return ::GetNextChild(GetParent(), m_hWnd);
		return HWND_INVALID;
	}

	inline HWND GetParent(){ return ::GetParent(m_hWnd);}

	inline HWND GetNextTabItem(HWND hWndCtrl,BOOL bPrevious=FALSE){ return GetNextDlgTabItem(m_hWnd,hWndCtrl,bPrevious); }

	inline BOOL ShowWindow(int iCmdShow=SW_SHOWNORMAL)
	{
		return ::ShowWindow(m_hWnd,iCmdShow);
	}

	inline void DestroyWindow(){
		if(IsWindow()){
			if(IsMainWindow())
				::DestroyMainWindow(m_hWnd);
			else
				::DestroyWindow(m_hWnd);
		}
	}

	inline void Destroy()
	{
		if(IsWindow())
		{
			SetWindowAdditionalData(m_hWnd, 0);
			m_hWnd = HWND_INVALID;
		}
	}

	inline HWND Focus(){ return SetFocus(m_hWnd);}

	inline void UpdateWindow(BOOL bErase=TRUE){ return ::UpdateWindow(m_hWnd,bErase); }

	inline BOOL EnableWindow(BOOL fEnable=TRUE){ return ::EnableWindow(m_hWnd,fEnable); }

	inline BOOL IsWindowEnabled(){ return ::IsWindowEnabled(m_hWnd); }

	inline BOOL GetWindowRect(PRECT prcWnd){ return ::GetWindowRect(m_hWnd,prcWnd); }

	inline int GetWindowBkColor(){ return ::GetWindowBkColor(m_hWnd); }

	inline int SetWindowBkColor(int new_bkcolor){ return ::SetWindowBkColor(m_hWnd,new_bkcolor); }

	inline PLOGFONT GetWindowFont(){ return ::GetWindowFont(m_hWnd); }

	inline PLOGFONT SetWindowFont(PLOGFONT new_font){ return ::SetWindowFont(m_hWnd,new_font); }

	inline HCURSOR GetWindowCursor(){ return ::GetWindowCursor(m_hWnd); }

	inline HCURSOR SetWindowCursor(HCURSOR hNewCursor){ return ::SetWindowCursor(m_hWnd,hNewCursor); }

	inline DWORD GetWindowStyle(){ return ::GetWindowStyle(m_hWnd); }

	inline DWORD GetWindowExStyle(){ return ::GetWindowExStyle(m_hWnd); }

	inline BOOL InvalidateRect(PRECT pclip=NULL,BOOL bEraseBkgnd=TRUE) {
		return ::InvalidateRect(m_hWnd,pclip,bEraseBkgnd); }

	inline BOOL GetUpdateRect(PRECT prc){ return ::GetUpdateRect(m_hWnd,prc); }

	inline void ClientToScreen(int *x,int *y){ return ::ClientToScreen(m_hWnd,x,y); }

	inline void ClientToScreen(PPOINT ppoint){ return ::ClientToScreen(m_hWnd,&ppoint->x,&ppoint->y); }

	inline void ScreenToClient(int *x,int *y){ return ::ScreenToClient(m_hWnd,x,y); }

	inline void ScreenToClient(PPOINT ppoint){ return ::ScreenToClient(m_hWnd,&ppoint->x,&ppoint->y); }

	inline  void ScrollWindow(int xoffset,int yoffset,const PRECT prc1=NULL,const PRECT prc2=NULL) {
		::ScrollWindow(m_hWnd,xoffset,yoffset,prc1,prc2);
	}

	inline BOOL GetClientRect(PRECT prc){ return ::GetClientRect(m_hWnd,prc); }


	inline HDC  BeginPaint(){ return ::BeginPaint(m_hWnd); }
	inline void EndPaint(HDC hdc){ return ::EndPaint(m_hWnd,hdc); }

	inline HDC GetClientDC(){ return ::GetClientDC(m_hWnd); }
	inline HDC GetWindowDC(){ return ::GetDC(m_hWnd); }
	//inline void ReleaseDC(HDC hdc){ ::ReleaseDC(hdc); }

	inline BOOL CreateCaret(PBITMAP pBitmap,int width,int height){ return ::CreateCaret(m_hWnd,pBitmap,width,height); }
	inline BOOL DestroyCaret(){ return ::DestroyCaret(m_hWnd); }
	inline BOOL ShowCaret(){ return ::ShowCaret(m_hWnd); }
	inline BOOL HideCaret(){ return ::HideCaret(m_hWnd); }

	/*
	 * Function GetParentAlways
	 *    Try to get parent window as possible.
	 */
	inline HWND GetParentAlways(){
		if(IsWindow())
		{
			HWND hWnd;
			hWnd = GetParent();
			if(hWnd==HWND_INVALID || hWnd==(HWND)NULL)
			{
				hWnd = ::GetHosting(m_hWnd);
				if(hWnd==HWND_INVALID)
					return HWND_DESKTOP;
			}
			return hWnd;
		}
		return HWND_DESKTOP;
	}

	/*
	 * Function GenterWindow
	 *    set this window into center of his parent
	 */
	inline void CenterWindow(BOOL fPaint=FALSE)
	{
		HWND hParent;
		RECT rcParent,rcSelf;
		int x,y;

		if(IsMainWindow())
			hParent = HWND_DESKTOP;
		else
			hParent = GetParent();

		::GetClientRect(hParent, &rcParent);
		GetWindowRect(&rcSelf);
		x = (rcParent.right + rcParent.left - rcSelf.right + rcSelf.left)/2;
		y = (rcParent.bottom + rcParent.top - rcSelf.bottom + rcSelf.top)/2;
		if(x<0) x = 0;
		if(y<0) y = 0;
		MoveWindow(x, y, RECTW(rcSelf), RECTH(rcSelf), fPaint);
	}

	inline BOOL SetCaretPos(int x,int y){ return ::SetCaretPos(m_hWnd,x,y); }
	inline BOOL SetCaretPos(POINT pt){ return ::SetCaretPos(m_hWnd,pt.x,pt.y); }
	inline BOOL GetCaretPos(int *px,int *py)
	{
		POINT pt;
		BOOL b = ::GetCaretPos(m_hWnd,&pt);
		if(b)
		{
			if(px) *px = pt.x;
			if(py) *py = pt.y;
		}
		return b;
	}
	inline BOOL GetCaretPos(PPOINT ppt){ return ::GetCaretPos(m_hWnd,ppt); }
	inline UINT GetCaretBlinkTime(){ return ::GetCaretBlinkTime(m_hWnd); }
	inline BOOL SetCaretBlinkTime(UINT uTime){ return ::SetCaretBlinkTime(m_hWnd,uTime); }

	inline void CheckRadioButton(int idFirstButton, int idLastButton, int idCheckButton)
	{
		::CheckRadioButton(m_hWnd,idFirstButton,idLastButton,idCheckButton);
	}

	inline int IsDlgButtonChecked(int idButton)
	{
		return ::IsDlgButtonChecked(m_hWnd,idButton);
	}

	inline void CheckDlgButton(int nItemID,int nCheck){ return ::CheckDlgButton(m_hWnd,nItemID,nCheck); }
	inline void SetDlgItemCheck(int nItemID) { CheckDlgButton(nItemID, BST_CHECKED); }
	inline void ClearDlgItemCheck(int nItemID) { CheckDlgButton(nItemID, BST_UNCHECKED); }
	inline void MyCheckRadioButton(int idFirstButton, int idLastButton, int idCheckButton)
	{
		for(int i=idFirstButton;i<=idLastButton;i++)
		{
			if(i==idCheckButton)
				SetDlgItemCheck(i);
			else
				ClearDlgItemCheck(i);
		}
	}

	inline void SetChildFocus(int nItemID){ SetFocus(GetChild(nItemID)); }

	inline int MessageBox(const char* pszCaption=NULL,const char* pszText=NULL,DWORD dwStyle = MB_OK|MB_ICONINFORMATION)
	{
		return ::MessageBox(m_hWnd,pszText,pszCaption,dwStyle);
	}

	inline int MessageBox(const char* pszCaption,DWORD dwStyle,const char* format,...)
	{
		va_list varg;
		char    szText[MAX_TEXT_BUF_SIZE];
		va_start(varg,format);
		vsprintf(szText,format,varg);
		return MessageBox(pszCaption,szText,dwStyle);
	}

	inline int InfoBox(const char* pszCaption,const char* format,...)
	{
		va_list varg;
		char    szText[MAX_TEXT_BUF_SIZE];
		va_start(varg,format);
		vsprintf(szText,format,varg);
		//printf("%s\n",szText);
		return MessageBox(pszCaption,szText,MB_OK|MB_ICONINFORMATION);
	}

	inline int YesNoBox(const char* pszCaption,const char* format,...)
	{
		va_list varg;
		char    szText[MAX_TEXT_BUF_SIZE];
		va_start(varg,format);
		vsprintf(szText,format,varg);
		return MessageBox(pszCaption,szText,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON1);
	}

	inline int YesNoCancelBox(const char* pszCaption,const char* format,...)
	{
		va_list varg;
		char    szText[MAX_TEXT_BUF_SIZE];
		va_start(varg,format);
		vsprintf(szText,format,varg);
		return MessageBox(pszCaption,szText,MB_YESNOCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1);
	}
	inline int OKCancelBox(const char* pszCaption,const char* format,...)
	{
		va_list varg;
		char    szText[MAX_TEXT_BUF_SIZE];
		va_start(varg,format);
		vsprintf(szText,format,varg);
		return MessageBox(pszCaption,szText,MB_OKCANCEL|MB_ICONASTERISK|MB_DEFBUTTON1);
	}

	//timer
	inline BOOL SetTimer(int id,unsigned int speed){ return ::SetTimer(m_hWnd,id,speed); }
	inline void KillTimer(int id){ ::KillTimer(m_hWnd,id); }

	//scroll control
	inline BOOL EnableScrollBar(int iSBar=SB_HORZ,BOOL bEnable=TRUE)
	{
		return ::EnableScrollBar(m_hWnd,iSBar,bEnable);
	}
	inline BOOL SetScrollInfo(int iSBar,const SCROLLINFO * lpsi, BOOL fRedraw=TRUE)
	{
		return ::SetScrollInfo(m_hWnd,iSBar,lpsi,fRedraw);
	}
	inline BOOL GetScrollInfo(int iSBar, PSCROLLINFO lpsi)
	{
		return ::GetScrollInfo(m_hWnd,iSBar,lpsi);
	}
	inline BOOL GetScrollPos(int iSBar,int *pPos)
	{
		return ::GetScrollPos(m_hWnd,iSBar,pPos);
	}
	inline int GetScrollPos(int iSBar)
	{
		int pos=0;
		return GetScrollPos(iSBar,&pos)?pos:0;
	}
	inline BOOL ShowScrollBar(int iSBar=SB_HORZ, BOOL bShow=TRUE)
	{
		return ::ShowScrollBar(m_hWnd,iSBar,bShow);
	}
	inline BOOL SetScrollRange(int iSBar, int iMinPos, int iMaxPos)
	{
		return ::SetScrollRange(m_hWnd,iSBar,iMinPos,iMaxPos);
	}
	inline BOOL SetScrollPos(int iSBar, int iNewPos)
	{
		return ::SetScrollPos(m_hWnd,iSBar,iNewPos);
	}
	inline void ScrollWindow(int iOffx, int iOffy,const RECT* rc1=NULL, const RECT* rc2=NULL)
	{
		::ScrollWindow(m_hWnd,iOffx,iOffy,rc1,rc2);
	}

	inline void SetCapure()
	{
		::SetCapture(m_hWnd);
	}
//	inline void ReleaseCapture(void)
//
#if MG_VER >= MG_VER_3_0_x
	inline DWORD GetWindowElementAttr(int we_attr_id){
		return ::GetWindowElementAttr(m_hWnd, we_attr_id);
	}
	inline DWORD SetWindowElementAttr(int we_attr_id, DWORD we_attr){
		return ::SetWindowElementAttr(m_hWnd, we_attr_id, we_attr);
	}
	inline BOOL SetWindowElementRenderer(const char* rdr_name, const WINDOW_ELEMENT_ATTR* we_attrs=NULL){
		return ::SetWindowElementRenderer(m_hWnd, rdr_name, we_attrs);
	}
	inline const WINDOW_ELEMENT_RENDERER* GetWindowElementRenderer(){
		const WINDOWINFO* wininfo = GetWindowInfo(m_hWnd);
		return wininfo?wininfo->we_rdr:NULL;
	}
#endif

protected:
	HWND m_hWnd;
};


/*
 * Class MGMainWnd extend from MGWnd. The Main Window Class
 */
class MGMainWnd:public MGWnd
{
public:
	MGMainWnd();
	~MGMainWnd();
	/*
	 * Create a Main Window for this Class
	 */
	BOOL Create(DWORD dwStyle,
					const char* spCaption,
					int x,int y,int rx,int by,
					HWND host=HWND_DESKTOP,
					HCURSOR hCursor=(HCURSOR)0,
					HMENU hMenu=(HMENU)0,
					HICON hIcon=(HICON)0,
					int iBackColor=COLOR_lightwhite,
					DWORD dwStyleEx=WS_EX_NONE);

	BOOL Create(HWND hWndParent,PDLGTEMPLATE pDlgTemplate, LPARAM lParam=0);

	inline HWND GetHosting(){return ::GetHosting(m_hWnd);}

	/*
	 * Show this window as a Dialog Mode
	 */
	DWORD DoMode(/*HWND hWndOwner=HWND_INVALID*/);

	/*
	 * Exit form Dialog Mode
	 */
	BOOL EndDialog(DWORD dwEndRet){ return ::EndDialog(m_hWnd,dwEndRet); }

	void Destroy();

	inline HMENU GetSystemMenu(){ return ::GetSystemMenu(m_hWnd,TRUE); }

	inline HMENU GetMenu(){ return ::GetMenu(m_hWnd); }

	inline HMENU SetMenu(HMENU hMenu){ return ::SetMenu(m_hWnd,hMenu); }

	inline int PostQuitMessage(){ return ::PostQuitMessage(m_hWnd); }

protected:
	/*
	 * Function WndProc
	 *       The Message process procude. overwrite by extend classes
	 * Parameter:
	 *       int iMsg: the message type
	 *       WPARAM wParam: the First param of message
	 *       LPARAM lParam: the long param of message
	 *       int *pret: the message return val.
	 * return
	 *       BOOL. if is TRUE, the *pret will be return to the caller.
	 *         FALSE, the *pret value is ignored.
	 */
	virtual BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret)=0;
	virtual void OnClose(){
		Destroy();
		PostQuitMessage();
	}

private:
	static int _mainWndProc(HWND hWnd,int iMsg,WPARAM wParam,LPARAM lParam);
};

/*
 * Class MGDialog extend from MGMainWnd. The Dialog Class
 */
class MGDialog:public MGMainWnd
{
public:
	MGDialog();
	~MGDialog();

protected:
	virtual BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret)=0;
};

#define RETURN(ret) do{if(pret)*pret=ret; return TRUE;}while(0)

/*
 * The Message MAP Marcos for WndProc
 * These the MAPs can be used in MGMainWnd, MGCtrl
 */

/*
 * Declear message mapping in extend class of MGWnd.
 */
#define DECLARE_MSG_MAP protected: BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret)

/*
 * begin message mapping. write into the implement files
 */
#define BEGIN_MSG_MAP(BaseClass) \
	BOOL BaseClass::WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret) { \
		switch(iMsg) {

#define END_MSG_MAP  } return FALSE; }

#define END_MSG_MAP_PARENT_PROC(ParentCls)    } return ParentCls::WndProc(iMsg, wParam, lParam, pret); }

//this marco support a hook proc to get all of message
// BOOL HookProc(int message, WPARAM, LPARAM, int *prt)
#define HOOK_MSG(HookProc)  }  \
	if(HookProc(iMsg, wParam, lParam, pret)) return TRUE; \
	switch(iMsg) {

//this marco only used by main window
//BOOL OnMainCreate(PMAINWINCREATE pcreate_info);
#define MAP_MAINCREATE(OnMainCreate) case MSG_CREATE: \
	{ \
		if(!OnMainCreate ((PMAINWINCREATE)lParam)) RETURN(-1); \
		break; \
	}
//this marco only used by control window
//BOOL OnControlCreate(HWND hParent,DWORD dwAddData);
#define MAP_CONTROLCREATE(OnControlCreate) case MSG_CREATE: \
	{ \
		if(!OnControlCreate((HWND)wParam,(DWORD)lParam)) RETURN(-1); \
		break; \
	}

//BOOL OnNcCreate(MAINWINCREATE *pCreateInfo);
#define MAP_NCCREATE(OnNcCreate) case MSG_NCCREATE: \
	{ \
		if(!OnNcCreate ((MAINWINCREATE*)lParam)) RETURN(-1); \
		break; \
	}
//BOOL OnInitDialog(HWND focus_hwnd,LPARAM lParam)
#define MAP_INITDIALOG(OnInitDialog) case MSG_INITDIALOG: \
	{ \
		if(OnInitDialog((HWND)wParam,lParam)) \
			RETURN(1); \
		else \
			RETURN(0); \
	}

//void OnSetFocus()
#define MAP_SETFOCUS(OnSetFocus) case MSG_SETFOCUS: \
	{ \
		OnSetFocus(); \
		break; \
	}

//void OnKillFocus()
#define MAP_KILLFOCUS(OnKillFocus) case MSG_KILLFOCUS: \
	{ \
		OnKillFocus(); \
		break; \
	}

//void OnActive()
#define MAP_ACTIVE(OnActive)  case MSG_ACTIVE: \
	{ \
		OnActive(); \
		break; \
	}

//void OnSizeChanging(PRECT prtIn,PRECT prtOut);
#define MAP_SIZECHANGING(OnSizeChanging) case MSG_SIZECHANGING: \
	{ \
		OnSizeChanging((PRECT)wParam,(PRECT)lParam); \
		RETURN(0); \
	}

//BOOL OnSizeChanged(PRECT prcWin,PRECT prcClient); True :Channged, FALSE use default;
#define MAP_SIZECHANGED(OnSizeChanged) case MSG_SIZECHANGED: \
	{ \
		int ret = OnSizeChanged((PRECT)wParam,(PRECT)lParam); \
		RETURN(ret); \
	}

//void OnCSizeChanged(int client_width,int client_height);
#define MAP_CSIZECHANGED(OnCSizeChanged) case MSG_CSIZECHANGED: \
	{ \
		OnCSizeChanged((int)wParam,(int)lParam); \
		break; \
	}

//BOOL OnMainWndCreate(MAINWINCREATE *pCreateInfo);
#define MAP_MAINWNDCREATE(OnMainWndCreate) case MSG_CREATE: \
	{ \
		if(OnMainWndCreate((MAINWINCREATE*)lParam)) RETURN(0); \
		RETURN(1); \
	}
//BOOL OnCtrlCreate(HWND hWndParent,DWORD dwAddData);
#define MAP_CTRLCREATE(OnCtrlCreate) case MSG_CREATE: \
	{ \
		if(OnCtrlCreate((HWND)wParam,(DWORD)lParam)) RETURN(0); \
		return (1); \
	}

//BOOL OnFontChanging(PLOGFONT log_font); TRUE: accept, FALSE refuse
#define MAP_FONTCHANGING(OnFontChanging) case MSG_FONTCHANGING: \
	{ \
		if(OnFontChanging((PLOGFONT)lParam)) RETURN(0); \
		RETURN(1); \
	}
//void OnFontChanged();
#define MAP_FONTCHANGED(OnFontChanged) case MSG_FONTCHANGED: \
	{ \
		OnFontChanged(); \
		RETURN(0); \
	}

//BOOL OnEraseBkgrnd(HDC hdc,PRECT clip);
#define MAP_ERASEBKGND(OnEraseBkgrnd) case MSG_ERASEBKGND: \
	{ \
		if(OnEraseBkgrnd((HDC)wParam,(PRECT)lParam)) RETURN(0); \
		break; \
	}

#ifdef MSG_ERASEBKGND_N
#define MAP_ERASEBKGNDN(OnEraseBkgrnd) case MSG_ERASEBKGND_N: \
	{ \
		if(OnEraseBkgrnd((HDC)wParam,(PRECT)lParam)) RETURN(0); \
		break; \
	}
#else
#define MAP_ERASEBKGNDNN MAP_ERASEBKGND
#endif

//void OnPaint(HDC hdc);
#define MAP_PAINT(OnPaint) case MSG_PAINT: \
	{ \
		HDC hdc = BeginPaint(); \
		OnPaint(hdc); \
		EndPaint(hdc); \
		RETURN(0); \
	}

//void OnClose()
#define MAP_CLOSE(OnClose) case MSG_CLOSE: \
	{ \
		OnClose(); \
		RETURN(0); \
	}

//void OnDestroy
#define MAP_DESTROY(OnDestroy) case MSG_DESTROY: \
	{ \
		OnDestroy(); \
		RETURN(0); \
	}

//BOOL OnKeyDown(int scancode,DWORD key_status); TRUE: user handled
#define MAP_KEYDOWN(OnKeyDown) case MSG_KEYDOWN: \
	{ \
		if(OnKeyDown((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//BOOL OnKeyUp(int scancode,DWORD key_status); TRUE: user handled
#define MAP_KEYUP(OnKeyUp) case MSG_KEYUP: \
	{ \
		if(OnKeyUp((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//BOOL OnChar(int ch,DWORD key_status); TRUE: user handled
#define MAP_CHAR(OnChar)  case MSG_CHAR: \
	{ \
		if(OnChar((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//BOOL OnSysKeyDown(int scancode,DWORD key_status); TRUE: user handled
#define MAP_SYSKEYDOWN(OnSysKeyDown) case MSG_SYSKEYDOWN: \
	{ \
		if(OnSysKeyDown((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//BOOL OnSysKeyUp(int scancode,DWORD key_status); TRUE: user handled
#define MAP_SYSKEYUP(OnSysKeyUp) case MSG_SYSKEYUP: \
	{ \
		if(OnSysKeyUp((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//BOOL OnSysChar(int ch,DWORD key_status); TRUE: user handled
#define MAP_SYSCHAR(OnSysChar)  case MSG_SYSCHAR: \
	{ \
		if(OnSysChar((int)wParam,lParam)) RETURN(0); \
		break; \
	}

//void OnKeyLongPress();
#define MAP_KEYLONGPRESS(OnKeyLongPress) case MSG_KEYLONGPRESS: \
	{ \
		OnKeyLongPress(); \
		RETURN(0); \
	}

//void OnKeyAlwaysPress();
#define MAP_KEYALWAYSPRESS(OnKeyAlwaysPress) case MSG_KEYALWAYSPRESS: \
	{ \
		OnKeyAlwaysPress(); \
		RETURN(0); \
	}

//void OnLButtonDown(int x,int y,DWORD key_status);
#define MAP_LBUTTONDOWN(OnLButtonDown) case MSG_LBUTTONDOWN: \
	{ \
		OnLButtonDown(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnLButtonUp(int x,int y,DWORD key_status);
#define MAP_LBUTTONUP(OnLButtonUp) case MSG_LBUTTONUP: \
	{ \
		OnLButtonUp(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnLButtonDblClk(int x,int y,DWORD key_status);
#define MAP_LBUTTONDBLCLK(OnLButtonDblClk) case MSG_LBUTTONDBLCLK: \
	{ \
		OnLButtonDblClk(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnMouseMove(int x,int y,DWORD key_status);
#define MAP_MOUSEMOVE(OnMouseMove) case MSG_MOUSEMOVE: \
	{ \
		OnMouseMove(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnRButtonDown(int x,int y,DWORD key_status);
#define MAP_RBUTTONDOWN(OnRButtonDown) case MSG_RBUTTONDOWN: \
	{ \
		OnRButtonDown(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnRButtonUp(int x,int y,DWORD key_status);
#define MAP_RBUTTONUP(OnRButtonUp) case MSG_RBUTTONUP: \
	{ \
		OnRButtonUp(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnRButtonDblClk(int x,int y,DWORD key_status);
#define MAP_RBUTTONDBLCLK(OnRButtonDblClk) case MSG_RBUTTONDBLCLK: \
	{ \
		OnRButtonDblClk(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCLButtonDown(int x,int y,DWORD hit_code);
#define MAP_NCLBUTTONDOWN(OnNCLButtonDown) case MSG_NCLBUTTONDOWN: \
	{ \
		OnNCLButtonDown(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCLButtonUp(int x,int y,DWORD hit_code);
#define MAP_NCLBUTTONUP(OnNCLButtonUp) case MSG_NCLBUTTONUP: \
	{ \
		OnNCLButtonUp(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCLButtonDblClk(int x,int y,DWORD hit_code);
#define MAP_NCLBUTTONDBLCLK(OnNCLButtonDblClk) case MSG_NCLBUTTONDBLCLK: \
	{ \
		OnNCLButtonDblClk(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCMouseMove(int x,int y,DWORD hit_code);
#define MAP_NCMOUSEMOVE(OnNCMouseMove) case MSG_NCMOUSEMOVE: \
	{ \
		OnNCMouseMove(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCRButtonDown(int x,int y,DWORD hit_code);
#define MAP_NCRBUTTONDOWN(OnNCRButtonDown) case MSG_NCRBUTTONDOWN: \
	{ \
		OnNCRButtonDown(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); \
		RETURN(0); \
	}

//void OnNCRButtonUp(int x,int y,int hit_code);
#define MAP_NCRBUTTONUP(OnNCRButtonUp) case MSG_NCRBUTTONUP: \
	{ \
		OnNCRButtonUp(LOSWORD(lParam),HISWORD(lParam),(int)wParam); \
		RETURN(0); \
	}

//void OnNCRButtonDblClk(int x,int y,int hit_code);
#define MAP_NCRBUTTONDBLCLK(OnNCRButtonDblClk) case MSG_NCRBUTTONDBLCLK: \
	{ \
		OnNCRButtonDblClk(LOSWORD(lParam),HISWORD(lParam),(int)wParam); \
		RETURN(0); \
	}

//void OnTimer(int time_id);
#define MAP_TIMER(OnTimer) case MSG_TIMER: \
	{ \
		OnTimer((int)wParam); \
		break; \
	}

//int OnHitTest(int x,int y); return : hit code
#define MAP_HITTEST(OnHitTest) case MSG_HITTEST: \
	{ \
		int hit_code = OnHitTest((int)wParam,(int)lParam); \
		RETURN(hit_code); \
	}

//int OnNCHitTest(int x,int y); return : hit code
#define MAP_NCHITTEST(OnNCHitTest) case MSG_NCHITTEST: \
	{ \
		int hit_code = OnNCHitTest((int)wParam,(int)lParam); \
		RETURN(hit_code); \
	}

//void OnHScroll(int hs_nc,int pos);
#define MAP_HSCROLL(OnHScroll) case MSG_HSCROLL: \
	{ \
		OnHScroll((int)wParam,(int)lParam); \
		break; \
	}

//void OnHScroll(int vs_nc,int pos);
#define MAP_VSCROLL(OnVScroll) case MSG_VSCROLL: \
	{ \
		OnVScroll((int)wParam,(int)lParam); \
		break; \
	}

//BOOL OnMessage(WPARAM wParam,LPARAM, int *pret);
#define MAP_MESSAGE(message,OnMessage) case message: \
	{ \
		return OnMessage(wParam,lParam,pret); \
	}
//BOOL OnMessageRange(int iMsg,WPARAM wParam,LPARAM lParam,int *pret);
#define MAP_MESSAGE_RANGE(message_begin,message_end,OnMessageRange)  case message_begin ... message_end: \
	return OnMessageRange(iMsg,wParam,lParam,pret);

/*
 * begin MSG_COMMAND message mapping
 */
#define BEGIN_COMMAND_MAP case MSG_COMMAND: \
	{ int ctrlId = LOWORD(wParam); \
		switch(ctrlId) {

// void OnCmd()
#define MAP_COMMAND(id,OnCmd) case id: \
	OnCmd(); \
	break;
// void OnCmdEx(int code, HWND hWnd)
#define MAP_COMMANDEX(id,OnCmdEx) case id: \
	{ int code = HIWORD(wParam); \
	OnCmdEx(code,(HWND)lParam); }\
	break;
// void OnCmdRang(int id,int code,HWND hWnd)
#define MAP_COMMAND_RANGE(idbegin,idend,OnCmdRange) case idbegin ... idend: \
	OnCmdRange(ctrlId); \
	break;
// void OnCmdRangEx(int id,int code,HWND hWnd)
#define MAP_COMMAND_RANGEEX(idbegin,idend,OnCmdRangeEx) case idbegin ... idend: \
	{ 	\
		int code = HIWORD(wParam); \
		OnCmdRangeEx(ctrlId,code,(HWND)lParam); \
	} \
	break;

//void OnNotify(HWND howner);
#define MAP_NOTIFY(id, notify_msg, OnNotify) case id: \
	{ 	int code = HIWORD(wParam); \
		if(code == notify_msg) \
		OnNotify((HWND)lParam); \
	}\
		break;

#define BEGIN_NOTIFY_MAP(id) case id: \
	{ int code = HIWORD(wParam); \
		switch(code) {

#define MAP_CTRL_NOTIFY(notify_msg, OnNotify) case notify_msg: \
		OnNotify((HWND)lParam); \
		break;

#define END_NOTIFY_MAP } \
		} \
		break;

#define END_COMMAND_MAP  } break;}


/*
 * call supper class' WndProc.
 */
#define CALL_BASE_CLASS_MAPS(BaseClass) default: \
	return BaseClass::WndProc(iMsg,wParam,lParam,pret);


//////////////////////////////////////////////////////////////////////////////////////
// menu
// menu template
typedef struct _pop_menu_template POPMENUTEMPLATE;
typedef POPMENUTEMPLATE * PPOPMENUTEMPLATE;
typedef struct _menu_item_template{
	int   Id;//=-1 mean create a sperator
	UINT  initState;
	const char* strCaption;
	PPOPMENUTEMPLATE subMenu;
}MENUITEMTEMPLATE,*PMUENUITEMTEMPLATE;

struct _pop_menu_template{
	const char* strCaption;
	int  menuItemCnt;
	PMUENUITEMTEMPLATE pMenuItems;
};

typedef struct _menu_bar_template{
	int MenuItemCnt;
	PMUENUITEMTEMPLATE pMenuItems;
}MENUBARTEMPLATE,*PMENUBARTEMPLATE;

/*
 * The Menu class
 */
class MGMenu
{
public:
	MGMenu(){m_hMenu=(HMENU)NULL;}
	MGMenu(HMENU hMenu):m_hMenu(hMenu){}
	~MGMenu(){}

	inline HMENU Attatch(HMENU hMenu){
		HMENU hOldMenu=m_hMenu;
		m_hMenu = hMenu;
		return hOldMenu;
	}

	inline HMENU Detach(){ return Attatch((HMENU)NULL);}

	inline operator HMENU(){ return m_hMenu; }

	inline BOOL CreateMenu(){m_hMenu=::CreateMenu(); return m_hMenu!=(HMENU)NULL;} //Create a Menu bar
	inline BOOL CreatePopupMenu(const char* strCaption,BOOL fStripPopup=FALSE){
		MENUITEMINFO mii;
		memset(&mii,0,sizeof(mii));
		mii.type = MFT_STRING;
		mii.typedata = (DWORD)strCaption;
		m_hMenu = ::CreatePopupMenu(&mii);
		if(fStripPopup && m_hMenu!=(HMENU)NULL)
			m_hMenu = ::StripPopupHead(m_hMenu);
		return m_hMenu!=(HMENU)NULL;
	}

	BOOL LoadMenuBar(PMENUBARTEMPLATE pMenuBarTemplate);
	BOOL LoadPopupMenu(PPOPMENUTEMPLATE pPopMenuTemplate);

	inline BOOL InsertMenuItem(int item,BOOL flag,PMENUITEMINFO pmii){ return ::InsertMenuItem(m_hMenu,item,flag,pmii)==0; }

	inline BOOL InsertMenuItem(int item,int id,const char* strCaption,UINT state=MFS_ENABLED,HMENU hSubmenu=(HMENU)NULL)
	{
		MENUITEMINFO mii;
		mii.id = id;
		mii.typedata = (DWORD)strCaption;
		//mii.cch = strCaption?strlen(strCaption):0;
		mii.cch = 0;
		mii.type = MFT_STRING;
		mii.state = state;
		mii.itemdata = 0;
		mii.checkedbmp = (PBITMAP)NULL;
		mii.uncheckedbmp = (PBITMAP)NULL;
		mii.hsubmenu = (HMENU)hSubmenu;
		mii.mask = MIIM_TYPE|MIIM_STATE;
		if(id>0)
		 	mii.mask |= MIIM_ID;
		if(strCaption)
			mii.mask |= MIIM_STRING;
		if(hSubmenu)
			mii.mask |= MIIM_SUBMENU;
		return InsertMenuItem(item,MF_BYPOSITION,&mii);
	}

	inline BOOL InsertSeparator(int item)
	{
		MENUITEMINFO mii;
		mii.type = MFT_SEPARATOR;
		mii.mask = MIIM_TYPE;
		return InsertMenuItem(item,MF_BYPOSITION,&mii);
	}

	inline int TrackPopupMenu(int x=0,int y=0,HWND hWnd=(HWND)NULL,UINT flag=TPM_LEFTALIGN)
	{
		return ::TrackPopupMenu(m_hMenu,flag,x,y,hWnd);
	}

	inline void StripPopupHead(){ m_hMenu = ::StripPopupHead(m_hMenu); }

	inline int GetMenuItemCount(){ return ::GetMenuItemCount(m_hMenu); }

	inline int GetMenuItemID(int iPos){ return ::GetMenuItemID(m_hMenu,iPos); }

	inline HMENU GetPopupSubMenu(){ return ::GetPopupSubMenu(m_hMenu); }

	inline HMENU GetSubMenu(int iPos){ return ::GetSubMenu(m_hMenu,iPos); }

	inline void EnableMenuItem(int item,BOOL fEnable=TRUE)
	{
		::EnableMenuItem(m_hMenu,item,MF_BYPOSITION|((fEnable)?MFS_ENABLED:MFS_DISABLED));
	}

	inline int GetMenuItemInfo(int item,PMENUITEMINFO pmii,BOOL flag=MF_BYCOMMAND)
	{
		return ::GetMenuItemInfo(m_hMenu,item,flag,pmii);
	}

	inline int DestroyMenu(){ return ::DestroyMenu(m_hMenu); }

	inline int IsMenu(){ return ::IsMenu(m_hMenu); }

	inline int DeleteMenu(int item,BOOL flag=MF_BYCOMMAND){ return ::DeleteMenu(m_hMenu,item,flag); }

	inline int RemoveMenu(int item,BOOL flag=MF_BYCOMMAND){ return ::RemoveMenu(m_hMenu,item,flag); }

	inline int CheckMenuRadioItem(int first,int last,int checkItem,UINT flag=MF_BYCOMMAND)
	{
		return ::CheckMenuRadioItem(m_hMenu,first,last,checkItem,flag);
	}

	inline int SetMenuItemBitmaps(int item,UINT flag=MF_BYCOMMAND,PBITMAP hBmpUnchecked=NULL,PBITMAP hBmpChecked=NULL)
	{
		return ::SetMenuItemBitmaps(m_hMenu,item,flag,hBmpUnchecked,hBmpChecked);
	}

	inline int SetMenuItemInfo(int item,PMENUITEMINFO pmii,UINT flag=MF_BYCOMMAND)
	{
		return ::SetMenuItemInfo(m_hMenu,item,flag,pmii);
	}

protected:
	HMENU m_hMenu;
};

BOOL LoadMenuItem(HMENU hMenu,int iPos,PMUENUITEMTEMPLATE pMenuItemTempl,BOOL fStripPopMenu=FALSE);

HMENU LoadPopMenuItem(PPOPMENUTEMPLATE pPopMenuTemplate,BOOL fStripPopMenu=FALSE);

//scroll window support
#define THIS  (static_cast<TMGWnd*>(this))
#define TMGSW_HORZ  0x01
#define TMGSW_VERT  0x02
/*
 * The Scroll Support template class
 */

extern BOOL UpdateHVScroll(HWND hwnd,
	const RECT* rtdoc,
	const RECT* rtview,
	int *x_offset,
	int *y_offset,
	BOOL bAutoUpdate);

extern int ProcessScrollMessage(HWND hwnd, int iSBar, int nc,int pos, int line, BOOL bRedraw);


template<class TMGWnd>
class TMGScrollWnd
{
public:
	/*
	 * class TMGWnd must implent UpdateScroll.
	 * this function will called when use act the scroll bar
	 */
	//void UpdateScrollWnd(int newXPos,int newYPos,DWORD dwFlag);


	/*
	 * Function UpdateScroll
	 *       Reset the Scroll infomation.
	 *       Call this function when your scrolled document size changed.
	 * Param
	 *       int iSBar SB_HORZ SB_VRET
	 *       int view_size: the size of total document
	 *       int client_size: the size of view port
	 *       BOOL bRedraw
	 * Return
	 *       BOOL TRUE: show scrollbar; FALSE hide scrollbar
	 */
	BOOL UpdateHVScroll(const RECT* rtdoc, const RECT* rtview=NULL, int *x_offset=NULL, int *y_offset=NULL, BOOL bAutoUpdate=TRUE)
	{
		return ::UpdateHVScroll(THIS->GetHandle(),
			rtdoc,
			rtview,
			x_offset,
			y_offset,
			bAutoUpdate);

	}

	/*
	 * Function OnScroll
	 *     process scroll message automately
	 *     call this function in message MSG_HSCROLL and MSG_VSCROLL
	 */
	void OnScroll(int iSBar,int nc,int pos,int line=1)
	{
		int newpos = ProcessScrollMessage(THIS->GetHandle(),iSBar,nc,pos,line,TRUE);
		if(newpos<0) // do not scroll
			return;
		if(iSBar==SB_HORZ)
			THIS->UpdateScrollWnd(newpos,0,TMGSW_HORZ);
		else
			THIS->UpdateScrollWnd(0,newpos,TMGSW_VERT);
	}
};

#endif

