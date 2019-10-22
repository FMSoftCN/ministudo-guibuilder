/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MGCTRL_H
#define MGCTRL_H

#include "mgcomm.h"
#include "mgwnd.h"

/*
 The subclass template Class
*/

/*
 this template is used for the static subclass.
 that mean's all instance of the class only subclass
 on uniqu class of window.
*/
#define MGSUBCLASS_HWND  (HWND)(*(static_cast<TMGCtrlWnd*>(this)))

template<class TMGCtrlWnd>
class TMGStaticSubclass:public TMGCtrlWnd
{
public:
	BOOL Subclass()
	{
		// m_hWnd from the TMGCtrlWnd
		WNDPROC oldProc;
		oldProc = SetWindowCallbackProc(MGSUBCLASS_HWND,_newCtrlProc);
		if(oldProc==NULL)
			return FALSE;
		if(_oldProc==NULL)
			_oldProc = oldProc;
		return TRUE;
	}
protected:
	static WNDPROC _oldProc;

	inline LRESULT CallOldProc(UINT iMsg,WPARAM wParam,LPARAM lParam){
		if(_oldProc)
			return (*_oldProc)(MGSUBCLASS_HWND,iMsg,wParam,lParam);
		return 0;
	}

	virtual BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)=0;
private:
	static LRESULT _newCtrlProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
	{
		TMGStaticSubclass<TMGCtrlWnd> *pThis = (TMGStaticSubclass<TMGCtrlWnd>*)
		GetWindowAdditionalData(hWnd);
		if(pThis)
		{
			int ret=0;
			if(iMsg == MSG_DESTROY){
				::SetWindowAdditionalData(hWnd, 0);
				if(_oldProc){
					SetWindowCallbackProc(hWnd, _oldProc);
					return (*_oldProc)(hWnd,iMsg,wParam,lParam);
				}
				return DefaultControlProc(hWnd,iMsg,wParam,lParam);
			}
			else
			{
				if(pThis->WndProc(iMsg,wParam,lParam,&ret))
					return ret;
			}
		}

		if(_oldProc)
			return (*_oldProc)(hWnd,iMsg,wParam,lParam);
		else
			return DefaultControlProc(hWnd,iMsg,wParam,lParam);
	}
};

template<class TMGCtrlWnd>
WNDPROC TMGStaticSubclass<TMGCtrlWnd>::_oldProc = NULL;
/*
 this template is used for the dnaynic subclass.
 That mean's every instance has it's own type of window
*/
template<class TMGCtrlWnd>
class TMGDynanicSubclass:public TMGCtrlWnd
{
public:
	BOOL Subclass()
	{
		m_oldProc = SetWindowCallbackProc(MGSUBCLASS_HWND, _newCtrlProc);
		return m_oldProc!=NULL;
	}
protected:
	WNDPROC m_oldProc;
	virtual BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)=0;
	inline LRESULT CallOldProc(UINT iMsg,WPARAM wParam,LPARAM lParam)
		{ if(m_oldProc) return (*m_oldProc)(MGSUBCLASS_HWND,iMsg,wParam,lParam); return 0; }
private:
	static LRESULT _newCtrlProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
	{
		TMGDynanicSubclass<TMGCtrlWnd> *pThis = (TMGDynanicSubclass<TMGCtrlWnd>*)
			GetWindowAdditionalData(hWnd);
		if(pThis)
		{
			int ret = 0;
			if(pThis->WndProc(iMsg,wParam,lParam,&ret))
				return ret;
			if(pThis->m_oldProc)
				return (pThis->m_oldProc)(hWnd,iMsg,wParam,lParam);
		}
		return DefaultControlProc(hWnd,iMsg,wParam,lParam);
	}
};

class MGCtrlNotification
{
public:
	virtual ~MGCtrlNotification(){}

	virtual void OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data) = 0;
};

class MGCtrlWnd:public MGWnd
{
protected:
	MGCtrlNotification * m_notification;
	static void _ctrl_notification(HWND hWnd, int id, int code, DWORD add_data){
		MGCtrlWnd* _this = (MGCtrlWnd*)WndFromHandle(hWnd);
		if(_this && _this->m_notification)
			_this->m_notification->OnCtrlNotified(_this,id, code, add_data);
	}
public:
	MGCtrlWnd(HWND hWnd):MGWnd(hWnd), m_notification(NULL){}
	MGCtrlWnd(HWND hParent,int Id){
		Attach(hParent,Id);
		m_notification = NULL;
	}
	MGCtrlWnd():MGWnd(), m_notification(NULL){}
	~MGCtrlWnd(){}

	MGCtrlNotification * SetNotification(MGCtrlNotification* notification){
		MGCtrlNotification * old_notif = m_notification;
		m_notification = notification;
		if(IsWindow())
			::SetNotificationCallback(m_hWnd,(NOTIFPROC)_ctrl_notification);
		return old_notif;
	}

	void Attach(HWND hParent,int Id){
		m_hWnd = HWND_INVALID;
		HWND hWnd = GetDlgItem(hParent,Id);
		MGWnd::Attach(hWnd);
		if(m_notification)
			::SetNotificationCallback(m_hWnd,(NOTIFPROC)_ctrl_notification);
	}

	virtual const char *GetClass(){return "Control Base Classes";}

	inline BOOL Create(const char* strCaption,HWND hWndParent,int x,int y,int w,int h,DWORD dwStyle,DWORD dwStylEx=0,int Id=-1,DWORD dwAddData=0)
	{
		m_hWnd = CreateWindowEx(GetClass(),strCaption?strCaption:"",dwStyle|WS_CHILD,dwStylEx,Id,x,y,w,h,hWndParent,dwAddData);
		if(IsWindow())
		{
			SetWindowAdditionalData(m_hWnd, (DWORD)this);
			if(m_notification)
				::SetNotificationCallback(m_hWnd,(NOTIFPROC)_ctrl_notification);
			return TRUE;
		}
		return FALSE;
	}
	inline BOOL Create(HWND hWndParent,int x,int y,int w,int h,DWORD dwStyle,DWORD dwStylEx=0,int Id=-1)
	{
		m_hWnd = CreateWindowEx(GetClass(),"",dwStyle|WS_CHILD,dwStylEx,Id,x,y,w,h,hWndParent,(DWORD)this);
		if(IsWindow()){
			::SetNotificationCallback(m_hWnd,(NOTIFPROC)_ctrl_notification);
			return TRUE;
		}
		return FALSE;
	}
	inline int GetId() {return GetDlgCtrlID(m_hWnd); }
};
#define DECLARE_CTRL_CLASS(className)  public: const char* GetClass(){return (className);}

class MGUserCtrl:public MGCtrlWnd
{
public:
	static BOOL Register(const char* spClass,
		DWORD opMask,
		DWORD dwStyle=WS_CHILD,
		DWORD dwStyleEx=WS_EX_NONE,
		HCURSOR hCursor=(HCURSOR)0,
		int iBkColor=COLOR_lightgray);
	MGUserCtrl():MGCtrlWnd(){}
	MGUserCtrl(HWND hWnd){Attach(hWnd); }
	MGUserCtrl(HWND hParent,int Id){ Attach(hParent,Id); }

	~MGUserCtrl(){}

	inline HWND Attach(HWND hWnd)
	{
		HWND hwndOld = m_hWnd;
		if(::IsWindow(hWnd))
		{
			hwndOld = MGWnd::Attach(hWnd);
			SetWindowCallbackProc(hWnd, MGUserCtrl::_WndProc);
		}
		return hwndOld;
	}

	inline HWND Attach(HWND hParent,int Id)
	{
		HWND hwnd = ::GetDlgItem(hParent,Id);
		return Attach(hwnd);
	}

	inline HWND Detach()
	{
		SetWindowCallbackProc(m_hWnd,DefaultControlProc);
		return MGWnd::Detach();
	}

protected:
	virtual BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)=0;

//private:
	static LRESULT _WndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
};


class MGStatic:public MGCtrlWnd
{
public:
	MGStatic(HWND hWnd):MGCtrlWnd(hWnd){}
	MGStatic(){}
	MGStatic(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	~MGStatic(){}

	inline HICON GetIcon(){ return (HICON)SendMessage(STM_GETIMAGE); }
	inline HICON SetIcon(HICON hIcon){ return (HICON)SendMessage(STM_SETIMAGE,(WPARAM)hIcon); }
	inline PBITMAP GetBitmap(){ return (PBITMAP)SendMessage(STM_GETIMAGE); }
	inline PBITMAP SetBitmap(PBITMAP pbmp){ return (PBITMAP)SendMessage(STM_SETIMAGE,(WPARAM)pbmp);}

	DECLARE_CTRL_CLASS(CTRL_STATIC)

};

class MGEdit:public MGCtrlWnd
{
public:
	MGEdit(HWND hWnd):MGCtrlWnd(hWnd){}
	MGEdit(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGEdit(){}

	~MGEdit(){}

	DECLARE_CTRL_CLASS(CTRL_TEXTEDIT)

	inline int GetInt(){
		char szText[32];
		if(GetWindowText(szText,sizeof(szText))>0)
			return strtol(szText,NULL,0);
		return 0;
	};
	inline UINT GetUInt(){
		char szText[32];
		if(GetWindowText(szText,sizeof(szText))>0)
			return strtoul(szText,NULL,0);
		return 0;
	};

	inline void SetInt(int ival){
		char szText[32];
		sprintf(szText,"%d",ival);
		SetWindowText(szText);
	};
	inline void SetInt(UINT uval){
		char szText[32];
		sprintf(szText,"%u",uval);
		SetWindowText(szText);
	};

	inline int GetSelText(char *buf,int nMax)
	{
		return (int)SendMessage(EM_GETSEL,(WPARAM)nMax,(LPARAM)buf);
	}

	inline int GetCaretPos(int *pline_pos,int *pchar_pos)
	{
		return (int)SendMessage(EM_GETCARETPOS,(WPARAM)pline_pos,(LPARAM)pchar_pos);
	}
	inline int SetCaretPos(int line_pos,int char_pos)
	{
		return (int)SendMessage(EM_SETCARETPOS,(WPARAM)line_pos,(LPARAM)char_pos);
	}

	inline int SetSel(int line_pos,int char_pos){
		return (int)SendMessage(EM_SETSEL,(WPARAM)line_pos,(LPARAM)char_pos);
	}
	inline int SetSel(int line_pos1,int char_pos1,int line_pos2,int char_pos2)
	{
		SetCaretPos(line_pos1,char_pos2);
		return SetSel(line_pos2,char_pos2);
	}

	inline int SelectAll()
	{
		return (int)SendMessage(EM_SELECTALL);
	}

	inline int GetSelPos(int* pline_pos,int *pchar_pos)
	{
		return (int)SendMessage(EM_GETSELPOS,(WPARAM)pline_pos,(LPARAM)pchar_pos);
	}

	inline int Copy()
	{
		return (int)SendMessage(EM_COPYTOCB);
	}
	inline int Cut()
	{
		return (int)SendMessage(EM_CUTTOCB);
	}
	inline int Paste()
	{
		return (int)SendMessage(EM_INSERTCBTEXT);
	}

	inline void SetLineSperatorCharDisp(unsigned char ch)
	{
		SendMessage(EM_SETLFDISPCHAR,0,(LPARAM)ch);
	}
	inline void SetLineSperatorChar(unsigned char ch)
	{
		SendMessage(EM_SETLINESEP,0,(LPARAM)ch);
	}
	inline int GetLineCount()
	{
		return (int)SendMessage(EM_GETLINECOUNT);
	}
	inline int GetLineHeight()
	{
		return (int)SendMessage(EM_GETLINEHEIGHT);
	}
	inline int SetLineHeight(int height)
	{
		return (int)SendMessage(EM_SETLINEHEIGHT,(WPARAM)height);
	}
	inline int LineScroll()
	{
		return (int)SendMessage(EM_LINESCROLL);
	}
	inline int InsertText(const char* text,int len)
	{
		return (int)SendMessage(EM_INSERTTEXT,(WPARAM)len,(LPARAM)text);
	}
	inline int InsertText(const char* text)
	{
		return InsertText(text,strlen(text));
	}
	inline int GetMaxLimit()
	{
		return (int)SendMessage(EM_GETMAXLIMIT);
	}
	inline int SetMaxLimit(int limit)
	{
		return (int)SendMessage(EM_LIMITTEXT,(WPARAM)limit);
	}
	inline void Redo()
	{
		SendMessage(EM_REDO);
	}
	inline void Undo()
	{
		SendMessage(EM_UNDO);
	}
	inline void SetPasswordChar(char ch)
	{
		SendMessage(EM_SETPASSWORDCHAR,(WPARAM)ch);
	}
	inline void SetReadOnly(BOOL ReadOnly=TRUE)
	{
		SendMessage(EM_SETREADONLY,(WPARAM)ReadOnly);
	}
	inline void SetDrawSelFunc(ED_DRAWSEL_FUNC drawSelFunc)
	{
		SendMessage(EM_SETDRAWSELECTFUNC,0,(LPARAM)drawSelFunc);
	}
	inline void SetGetCaretWidthCallback(int(*get_caret_width) (HWND, int))
	{
		SendMessage(EM_SETGETCARETWIDTHFUNC,0,(LPARAM)get_caret_width);
	}
	inline char GetPasswordChar()
	{
		return (char)SendMessage(EM_GETPASSWORDCHAR);
	}
	inline int ChangeCaretSharp(int new_sharp)
	{
		return (int)SendMessage(EM_CHANGECARETSHAPE,(WPARAM)new_sharp);
	}
	inline void RefreshCaret()
	{
		SendMessage(EM_REFRESHCARET);
	}
	inline BOOL EnableCaret(BOOL fEnable=TRUE)
	{
		return (BOOL)SendMessage(EM_ENABLECARET,(WPARAM)fEnable);
	}
	inline void SetTitleText(const char* titleText,int len)
	{
		SendMessage(EM_SETTITLETEXT,(WPARAM)len,(LPARAM)titleText);
	}
	inline void SetTitleText(const char* titleText)
	{
		SetTitleText(titleText,strlen(titleText));
	}
	inline int GetTitleText(char* titleText,int nMax)
	{
		return (int)SendMessage(EM_GETTITLETEXT,(WPARAM)nMax,(LPARAM)titleText);
	}
	inline void SetTipText(const char* tipText,int len)
	{
		SendMessage(EM_SETTIPTEXT,(WPARAM)len,(LPARAM)tipText);
	}
	inline void SetTipText(const char* tipText)
	{
		SetTipText(tipText,strlen(tipText));
	}
	inline int GetTipText(char* tipText,int nMax)
	{
		return (int)SendMessage(EM_GETTIPTEXT,(WPARAM)nMax,(LPARAM)tipText);
	}

	int Find(const char* pFind,BOOL fDown=TRUE,BOOL fCaps=FALSE);

	inline int Replace(const char* pFind,const char* pReplace,BOOL fDown=TRUE,BOOL fCaps=FALSE)
	{
		int ipos = Find(pFind,fDown,fCaps);
		//printf("ipos:%d\n",ipos);
		if(ipos>=0)
		{
			int len = pReplace?strlen(pReplace):0;
			if(len<=0) len ++;
			InsertText(pReplace,len);
		}
		return ipos;
	}
	inline void ReplaceAll(const char* pFind,const char* pReplace,BOOL fCaps=FALSE)
	{
		SetCaretPos(0,0);
		while(Replace(pFind,pReplace,TRUE,fCaps)>=0);
	}
};


class MGButton:public MGCtrlWnd
{
public:
	MGButton():MGCtrlWnd(){}
	MGButton(HWND hWnd):MGCtrlWnd(hWnd){}
	MGButton(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	~MGButton(){}
	DECLARE_CTRL_CLASS(CTRL_BUTTON)

	inline int GetCheck()
	{
		return (int)SendMessage(BM_GETCHECK);
	}
	inline int SetCheck(int check_state)
	{
		return (int)SendMessage(BM_SETCHECK,(WPARAM)check_state);
	}
	inline int GetState()
	{
		return (int)SendMessage(BM_GETSTATE);
	}
	inline int SetState(int push_state)
	{
		return (int)SendMessage(BM_SETSTATE,(WPARAM)push_state);
	}
	inline void SetStyle(int button_style)
	{
		SendMessage(BM_SETSTYLE,(WPARAM)button_style);
	}
	inline void Click()
	{
		SendMessage(BM_CLICK);
	}
	inline void* GetImage(int *pimgType)
	{
		return (void*)SendMessage(BM_GETIMAGE,(WPARAM)pimgType);
	}
	inline void SetImage(int imgType,void* img)
	{
		SendMessage(BM_SETIMAGE,(WPARAM)imgType,(LPARAM)img);
	}
};


////////////////////////////////////////////////
class MGListBox:public MGCtrlWnd
{
public:
	MGListBox(HWND hWnd):MGCtrlWnd(hWnd){}
	MGListBox(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGListBox(){}
	~MGListBox(){}

	DECLARE_CTRL_CLASS(CTRL_LISTBOX);

	inline int AddString(const char*text)
	{
		return (int)SendMessage(LB_ADDSTRING,0,(LPARAM)text);
	}
	inline int AddItem(PLISTBOXITEMINFO plbii)
	{
		return (int)SendMessage(LB_ADDSTRING,0,(LPARAM)plbii);
	}
	inline int AddItem(char* string,DWORD cmFlag,HICON hIcon)
	{
		LISTBOXITEMINFO lbii ;
			lbii.string = string;
			lbii.cmFlag = cmFlag;
			lbii.hIcon  = hIcon;
		return AddItem(&lbii);
	}
	inline int InsertString(int idx,const char* text)
	{
		return (int)SendMessage(LB_INSERTSTRING,(WPARAM)idx,(LPARAM)text);
	}
	inline int InsertItem(int idx,PLISTBOXITEMINFO plbii)
	{
		return (int)SendMessage(LB_INSERTSTRING,(WPARAM)idx,(LPARAM)plbii);
	}
	inline int InsertItem(int idx,char* string,DWORD cmFlag,HICON hIcon)
	{
		LISTBOXITEMINFO lbii;
			lbii.string = string;
			lbii.cmFlag = cmFlag;
			lbii.hIcon  = hIcon;
		return InsertItem(idx,&lbii);
	}
	inline int DeleteItem(int itemIdx)
	{
		return (int)SendMessage(LB_DELETESTRING,(WPARAM)itemIdx);
	}

	inline void ResetContent(){ SendMessage(LB_RESETCONTENT); }

	inline int IsSelect(int idx){ return (int)SendMessage(LB_GETSEL,(WPARAM)idx); }

	inline int Select(int idx)
	{
		return (int)SendMessage(LB_SETSEL,(WPARAM)1,(LPARAM)idx);
	}
	inline int Unselect(int idx)
	{
		return (int)SendMessage(LB_SETSEL,(WPARAM)0,(LPARAM)idx);
	}
	inline int AutoSelect(int idx)
	{
		return (int)SendMessage(LB_SETSEL,(WPARAM)-1,(LPARAM)idx);
	}

	inline int GetCurSel(){ return (int)SendMessage(LB_GETCURSEL); }

	inline int SetCurSel(int cursel){ return (int)SendMessage(LB_SETCURSEL,(WPARAM)cursel); }

	inline int GetItemText(int idx,char *text)
	{
		return (int)SendMessage(LB_GETTEXT,(WPARAM)idx,(LPARAM)text);
	}

	inline int GetItemTextLen(int idx)
	{
		return (int)SendMessage(LB_GETTEXTLEN,(WPARAM)idx);
	}

	inline int GetCount(){ return (int)SendMessage(LB_GETCOUNT); }

	inline int GetTopIndex(){ return (int)SendMessage(LB_GETTOPINDEX); }

	inline int SetTopIndex(int idx){ return (int)SendMessage(LB_SETTOPINDEX,(WPARAM)idx); }

	inline int FindString(int idx,const char* string)
	{
		return (int)SendMessage(LB_FINDSTRING,(WPARAM)idx,(LPARAM)string);
	}

	inline int GetSelCount(){ return (int)SendMessage(LB_GETSELCOUNT); }

	inline int GetSelItems(int nItem,int *pInt)
	{
		return (int)SendMessage(LB_GETSELITEMS,(WPARAM)nItem,(LPARAM)pInt);
	}
	inline int *GetSelItems(int *npItem)
	{
		int nItem = GetSelCount();
		if(nItem<=0)
			return NULL;
		int *items = new int[nItem];

		nItem = GetSelItems(nItem,items);

		if(nItem<=0)
		{
			delete[] items;
			return NULL;
		}

		if(npItem)
			*npItem = nItem;
		return items;
	}

	inline int GetItemRect(int idx,PRECT prc)
	{
		return (int)SendMessage(LB_GETITEMRECT,(WPARAM)idx,(LPARAM)prc);
	}

	inline int GetItemData(int idx,PLISTBOXITEMINFO plbii)
	{
		return (int)SendMessage(LB_GETITEMDATA,(WPARAM)idx,(LPARAM)plbii);
	}

	inline int SetItemData(int idx,PLISTBOXITEMINFO plbii)
	{
		return (int)SendMessage(LB_SETITEMDATA,(WPARAM)idx,(LPARAM)plbii);
	}
	inline int SetItemData(int idx,char* string,DWORD cmFlag,HICON hIcon)
	{
		LISTBOXITEMINFO lbii;
			lbii.string = string;
			lbii.cmFlag = cmFlag;
			lbii.hIcon  = hIcon;
		return SetItemData(idx,&lbii);
	}

	inline int SetCaretIndex(int idx)
	{
		return (int)SendMessage(LB_SETCARETINDEX,(WPARAM)idx);
	}

	inline int GetCaretIndex(int idx)
	{
		return (int)SendMessage(LB_GETCARETINDEX);
	}

	inline void SetItemHeight(int height)
	{
		SendMessage(LB_SETITEMHEIGHT,0,(LPARAM)height);
	}

	inline int GetItemHeight(){ return (int)SendMessage(LB_GETITEMHEIGHT); }

	inline int FindStringExact(int idx,const char* string)
	{
		return (int)SendMessage(LB_FINDSTRINGEXACT,(WPARAM)idx,(LPARAM)string);
	}

	inline int SetItemText(int idx,const char* string)
	{
		return (int)SendMessage(LB_SETTEXT,(WPARAM)idx,(LPARAM)string);
	}

	inline int GetCheckMark(int idx)
	{
		return (int)SendMessage(LB_GETCHECKMARK,(WPARAM)idx);
	}
	#define IsItemChecked(idx) (GetCheckMark(idx)==CMFLAG_CHECKED)

	inline int SetCheckMark(int idx,int status)
	{
		return (int)SendMessage(LB_SETCHECKMARK,(WPARAM)idx,(LPARAM)status);
	}
	#define CheckItem(idx)     SetCheckMark(idx,CMFLAG_CHECKED)
	#define PartCheckItem(idx) SetCheckMark(idx,CMFLAG_PARTCHECKED)
	#define UncheckItem(idx)   SetCheckMark(idx,CMFLAG_BLANK)

	inline DWORD GetItemAddData(int idx)
	{
		return (DWORD)SendMessage(LB_GETITEMADDDATA,(WPARAM)idx);
	}

	inline int SetItemAddData(int idx,DWORD addData)
	{
		return (int)SendMessage(LB_SETITEMADDDATA,(WPARAM)idx,(LPARAM)addData);
	}

	inline int SetStrCmpFunc(int (*my_strcmp)(const char*,const char*,size_t))
	{
		return (int)SendMessage(LB_SETSTRCMPFUNC,0,(LPARAM)my_strcmp);
	}
};

class MGComboBox:public MGCtrlWnd
{
public:
	MGComboBox(HWND hWnd):MGCtrlWnd(hWnd){}
	MGComboBox(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGComboBox(){}
	~MGComboBox(){}

	DECLARE_CTRL_CLASS(CTRL_COMBOBOX)

	inline int GetEditSel(int *pstart,int *pend)
	{
		return (int)SendMessage(CB_GETEDITSEL,(WPARAM)pstart,(LPARAM)pend);
	}

	inline void SetLimitText(int newLimit)
	{
		SendMessage(CB_LIMITTEXT,(WPARAM)newLimit);
	}

	inline int SetEditSel(int start,int end)
	{
		return (int)SendMessage(CB_SETEDITSEL,(WPARAM)start,(LPARAM)end);
	}

	inline int AddString(const char* string)
	{
		return (int)SendMessage(CB_ADDSTRING,0,(LPARAM)string);
	}

	inline int DeleteString(int idx)
	{
		return (int)SendMessage(CB_DELETESTRING,(WPARAM)idx);
	}

	inline int GetCount(){ return (int)SendMessage(CB_GETCOUNT); }

	inline int GetCurSel(){ return (int)SendMessage(CB_GETCURSEL); }

	inline int SetCurSel(int idx){ return (int)SendMessage(CB_SETCURSEL,(WPARAM)idx); }

	inline int GetLBText(int idx,char* string)
	{
		return (int)SendMessage(CB_GETLBTEXT,(WPARAM)idx,(LPARAM)string);
	}

	inline int GetLBTextLen(int idx)
	{
		return (int)SendMessage(CB_GETLBTEXTLEN,(WPARAM)idx);
	}

	inline int InsertString(int idx,char *string)
	{
		return (int)SendMessage(CB_INSERTSTRING,(WPARAM)idx,(LPARAM)string);
	}

	inline void ResetContent(){ SendMessage(CB_RESETCONTENT); }

	inline int FindString(int idxStart,char* string)
	{
		return (int)SendMessage(CB_FINDSTRING,(WPARAM)idxStart,(LPARAM)string);
	}

	inline DWORD GetItemAddData(int idx)
	{
		return (DWORD)SendMessage(CB_GETITEMADDDATA,(WPARAM)idx);
	}

	inline int SetItemAddData(int idx,DWORD addData)
	{
		return (int)SendMessage(CB_SETITEMADDDATA,(WPARAM)idx,(LPARAM)addData);
	}

	inline int GetDroppedControlRect(PRECT *prc)
	{
		return (int)SendMessage(CB_GETDROPPEDCONTROLRECT,0,(LPARAM)prc);
	}

	inline int SetItemHeight(int height)
	{
		return (int)SendMessage(CB_SETITEMHEIGHT,0,(LPARAM)height);
	}

	inline int GetItemHeight(){ return (int)SendMessage(CB_GETITEMHEIGHT); }

	inline BOOL GetDroppedState(){ return (BOOL)SendMessage(CB_GETDROPPEDSTATE); }

	inline int FindStringExact(int idxStart,char* string)
	{
		return (int)SendMessage(CB_FINDSTRINGEXACT,(WPARAM)idxStart,(LPARAM)string);
	}

	inline int SetSpinFormat(const char* format)
	{
		return (int)SendMessage(CB_SETSPINFORMAT,0,(LPARAM)format);
	}

	inline int SetSpinRange(int new_min,int new_max)
	{
		return (int)SendMessage(CB_SETSPINRANGE,(WPARAM)new_min,(LPARAM)new_max);
	}

	inline void GetSpinRange(int *pspin_min,int *pspin_max)
	{
		SendMessage(CB_GETSPINRANGE,(WPARAM)pspin_min,(LPARAM)pspin_max);
	}

	inline void SetSpinValue(int new_value)
	{
		SendMessage(CB_SETSPINVALUE,(WPARAM)new_value);
	}

	inline int GetSpinValue(){ return (int)SendMessage(CB_GETSPINVALUE); }

	inline int SetSpinPace(int new_pace,int new_fastpace)
	{
		return (int)SendMessage(CB_SETSPINPACE,(WPARAM)new_pace,(LPARAM)new_fastpace);
	}

	inline int GetSpinPace(int *pspin_pace,int *pspin_fastpace)
	{
		return (int)SendMessage(CB_GETSPINPACE,(WPARAM)pspin_pace,(LPARAM)pspin_fastpace);
	}

	#define SPIN_UP    0
	#define SPIN_DOWN  1
	inline void Spin(int direct=SPIN_UP)
	{
		SendMessage(CB_SPIN,(WPARAM)direct);
	}

	inline void FastSpin(int direct=SPIN_UP)
	{
		SendMessage(CB_FASTSPIN,(WPARAM)direct);
	}

	inline int SetStrCmpFunc(int (*my_strcmp)(const char*,const char*,size_t))
	{
		return (int)SendMessage(CB_SETSTRCMPFUNC,0,(LPARAM)my_strcmp);
	}

	inline void GetChildren(HWND *pedit,HWND *plistbox)
	{
		SendMessage(CB_GETCHILDREN,(WPARAM)pedit,(LPARAM)plistbox);
	}
};

///////////////////////////////////////////////////////////
class MGProgressBar:public MGCtrlWnd
{
public:
	MGProgressBar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGProgressBar(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGProgressBar(){}
	~MGProgressBar(){}

	DECLARE_CTRL_CLASS(CTRL_PROGRESSBAR)

	inline int SetRange(int min,int max)
	{
		return (int)SendMessage(PBM_SETRANGE,(WPARAM)min,(LPARAM)max);
	}

	inline int SetStep(int stepinc)
	{
		return (int)SendMessage(PBM_SETSTEP,(WPARAM)stepinc);
	}

	inline int SetPos(int nPos)
	{
		return (int)SendMessage(PBM_SETPOS,(WPARAM)nPos);
	}

	inline void SetDeltaPos(int posInc)
	{
		SendMessage(PBM_DELTAPOS,(WPARAM)posInc);
	}

	inline void StepIt(){ SendMessage(PBM_STEPIT); }
};

/////////////////////////////////////////////////////////////
class MGTrackBar:public MGCtrlWnd
{
public:
	MGTrackBar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGTrackBar(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}

	DECLARE_CTRL_CLASS(CTRL_TRACKBAR)

	inline int SetRange(int min,int max)
	{
		return (int)SendMessage(TBM_SETRANGE,(WPARAM)min,(LPARAM)max);
	}

	inline int GetMin(){ return (int)SendMessage(TBM_GETMIN); }

	inline int GetMax(){ return (int)SendMessage(TBM_GETMAX); }

	inline void SetPos(int pos)
	{
		SendMessage(TBM_SETPOS,(WPARAM)pos);
	}

	inline int GetPos(){ return (int)SendMessage(TBM_GETPOS); }

	inline int SetLineSize(int linesize)
	{
		return (int)SendMessage(TBM_SETLINESIZE,(WPARAM)linesize);
	}

	inline int GetLineSize(){ return (int)SendMessage(TBM_GETLINESIZE); }

	inline int SetPageSize(int pagesize)
	{
		return (int)SendMessage(TBM_SETPAGESIZE,(WPARAM)pagesize);
	}

	inline int GetPageSize(){ return (int)SendMessage(TBM_GETPAGESIZE); }

	inline void SetTip(const char* starttip,const char* endtip)
	{
		SendMessage(TBM_SETTIP,(WPARAM)starttip,(LPARAM)endtip);
	}

	inline int GetTip(char* starttip,char* endtip)
	{
		return (int)SendMessage(TBM_GETTIP,(WPARAM)starttip,(LPARAM)endtip);
	}

	inline int SetTickFreq(int tickfreq)
	{
		return (int)SendMessage(TBM_SETTICKFREQ,(WPARAM)tickfreq);
	}

	inline int GetTickFreq(){ return (int)SendMessage(TBM_GETTICKFREQ); }

	inline int SetMin(int min)
	{
		return (int)SendMessage(TBM_SETMIN,(WPARAM)min);
	}

	inline int SetMax(int max)
	{
		return (int)SendMessage(TBM_SETMAX,(WPARAM)max);
	}
};

#if 0
////////////////////////////////////////////////////////////////////
class MGToolBar:public MGCtrlWnd
{
public:
	MGToolBar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGToolBar(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGToolBar(){}
	~MGToolBar(){}

	DECLARE_CTRL_CLASS(CTRL_TOOLBAR)

	inline BOOL Create(HWND hWndParent,int x,int y,int w,int h,DWORD dwStyle,int itemWidth,int itemHeight,DWORD dwStylEx=0,int Id=-1)
	{
		return MGCtrlWnd::Create(NULL,hWndParent,x,y,w,h,dwStyle,dwStylEx,Id,(DWORD)MAKELONG(itemHeight,itemWidth));
	}

	inline int AddItem(PTOOLBARITEMINFO ptbii)
	{
		return (int)SendMessage(TBM_ADDITEM,0,(LPARAM)ptbii);
	}

	inline int AddItem(int insPos,int Id,char* NBmpPath,char* HBmpPath,char* DBmpPath,DWORD dwAddData=0)
	{
		TOOLBARITEMINFO tbii;
			tbii.insPos = insPos;
			tbii.id = Id;
			strcpy(tbii.NBmpPath,NBmpPath);
			strcpy(tbii.HBmpPath,HBmpPath);
			strcpy(tbii.DBmpPath, DBmpPath);
			tbii.dwAddData = dwAddData;
		return AddItem(&tbii);
	}
};
#endif

class MGNewToolBar:public MGCtrlWnd
{
public:
	MGNewToolBar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGNewToolBar(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGNewToolBar(){}
	~MGNewToolBar(){}

	DECLARE_CTRL_CLASS(CTRL_NEWTOOLBAR)

	inline BOOL Create(HWND hWndParent,int x,int y,int w,int h,DWORD dwStyle,NTBINFO* pntbi,DWORD dwStylEx=0,int Id=-1)
	{
		return MGCtrlWnd::Create(NULL,hWndParent,x,y,w,h,dwStyle,dwStylEx,Id,(DWORD)pntbi);
	}

	inline BOOL Create(HWND hWndParent,int x,int y,int w,int h,DWORD dwStyle,DWORD dwStylEx=0,int Id=-1)
	{
		NTBINFO ntbi;
		ntbi.image = NULL;
		ntbi.nr_cols = 4;
		ntbi.nr_cells = 1;
		ntbi.h_cell = 1;
		ntbi.w_cell = 1;
		return Create(hWndParent,x,y,w,h,dwStyle,&ntbi,dwStylEx,Id);
	}

	inline int AddItem(PNTBITEMINFO pntbii)
	{
		//printf("addItem:%d\n",pntbii);
		return (int)SendMessage(NTBM_ADDITEM,0,(LPARAM)pntbii);
		//return 0;
	}
	inline int AddItem(DWORD which,DWORD flags,int id,const char* text,
		const char* tip,int bmp_cell,
		HOTSPOTPROC hotspot_proc,
		PRECT prc_hotspot,DWORD addData=0)
	{
		NTBITEMINFO ntbii;
		//printf("AddItem\n");
			ntbii.which = which;
			ntbii.flags = flags;
			ntbii.id = id;
			ntbii.text = (char*)text;
			ntbii.tip = (char*)tip;
			ntbii.bmp_cell = bmp_cell;
			ntbii.hotspot_proc = hotspot_proc;
			if(prc_hotspot)
				ntbii.rc_hotspot = *prc_hotspot;
			else
			{
				ntbii.rc_hotspot.left = ntbii.rc_hotspot.right = ntbii.rc_hotspot.top = ntbii.rc_hotspot.bottom = 0;
			}
			ntbii.add_data = addData;
			//printf("addItem\n");
		return AddItem(&ntbii);
	}

	inline int GetItem(int id,PNTBITEMINFO pntbii)
	{
		return (int)SendMessage(NTBM_GETITEM,(WPARAM)id,(LPARAM)pntbii);
	}

	inline int SetItem(int id,PNTBITEMINFO pntbii)
	{
		return (int)SendMessage(NTBM_SETITEM,(WPARAM)id,(LPARAM)pntbii);
	}

	inline int SetItem(int id,DWORD which,DWORD flags,const char* text,
		const char* tip,int bmp_cell,
		HOTSPOTPROC hotspot_proc,
		PRECT prc_hotspot,DWORD addData=0)
	{
		NTBITEMINFO ntbii ;
			ntbii.which = which;
			ntbii.flags = flags;
			ntbii.id = id;
			ntbii.text = (char*)text;
			ntbii.tip = (char*)tip;
			ntbii.bmp_cell = bmp_cell;
			ntbii.hotspot_proc = hotspot_proc;
			ntbii.rc_hotspot = *prc_hotspot;
			ntbii.add_data = addData;
		return SetItem(id,&ntbii);
	}

	inline int EnableItem(int id,BOOL fEnable=TRUE)
	{
		return (int)SendMessage(NTBM_ENABLEITEM,(WPARAM)id,(LPARAM)fEnable);
	}

	inline int SetBitmap(NTBINFO* pntbi)
	{
		return (int)SendMessage(NTBM_SETBITMAP,0,(LPARAM)pntbi);
	}
	inline int SetBitmap(PBITMAP image,int h_cell,int w_cell,int nr_cells,int nr_cols=4)
	{
		NTBINFO ntbi;
		ntbi.image = image;
		ntbi.h_cell = h_cell;
		ntbi.w_cell = w_cell;
		ntbi.nr_cells = nr_cells;
		ntbi.nr_cols = nr_cols;
		return SetBitmap(&ntbi);
	}
	inline PBITMAP SetBitmap(const char* strBmp,int h_cell,int w_cell,int nr_cells,int nr_cols=4)
	{
		PBITMAP pbmp = new BITMAP;
		if(LoadBitmapFromFile((HDC)NULL,pbmp,strBmp)==0)
		{
			if(NTB_OKAY==SetBitmap(pbmp,h_cell,w_cell,nr_cells,nr_cols))
			{
				return pbmp;
			}
		}
		delete pbmp;
		return NULL;
	}
};

class MGCoolToolBar:public MGCtrlWnd
{
public:
	MGCoolToolBar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGCoolToolBar(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGCoolToolBar(){}

	~MGCoolToolBar(){}

	DECLARE_CTRL_CLASS(CTRL_COOLBAR)

	inline int AddItem(PCOOLBARITEMINFO pcbii)
	{
		return (int)SendMessage(CBM_ADDITEM,0,(LPARAM)pcbii);
	}

	inline int AddItem(int insPos,int id,int ItemType,PBITMAP Bmp,const char*ItemHint,const char* Caption,DWORD dwAddData=0)
	{
		COOLBARITEMINFO cbii;
			cbii.insPos = insPos;
			cbii.id = id;
			cbii.ItemType = ItemType;
			cbii.Bmp = Bmp;
			cbii.ItemHint = ItemHint;
			cbii.Caption = Caption;
			cbii.dwAddData = dwAddData;
		return AddItem(&cbii);
	}

	inline int EnableItem(int id,BOOL fEnable=TRUE)
	{
		return (int)SendMessage(CBM_ENABLE,(WPARAM)id,(LPARAM)fEnable);
	}
};

///////////////////////////////////////////////////////////////////
class MGScrollWnd:public MGCtrlWnd
{
public:
	MGScrollWnd(HWND hWnd):MGCtrlWnd(hWnd){}
	MGScrollWnd(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGScrollWnd(){}
	~MGScrollWnd(){}

	DECLARE_CTRL_CLASS(CTRL_SCROLLWND)

	inline int AddCtrls(int ctrlNum,PCTRLDATA pctrls)
	{
		return (int)SendMessage(SVM_ADDCTRLS,(WPARAM)ctrlNum,(LPARAM)pctrls);
	}

	inline int SetContWidth(int cont_w)
	{
		return (int)SendMessage(SVM_SETCONTWIDTH,(WPARAM)cont_w);
	}

	inline int SetContHeight(int cont_h)
	{
		return (int)SendMessage(SVM_SETCONTHEIGHT,(WPARAM)cont_h);
	}

	inline HWND GetCtrl(int id)
	{
		return (HWND)SendMessage(SVM_GETCTRL,(WPARAM)id);
	}

	inline int ResetContent(){ return (int)SendMessage(SVM_RESETCONTENT); }

	inline int GetMargins(PRECT prcMargin)
	{
		return (int)SendMessage(SVM_GETMARGINS,0,(LPARAM)prcMargin);
	}

	inline int SetMargins(const PRECT prcMargin)
	{
		return (int)SendMessage(SVM_SETMARGINS,0,(LPARAM)prcMargin);
	}

	inline int GetLeftMargin(){ return (int)SendMessage(SVM_GETLEFTMARGIN); }

	inline int GetTopMargin(){ return (int)SendMessage(SVM_GETTOPMARGIN); }

	inline int GetRightMargin(){ return (int)SendMessage(SVM_GETRIGHTMARGIN); }

	inline int GetBottomMargin(){ return (int)SendMessage(SVM_GETBOTTOMMARGIN); }

	inline int GetVisibleWidth(){ return (int)SendMessage(SVM_GETVISIBLEWIDTH); }

	inline int GetVisibleHeight(){ return (int)SendMessage(SVM_GETVISIBLEHEIGHT); }

	inline int GetContWidth(){ return (int)SendMessage(SVM_GETCONTWIDTH); }

	inline int GetContHeight(){ return (int)SendMessage(SVM_GETCONTHEIGHT); }

	inline int SetContRange(int cont_w,int cont_h)
	{
		return (int)SendMessage(SVM_SETCONTRANGE,(WPARAM)cont_w,(LPARAM)cont_h);
	}

	inline int GetContentX(){ return (int)SendMessage(SVM_GETCONTENTX); }

	inline int GetContentY(){ return (int)SendMessage(SVM_GETCONTENTY); }

	inline int SetContenPos(int cont_x,int cont_y)
	{
		return (int)SendMessage(SVM_SETCONTPOS,(WPARAM)cont_x,(LPARAM)cont_y);
	}

	inline int SetContainerProc(WNDPROC pfn)
	{
		return (int)SendMessage(SVM_SETCONTAINERPROC,0,(LPARAM)pfn);
	}

	inline int GetFocusChild(){ return (int)SendMessage(SVM_GETFOCUSCHILD); }

	inline int GetHScrollVal(){ return (int)SendMessage(SVM_GETHSCROLLVAL); }

	inline int GetVScrollVal(){ return (int)SendMessage(SVM_GETVSCROLLVAL); }

	inline int GetHScrollPageVal(){ return (int)SendMessage(SVM_GETHSCROLLPAGEVAL); }

	inline int GetVScrollPageVal(){ return (int)SendMessage(SVM_GETVSCROLLPAGEVAL); }

	inline int SetScrollVal(int hval,int vval)
	{
		return (int)SendMessage(SVM_SETSCROLLVAL,(WPARAM)hval,(LPARAM)vval);
	}

	inline int SetScrollPageVal(int hval,int vval)
	{
		return (int)SendMessage(SVM_SETSCROLLPAGEVAL,(WPARAM)hval,(LPARAM)vval);
	}

	inline int MakePosVisible(int pos_x,int pos_y)
	{
		return (int)SendMessage(SVM_MAKEPOSVISIBLE,(WPARAM)pos_x,(LPARAM)pos_y);
	}

};

class MGScrollView:public MGScrollWnd
{
public:
	MGScrollView(HWND hWnd):MGScrollWnd(hWnd){}
	MGScrollView(HWND hParent,int Id):MGScrollWnd(hParent,Id){}
	MGScrollView(){}
	~MGScrollView(){}

	DECLARE_CTRL_CLASS(CTRL_SCROLLVIEW)

	inline int AddItem(PSVITEMINFO psvii,HSVITEM *phsvi)
	{
		return (int)SendMessage(SVM_ADDITEM,(WPARAM)phsvi,(LPARAM)psvii);
	}

	inline int DeleteItem(int nItem,HSVITEM hsvi)
	{
		return (int)SendMessage(SVM_DELITEM,(WPARAM)nItem,(LPARAM)hsvi);
	}

	inline int SetItemDraw(SVITEM_DRAWFUNC pfn)
	{
		return (int)SendMessage(SVM_SETITEMDRAW,0,(LPARAM)pfn);
	}

	inline int SetItemOPS(PSVITEMOPS piop)
	{
		return (int)SendMessage(SVM_SETITEMOPS,0,(LPARAM)piop);
	}

	inline int SetItemOPS(SVITEM_INITFUNC initItem,SVITEM_DESTROYFUNC destroyItem,SVITEM_DRAWFUNC drawItem)
	{
		SVITEMOPS iop ;
			iop.initItem = initItem;
			iop.destroyItem = destroyItem;
			iop.drawItem = drawItem;
		return SetItemOPS(&iop);
	}

	inline int GetCurSel(){ return (int)SendMessage(SVM_GETCURSEL); }

	inline int SelectItem(int nItem,BOOL bSel=TRUE)
	{
		return (int)SendMessage(SVM_SELECTITEM,(WPARAM)nItem,(LPARAM)bSel);
	}

	inline int ShowItem(int nItem,HSVITEM hsvi)
	{
		return (int)SendMessage(SVM_SHOWITEM,(WPARAM)nItem,(LPARAM)hsvi);
	}

	inline int ChooseItem(int nItem,HSVITEM hsvi)
	{
		return (int)SendMessage(SVM_CHOOSEITEM,(WPARAM)nItem,(LPARAM)hsvi);
	}

	inline int SetCurSel(int nItem,BOOL bVisible=TRUE)
	{
		return (int)SendMessage(SVM_SETCURSEL,(WPARAM)nItem,(LPARAM)bVisible);
	}

	inline int SetItemInit(SVITEM_INITFUNC pfn)
	{
		return (int)SendMessage(SVM_SETITEMINIT,0,(LPARAM)pfn);
	}

	inline int SetItemDestroy(SVITEM_DESTROYFUNC pfn)
	{
		return (int)SendMessage(SVM_SETITEMDESTROY,0,(LPARAM)pfn);
	}

	inline int SetItemCmp(SVITEM_CMP pfn)
	{
		return  SendMessage(SVM_SETITEMCMP,0,(LPARAM)pfn);
	}

	inline int SortItems(SVITEM_CMP pfn)
	{
		return (int)SendMessage(SVM_SORTITEMS,0,(LPARAM)pfn);
	}

	inline int GetItemCount(){ return (int)SendMessage(SVM_GETITEMCOUNT); }

	inline int GetItemAddData(int nItem,HSVITEM hsvi)
	{
		return (int)SendMessage(SVM_GETITEMADDDATA,(WPARAM)nItem,(LPARAM)hsvi);
	}

	inline int SetItemAddData(int nItem,int addData)
	{
		return (int)SendMessage(SVM_SETITEMADDDATA,(WPARAM)nItem,(LPARAM)addData);
	}

	inline int RefreshItem(int nItem,HSVITEM hsvi)
	{
		return (int)SendMessage(SVM_REFRESHITEM,(WPARAM)nItem,(LPARAM)hsvi);
	}

	inline int SetItemHeight(int nItem,int height)
	{
		return (int)SendMessage(SVM_SETITEMHEIGHT,(WPARAM)nItem,(LPARAM)height);
	}

	inline int GetFirstVisibleItem()
	{
		return (int)SendMessage(SVM_GETFIRSTVISIBLEITEM);
	}
};

/////////////////////////////////////////////////////////////////////////////////////
class MGTreeView:public MGCtrlWnd
{
public:
	MGTreeView(HWND hWnd):MGCtrlWnd(hWnd){}
	MGTreeView(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGTreeView(){}
	~MGTreeView(){}

	DECLARE_CTRL_CLASS(CTRL_TREEVIEW)

	inline GHANDLE AddItem(GHANDLE parent,PTVITEMINFO pnewItemInfo)
	{
		return (GHANDLE)SendMessage(TVM_ADDITEM,(WPARAM)parent,(LPARAM)pnewItemInfo);
	}

	inline GHANDLE AddItem(GHANDLE parent,const char* text,DWORD dwFlags,DWORD dwAddData=0)
	{
		TVITEMINFO tvItem ;
			tvItem.text = (char*)text;
			tvItem.dwFlags = dwFlags;
			tvItem.dwAddData = dwAddData;
		return AddItem(parent,&tvItem);
	}

	//#define InsertItem(parent,pnewItemInfo) AddItem(parent,pnewItemInfo)
	//#define InsertItem(parent,text,dwFlags,hIconFold,hIconUnfold,dwAddData)  AddItem(parent,text,dwFlags,hIconFold,hIconUnfold,dwAddData)

	inline GHANDLE GetRoot(){ return (GHANDLE)SendMessage(TVM_GETROOT); }

	inline int DeleteTree(GHANDLE item)
	{
		return (int)SendMessage(TVM_DELTREE,(WPARAM)item);
	}

	inline int SearchItem(GHANDLE item,const char* string)
	{
		return (int)SendMessage(TVM_SEARCHITEM,(WPARAM)item,(LPARAM)string);
	}

	inline GHANDLE FindChild(GHANDLE item,const char* string)
	{
		return (GHANDLE)SendMessage(TVM_FINDCHILD,(WPARAM)item,(LPARAM)string);
	}

	inline GHANDLE GetSelItem(){ return (GHANDLE)SendMessage(TVM_GETSELITEM); }

	inline GHANDLE SetSelItem(GHANDLE item)
	{
		return (GHANDLE)SendMessage(TVM_SETSELITEM,(WPARAM)item);
	}

	inline GHANDLE GetItemTextLen(GHANDLE item)
	{
		return (GHANDLE)SendMessage(TVM_GETITEMTEXTLEN,(WPARAM)item);
	}

	inline int GetItemText(GHANDLE item,char* buffer)
	{
		return (int)SendMessage(TVM_GETITEMTEXT,(WPARAM)item,(LPARAM)buffer);
	}

	inline int GetItemInfo(GHANDLE item,TVITEMINFO *ptvii)
	{
		return (int)SendMessage(TVM_GETITEMINFO,(WPARAM)item,(LPARAM)ptvii);
	}

	inline DWORD GetItemAddData(GHANDLE item)
	{
		char tmp[256];
		TVITEMINFO tvii;
		tvii.text = tmp;

		if(GetItemInfo(item,&tvii)==0)
		{
			return tvii.dwAddData;
		}
		return 0;
	}


	inline int SetItemInfo(GHANDLE item,TVITEMINFO *ptvii)
	{
		return (int)SendMessage(TVM_SETITEMINFO,(WPARAM)item,(LPARAM)ptvii);
	}

	inline int SetItemInfo(GHANDLE item,const char* text,DWORD dwFlags=0,DWORD dwAddData=0)
	{
		TVITEMINFO tvii;
		tvii.text = (char*)text;
		tvii.dwFlags = dwFlags;
		tvii.dwAddData = dwAddData;
		return SetItemInfo(item,&tvii);
	}

	inline int SetItemText(GHANDLE item,const char* text)
	{
		char tmp[256];
		TVITEMINFO tvii;
		tvii.text = tmp;
		if(text==NULL)
			return -1;
		tvii.text = NULL;
		if(GetItemInfo(item,&tvii) == 0)
		{
			tvii.text = (char*)text;
			return SetItemInfo(item,&tvii);
		}
		return -1;
	}

	inline int SetItemAddData(GHANDLE item,DWORD dwAddData)
	{
		TVITEMINFO tvii;
		tvii.text = NULL;
		GetItemInfo(item,&tvii);
		tvii.dwAddData = dwAddData;
		return SetItemInfo(item,&tvii);
	}

	inline void SetItemFold (GHANDLE item, BOOL fold)
	{
		TVITEMINFO tvii;
		tvii.text = NULL;
		GetItemInfo(item,&tvii);
		if (fold)
			tvii.dwFlags |= TVIF_FOLD;
		else
			tvii.dwFlags &= ~TVIF_FOLD;
		SetItemInfo(item,&tvii);
	}

	inline GHANDLE GetRelatedItem(GHANDLE item,int related)
	{
		return (GHANDLE)SendMessage(TVM_GETRELATEDITEM,(WPARAM)related,(LPARAM)item);
	}
	inline GHANDLE GetParentItem(GHANDLE item)
	{
		return GetRelatedItem(item,TVIR_PARENT);
	}
	inline GHANDLE GetFirstChild(GHANDLE item)
	{
		return GetRelatedItem(item,TVIR_FIRSTCHILD);
	}
	inline GHANDLE GetNextSibling(GHANDLE item)
	{
		return GetRelatedItem(item,TVIR_NEXTSIBLING);
	}
	inline GHANDLE GetPrevSibling(GHANDLE item)
	{
		return GetRelatedItem(item,TVIR_PREVSIBLING);
	}

	inline int SetStrCmpFunc(int(*my_strcmp)(const char*,const char*,size_t))
	{
		return (int)SendMessage(TVM_SETSTRCMPFUNC,0,(LPARAM)my_strcmp);
	}
};

/////////////////////////////////////////
class MGListView:public MGCtrlWnd
{
public:
	MGListView(HWND hWnd):MGCtrlWnd(hWnd){}
	MGListView(HWND hParent,int Id):MGCtrlWnd(hParent,Id){}
	MGListView(){}
	~MGListView(){}

	DECLARE_CTRL_CLASS(CTRL_LISTVIEW)

	inline HLVITEM AddItem(HLVITEM parent, PLVITEM plvitem)
	{
		return (HLVITEM)SendMessage(LVM_ADDITEM,(WPARAM)parent,(LPARAM)plvitem);
	}

	inline HLVITEM AddItem(HLVITEM parent,int nItem=0,int nItemHeight=0,DWORD itemData=0,DWORD dwFlags=0)
	{
		LVITEM lvItem;
			lvItem.nItem = nItem;
			lvItem.nItemHeight = nItemHeight;
			lvItem.itemData = itemData;
			lvItem.dwFlags = dwFlags;
		return AddItem(parent,&lvItem);
	}

	inline int FillSubitem(HLVITEM pi,PLVSUBITEM plvitem)
	{
		return (int)SendMessage(LVM_FILLSUBITEM,(WPARAM)pi,(LPARAM)plvitem);
	}

	inline int FillSubitem(PLVSUBITEM plvsubitem)
	{
		return FillSubitem((HLVITEM)NULL,plvsubitem);
	}

	inline int FillSubitem(HLVITEM pi,DWORD flags,int nItem,int subItem,const char* pszText,int nTextColor,DWORD image)
	{
		LVSUBITEM lvSubitem;
			lvSubitem.flags = flags;
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = (char*)pszText;
			lvSubitem.nTextMax = pszText?strlen(pszText):0;
			lvSubitem.nTextColor = nTextColor;
			lvSubitem.image = image;
		return FillSubitem(pi,&lvSubitem);
	}

	inline void AddColumn(PLVCOLUMN pcol)
	{
		SendMessage(LVM_ADDCOLUMN,0,(LPARAM)pcol);
	}

	inline void AddColumn(int nCols,int width,const char* pszHeadText,DWORD image,PFNLVCOMPARE pfnCompare,DWORD colFlags)
	{
		LVCOLUMN lvColumn ;
			lvColumn.nCols = nCols;
			lvColumn.width = width;
			lvColumn.pszHeadText = (char*)pszHeadText;
			lvColumn.nTextMax = pszHeadText?strlen(pszHeadText):0;
			lvColumn.image = image;
			lvColumn.pfnCompare = pfnCompare;
			lvColumn.colFlags = colFlags;
		return AddColumn(&lvColumn);
	}

	inline int DeleteItem(int nRow,HLVITEM pi)
	{
		return (int)SendMessage(LVM_DELITEM,(WPARAM)nRow,(LPARAM)pi);
	}

	inline int ClearSubitem(HLVITEM pi,PLVSUBITEM psubitem=NULL)
	{
		return (int)SendMessage(LVM_CLEARSUBITEM,(WPARAM)pi,(LPARAM)psubitem);
	}

	inline int DeleteColumn(int nCol)
	{
		return (int)SendMessage(LVM_DELCOLUMN,(WPARAM)nCol);
	}

	inline int ColumnSort(int nCol)
	{
		return (int)SendMessage(LVM_COLSORT,(WPARAM)nCol);
	}

	inline void SetSubitemColor(HLVITEM pi,PLVSUBITEM psubitem)
	{
		SendMessage(LVM_SETSUBITEMCOLOR,(WPARAM)pi,(LPARAM)psubitem);
	}

	inline void SetSubitemColor(HLVITEM pi,int nItem,int subItem,int nTextColor)
	{
		LVSUBITEM lvSubitem;
		//	.flags = 0,
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;
		//	.pszText = NULL;
		//	.nTextMax = 0,
			lvSubitem.nTextColor = nTextColor;/*,
			.image = 0*/
		SetSubitemColor(pi,&lvSubitem);
	}

	inline HLVITEM FindItem(HLVITEM parent,PLVFINDINFO plvfi)
	{
		return (HLVITEM)SendMessage(LVM_FINDITEM,(WPARAM)parent,(LPARAM)plvfi);
	}

	inline int GetSubitemText(HLVITEM pi,PLVSUBITEM plvfi)
	{
		return (int)SendMessage(LVM_GETSUBITEMTEXT,(WPARAM)pi,(LPARAM)plvfi);
	}

	inline int GetSubitemText(HLVITEM pi,int subItem,char *buf,int nMax)
	{
		LVSUBITEM lvSubitem;
		/*	.flags = 0,*/
			lvSubitem.nItem = 0;
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = buf;
			lvSubitem.nTextMax = nMax;/*,
			.nTextColor = 0,
			.image = 0*/
		return GetSubitemText(pi,&lvSubitem);
	}

	inline int GetSubitemText(int nItem,int subItem,char *buf,int nMax)
	{
		LVSUBITEM lvSubitem;
	/*		.flags = 0,*/
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = buf;
			lvSubitem.nTextMax = nMax;/*,
			.nTextColor = 0,*/
	/*		.image = 0*/
		return GetSubitemText((HLVITEM)NULL,&lvSubitem);
	}

	inline int GetItemCount(){ return (int)SendMessage(LVM_GETITEMCOUNT); }

	inline int GetColumnCount(){ return (int)SendMessage(LVM_GETCOLUMNCOUNT); }

	inline HLVITEM GetSelectedItem(){ return (HLVITEM)SendMessage(LVM_GETSELECTEDITEM); }

	inline BOOL DeleteAllItem(){ return (BOOL)SendMessage(LVM_DELALLITEM); }

	inline int ModifyHead(PLVCOLUMN pcol)
	{
		return (int)SendMessage(LVM_MODIFYHEAD,0,(LPARAM)pcol);
	}

	inline void SelectItem(int nRow,HLVITEM pi)
	{
		SendMessage(LVM_SELECTITEM,(WPARAM)nRow,(LPARAM)pi);
	}

	inline void ShowItem(int nRow,HLVITEM pi)
	{
		SendMessage(LVM_SHOWITEM,(WPARAM)nRow,(LPARAM)pi);
	}

	inline int GetSubitemLen(HLVITEM pi,PLVSUBITEM plvsubitem)
	{
		return (int)SendMessage(LVM_GETSUBITEMLEN,(WPARAM)pi,(LPARAM)plvsubitem);
	}

	inline int GetSubitemLen(HLVITEM pi,int subItem)
	{
		LVSUBITEM lvSubitem;
//			.flags = 0,
//			.nItem = 0,
			lvSubitem.subItem = subItem;/*,
//			.pszText = NULL;
//			.nTextMax = 0,
//			.nTextColor = 0,
			.image = 0*/
		return GetSubitemLen(pi,&lvSubitem);
	}

	inline int GetSubitemLen(int nItem,int subItem)
	{
		LVSUBITEM lvSubitem;
//			.flags = 0,
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;/*,
//			.pszText = NULL;
//			.nTextMax = 0,
//			.nTextColor = 0,
			.image = 0*/
		return GetSubitemLen((HLVITEM)NULL,&lvSubitem);
	}

	inline int SetColumn(PLVCOLUMN pcol)
	{
		return (int)SendMessage(LVM_SETCOLUMN,0,(LPARAM)pcol);
	}

	inline int SetSubitemText(HLVITEM pi,PLVSUBITEM plvsubitem)
	{
		return (int)SendMessage(LVM_SETSUBITEMTEXT,(WPARAM)pi,(LPARAM)plvsubitem);
	}

	inline int SetSubitemText(HLVITEM pi,int subItem,const char* szText)
	{
		LVSUBITEM lvSubitem;
//			.flags = 0,
//			.nItem = 0,
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = (char*)szText;
			lvSubitem.nTextMax = strlen(szText);/*,
			.nTextColor = 0,
			.image = 0*/
		return SetSubitemText(pi,&lvSubitem);
	}

	inline int SetSubitemText(int nItem,int subItem,const char* szText)
	{
		LVSUBITEM lvSubitem;
//			.flags = 0,
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = (char*)szText;
			lvSubitem.nTextMax = strlen(szText)/*,
			.nTextColor = 0,
			.image = 0*/;
		return SetSubitemText((HLVITEM)NULL,&lvSubitem);
	}


	inline int SetSubitem(HLVITEM pi,PLVSUBITEM plvsubitem)
	{
		return (int)SendMessage(LVM_SETSUBITEM,(WPARAM)pi,(LPARAM)plvsubitem);
	}
	inline int SetSubitem(PLVSUBITEM plvsubitem)
	{
		return SetSubitem((HLVITEM)NULL,plvsubitem);
	}

	inline int SetSubitem(HLVITEM pi,DWORD flags,int nItem,int subItem,const char* pszText,int nTextColor,DWORD image)
	{
		LVSUBITEM lvSubitem;
			lvSubitem.flags = flags;
			lvSubitem.nItem = nItem;
			lvSubitem.subItem = subItem;
			lvSubitem.pszText = (char*)pszText;
			lvSubitem.nTextMax = pszText?strlen(pszText):0;
			lvSubitem.nTextColor = nTextColor,
			lvSubitem.image = image;
		return SetSubitem(pi,&lvSubitem);
	}

	inline BOOL GetColumn(int nCol,PLVCOLUMN pcol)
	{
		return (BOOL)SendMessage(LVM_GETCOLUMN,(WPARAM)nCol,(LPARAM)pcol);
	}

	inline int GetColumnWidth(int nCol)
	{
		return (int)SendMessage(LVM_GETCOLUMNWIDTH,(WPARAM)nCol);
	}

	inline int GetItem(HLVITEM pi,PLVITEM pitem_info)
	{
		return (int)SendMessage(LVM_GETITEM,(WPARAM)pi,(LPARAM)pitem_info);
	}

	inline DWORD GetItemData(HLVITEM pi)
	{
		LVITEM lvitem;
		memset(&lvitem, 0, sizeof(lvitem));
		if(GetItem(pi, &lvitem) == LV_OKAY){
			return lvitem.itemData;
		}

		return 0;
	}

	inline int GetItemState(HLVITEM pi,UINT mask)
	{
		return (int)SendMessage(LVM_GETITEMSTATE,(WPARAM)pi,(LPARAM)mask);
	}

	inline int SetItemState(HLVITEM pi,UINT mask)
	{
		return (int)SendMessage(LVM_SETITEMSTATE,(WPARAM)pi,(LPARAM)mask);
	}

	inline int GetSelectedColumn(){ return (int)SendMessage(LVM_GETSELECTEDCOLUMN); }

	inline BOOL GetSelectedItemRect(RECT* prt){
		return (int)SendMessage(LVM_GETSELECTEDITEMRECT, 0, (LPARAM)prt);
	}

	inline int GetSelectedCount(){ return (int)SendMessage(LVM_GETSELECTEDCOUNT); }

	inline int GetTopvisible(){ return (int)SendMessage(LVM_GETTOPVISIBLE); }

	inline BOOL SortItems(PLVSORTDATA sortData,PFNLVCOMPARE pfnCompare)
	{
		return (BOOL)SendMessage(LVM_SORTITEMS,(WPARAM)sortData,(LPARAM)pfnCompare);
	}

	inline BOOL SetItemHeight(HLVITEM pi,int height)
	{
		return (BOOL)SendMessage(LVM_SETITEMHEIGHT,(WPARAM)pi,(LPARAM)height);
	}

	inline BOOL SetHeadHeight(int height)
	{
		return (BOOL)SendMessage(LVM_SETHEADHEIGHT,(WPARAM)height);
	}

	inline DWORD GetItemAddData(HLVITEM pi,int index)
	{
		return (DWORD)SendMessage(LVM_GETITEMADDDATA,(WPARAM)index,(LPARAM)pi);
	}

	inline int SetItemAddData(HLVITEM pi,DWORD dwAddData)
	{
		return (int)SendMessage(LVM_SETITEMADDDATA,(WPARAM)pi,(LPARAM)dwAddData);
	}

	inline void ChooseItem(HLVITEM pi,int nRow)
	{
		SendMessage(LVM_CHOOSEITEM,(WPARAM)pi,(LPARAM)pi);
	}

	inline int SetStrCmpFunc(int (*my_strcmp)(const char*,const char*,size_t))
	{
		return (int)SendMessage(LVM_SETSTRCMPFUNC,0,(LPARAM)my_strcmp);
	}

	inline HLVITEM GetRelatedItem(HLVITEM item,int related)
	{
		return (HLVITEM)SendMessage(LVM_GETRELATEDITEM,(WPARAM)related,(LPARAM)item);
	}
	inline HLVITEM GetParentItem(HLVITEM item) { return GetRelatedItem(item,TVIR_PARENT); }
	inline HLVITEM GetFirstChildItem(HLVITEM item){ return GetRelatedItem(item,TVIR_FIRSTCHILD); }
	inline HLVITEM GetNextSlibingItem(HLVITEM item){ return GetRelatedItem(item,TVIR_NEXTSIBLING); }
	inline HLVITEM GetPrevSlibingItem(HLVITEM item){ return GetRelatedItem(item,TVIR_PREVSIBLING); }

	inline void FoldItem(HLVITEM pi,BOOL bFold=TRUE)
	{
		SendMessage(LVM_FOLDITEM,(WPARAM)bFold,(LPARAM)pi);
	}

	inline void SetCustomDraw(LVCUSTOMDRAWFUNCS* myFuncs)
	{
		SendMessage(LVM_SETCUSTOMDRAW,0,(LPARAM)myFuncs);
	}

};

///////////////////////////////////////////////////////////
class MGMonthCalendar:public MGCtrlWnd
{
public:
	MGMonthCalendar(HWND hWnd):MGCtrlWnd(hWnd){}
	MGMonthCalendar(HWND hParent,int id):MGCtrlWnd(hParent,id){}
	MGMonthCalendar(){}
	~MGMonthCalendar(){}

	DECLARE_CTRL_CLASS(CTRL_MONTHCALENDAR)

	inline int GetCurDay(){ return (int)SendMessage(MCM_GETCURDAY); }
	inline void SetCurDay(int day){ SendMessage(MCM_SETCURDAY,(WPARAM)day); }

	inline int GetCurMonth(){ return (int)SendMessage(MCM_GETCURMONTH); }
	inline void SetCurMonth(int month){ SendMessage(MCM_SETCURMONTH,(WPARAM)month); }

	inline int GetCurYear(){ return (int)SendMessage(MCM_GETCURYEAR); }
	inline void SetCurYear(int year){ SendMessage(MCM_SETCURYEAR,(WPARAM)year); }

	inline int GetCurWeekday(){ return (int)SendMessage(MCM_GETCURWEEKDAY); }

	inline int GetCurMonthLen(){ return (int)SendMessage(MCM_GETCURMONLEN); }

	inline void SetToday(){ SendMessage(MCM_SETTODAY); }

	inline int GetFirstWeekday(){ return (int)SendMessage(MCM_GETFIRSTWEEKDAY); }

	inline void GetCurDate(PSYSTEMTIME psysTm){ SendMessage(MCM_GETCURDATE,0,(LPARAM)psysTm); }

	inline int GetMinRerqWidth(){ return (int)SendMessage(MCM_GETMINREQRECTW); }
	inline int GetMinRerqHeight(){ return (int)SendMessage(MCM_GETMINREQRECTH); }

	inline void SetCurDate(PSYSTEMTIME psysTm){ SendMessage(MCM_SETCURDATE,0,(LPARAM)psysTm); }

	inline void SetColor(PMCCOLORINFO pmci){ SendMessage(MCM_SETCOLOR,0,(LPARAM)pmci); }

	inline void SetDayColor(int day,int color){ SendMessage(MCM_SETDAYCOLOR,(WPARAM)day,(LPARAM)color); }

	inline void ClearDayColor(){ SendMessage(MCM_CLEARDAYCOLOR); }

};

/////////////////////////////////////////////////////////////
class MGSpinbox:public MGCtrlWnd
{
public:
	MGSpinbox(HWND hWnd):MGCtrlWnd(hWnd){}
	MGSpinbox(HWND hParent,int id):MGCtrlWnd(hParent,id){}
	MGSpinbox(){}
	~MGSpinbox(){}

	DECLARE_CTRL_CLASS(CTRL_SPINBOX)

	inline BOOL GetSpinInfo(SPININFO *pspinfo)
	{
		return (BOOL)SendMessage (SPM_GETINFO, 0, (LPARAM)pspinfo)==0 ;
	}

	inline int GetMax()
	{
		SPININFO spinfo;
		if(GetSpinInfo(&spinfo))
			return spinfo.max;
		return 0;
	}

	inline int GetMin()
	{
		SPININFO spinfo;
		if(GetSpinInfo(&spinfo))
			return spinfo.min;
		return 0;
	}

	inline int GetCur()
	{
		SPININFO spinfo;
		if(GetSpinInfo(&spinfo))
			return spinfo.cur;
		return 0;
	}

	inline BOOL SetSpinInfo(PSPININFO pspinfo)
	{
		return (BOOL)SendMessage(SPM_SETINFO,0,(LPARAM)pspinfo)==0;
	}

	inline BOOL SetSpinInfo(int max,int min,int cur)
	{
		SPININFO spinfo;
		spinfo.max = max;
		spinfo.min = min;
		spinfo.cur = cur;
		return SetSpinInfo(&spinfo);
	}

	inline void SetCur(int cur)
	{
		SendMessage(SPM_SETCUR,(WPARAM)cur);
	}


	inline void SetTarget(HWND hWndTarget){ SendMessage(SPM_SETTARGET,0,(LPARAM)hWndTarget); }

	inline HWND GetTarget(){ return (HWND)SendMessage(SPM_GETTARGET); }

	inline void DisableLineUp(){ SendMessage(SPM_DISABLEUP); }
	inline void EnableLineUp(){ SendMessage(SPM_ENABLEUP); }

	inline void DisableLineDown(){ SendMessage(SPM_DISABLEDOWN); }
	inline void EnableLineDown(){ SendMessage(SPM_ENABLEDOWN); }

};

#define DECLEAR_SHEETPAGE(class_name) public: static MGSheetPage* GetSheetPage(){ return (MGSheetPage*)(new class_name); }

class MGSheetPage:public MGUserCtrl
{
public:
	DECLEAR_SHEETPAGE(MGSheetPage)
	MGSheetPage(){}
	~MGSheetPage(){}
	virtual BOOL OnInitPage(DLGTEMPLATE *pdlg_temp);
	virtual BOOL OnShowPage(HWND focus_hwnd,int show_cmd);
	virtual int   OnSheetCmd(WPARAM wParam,LPARAM lParam);
	virtual void OnOK(LPARAM lParam){}
	virtual void OnApply(LPARAM lParam){}
	virtual void OnCancel(LPARAM lParam){}

protected:
	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);
};


typedef MGSheetPage* (*PSHEETPAGE_FACTORY)(void);
#ifndef IDAPPLY
#define IDAPPLY  IDOK + 2
#endif

class MGPropSheet:public MGCtrlWnd
{
public:
	MGPropSheet(HWND hWnd):MGCtrlWnd(hWnd){}
	MGPropSheet(HWND hWndParent,int id):MGCtrlWnd(hWndParent,id){}
	MGPropSheet(){}
	~MGPropSheet(){}
	DECLARE_CTRL_CLASS(CTRL_PROPSHEET)

	inline HWND GetActivePageHandle(){ return (HWND)SendMessage(PSM_GETACTIVEPAGE);}
	inline MGUserCtrl* GetActivePage(){ return (MGUserCtrl*)MGWnd::WndFromHandle(GetActivePageHandle()); }
	inline int  GetActiveIndex(){ return (int)SendMessage(PSM_GETACTIVEINDEX); }
	inline BOOL SetActiveIndex(int nPage){ return (BOOL)SendMessage(PSM_SETACTIVEINDEX,(WPARAM)nPage)==PS_OKAY; }
	inline HWND GetPageHandle(int index){ return (HWND)SendMessage(PSM_GETPAGE,(WPARAM)index); }
	inline MGSheetPage* GetPage(int index){ return (MGSheetPage*)MGWnd::WndFromHandle(GetPageHandle(index)); }
	inline BOOL GetPageIndex(HWND hWndPage){ return (SendMessage(PSM_GETPAGEINDEX,(WPARAM)hWndPage)==PS_OKAY); }
	inline int  GetPageCount(){ return (int)SendMessage(PSM_GETPAGECOUNT); }
	inline int  GetPageTitleLength(int index){ return (int)SendMessage(PSM_GETTITLELENGTH,(WPARAM)index); }
	inline BOOL GetPageTitle(int index,char* buffer){ return (SendMessage(PSM_GETTITLE,(WPARAM)index,(LPARAM)buffer)==PS_OKAY); }
	inline char* GetPageTitle(int index){
		int len = GetPageTitleLength(index);
		if(len<=0) return NULL;
		char *buffer = new char[len+1];
		if(!GetPageTitle(index,buffer)){
			delete[] buffer;
			return NULL;
		}
		return buffer;
	}
	inline BOOL SetPageTitle(int index,const char* strTitle){ return (SendMessage(PSM_SETTITLE,(WPARAM)index,(LPARAM)strTitle)==PS_OKAY); }
	inline int AddPage(DLGTEMPLATE* pdlg_tmp,WNDPROC proc){
		return (int)SendMessage(PSM_ADDPAGE,(WPARAM)pdlg_tmp,(LPARAM)proc);
	}

	inline int AddPage(DLGTEMPLATE* pdlg_tmp,PSHEETPAGE_FACTORY SHEETPAGE_FACTORY){
		if(SHEETPAGE_FACTORY==NULL) return -1;
		int idx = AddPage(pdlg_tmp,(WNDPROC)NULL);
		if(idx<0) return idx;
		HWND hwnd = GetPageHandle(idx);
		if(hwnd==HDC_INVALID) return -1;
		MGSheetPage* psheetpage = SHEETPAGE_FACTORY();
		psheetpage->Attach(hwnd);
		return idx;
	}

	inline BOOL RemovePageHandle(int index){ return (SendMessage(PSM_REMOVEPAGE,(WPARAM)index)==PS_OKAY); }
	inline BOOL RemovePage(int index){
		MGSheetPage * psheetpage = GetPage(index);
		if(psheetpage) psheetpage->Detach();
		return RemovePageHandle(index);
	}

	inline int SheetCmd(WPARAM wParam,LPARAM lParam=0){
		return (int)SendMessage(PSM_SHEETCMD,wParam,lParam);
	}
	inline void OkaySheets(){ SheetCmd(IDOK); }
	inline void CancelSheets(){ SheetCmd(IDCANCEL); }
	inline void ApplySheets(){ SheetCmd(IDAPPLY); }
};

#endif

