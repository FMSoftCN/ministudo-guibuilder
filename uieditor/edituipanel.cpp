/*
 * edituipanel.cpp
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include "mapex.h"

#include "log.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

#include <mgncs/mgncs.h>
#include "msd_intl.h"

using namespace std;

#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"
#include "window-instance.h"
#include "page-window-instance.h"

#include "ui-event-id.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "editable-listview.h"
#include "fieldpanel.h"

#include "defundo-redo-observer.h"
#include "edituipanel.h"

#include "rdrpanel.h"
#include "rdreditor/rdr-instance.h"
#include "rdreditor/rdrtreepanel.h"
#include "rdreditor/rdr-preview-panel.h"
#include "rdreditor/rdreditor.h"

#include "codecombin.h"
#include "socket-client.h"
#include "connect-events-wnd.h"

#include "qvfb.h"

extern ResEditorEnv *g_env;
extern string getConfigFile(const char* cfgFileName);
extern SIZE getScreenSize();
extern const char *getDefaultClientFont(void);

BOOL EditUIPanel::select_key_state = FALSE;
static vector<DWORD> _move_old_list;
static RECT _size_old;

#define ANCHOR_SIZE  4

#define MIN_WIDTH 20
#define MIN_HEIGHT 20

#define INIT_SCROLLINFO(si)  do{ \
	memset(&si, 0, sizeof(si)); \
	si.cbSize = sizeof(si); \
	si.fMask = SIF_ALL; \
}while(0)

static inline void set_scroll_info(HWND hwnd, int iSbar, SCROLLINFO *psi, int doc_size, int view_size, BOOL bupdate)
{
	if(doc_size < view_size)
		doc_size = view_size;
	psi->fMask = SIF_ALL;
	GetScrollInfo(hwnd, iSbar, psi);
	psi->nMin = 0;
	psi->nMax = doc_size ;
	psi->nPage = view_size;
	if(psi->nPos < psi->nMin)
		psi->nPos = psi->nMin;
	else if(psi->nPos > (int)(psi->nMax - psi->nPage))
		psi->nPos = psi->nMax - psi->nPage + 1;

	SetScrollInfo(hwnd, iSbar, psi,bupdate);
}


EditUIPanel::EditUIPanel(PanelEventHandler* handler)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
	curContainer = NULL;
	baseInstance = NULL;
	xpos = ypos = 0;
	curWnd = HWND_INVALID;
	curState = Normal;
	flags = 0;
	hdcPreviewWnd = HDC_INVALID;
	hwndTopMost = HWND_INVALID;

	editor_win_rdr = (WINDOW_ELEMENT_RENDERER *)calloc(1, sizeof(WINDOW_ELEMENT_RENDERER));

	//init outline bitmap
	InitBitmap(HDC_SCREEN, 96, 96, 0, NULL, &bmpOutline);
	//set undo redo
	setUndoRedoObserver(&undoRedoObserver);
}

EditUIPanel::~EditUIPanel() {
	// TODO Auto-generated destructor stub
    delete baseInstance;

	UnloadBitmap(&bmpOutline);
	free(editor_win_rdr);
	baseInstance = NULL;
}

void EditUIPanel::selectInstance(ComponentInstance * instance)
{
	if(instance == NULL)
		return ;

	if(instance == ComponentInstance::FromHandler(curWnd))
		return ;

	cancelSelectAll();
	insertSelect(instance->getPreviewHandler(), TRUE);
	ComponentInstance *parent=NULL, *prevParent = instance;
	parent = instance->getParent();
	while(parent && !(dynamic_cast<PageWindowInstance*>(parent))){
		prevParent = parent;
		parent = parent->getParent();
	}
	if(parent){
		PageWindowInstance* pwi = (PageWindowInstance*)parent;
		if(pwi->actviePage(prevParent))
			flags |= UpdatePreviewWindow;
	}

	//if(instance->makeVisible())
	InvalidateRect();
	notifySelChanged();
}

HWND EditUIPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);
	TMGStaticSubclass<MGStatic>::Create("", hParent, 0, 0,
		RECTW(rt),RECTH(rt), WS_VISIBLE|WS_HSCROLL|WS_VSCROLL, 0, -1);

	Subclass();

	_oldProc = DefaultControlProc;

	SCROLLINFO sci;
	INIT_SCROLLINFO(sci);
	set_scroll_info(m_hWnd, SB_HORZ, &sci, 100,10,FALSE);
	set_scroll_info(m_hWnd, SB_VERT, &sci, 100,10,FALSE);

	ShowScrollBar(SB_HORZ, FALSE);
	ShowScrollBar(SB_VERT, FALSE);
	EnableScrollBar(SB_HORZ);
	EnableScrollBar(SB_VERT);

	return getHandler();
}

BEGIN_MSG_MAP(EditUIPanel)
	MAP_PAINT(onPaint)
	MAP_LBUTTONDOWN(onLButtonDown)
	MAP_LBUTTONUP(onLButtonUp)
	MAP_MOUSEMOVE(onMouseMove)
	MAP_RBUTTONUP(onRButtonUp)
	MAP_LBUTTONDBLCLK(onLButtonDblClk)
	MAP_HSCROLL(onHScroll)
	MAP_VSCROLL(onVScroll)
	MAP_ERASEBKGND(onEraseBkgnd)
	//MAP_ERASEBKGNDN(onEraseBkgnd)
	MAP_KEYDOWN(onKeyDown)
	MAP_CSIZECHANGED(onCSizeChanged)
	MAP_KEYUP(onKeyUp)
#ifndef WIN32
	BEGIN_COMMAND_MAP
	MAP_COMMAND_RANGE(ResEditor::MENUCMD_COMMID_BASE, ResEditor::MENUCMD_COMMID_END, onPopMenuCmd)
	END_COMMAND_MAP
#else
	case MSG_COMMAND:
	{
		int ctrlId = LOWORD(wParam);
		if (ctrlId >= ResEditor::MENUCMD_COMMID_BASE && ctrlId <= ResEditor::MENUCMD_COMMID_END)
		{
			onPopMenuCmd(ctrlId);
		}
		break;
	}
#endif
END_MSG_MAP

void EditUIPanel::active()
{
	if (baseInstance)
		baseInstance->updatePrevWindow(FALSE);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_UNDO,canUndo());
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_REDO,canRedo());
	//sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_PASTE,(DWORD)(Instance::paste() != NULL));
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_PASTE,(DWORD)(Instance::paste()));
	sendEvent(UIMENUITEM_CHECK,ResEditor::UI_MENUCMD_SNAPGRID,isSnapeGrid());
	notifySelChanged();
}

BOOL EditUIPanel::instanceProcessMouseMessage(int message, int x, int y, DWORD key_data, int *pret)
{
	if(flags&RequestMouseMsg)
	{
		ComponentInstance * cinst = ComponentInstance::FromHandler(curWnd);
		if(cinst){
			//Mouse is In the process?
			mainWndToClient(curWnd, x, y);
			RECT rt;
			::GetClientRect(curWnd, &rt);
			if(PtInRect(&rt, x, y))
			{
				int ret = cinst->processMessage(message,  key_data, ((DWORD)((SWORD)x))|(((DWORD)((SWORD)y))<<16));
				if(pret)
					*pret = ret;
				flags |= UpdatePreviewWindow;
				InvalidateRect();
				return TRUE;
			}
		}
	}

	return FALSE;
}

#define GRID_SIZE  4
void EditUIPanel::snapeGrid(int &x, int &y)
{
	if(flags & AutoSnapeGrid)
	{
		x = ((x+GRID_SIZE/2)/GRID_SIZE)*GRID_SIZE;
		y = ((y+GRID_SIZE/2)/GRID_SIZE)*GRID_SIZE;
	}
}

static HDC _hdc = (HDC)0;
static RECT _select_rect ;
static DWORD _move_size_flag = 0;
static ComponentInstance * _accept_container = NULL;

void EditUIPanel::onLButtonDown(int x, int y, DWORD key_flag)
{
	::SetCapture(m_hWnd);

	x += xpos;
	y += ypos;

	//if Ready to create New Control
	if(flags & CreateControl)
	{
		curContainer = testContainer(x, y, NULL);
		if(curContainer == NULL)
		{
			ReleaseCapture();
			curState = Normal;
			return ;
		}
		else if(curContainer == (ComponentInstance*)-1)
			return;
		//set state as creating
		curState = Creating;
		//save currenct selected rect
		snapeGrid(x, y);
		_select_rect.left = _select_rect.right = x;
		_select_rect.top = _select_rect.bottom = y;
		return ;
	}

	if(PreSizing == curState)
	{
		curState = Sizing;
		ComponentInstance *cinst = ComponentInstance::FromHandler(_sei.hwnd);
		if(cinst == NULL){
			ReleaseCapture();
			curState = Normal;
			return ;
		}
		_move_size_flag = cinst->getBoundMask();
		::GetWindowRect(_sei.hwnd,&_size_old);
		DPRINT("Ready Sizing");
		return ;
	}

	//1. the mouse pointer on which container?
	ComponentInstance *cinst = NULL;
	ComponentInstance * container = testContainer(x, y, &cinst);

	if(container == NULL){
		ReleaseCapture();
		curState = Normal;
		return;
	}


	curContainer = container;
	if(cinst == NULL){	//2. mouse pointer on the space of container

		//if the mouse pointer is on a selected container
		if(container != baseInstance && isInSelectedList(container->getPreviewHandler()))
		{
			//Moving it
			cinst = container;
			curContainer = container->getParent();
			goto MOVING;
		}

		//begin selecting
		curState = Selecting;

		//cancel All the selection
		if(!selectedWnd.empty()){
				cancelSelectAll();

			insertSelect(curContainer->getPreviewHandler(), TRUE);
			//TODO notify parent, selection changed
			notifySelChanged();
			InvalidateRect();
		}
		else {
			//BUGFIXING:3564, when switch focus between container and mainframe, property refresh error.
			insertSelect(curContainer->getPreviewHandler(), TRUE);
			//TODO notify parent, selection changed
			notifySelChanged();
		}

		_select_rect.left = _select_rect.right = x;
		_select_rect.top = _select_rect.bottom = y;

		return;
	}

MOVING:
	{
		HWND hctrl = cinst->getPreviewHandler();
		curState = Moving;

		_accept_container = curContainer;

		_move_size_flag = cinst->getBoundMask();

		_select_rect.left = _select_rect.right = x;
		_select_rect.top = _select_rect.bottom = y;

		if(isInSelectedList(hctrl)){ //3. mouse pointer on the selected children
			//set hctrl as current selecting
			if(hctrl != curWnd){
				//changed the current selecting
				HDC hdc = GetClientDC();
				HWND hold = curWnd;
				curWnd = hctrl;
				notifySelChanged();

				if(::IsControl(hold))
					drawSelection(hdc, hold);
				// draw new ctrl
				drawSelection(hdc, hctrl);
				ReleaseDC(hdc);
			}
		}
		else{
			//4. mouse pointer on the unselected children
			//cancel current selection
			//set hctrl as new selected control
			if(!(select_key_state
						&&	::IsWindow(curWnd)
						&& ComponentInstance::FromHandler(curWnd)->getParent() == curContainer))
				cancelSelectAll();
			insertSelect(hctrl, TRUE);
			//DP("<<<<< selection size=%d", selectedWnd.size());
			notifySelChanged();
			InvalidateRect();
		}
	}

	//save old for undo redo
	_move_old_list.clear();
	for(int i=0; i< (int)selectedWnd.size(); i++)
	{
		RECT rc;
		::GetWindowRect(selectedWnd[i], &rc);
		_move_old_list.push_back(MAKELONG(rc.left, rc.top));
	}

	//process Message
	instanceProcessMouseMessage(MSG_LBUTTONDOWN, x, y, key_flag);

}

void EditUIPanel::onLButtonUp(int x, int y, DWORD key_flag)
{
	x += xpos;
	y += ypos;

	instanceProcessMouseMessage(MSG_LBUTTONUP, x, y, key_flag);
	flags &= ~RequestMouseMsg;

	ReleaseCapture();

	switch(curState){
	case Moving:
		//do nothing
		onMovingUp(x,y);
		break;
	case Selecting:
		onSelectingUp(x, y);
		break;
	case Sizing:
		onSizingUp(x,y);
		break;
	case Creating:
		onCreatingUp(x,y);
		break;
	}

	curState = Normal;
	_move_size_flag = 0;

	memset(&_select_rect, 0, sizeof(RECT));

	if(_hdc != (HDC)0)
	{
		ReleaseDC(_hdc);
		_hdc = (HDC)0;
	}
}



void EditUIPanel::onMouseMove(int x, int y, DWORD key_flag)
{
	x += xpos;
	y += ypos;

	//if(instanceProcessMouseMessage(MSG_MOUSEMOVE, x, y, key_flag))
	//	return ;

	if(flags & CreateControl){
		//set cursor
		SetCursor(GetSystemCursor(IDC_CROSS));
		if(curState != Creating)
			return;
	}

	//DP("curState=%d", curState);

	if(curState == Normal || curState == PreSizing)
	{
		if(getEditInfo(x, y)){
			SetCursor(_sei.hcur);
			curState = PreSizing;
			DPRINT("PreSizing: hwnd=%p, Anchor=%X", _sei.hwnd, _sei.anchor);
		}
		else
		{
			curState = Normal;
		}
		return ;
	}

	if((curState == Selecting || curState == Creating) && _hdc == (HDC)0)
	{
		RECT rt;
		int cx=0, cy=0;
		_hdc = GetClientDC();
		SetRasterOperation(_hdc, ROP_XOR);
		SetPenColor(_hdc, COLOR_lightgray);
		SetPenType(_hdc, PT_ON_OFF_DASH);

		if(curContainer == NULL)
			curContainer = baseInstance;

		::GetClientRect(curContainer->getPreviewHandler(), &rt);
		//get offset
		::ClientToScreen(curContainer->getPreviewHandler(), &cx, &cy);
		::ScreenToWindow(hwndTopMost, &cx, &cy);
		cx -= xpos;
		cy -= ypos;
		OffsetRect(&rt, cx, cy);
		SelectClipRect(_hdc, &rt);
	}

	//
	ScreenToClient(&x, &y);
	switch(curState)
	{
	case Moving:
		onMovingMove(x, y);
		break;
	case Sizing:
		onSizingMove(x, y);
		break;
	case Creating:
		onCreatingMove(x, y);
		break;
	case Selecting:
		onSelectingMove(x, y);
		break;
	}
}

void EditUIPanel::mainWndToClient(HWND hwnd, int &x, int &y)
{
	if(hwnd == hwndTopMost){
		::WindowToClient(hwnd, &x, &y);
	}
	else
	{
		int clientx = 0;
		int clienty = 0;
		::ClientToScreen(hwnd, &clientx, &clienty);
		::WindowToScreen(hwndTopMost, &x, &y);
		x -= clientx;
		y -= clienty;
	}


}

void EditUIPanel::clientToMainWnd(HWND hwnd,int &x, int &y)
{
	if(hwnd == hwndTopMost)
	{
		::ClientToWindow(hwnd, &x, &y);
	}
	else
	{
		int mainx = 0;
		int mainy = 0;
		::ClientToScreen(hwnd, &x, &y);
		::WindowToScreen(hwndTopMost, &mainx, &mainy);
		x -= mainx;
		y -= mainy;
	}
}

void EditUIPanel::clientToMainWnd(HWND hwnd,RECT &rt)
{
	int x = rt.left;
	int y = rt.top;
	clientToMainWnd(hwnd, x, y);
	rt.right = x + RECTW(rt);
	rt.bottom = y + RECTH(rt);
	rt.left = x;
	rt.top = y;
}

/////////////////////////////////
static BOOL RectMostInRect(const RECT* rc1, const RECT* rc2)
{
	//if 2/3 of rc1 in rc2, it return TRUE
	if(rc1 == NULL || rc2 == NULL)
		return FALSE;

	RECT rt;
	//get the intersect rect
	if(!IntersectRect(&rt, rc1, rc2))
		return FALSE;

	//DP("RectMoseInRect:"RECT_FROMAT, EXPEND_RECT(rt));

	//get area of rt
	int area = RECTW(rt)*RECTH(rt);

	//get area of rc2
	int area_rc1 = RECTWP(rc1)*RECTHP(rc1);

	//if area >= area_rc2 *(2/3)

	return area*3 >= area_rc1 * 2;

}
//////////////////////////////////
//////////////////////////
void EditUIPanel::onMovingMove(int x, int y)
{
	if(!(_move_size_flag & (ComponentInstance::BOUND_MASK_X|ComponentInstance::BOUND_MASK_Y)))
		return;

	if(_select_rect.left == x || _select_rect.top == y)
		return ;

	snapeGrid(x, y);

	//1. Move All selected Controls
	int xoff = x  -_select_rect.left;
	int yoff = y  - _select_rect.top;
	//for(vector<HWND>::iterator it = selectedWnd.begin();
	//	it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		RECT rt;
		HWND hwnd = selectedWnd[i];
		int wtmp, htmp;
		int xtmp, ytmp;
		ComponentInstance* cinst =ComponentInstance::FromHandler(hwnd);
		::GetWindowRect(hwnd, &rt);
		wtmp = RECTW(rt);
		htmp = RECTH(rt);
		xtmp = rt.left + xoff;
		ytmp = rt.top + yoff;
		::MoveWindow(hwnd, xtmp, ytmp, wtmp, htmp,FALSE);
		if(cinst){
			cinst->setLocation(xtmp, ytmp);
			DP("location:%d,%d",xtmp, ytmp);
		}
	}
	//TODO notify parent
	notifyBoundChanged(TRUE,FALSE);

	//2. Get Current accept Container
	// curWnd is wihch container, the All selection control in which container
	{
		RECT rtCur;
		RECT rtContainer;
		ComponentInstance * old_container;
		HWND hcon;

		::GetWindowRect(curWnd, &rtCur);
		clientToMainWnd(::GetParent(curWnd), rtCur);

		hcon = _accept_container->getPreviewHandler();
		::GetClientRect(hcon, &rtContainer);
		clientToMainWnd(hcon, rtContainer);

		//DP("onMovingMove: rtCur "RECT_FROMAT "; rtContainer " RECT_FROMAT , EXPEND_RECT(rtCur), EXPEND_RECT(rtContainer));

		if(RectMostInRect(&rtCur, &rtContainer)){
			old_container = _accept_container;
		}
		else{
			old_container= baseInstance;
		}

		ComponentInstance *container = old_container->getChildren();
		while(container){
			if(isInSelectedList(container->getPreviewHandler()))
			{
				container = container->getNext();
				continue;
			}

			PageWindowInstance * pageContainer=  dynamic_cast<PageWindowInstance*>(container);
			if(pageContainer){
				container = pageContainer->getActiveInstance();
				if(container == NULL)
				{
					container = pageContainer->getNext();
					continue;
				}
			}

			hcon = container->getPreviewHandler();
			//BUGFIXING:3566, add !isInSelectedList(hcon)
			if(hcon != curWnd && container->isContainer()
					&& ::IsWindowVisible(hcon))
			{
				::GetClientRect(hcon, &rtContainer);
				clientToMainWnd(hcon, rtContainer);
				if(RectMostInRect(&rtCur, &rtContainer))
				{
					//printf( "onMovingMove: container=%p rtContainer \n" RECT_FROMAT, container , EXPEND_RECT(rtContainer));
					//push the into container
					old_container = container;
					container = container->getChildren();
					continue;
				}
			}
			if(pageContainer)
				container = pageContainer->getNext();
			else
				container = container->getNext();
		}
		_accept_container = old_container;
	}

	//update window
	_select_rect.left = x;
	_select_rect.top = y;
	flags |=UpdatePreviewWindow;
	InvalidateRect();
}

void EditUIPanel::onMovingUp(int x, int y)
{
	if(!(_move_size_flag & (ComponentInstance::BOUND_MASK_X|ComponentInstance::BOUND_MASK_Y))){
		_accept_container = NULL;
		return;
	}

	BOOL bUndoRedo = TRUE;
	ComponentInstance *oldContainer = curContainer;
	memset(&_select_rect, 0, sizeof(_select_rect));

	if(flags & AutoSnapeGrid)
	{
		//adjust the window
		//for(vector<HWND>::iterator it = selectedWnd.begin();
		//	it != selectedWnd.end(); ++it)
		for(int i=0; i<(int)selectedWnd.size(); i++)
		{
			RECT rt;
			HWND hwnd = selectedWnd[i];
			int xtmp, ytmp;
			::GetWindowRect(hwnd, &rt);
			xtmp = rt.left;
			ytmp = rt.top;
			snapeGrid(xtmp, ytmp);
			if(xtmp != rt.left || ytmp != rt.top){
				ComponentInstance *cinst = ComponentInstance::FromHandler(hwnd);
				if(cinst)
					cinst->setLocation(xtmp, ytmp);
				::MoveWindow(hwnd, xtmp, ytmp, RECTW(rt), RECTH(rt), TRUE);
				notifyBoundChanged(TRUE,FALSE, hwnd);
				flags |= UpdatePreviewWindow;
			}
		}
	}

	//test need undoredo
	if(_accept_container == curContainer)
	{
		DWORD v = _move_old_list[0];
		int x = LOSWORD(v);
		int y = HISWORD(v);
		RECT rc;
		::GetWindowRect(selectedWnd[0],&rc);
		if(isSnapeGrid()){
			if(ABS(rc.left-x)<GRID_SIZE && ABS(rc.top-y)<GRID_SIZE)
				bUndoRedo = FALSE;
		}
		else
		{
			if(rc.left == x && rc.top == y)
				bUndoRedo = FALSE;
		}
	}

	if(_accept_container && _accept_container != curContainer)
	{
		//get offset of curContainer;
		int xoff = 0;
		int yoff = 0;
		{
			int tempx = 0, tempy = 0;
			clientToMainWnd(curContainer->getPreviewHandler(), tempx, tempy);
			clientToMainWnd(_accept_container->getPreviewHandler(), xoff, yoff);
			xoff -= tempx;
			yoff -= tempy;
		}
		//change the parent
		//remove all selection form curContainer
		for(int i=0; i<(int)selectedWnd.size(); i++)
		{
			int x, y;
			//remove for curcontainer
			ComponentInstance *cinst = ComponentInstance::FromHandler(selectedWnd[i]);
			cinst->getLocation(x,y);

			curContainer->remove(cinst);
			//offset the x, y
			x -= xoff;
			y -= yoff;
			snapeGrid(x, y);
			cinst->setLocation(x, y);
			//insert into new _accept_container
			_accept_container->insert(cinst);
			//HWND have been updated
			selectedWnd[i] = cinst->getPreviewHandler();
		}

		sendEvent(EDITUIPANEL_CHANGE_PARENT, (DWORD)&selectedWnd, (DWORD)curContainer);

		curContainer = _accept_container;
		_accept_container = NULL;
		//TODO notify parent
		notifyBoundChanged(TRUE,FALSE);
		//update
		flags |= UpdatePreviewWindow;
	}
	else
	{
		_accept_container = NULL;
	}

	if(bUndoRedo)
		pushMoveUndoRedoCommand(oldContainer);

	if(flags & UpdatePreviewWindow)
		InvalidateRect();

	setPropertyChanged();

}

///////
void EditUIPanel::onSelectingMove(int x, int y)
{
	if(_select_rect.left != _select_rect.right || _select_rect.top != _select_rect.bottom)
		Rectangle(_hdc,_select_rect.left - xpos, _select_rect.top - ypos, _select_rect.right - xpos, _select_rect.bottom - ypos);

	//DPRINT( "_hdc=%p,rect:" RECT_FROMAT, _hdc, EXPEND_RECT(_select_rect));

	_select_rect.right = x;
	_select_rect.bottom = y;
	if(_select_rect.left != _select_rect.right || _select_rect.top != _select_rect.bottom)
		Rectangle(_hdc,_select_rect.left - xpos, _select_rect.top - ypos, _select_rect.right - xpos, _select_rect.bottom - ypos);

	//DPRINT( "rect:" RECT_FROMAT, EXPEND_RECT(_select_rect));
}

void EditUIPanel::onSelectingUp(int x, int y)
{
	//clear the select box
	if(_select_rect.left != _select_rect.right || _select_rect.top != _select_rect.bottom)
				Rectangle(_hdc,_select_rect.left - xpos, _select_rect.top - ypos, _select_rect.right - xpos, _select_rect.bottom - ypos);

	NormalizeRect(&_select_rect);


	//translate _select_rect to curConatiner's rect
	// _select_rect is base on hwndTopMost's Window's left corner
	HWND hcon = curContainer->getPreviewHandler();
	{
		int cx = 0, cy = 0;
		::ClientToScreen(hcon, &cx, &cy);
		::ScreenToWindow(hwndTopMost, &cx, &cy);
		OffsetRect(&_select_rect, -cx, -cy);
	}

	cancelSelectAll();

	//test which children in the selection
	ComponentInstance *cinst = curContainer->getChildren();
	while(cinst)
	{
		RECT rt;
		::GetWindowRect(cinst->getPreviewHandler(), &rt);
		if(DoesIntersect(&rt, &_select_rect))
		{
			insertSelect(cinst->getPreviewHandler());
		}
		cinst = cinst->getNext();
	}

	if(!selectedWnd.empty() && !isInSelectedList(curWnd))
	{
		curWnd = selectedWnd.front();
		notifySelChanged();
		//update
		InvalidateRect();
	}
	else //no children select, select container itself
	{
		insertSelect(curContainer->getPreviewHandler(), TRUE);
		if(curContainer != baseInstance)
		{
			InvalidateRect();
		}
	}
}

/////////////////
// set size of rect
BOOL EditUIPanel::setSizedRect(RECT *prt, int x, int y, DWORD anchor)
{
	BOOL bset = FALSE;
	if(anchor & AnchorLeft){
		if( x!= prt->left && x < prt->right){
			prt->left = x;
			bset = TRUE;
		}
	}
	else if(anchor & AnchorRight){
		if(x!= prt->right && x  > prt->left){
			prt->right = x;
			bset = TRUE;
		}
	}

	if(anchor & AnchorTop){
		if(y != prt->top && y < prt->bottom){
			prt->top = y;
			bset = TRUE;
		}
	}
	else if(anchor & AnchorBottom){
		if(y!= prt->bottom && y > prt->top){
			prt->bottom = y;
			bset = TRUE;
		}
	}

	return bset;
}

void EditUIPanel::onSizingMove(int x, int y)
{
	if(!(_move_size_flag & (ComponentInstance::BOUND_MASK_WIDTH|ComponentInstance::BOUND_MASK_HEIGHT)))
			return;


	RECT rt;
	HWND hcon;

	//clientToMainWnd(x, y);
	snapeGrid(x, y);

	::GetWindowRect(_sei.hwnd, &rt);
	if(_sei.hwnd == hwndTopMost){ //change the topmose instance's sizing
		OffsetRect(&rt, -rt.left, -rt.top);
		hcon = HWND_DESKTOP;
	}
	else{
		RECT rtMain;
		::GetWindowRect(hwndTopMost, &rtMain);
		x += rtMain.left;
		y += rtMain.top;
		ComponentInstance *cinst = ComponentInstance::FromHandler(_sei.hwnd);
		if(cinst == NULL || (cinst = cinst->getParent())==NULL)
			return ;
		hcon = cinst->getPreviewHandler();
		::ScreenToClient(hcon, &x, &y);
	}

	if(setSizedRect(&rt, x, y, _sei.anchor))
	{
		if(RECTW(rt) > RECTW(g_rcScr))
			rt.right =  rt.left + RECTW(g_rcScr);
		if(RECTH(rt) > RECTH(g_rcScr))
					rt.bottom = rt.top + RECTH(g_rcScr);

		if(_sei.hwnd == hwndTopMost)
		{
			rt.left = -rt.right; rt.top = -rt.bottom;
			rt.right = rt.bottom = 0;
		}
		::MoveWindow(_sei.hwnd, rt.left, rt.top, RECTW(rt), RECTH(rt), FALSE);

		ComponentInstance *cinst = ComponentInstance::FromHandler(_sei.hwnd);

		//set the cinsit
		//DP("---set before:%d,%d", RECTW(rt), RECTH(rt));
		if(_sei.anchor&(AnchorLeft|AnchorTop))
		{
			cinst->setLocation(rt.left, rt.top);
			//fix x<->width and y<->height sync bug
			cinst->setSize(RECTW(rt), RECTH(rt));
		}
		else if(_sei.anchor&(AnchorRight|AnchorBottom))
			cinst->setSize(RECTW(rt), RECTH(rt));
		/*{
			int cx, cy;
			cinst->getSize(cx, cy);
			DP("---set end cinst %p(hwnd:%p):%d,%d",cinst, _sei.hwnd, cx,cy);
		}*/
		//TODO Notify parent
		notifyBoundChanged(_sei.anchor&(AnchorLeft|AnchorTop),_sei.anchor&(AnchorRight|AnchorBottom), _sei.hwnd);

	//	if(_sei.hwnd == hwndTopMost)
	//		updateScrollbar();
		flags |= UpdatePreviewWindow;
		InvalidateRect();
	}
}

void EditUIPanel::onSizingUp(int x, int y)
{
	if(!(_move_size_flag & (ComponentInstance::BOUND_MASK_WIDTH|ComponentInstance::BOUND_MASK_HEIGHT)))
			return;
/*	{
		int cx, cy;
		ComponentInstance* cinst = ComponentInstance::FromHandler(_sei.hwnd);
		cinst->getSize(cx, cy);
		DP("---set sizing up(%p hwnd=%p):%d,%d",cinst, _sei.hwnd, cx,cy);
	}*/
	//adjust the size
	if(flags & AutoSnapeGrid)
	{
		//adjust the window
		//for(vector<HWND>::iterator it = selectedWnd.begin();
		//	it != selectedWnd.end(); ++it)
		for(int i=0; i<(int)selectedWnd.size(); i++)
		{
			RECT rt;
			BOOL locationChanged = FALSE;
			BOOL sizeChanged = FALSE;
			HWND hwnd = selectedWnd[i];
			int left, top, right, bottom;
			::GetWindowRect(hwnd, &rt);
			left   = rt.left;
			top    = rt.top;
			right  = rt.right;
			bottom = rt.bottom;
			snapeGrid(left, top);
			snapeGrid(right, bottom);
			ComponentInstance *cinst = ComponentInstance::FromHandler(hwnd);
			if(!cinst)
				return;

			if(left != rt.left || top != rt.top){
				cinst->setLocation(left, top);
				locationChanged = TRUE;
			}
			if(right != rt.right || bottom != rt.bottom){
				cinst->setSize(right - left, bottom - top);
				sizeChanged = TRUE;
			}
			if(locationChanged || sizeChanged)
			{
				::MoveWindow(hwnd, left, top, right - left, bottom - top, TRUE);
				notifyBoundChanged(locationChanged,sizeChanged, _sei.hwnd);
				flags |= UpdatePreviewWindow;
			}
		}
	}

	//undo redo support
	pushSizeUndoRedoCommand(ComponentInstance::FromHandler(_sei.hwnd));

	_sei.hwnd = HWND_INVALID;
	_sei.anchor = 0;
	SetCursor(GetSystemCursor(IDC_ARROW));

	if(flags & UpdatePreviewWindow)
		InvalidateRect();

	setPropertyChanged();
}
/////
/*
void EditUIPanel::onCreatingMove(int x, int y)
{

}
*/
void EditUIPanel::onCreatingUp(int x, int y)
{
	if(!(flags & CreateControlAlways))
			SetCursor(GetSystemCursor(IDC_ARROW));

	if(_select_rect.left != _select_rect.right || _select_rect.top != _select_rect.bottom)
		Rectangle(_hdc,_select_rect.left  - xpos, _select_rect.top - ypos, _select_rect.right - xpos, _select_rect.bottom - ypos);

	NormalizeRect(&_select_rect);

	if(curClassName.length() > 0)
	{
		//create instance from name
		ComponentInstance *cinst = ComponentInstance::createFromClassName(NULL,curClassName.c_str());

		if(cinst == NULL){
			LOG_WARNING("Cannot Create ComponentInstance(name=\"%s\"", curClassName.c_str());
			return ;
		}

		//set Bound of this instance
		//translate _size_rect to screen
		//translate _select_rect to curConatiner's rect
		HWND hcon = curContainer->getPreviewHandler();
		{
			int cx = 0, cy = 0;
			clientToMainWnd(hcon, cx, cy);
			OffsetRect(&_select_rect, -cx, -cy);
		}

		int width, height, defw, defh;
		cinst->getSize(defw, defh);
		width = RECTW(_select_rect);
		height = RECTH(_select_rect);

		if(width <= 0)
			width = defw;

		if(height <= 0)
			height = defh;

		width = width < MIN_WIDTH?MIN_WIDTH: width;
		height = height < MIN_HEIGHT?MIN_HEIGHT:height;

		snapeGrid(_select_rect.left, _select_rect.top);
		snapeGrid(width, height);

		cinst->setSize(width, height);
		cinst->setLocation(_select_rect.left, _select_rect.top);

		if(!(cinst->getFieldAttr(ComponentInstance::PropText) &(Class::FIELD_ATTR_FIXED|Class::FIELD_ATTR_HIDE)))
		{
			//set the instance name
			char szText[100];
			sprintf(szText, "%s%d",curClassName.c_str(),cinst->getClass()->classRefCount());
			cinst->setCaption(szText);
		}

		//add into the resource, and cinst will get a valid ID
		addResource(cinst);

		//insert into curcontainer, cinst must have insert into resource,
		//so that, it can get a valid ID
		curContainer->insert(cinst);

		//create the instance
		HWND hctrl = cinst->createPreviewWindow();

		const char *def_f = getDefaultClientFont();
		if(def_f)
			setPreviewWindowClientFont((LOGFONT*)LoadResource(def_f, RES_TYPE_FONT, 0L), cinst);
		//cancel all the selection
		cancelSelectAll();
		insertSelect(hctrl, TRUE);

		//TODO notify
		notifySelChanged();

		if(!(CreateControlAlways & flags)){
			flags &= ~CreateControl;
			//TODO Change cursor
			sendEvent(EDITUIPANEL_FINISH_CREATE);
		}
		sendEvent(EDITUIPANEL_INSTANCE_ADDED,(DWORD)cinst);
		//update window
		flags |= UpdatePreviewWindow;
		InvalidateRect();

		setPropertyChanged();

		//undo redo
		pushCreateDeleteUndoRedoCommand(TRUE);
	}
}

////////////////////////////
/*
void EditUIPanel::onMutilSelect(int x, int y)
{
	ComponentInstance *cinst = curContainer;
	if(cinst == NULL)
		cinst = baseInstance;
	if(!cinst)
		return;

	for(cinst = cinst->getChildren(); cinst; cinst = cinst->getNext())
	{
		if(cinst->hittest(x, y)!=ComponentInstance::OUT)
		{
			if(insertSelect(cinst->getPreviewHandler(), TRUE))
			{
				notifySelChanged();
				InvalidateRect();
				return;
			}
		}
	}
}*/
//////////////////////////

void EditUIPanel::onLButtonDblClk(int x, int y, DWORD key_flag)
{

}

/*
 *   define the prop menu
 */
void EditUIPanel::onRButtonUp(int x, int y, DWORD key_flag)
{
	int oldx = x;
	int oldy = y;
	x += xpos;
	y += ypos;
	int align_state = selectedWnd.size()>1?0:MFS_DISABLED;

	//get the current instance
	ComponentInstance *cinst = ComponentInstance::FromHandler(curWnd);
	if(cinst == NULL)
		cinst = baseInstance;

	if(cinst == NULL)
		return ;

    mapex<int, int>idsets;
    int mask_state;

    PageWindowInstance* pageInst = dynamic_cast<PageWindowInstance*>(cinst);
    if (pageInst) {
    	int pageCount = pageInst->getPageCount();
    	mask_state = MAP_MASK_STATE(MIIM_ID | MIIM_TYPE | MIIM_DATA, 0);
        idsets[ResEditor::UI_MENUCMD_ADDPAGE] = mask_state;
        if (pageCount > 0)
        	idsets[ResEditor::UI_MENUCMD_DELPAGE] = mask_state;
        else
        	idsets[ResEditor::UI_MENUCMD_DELPAGE] =
					MAP_MASK_STATE(MIIM_ID | MIIM_TYPE | MIIM_DATA, MFS_DISABLED);
    }

    if (!(selectedWnd.empty())) {
        idsets[ResEditor::GBC_COPY]     = MAP_MASK_STATE(0, 0);
        idsets[ResEditor::GBC_CUT]      = MAP_MASK_STATE(0, 0);
        idsets[ResEditor::GBC_DELETE]   = MAP_MASK_STATE(0, 0);
    }
    else {
        idsets[ResEditor::GBC_COPY]     = MAP_MASK_STATE(0, MFS_DISABLED);
        idsets[ResEditor::GBC_CUT]      = MAP_MASK_STATE(0, MFS_DISABLED);
        idsets[ResEditor::GBC_DELETE]   = MAP_MASK_STATE(0, MFS_DISABLED);
    }

    if (Instance::paste())
    	idsets[ResEditor::GBC_PASTE]    = MAP_MASK_STATE(0, 0);
    else
    	idsets[ResEditor::GBC_PASTE]    = MAP_MASK_STATE(0, MFS_DISABLED);

    mask_state = MAP_MASK_STATE(MIIM_ID | MIIM_TYPE | MIIM_STATE, align_state);
    for (unsigned int i = 0; i < 10; i++) {
    	idsets[ResEditor::UI_MENUCMD_LEFT + i] = mask_state;
    }

    HMENU hMenu = g_env->createPopMenuFromConfig(ResEditor::UI_PROP_POPMENU, idsets);

    //show pop menu
	ClientToScreen(&oldx, &oldy);
	TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_DESTROY, oldx, oldy,m_hWnd);
}


void EditUIPanel::drawAcceptContainer(HDC hdc, HWND hwnd)
{
	if(_accept_container && _accept_container != curContainer && _accept_container != baseInstance)
	{
		HWND hcon = _accept_container->getPreviewHandler();

		RECT rtCon;
		::GetWindowRect(hcon,&rtCon);
		clientToMainWnd(::GetParent(hcon), rtCon);

		//Draw Selection
		rtCon.left -= 1;
		rtCon.top -= 1;
		rtCon.right += 1;
		rtCon.bottom += 1;
		Rectangle(hdc, rtCon.left, rtCon.top, rtCon.right, rtCon.bottom);
	}
}

void EditUIPanel::notifySelChanged(){
	BOOL bEnable = ::IsWindow(curWnd) && curWnd != hwndTopMost;
	if(bEnable){
		sendEvent(EDITUIPANEL_SELCHANGE, (DWORD)(ComponentInstance::FromHandler(curWnd)), (DWORD)observer);
	}
	else
	{
		sendEvent(EDITUIPANEL_SELCHANGE, (DWORD)baseInstance,(DWORD)observer);
	}

	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_COPY,bEnable);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_CUT,bEnable);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_DELETE,bEnable);

	int selsize = selectedWnd.size();
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_LEFT,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_RIGHT,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_CENTER,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_TOP,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_BOTTOM,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_MIDDLE,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_SAMEWIDTH,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_SAMEHEIGHT,selsize>1);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_HSPREED,selsize>2);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::UI_MENUCMD_VSPREED,selsize>2);

}

void EditUIPanel::notifyBoundChanged(BOOL bLocation/* = TRUE*/, BOOL bSize/*=TRUE*/, HWND hwnd/* = HWND_INVALID*/)
{
	if(!bLocation && !bSize)
		return;

	if(hwnd == HWND_INVALID)
		hwnd = curWnd;

	ComponentInstance *inst = ComponentInstance::FromHandler(hwnd);
	if(inst == NULL)
		inst = baseInstance;

	DWORD bound = 0;
	if(bLocation)
		bound |= ComponentInstance::BOUND_MASK_X|ComponentInstance::BOUND_MASK_Y;
	if(bSize)
		bound |= ComponentInstance::BOUND_MASK_WIDTH|ComponentInstance::BOUND_MASK_HEIGHT;

	sendEvent(EDITUIPANEL_BOUND_CHANGE, (DWORD)(Instance*)inst, bound);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_SAVE,TRUE);
}

void EditUIPanel::onPaint(HDC hdc)
{

	if(!::IsWindow(hwndTopMost))
		return;

	HDC hSecondary = updatePreviewWindow();

	if(hSecondary)
	{
		RECT rt, rtClip;
		::GetWindowRect(hwndTopMost, &rt);
		RECT rtClient;
		GetClientRect(&rtClient);
		rtClip.left = rtClip.top = 0;
		rtClip.right = RECTW(rt) + ANCHOR_SIZE + 1;
		rtClip.bottom = RECTH(rt)+ ANCHOR_SIZE + 1;
		IntersectRect(&rtClip, &rtClip, &rtClient);
		SelectClipRect(hdc, &rtClip);
		if(!(PreviewMode & flags))
					//update Out line
					updateOutline();
		if(RECTW(rt)>xpos && RECTH(rt)>ypos){
			//printf("----\n");
			BitBlt(hSecondary, xpos, ypos, RECTW(rt)-xpos, RECTH(rt)-ypos, hdc, 0, 0, 0);
			//draw selection
			drawSelections(hdc);
		}

		rtClip.left = rtClip.top = 0;
		rtClip.right = RECTW(rt);
		rtClip.bottom = RECTH(rt);
		IntersectRect(&rtClip, &rtClip, &rtClient);
		SelectClipRect(hdc, &rtClip);

		//draw AcceptContainer Selection
		drawAcceptContainer(hdc, m_hWnd);

		drawInvisibleAndContainer(hdc,baseInstance);
	}
}

static void dash_hline(HDC hdc, int x0, int x1, int y,int size=3)
{
	while(x0 < x1)
	{
		MoveTo(hdc,x0,y);
		x0 += size;
		if(x0 > x1) x0 = x1;
		LineTo(hdc,x0, y);
		x0 += size;
	}
}
static void  dash_vline(HDC hdc, int x, int y0, int y1, int size=3)
{
	while(y0 < y1)
	{
		MoveTo(hdc,x,y0);
		y0 += size;
		if(y0 > y1) y0 = y1;
		LineTo(hdc,x, y0);
		y0 += size;
	}
}
static void frame_dashes_rect(HDC hdc, const RECT* rc, int size)
{
	dash_hline(hdc, rc->left, rc->right, rc->top, size);
	dash_hline(hdc, rc->left, rc->right, rc->bottom, size);
	dash_vline(hdc, rc->left, rc->top, rc->bottom, size);
	dash_vline(hdc, rc->right, rc->top, rc->bottom, size);
}

void EditUIPanel::drawInvisibleAndContainer(HDC hdc, ComponentInstance* inst)
{
	if(!hdc || !inst)
		return ;

	if(inst == baseInstance)
	{
		SetRasterOperation(hdc, ROP_XOR);
		SetPenColor(hdc, COLOR_lightwhite);
	}

	HWND hParent = inst->getPreviewHandler();

	for(ComponentInstance *cinst = inst->getChildren();
		cinst; cinst = cinst->getNext())
	{
		RECT rc;
		HWND hwnd = cinst->getPreviewHandler();
		PageWindowInstance *pinst;
		if(!::IsWindow(hwnd))
			continue;

		if(!(::GetWindowStyle(hwnd) & WS_VISIBLE))
		{
			::GetWindowRect(hwnd, &rc);

			clientToMainWnd(hParent, rc);

			rc.left --;
			rc.top --;
			rc.right ++;
			rc.bottom ++;
			OffsetRect(&rc, -xpos, -ypos);
			frame_dashes_rect(hdc, &rc, 3);
			continue;
		}

		if((pinst=dynamic_cast<PageWindowInstance*>(cinst)) != NULL)
		{
			drawInvisibleAndContainer(hdc,pinst->getActiveInstance());
			continue;
		}

		if(cinst->isContainer())
		{
				::GetWindowRect(hwnd, &rc);

				clientToMainWnd(hParent, rc);

				rc.left --;
				rc.top --;
				rc.right ++;
				rc.bottom ++;
				OffsetRect(&rc, -xpos, -ypos);
				frame_dashes_rect(hdc, &rc, 8);
		}


		drawInvisibleAndContainer(hdc, cinst);

	}

}

BOOL EditUIPanel::onEraseBkgnd(HDC hdc,PRECT clip)
{
	if(baseInstance && ::IsWindow(hwndTopMost))
	{
		HDC hdct = GetClientDC();
		RECT rt;
		RECT client;
		SIZE szScreen;
		::GetWindowRect(hwndTopMost, &rt);
		szScreen = getScreenSize();
		rt.right = RECTW(rt) - xpos;
		rt.bottom = RECTH(rt) - ypos;

		szScreen.cx -= xpos;
		szScreen.cy -= ypos;

		rt.left = -xpos;
		rt.top = -ypos;
		GetClientRect(&client);
		int color = GetWindowBkColor();

		SetBrushColor(hdct, color);

		if(rt.right < szScreen.cx)
			FillBox(hdct, rt.right, 0, szScreen.cx - rt.right,  szScreen.cy);

		if(rt.bottom < szScreen.cy)
			FillBox(hdct, 0, rt.bottom, szScreen.cx, szScreen.cy - rt.bottom);

		//FillBox(hdct,  rt.right, rt.bottom, client.right - rt.right, client.bottom - rt.bottom);

		//draw screen
		if(szScreen.cx < RECTW(client) || szScreen.cy < RECTH(client))
		{
			gal_pixel color = RGB2Pixel(hdct,80,80,80);
			SetBrushColor(hdct, color);
			int x = rt.right > szScreen.cx?rt.right:szScreen.cx;
			if(szScreen.cx < RECTW(client)){
				FillBox(hdct, x, 0, client.right - x,RECTH(client));
			}

			if(szScreen.cy < RECTH(client)){
				int y = rt.bottom > szScreen.cy ? rt.bottom : szScreen.cy;
				FillBox(hdct, 0, y, x, client.bottom - y);
			}

			if(rt.right < szScreen.cx && rt.bottom > szScreen.cy){
				FillBox(hdct, rt.right, szScreen.cy, szScreen.cx - rt.right, rt.bottom - szScreen.cy);
			}
			else if(rt.right > szScreen.cx && rt.bottom < szScreen.cy)
			{
				FillBox(hdct, szScreen.cx, rt.bottom, rt.right - szScreen.cx , szScreen.cy - rt.bottom);
			}

		}

		SetBkMode(hdct,BM_TRANSPARENT);
		SetTextColor(hdct,RGB2Pixel(hdct,200,200,200));
		//draw screensize
		char szText[100];
		sprintf(szText,"screen size:%dx%d",szScreen.cx+xpos, szScreen.cy + ypos);
		SIZE sz;
		GetTextExtent(hdct, szText, -1, &sz);
		TextOut(hdct, szScreen.cx - sz.cx, szScreen.cy, szText);

		ReleaseDC(hdct);
		return TRUE;
	}
	return FALSE;
}

void EditUIPanel::updateOutline(BOOL bUpdatePrevWindow/*=FALSE*/, BOOL bsendEvent /*=TRUE*/)
{
	HDC hSecondary ;
	if(bUpdatePrevWindow){
		flags |= UpdatePreviewWindow;
		hSecondary = updatePreviewWindow();
	}
	else
		hSecondary = ::GetSecondaryDC(hwndTopMost);

	if(hSecondary)
	{
		HDC hdc = CreateMemDCFromBitmap(hSecondary,&bmpOutline);
		if(hdc == (HDC)0)
			return;
		RECT rt;
		::GetWindowRect(hwndTopMost, &rt);
		//save to dc
		StretchBlt(hSecondary, 0, 0,RECTW(rt), RECTH(rt), hdc, 0, 0, bmpOutline.bmWidth, bmpOutline.bmHeight,0);

		if(isHidden())
		{
			//draw text info
			RECT rt = {0,bmpOutline.bmHeight*20/100 , bmpOutline.bmWidth, bmpOutline.bmHeight};
			const char* str = "Hidden!\nDouble Click Me!";
			DrawText(hdc, str,-1, &rt, DT_CENTER|DT_CALCRECT);
/*			int height = RECTH(rt);
			rt.top = (bmpOutline.bmHeight - height)/2;
			rt.bottom = rt.top + height;
			DrawText(hdc, str,-1, &rt, DT_CENTER);
	*/	}

		DeleteMemDC(hdc);

		//send event
		if(bsendEvent)
			sendEvent(EDITUIPANEL_UPDATE, (DWORD)this);
	}
}

static BOOL MyUpdateHVScroll(HWND hwnd,
	const RECT* rtdoc,
	const RECT* rtview,
	int *x_offset,
	int *y_offset,
	BOOL bAutoUpdate)
{
	SCROLLINFO hsi, vsi;
	int h_view_width;
	int h_doc_width;
	int v_view_height;
	int v_doc_height;

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


	INIT_SCROLLINFO(hsi);
	INIT_SCROLLINFO(vsi);

	set_scroll_info(hwnd, SB_HORZ, &hsi, h_doc_width, h_view_width,bAutoUpdate);
	set_scroll_info(hwnd, SB_VERT, &vsi, v_doc_height, v_view_height,bAutoUpdate);

	if(x_offset)
		*x_offset = hsi.nPos;
	if(y_offset)
		*y_offset = vsi.nPos;

	return TRUE;
}

void EditUIPanel::updateScrollbar(BOOL bUpdate)
{
	ShowScrollBar(SB_HORZ, TRUE);
	ShowScrollBar(SB_VERT, TRUE);
	//if(baseInstance){
	//	if(::IsMainWindow(hwndTopMost))
	//	{
			RECT rtdoc;
			RECT rtview;
			SIZE szScreen = getScreenSize();
			//::GetWindowRect(hwndTopMost, &rtdoc);
			//rtdoc.right += ANCHOR_SIZE;
			//rtdoc.bottom += ANCHOR_SIZE;
			FONTMETRICS fontMetrics;
			::GetFontMetrics(::GetWindowFont(GetHandle()),&fontMetrics);
			rtdoc.left = 0;
			rtdoc.top = 0;
			rtdoc.right = szScreen.cx + ANCHOR_SIZE * 2;
			rtdoc.bottom = szScreen.cy + fontMetrics.font_height + ANCHOR_SIZE * 2;
			GetClientRect(&rtview);
			MyUpdateHVScroll(m_hWnd,&rtdoc, &rtview, &xpos, &ypos, bUpdate && IsVisible());
			return ;
	//	}
	//}
}

void EditUIPanel::onScroll(int hs_nc, int pos, int sb)
{
	int newpos = ProcessScrollMessage(m_hWnd, sb, hs_nc, pos, 10, TRUE);
	if(newpos < 0)
		return;
	if(sb==SB_HORZ)
		xpos = newpos;
	else
		ypos = newpos;
	InvalidateRect();
}

// ComponetInstance Interface Implements
static int DoubleBufferProc(HWND hwnd, HDC private_dc, HDC real_dc,  const RECT* update_rc, const RECT* real_rc, const RECT* main_rc)
{
	//printf("update_rc={%d,%d,%d,%d}\n", update_rc->left, update_rc->top, update_rc->right, update_rc->bottom);
	//printf("real_rc={%d,%d,%d,%d}\n", real_rc->left, real_rc->top, real_rc->right, real_rc->bottom);
	BitBlt(private_dc,update_rc->left, update_rc->top, RECTWP(update_rc), RECTHP(update_rc), real_dc, real_rc->left, real_rc->top,0);
	return 0;
}
BOOL EditUIPanel::open(const char* xmlFile)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	if(xmlFile == NULL){
		LOG_WARNING("Canot open xmlFile(is NULL)");
		return FALSE;
	}

	doc = xmlParseFile(xmlFile);
	if(doc == NULL){
		LOG_WARNING("Cannot open \"%s\"", xmlFile);
		return FALSE;
	}

	node = xmlDocGetRootElement(doc);

	//open instance
	baseInstance = ComponentInstance::createFromXmlNode(node);

	xmlFreeDoc(doc);

	if(baseInstance == NULL){
		LOG_WARNING("Create Instance From \"%s\" Field", xmlFile);
		return FALSE;
	}

	//add resource
	addResource(baseInstance, xmlFile);

	hwndTopMost = HWND_NULL;
	hwndTopMost = baseInstance->createPreviewWindow();

	if(!::IsWindow(hwndTopMost))
	{
		LOG_WARNING("Get The Preview form Instance Failure(File:\"%s\")", xmlFile);
		delete baseInstance;
		baseInstance = NULL;
		return FALSE;
	}

	const char *def_f = getDefaultClientFont();
	if(def_f)
		setDefClientFont((LOGFONT*)LoadResource(def_f, RES_TYPE_FONT, 0L));

	curContainer = baseInstance;
	cancelSelectAll();
	insertSelect(hwndTopMost, TRUE);
	curState = Normal;

	//set renderer
	WINDOWINFO *win_info = (WINDOWINFO*)::GetWindowInfo(hwndTopMost);
	dumpWindowRenderer(win_info->we_rdr);
	win_info->we_rdr = editor_win_rdr;
	//set Secodery data
	HDC hdc_secd = ::GetSecondaryDC(hwndTopMost);
	::SetSecondaryDC(hwndTopMost, hdc_secd, DoubleBufferProc);

	RECT rt;
	::GetWindowRect(hwndTopMost, &rt);
	::MoveWindow(hwndTopMost, -RECTW(rt), -RECTH(rt), RECTW(rt), RECTH(rt), FALSE);

	::ShowWindow(hwndTopMost, SW_SHOW);

	strXmlFile = xmlFile;

	baseInstance->setSnapGrid(isSnapeGrid());

	//clear state
	clearPropertyChanged();
	clearSourceChanged();


	if(isHidden())
	{

		hideInstances();
		return TRUE;
	}

	notifySelChanged();

	sendEvent(EDITUIPANEL_INSTANCE_ADDED,(DWORD)baseInstance);

	flags |= UpdatePreviewWindow;
	InvalidateRect();
	sendEvent(UIMENUITEM_ENABLE, ResEditor::GBC_PASTE, FALSE);
	sendEvent(UIMENUITEM_ENABLE, ResEditor::UI_MENUCMD_REMOVE_CUR, TRUE);
	return TRUE;
}

void EditUIPanel::removeInstanceId(ResManager *resMgr,ComponentInstance *cinst)
{
	if(!cinst || !resMgr)
		return;

	//delete ref
	//cinst->removeRefReses();
	resMgr->removeRes(cinst->getID(), (DWORD)cinst);
	for(ComponentInstance* child = cinst->getChildren(); child; child = child->getNext())
	{
		removeInstanceId(resMgr, child);
	}
}

void EditUIPanel::close(BOOL bSaveInpdentXMLFile/*=TRUE*/)
{
	if(baseInstance)
	{

		if(bSaveInpdentXMLFile)
		{
			FileStreamStorage fss(strXmlFile.c_str());
			TextStream stream(&fss);

			//save instances
			baseInstance->saveIndependXML(&stream);
		}

			ResManager *resMgr = g_env->getResManager(NCSRT_UI);
			if (resMgr){
				removeInstanceId(resMgr, baseInstance);
		}

		sendEvent(EDITUIPANEL_INSTANCE_DELETED, (DWORD)baseInstance);
		baseInstance->release();
		baseInstance = NULL;
		disableSource();
		InvalidateRect();
	}
	sendEvent(UIMENUITEM_ENABLE, ResEditor::GBC_PASTE, FALSE);
}

HDC EditUIPanel::updatePreviewWindow()
{
	HDC hSecondary ;
	if(!baseInstance)
		return (HDC)0;

	if(PreviewMode & flags){
		return hdcPreviewWnd;
	}
	else{
		hSecondary = ::GetSecondaryDC(baseInstance->getPreviewHandler());
	}

	if(hSecondary == (HDC)0)
		return (HDC)0;

	if(UpdatePreviewWindow & flags)
	{
		baseInstance->updatePrevWindow();
		flags &= ~UpdatePreviewWindow;
	}

	return hSecondary;
}

BOOL EditUIPanel::saveXML(BOOL bForce)
{
	if(strXmlFile.length() <= 0 || baseInstance == NULL)
		return FALSE;

	//save the xml

	if(!bForce && (!isSourceChanged() && !isPropertyChanged() && isFileExist(strXmlFile.c_str())))
		return TRUE;

	FileStreamStorage fss(strXmlFile.c_str());
	TextStream stream(&fss);

	//save instances
	baseInstance->saveXMLToStream(&stream);
	return TRUE;
}

string EditUIPanel::saveBin(BinStream *bin)
{
	if(strXmlFile.length() <= 0 || baseInstance == NULL)
		return string("");

	const char* fileName = strrchr(strXmlFile.c_str(),'/');
	if(fileName == NULL)
		fileName = strrchr(strXmlFile.c_str(), '\\');

	if(fileName == NULL)
		fileName = strXmlFile.c_str();

	if(fileName == NULL)
		return string("");

	fileName ++;

	//save instance as bin
	baseInstance->saveBinToStream(bin);
    bin->seek(0, StreamStorage::seek_end);

	clearPropertyChanged();
	return string(fileName);
}

BOOL EditUIPanel::saveSource()
{
	if(strXmlFile.length() <= 0)
		return FALSE;

	//TODO: save source
	//ComponentInstance::saveSrouce("test.c",baseInstance,"./etc/uieditor/trans/ncs-ctmpl.lua", "ncs_trans");
	//create srouce file name
	char szFileName[256] = "/";

	getSourceFileName(szFileName+1);

	//get source file name
	string strSrcFile = g_env->getSourcePath();
	strSrcFile += szFileName;
	//get the templfile of source
	string strSrcFileTmpl = strSrcFile + ".tmpl";

	if(!isSourceChanged() && isFileExist(strSrcFile.c_str()))
		return TRUE;

	//get script file
	const char* strFile = g_env->getSysConfig("uieditor/trans/template","uieditor/trans/ncs-ctmpl.lua");
	string strScriptFile = g_env->getConfigFile(strFile);

	//get script function name
	//const char* scriptFunc = g_env->getSysConfig("uieditor/trans/transfunc","ncs_trans");

	ComponentInstance::saveSrouce(strSrcFileTmpl.c_str(), baseInstance, strScriptFile.c_str(), isStartWnd()?"-start-wnd":"");

	//combin the instance
	//CodeCombinFiles(strSrcFileTmpl.c_str(), strSrcFile.c_str());
	merge(strSrcFileTmpl.c_str(), strSrcFile.c_str());

	//delete file2
	remove(strSrcFileTmpl.c_str());

	//TODO: combine source

	clearSourceChanged();
	return TRUE;
}

void EditUIPanel::disableSource()
{
	if(strXmlFile.length() <= 0)
		return ;

	//TODO: save source
	//ComponentInstance::saveSrouce("test.c",baseInstance,"./etc/uieditor/trans/ncs-ctmpl.lua", "ncs_trans");
	//create srouce file name
	char szFileName[256] = "/";

	getSourceFileName(szFileName+1);

	//get source file name
	string strSrcFile = g_env->getSourcePath();
	strSrcFile += szFileName;

	if(!::isFileExist(strSrcFile.c_str()))
		return ;

	FILE* fp = fopen(strSrcFile.c_str(), "r+");
	if(!fp)
		return ;

	fseek(fp,0, SEEK_END);
	long len = ftell(fp);
	if(len <= 0)
	{
		fclose(fp);
		return;
	}

	unsigned char* buf = new unsigned char[len+1];
	memset(buf, 0, len+1);
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1,len,fp);

	fseek(fp,0, SEEK_SET);
	fprintf(fp,"#if 0\n");
	//fwrite(buf, 1,len, fp);
	fprintf(fp, "%s", buf);
	fprintf(fp,"\n#endif\n");
	fclose(fp);

}

void EditUIPanel::addResource(ComponentInstance *instance, const char* source)
{
	int type = 0;
	if(instance == NULL)
		return;

	if(source)
	{
		const char* str = strstr(source,"res/");
		if(str != NULL)
			source = str;
	}

	//add instance into resource
	if(source)
		type = NCSRT_UI;
	else
		type = NCSRT_CONTRL;

	ResManager * resMgr = g_env->getResManager(type/*NCSRT_UI*/);

	if(resMgr)
	{
		int id = instance->getID();
		if(source || id == -1) //no id
		{
			//create a id or reset source
			id = resMgr->createRes(type, instance->newName().c_str(),id, source, (DWORD)(ComponentInstance*)instance);
				//TODO: createRes failure, how to process instance?
			if (id == -1) {
					//delete instance;
					return;
			}
			instance->setID(id);
    setPropertyChanged();
		}
		else
		{
			//try to get resource firstly
			if(!resMgr->idToName(id)) //not in the resource, add it
			{
                //change name
				resMgr->createRes(type, instance->newName().c_str(), id,  source, (DWORD)(ComponentInstance*)instance);
			}
		}
		//TODO: add to insts list, maybe need to add to createRes
		//resMgr->setRes(id, (DWORD)instance);
		//increase the use ref
		instance->incUseOfRefReses();
	}
}

PBITMAP EditUIPanel::getOutlineBmp()
{

	return &bmpOutline;
}

/////////////////
//selection
///////////////////////////////////////////////////////////////

static inline void draw_anchor(HDC hdc, int x, int y)
{
		FillBox(hdc, x-ANCHOR_SIZE/2, y-ANCHOR_SIZE/2, ANCHOR_SIZE, ANCHOR_SIZE);
}

static inline void draw_frame_anchor(HDC hdc, int x, int y)
{
	Rectangle(hdc, x-ANCHOR_SIZE/2, y-ANCHOR_SIZE/2, x+ANCHOR_SIZE/2, y+ANCHOR_SIZE/2);
}

static void draw_selected_box(HDC hdc, const RECT *rt, gal_pixel color)
{
	gal_pixel old = SetBrushColor(hdc, color);

	draw_anchor(hdc, rt->left, rt->top);
	draw_anchor(hdc, (rt->left+rt->right)/2, rt->top);
	draw_anchor(hdc, rt->right, rt->top);
	draw_anchor(hdc, rt->right,( rt->top+rt->bottom)/2);
	draw_anchor(hdc, rt->right, rt->bottom);
	draw_anchor(hdc, rt->left,( rt->top+rt->bottom)/2);
	draw_anchor(hdc, rt->left, rt->bottom);
	draw_anchor(hdc, (rt->left+rt->right)/2, rt->bottom);

	SetBrushColor(hdc, old);
}


static void draw_mainwind_anchor(HWND hwnd, HDC hdc, int xoff, int yoff, bool bSelected)
{
	void (*drawAnchor)(HDC, int,int);

	drawAnchor = bSelected?draw_anchor:draw_frame_anchor;

	RECT rt;
	::GetWindowRect(hwnd, &rt);
	int w = RECTW(rt) + ANCHOR_SIZE/2;
	int h = RECTH(rt) + ANCHOR_SIZE/2;

	gal_pixel old = SetBrushColor(hdc, 0);

	drawAnchor(hdc, w/2 + xoff, h + yoff);
	drawAnchor(hdc, w + xoff, h + yoff);
	drawAnchor(hdc, w + xoff, h/2 + yoff);

	old = SetBrushColor(hdc, old);

}

///////////////////////////////////////////////////
void EditUIPanel::drawSelections(HDC hdc)
{
	HDC hdct = (HDC)0;
	if(hdc == (HDC)0){
		hdct = GetClientDC();
	}
	else
		hdct = hdc;


	//for(vector<HWND>::iterator it=selectedWnd.begin(); it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		drawSelection(hdct, selectedWnd[i]);
	}

	draw_mainwind_anchor(hwndTopMost, hdct, -xpos, -ypos, selectedWnd.empty());

	if(hdct != hdc)
		ReleaseDC(hdct);
}

void EditUIPanel::drawSelection(HDC hdc, HWND hctrl)
{
	RECT rt;
	HDC hdct = (HDC)0;
	if(hdc == (HDC)0)
		hdct = GetClientDC();
	else
		hdct = hdc;

	::GetWindowRect(hctrl, &rt);
	clientToMainWnd(::GetParent(hctrl), rt);

	OffsetRect(&rt, -xpos, -ypos);

	//draw selection
	draw_selected_box(hdct, &rt, hctrl==curWnd?COLOR_blue:PIXEL_black);

	if(hdct != hdc)
		ReleaseDC(hdct);

}

//true: have insert, false, had inserted, don't insert too
BOOL EditUIPanel::insertSelect(HWND hctrl, BOOL bcur)
{
	if(!::IsControl(hctrl))
		return FALSE;

	if(hctrl == curWnd)
		return FALSE;

	//for(vector<HWND>::iterator it=  selectedWnd.begin();
	//	it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
		if(selectedWnd[i] == hctrl)
			return FALSE;
	selectedWnd.push_back(hctrl);

	if(bcur){
		curWnd = hctrl;
		//notifySelChanged();
	}

	return TRUE;
}


//true have moved, false not removed
BOOL EditUIPanel::removeSelect(HWND hctrl)
{
	for(vector<HWND>::iterator it=  selectedWnd.begin();
		it != selectedWnd.end(); ++it)
	{
		if(*it == hctrl){
			selectedWnd.erase(it);
			if(hctrl == curWnd){
				curWnd = selectedWnd.empty()?hwndTopMost:selectedWnd.front();
				//TODO : Notif parent curWnd Changed
				notifySelChanged();
			}
			return TRUE;
		}
	}
	return FALSE;
}

HWND EditUIPanel::getWndAt(int x, int y, int& hitcode)
{
	RECT rt;
	if(baseInstance && ::IsWindow(hwndTopMost))
	{
		::GetWindowRect(hwndTopMost, &rt);
		if(!(x >= 0 && x<=RECTW(rt) && y >=0 && y<=RECTH(rt)))
			return HWND_INVALID;
		//translate x,  y as hwndTopMost's x y
		x += rt.left;
		y += rt.top;
		// hit test
		ComponentInstance * cinst =  baseInstance->getChildren();
		while(cinst)
		{
			hitcode = cinst->hittest(x, y);
			if(hitcode == ComponentInstance::OUT)
			{
				cinst = cinst->getNext();
				continue;
			}

			if(hitcode == ComponentInstance::IN)
			{
				return cinst->getPreviewHandler();
			}

			if(hitcode == ComponentInstance::CONTAINER){
				cinst = cinst->getChildren();
				continue;
			}
		}
		hitcode = ComponentInstance::IN;
		return hwndTopMost;
	}

	return HWND_INVALID;
}

////////////////////////////////////////
///////////////////////////////////////////////////////

BOOL EditUIPanel::inAnchor(int x, int y, int xanchor, int yanchor)
{
	RECT rtAnchor = {
			xanchor  -ANCHOR_SIZE/2,
			yanchor  -ANCHOR_SIZE/2,
			xanchor + ANCHOR_SIZE/2,
			yanchor + ANCHOR_SIZE/2
	};
	return PtInRect(&rtAnchor, x, y);
}

EditUIPanel::SizeEditInfo EditUIPanel::_sei;

HCURSOR EditUIPanel::getCursorByAnchor(DWORD anchor)
{
	switch(anchor)
	{
	case AnchorLeft:
	case AnchorRight:
		return GetSystemCursor(IDC_SIZEWE);
	case AnchorTop:
	case AnchorBottom:
		return GetSystemCursor(IDC_SIZENS);
	case AnchorLeft|AnchorTop:
	case AnchorRight|AnchorBottom:
		return GetSystemCursor(IDC_SIZENWSE);
	case AnchorLeft|AnchorBottom:
	case AnchorRight|AnchorTop:
		return GetSystemCursor(IDC_SIZENESW);
	default:
		return GetSystemCursor(IDC_ARROW);
	}
}

EditUIPanel::SizeEditInfo * EditUIPanel::getEditInfo(int x, int y)
{
	RECT rt;
	DWORD anchor = 0;
	if(baseInstance && !::IsWindow(hwndTopMost))
		return NULL;

	{
		int w, h;
		//test is on the MainWindow's
		::GetWindowRect(hwndTopMost, &rt);
		w = RECTW(rt)+ANCHOR_SIZE/2;
		h = RECTH(rt)+ANCHOR_SIZE/2;
		if(inAnchor(x, y, w, h/2 ))
			anchor |= AnchorRight;
		else if(inAnchor(x, y, w, h))
			anchor |= AnchorRight|AnchorBottom;
		else if(inAnchor(x, y, w/2, h))
			anchor |= AnchorBottom;
		if(anchor){
			_sei.hwnd = hwndTopMost;
			_sei.anchor = anchor;
			_sei.hcur = getCursorByAnchor(_sei.anchor);
			return &_sei;
		}
	}

//	for(vector<HWND>::iterator it = selectedWnd.begin();
		//it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		anchor = 0;
		HWND hwnd = selectedWnd[i];
		::GetWindowRect(hwnd, &rt);
		clientToMainWnd(::GetParent(hwnd), rt);

		//get anchor
		if(inAnchor(x,y, rt.left, rt.top))
			anchor = AnchorLeft|AnchorTop;
		else if(inAnchor(x,y, (rt.left+rt.right)/2, rt.top))
			anchor = AnchorTop;
		else if(inAnchor(x,y,rt.right, rt.top))
			anchor = AnchorRight|AnchorTop;
		else if(inAnchor(x,y,rt.right, (rt.top+rt.bottom)/2))
			anchor = AnchorRight;
		else if(inAnchor(x,y,rt.right, rt.bottom))
			anchor = AnchorRight|AnchorBottom;
		else if(inAnchor(x,y,(rt.left+rt.right)/2, rt.bottom))
			anchor = AnchorBottom;
		else if(inAnchor(x,y,rt.left, rt.bottom))
			anchor = AnchorLeft|AnchorBottom;
		else if(inAnchor(x,y, rt.left, (rt.top+rt.bottom)/2))
			anchor = AnchorLeft;
		if(anchor)
		{
			_sei.hwnd = hwnd;
			_sei.anchor = anchor;
			_sei.hcur = getCursorByAnchor(_sei.anchor);
			return &_sei;
		}
	}

	return NULL;
}

ComponentInstance* EditUIPanel::testContainer(int x, int y, ComponentInstance** pcinst)
{
	ComponentInstance * container = baseInstance;
	ComponentInstance *cinst;
	int tempx = x;
	int tempy = y;
	{
		//translate tempx, tempy
		RECT rt;
		::GetWindowRect(hwndTopMost, &rt);
		if(tempx < 0 || tempy < 0 || tempx >= RECTW(rt) || tempy >= RECTH(rt)){
			return NULL;
		}
		::WindowToClient(hwndTopMost, &tempx, &tempy);
	}

	{
CONTINUE:
		cinst = container->getChildren();
		while(cinst){
			int hitcode = cinst->hittest(tempx, tempy);
			if(hitcode == ComponentInstance::CONTAINER)
			{
				RECT rt;
				//continue test to find the next container
				PageWindowInstance *pageInstance = dynamic_cast<PageWindowInstance*>(cinst);
				if(pageInstance){
					container = pageInstance->getActiveInstance();
					::GetWindowRect(cinst->getPreviewHandler(), &rt);
					tempx -= rt.left;
					tempy -= rt.top;
					::WindowToClient(cinst->getPreviewHandler(), &tempx, &tempy);
				}
				else
					container = cinst;
				if(!container)
					goto CONTINUE;

				::GetWindowRect(container->getPreviewHandler(), &rt);
				tempx -= rt.left;
				tempy -= rt.top;
				::WindowToClient(container->getPreviewHandler(), &tempx, &tempy);
				goto CONTINUE;
			}
			else if(hitcode == ComponentInstance::IN) //in a child
			{
				break;
			}
			else if(hitcode == ComponentInstance::REQ_MOUSE_AREA) //process the mouse
			{
				//HWND holdCur = curWnd;
				//select curContainer
				//curState = Normal;
				flags |= RequestMouseMsg;
				/*cancelSelectAll();
				insertSelect(cinst->getPreviewHandler(), TRUE);
				if(holdCur != cinst->getPreviewHandler())
				{
					//notify
					notifySelChanged();
				}*/
				break;
			}
			cinst = cinst->getNext();
		}
	}

	if(pcinst)
		*pcinst = cinst;
	return container;
}

int EditUIPanel::updateInstanceField(Instance *instance, int id)
{
	ComponentInstance * cinst = dynamic_cast<ComponentInstance *>(instance);
	if(cinst == NULL)
		return ComponentInstance::SPWE_IGNORED;

	if(!isInstanceIn(cinst))
		return ComponentInstance::SPWE_IGNORED;

	if(id == 0) //it ID of instance
	{
		setPropertyChanged();
		setSourceChanged(); // ID name changed the source, source use ID name as Marco
		sendEvent(EDITUIPANEL_INSTANCE_REFRESH, (DWORD)cinst, FALSE);
		return ComponentInstance::SPWE_OK;
	}

	HWND hold = cinst->getPreviewHandler();

	int ret = cinst->syncPreviewWindow(id);
	if(ret == ComponentInstance::SPWE_OK
			|| ret == ComponentInstance::SPWE_NEWVALUE){
		//reupdate the current window
		// same property, such as PropRenderer, would destory window and recreate it
		// so, we should update the selection info
   //     if((id == ComponentInstance::PropWidth || id == ComponentInstance::PropHeight)
   //         && (hold == hwndTopMost))
   //         updateScrollbar();

		syncSelectedWndFromInstance(cinst,hold);

		flags |= UpdatePreviewWindow;
		InvalidateRect();
	/*	if(ret == ComponentInstance::SPWE_NEWVALUE)
		{
			//send event
			//sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)(Instance*)cinst, (DWORD)id);
		}*/
	}
	else{
		if(ret == ComponentInstance::SPWE_REJECT)
		{
			string str;
			instance->getFieldErrorTip(id, str);
			InfoBox(_("Error"), 
                    _("The Value is invalid or the control cannot accept this value now!\n\nPlease check the value range or other dependency properties.\n\nError Tip: %s"), str.c_str()
				 );
			return ret;
		}
	}

	if(id >= ComponentInstance::PropEventBegin && id <= ComponentInstance::PropEventEnd)
	{
		setSourceChanged();
	}
	else
	{
		setPropertyChanged();
	}

	return ret;
}

void EditUIPanel::updateRdrElements(Instance *inst, int* ele_ids)
{
	int id = inst->getID();
	DWORD params[2] = {(DWORD)ele_ids, (DWORD)((RendererInstance*)inst)};
	if(updateInstanceRdr(baseInstance, id, params))
		flags |= UpdatePreviewWindow;
}

BOOL EditUIPanel::updateInstanceRdr(ComponentInstance *cinst,int id, DWORD params[2])
{
	BOOL bupdate = FALSE;
	if(!cinst)
		return FALSE;

	int updatedFields[1];
	int count = cinst->getReferencedFieldIds(id, updatedFields,1);
	if(count > 0)
	{
		bupdate = cinst->updateSpecialField(updatedFields[0],(DWORD)params) || bupdate;
	}

	for(ComponentInstance *child = cinst->getChildren(); child; child = child->getNext())
	{
		bupdate = updateInstanceRdr(child, id, params) || bupdate ;
	}
	return bupdate;
}

void EditUIPanel::syncSelectedWndFromInstance(ComponentInstance* cinst, HWND holdWnd)
{

	HWND hnewWnd = cinst->getPreviewHandler();

	if(hnewWnd == holdWnd)
		return;

	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		if(holdWnd == selectedWnd[i])
		{
			selectedWnd[i] = hnewWnd;
		}
	}
	if(curWnd == holdWnd)
		curWnd = hnewWnd;

	if(hwndTopMost == holdWnd)
	{
		hwndTopMost = hnewWnd;
		//set renderer
		WINDOWINFO *win_info =  (WINDOWINFO*)::GetWindowInfo(hwndTopMost);
		dumpWindowRenderer(win_info->we_rdr);
		win_info->we_rdr = editor_win_rdr;

		//set Secodery data
		HDC hdc_secd = ::GetSecondaryDC(hwndTopMost);
		::SetSecondaryDC(hwndTopMost, hdc_secd, DoubleBufferProc);

		RECT rt;
		::GetWindowRect(hwndTopMost, &rt);
		::MoveWindow(hwndTopMost, -RECTW(rt), -RECTH(rt), RECTW(rt), RECTH(rt), FALSE);

		::ShowWindow(hwndTopMost, SW_SHOW);
	}
}

void EditUIPanel::recreatePreviewWnd(Instance* instance)
{
	ComponentInstance * cinst = dynamic_cast<ComponentInstance *>(instance);
	if(cinst == NULL)
		return ;

	if(!isInstanceIn(cinst))
		return ;

	HWND hold = cinst->getPreviewHandler();

	cinst->recreatePreviewWindow();

	syncSelectedWndFromInstance(cinst,hold);

	flags |= UpdatePreviewWindow;
	InvalidateRect();
}

void EditUIPanel::onPopMenuCmd(int id)
{
	switch(id)
	{
	case ResEditor::GBC_COPY://COPY
		copy();
		break;
	case ResEditor::GBC_CUT://Cut
		copy(TRUE);
		break;
	case ResEditor::GBC_PASTE://Paste
		paste();
		break;
	case ResEditor::GBC_DELETE: //Delete
		deleteSelectedInstance();
		break;
	case ResEditor::UI_MENUCMD_LEFT: //left align
	case ResEditor::UI_MENUCMD_RIGHT: //right align
	case ResEditor::UI_MENUCMD_CENTER: // center align
	case ResEditor::UI_MENUCMD_TOP: //top align
	case ResEditor::UI_MENUCMD_BOTTOM: //bottom align
	case ResEditor::UI_MENUCMD_MIDDLE: //middle align
		align(id - ResEditor::UI_MENUCMD_LEFT);
		break;
	case ResEditor::UI_MENUCMD_HSPREED: //spreed out hroz
		spreedOut();
		break;
	case ResEditor::UI_MENUCMD_VSPREED: //spreed out vertial
		spreedOut(FALSE);
		break;
	case ResEditor::UI_MENUCMD_SAMEWIDTH: //same size width
		sameSize();
		break;
	case ResEditor::UI_MENUCMD_SAMEHEIGHT: //same size height
		sameSize(FALSE);
		break;
	}
	onPopMenuUser(id);
}

void EditUIPanel::onPopMenuUser(int id)
{
	ComponentInstance *cinst = ComponentInstance::FromHandler(curWnd);
	if(cinst)
	{
		int ret = cinst->processMenuCommand(id);
		if(ret & ComponentInstance::INSTANCE_REFRESH){
			sendEvent(EDITUIPANEL_INSTANCE_REFRESH, (DWORD)cinst, TRUE);
		}
		if(ret & ComponentInstance::NEED_UPDATE)
		{
			flags |= UpdatePreviewWindow;
			InvalidateRect();
		}
	}
}

void EditUIPanel::deleteSelectedInstance()
{
	//ResManager * resMgr;

	if(selectedWnd.empty())
		return ;

	if(curWnd == hwndTopMost)
		return;

	//resMgr = g_env->getResManager(NCSRT_UI);

    //When delete a item in structpanel, it caused selected item changed.
    //So it will cause selectedWnd be changed.
    //Issue: we need backup selectedWnd for delete operation.
	vector<HWND> selWnd = selectedWnd;

	//undo redo support
	pushCreateDeleteUndoRedoCommand(FALSE);

	for(int i=0; i< (int)selWnd.size(); i++)
	{
		ComponentInstance *instance = ComponentInstance::FromHandler(selWnd[i]);
		if(instance == NULL)
			continue;

		ComponentInstance *parent = instance->getParent();
		if(parent == NULL)
			continue;

		sendEvent(EDITUIPANEL_INSTANCE_DELETED,(DWORD)instance);

		//release instance
		parent->remove(instance,TRUE);
		//dec useage
		instance->decUseOfRefReses();
		//TODO : here undo redo support
		instance->release();

		//refresh the source
		setSourceChanged();
	}

	cancelSelectAll();
	insertSelect(hwndTopMost, TRUE);
	notifySelChanged();

	flags |= UpdatePreviewWindow;
	InvalidateRect();
}

//the offset of origin coordinates when paste instance.
//static int offset_pos = 0;

void EditUIPanel::copy(BOOL bremove /*= FALSE*/)
{
	if(selectedWnd.size() > 0 && curWnd != hwndTopMost)
	{
        //reset offset
  //      offset_pos = 0;
		int size = selectedWnd.size();
		Instance** instances = new Instance*[size];
		for(int i=0; i< size; i++)
		{
			instances[i] = (Instance*)ComponentInstance::FromHandler(selectedWnd[i]);
			if(!instances[i])
			{
				delete[] instances;
				return;
			}
		}
		Instance::copy(instances,size);

		if(bremove)
		{
/*			ComponentInstance *parent = ((ComponentInstance*)instances[0])->getParent();

			if(parent)
			{
				for(int i=0; i<size; i++)
				{
					sendEvent(EDITUIPANEL_INSTANCE_DELETED,(DWORD)instances[i]);
					parent->remove((ComponentInstance*)instances[i],TRUE);
				}
				//update window
				flags |= UpdatePreviewWindow;
				InvalidateRect();
			}*/

			deleteSelectedInstance();
		}

		delete[] instances;
		//enable paste
		sendEvent(UIMENUITEM_ENABLE, ResEditor::GBC_PASTE, TRUE);
	}
}


void EditUIPanel::paste()
{
	InstanceArray instances;

	ComponentInstance* container = curContainer;
	if(container == NULL)
		container = ComponentInstance::FromHandler(curWnd);
	if(container == NULL )
		container = baseInstance;
	else if(!container->isContainer())
		container = container->getParent();

	if(container && (instances = Instance::paste()))
	{
		//cancel the selected
		cancelSelectAll();
		for(int i=0; instances[i]; i++){
			ComponentInstance* cinst = dynamic_cast<ComponentInstance*>(instances[i]);
			if(cinst == NULL)
				continue;

			int x,y;
			cinst->getLocation(x, y);
  //          offset_pos += 10;
			x += 10; //offset_pos;
			y += 10; //offset_pos;
			snapeGrid(x,y);
			//cinst->setLocation(x,y);
			cinst = (ComponentInstance*)cinst->clone();
			if(!cinst)
				continue;

			cinst->setLocation(x, y);
			if(!container->insert(cinst,TRUE)){
				cinst->release();
				continue;
			}

			//add into the resource
			addResource(cinst);

			sendEvent(EDITUIPANEL_INSTANCE_ADDED,(DWORD)(ComponentInstance*)cinst);
			//insert into selection
			insertSelect(cinst->getPreviewHandler(), i==0);
			const char *def_f = getDefaultClientFont();
			
			if(def_f)
				setPreviewWindowClientFont((LOGFONT*)LoadResource(def_f, RES_TYPE_FONT, 0L), cinst);
		}
		
		setSourceChanged();
		//send event
		notifySelChanged();
		//update window
		flags |= UpdatePreviewWindow;
		InvalidateRect();
		//undo redo support
		pushCreateDeleteUndoRedoCommand(TRUE);
	}
}

void EditUIPanel::align(int at)
{
	RECT rtStandar;

	if(at < 0 || at >= ALIGN_MAX || selectedWnd.size() <= 1)
		return;

	::GetWindowRect(curWnd, &rtStandar);

	_move_old_list.clear();

//	for(vector<HWND>::iterator it = selectedWnd.begin();
//		it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		RECT rt;
		HWND hwnd = selectedWnd[i];
		::GetWindowRect(hwnd, &rt);
		_move_old_list.push_back(MAKELONG(rt.left, rt.top));
		if(hwnd != curWnd)
		{
			int width = RECTW(rt);
			int height = RECTH(rt);
			switch(at)
			{
			case ALIGN_LEFT:
				rt.left = rtStandar.left;
				break;
			case ALIGN_RIGHT:
				rt.left = rtStandar.right - width;
				break;
			case ALIGN_CENTER:
				rt.left = (rt.left + rtStandar.left + rtStandar.right - rt.right) / 2;
				break;
			case ALIGN_TOP:
				rt.top = rtStandar.top;
				break;
			case ALIGN_BOTTOM:
				rt.top = rtStandar.bottom - height;
				break;
			case ALIGN_MIDDLE:
				rt.top = (rt.top + rtStandar.top + rtStandar.bottom - rt.bottom ) / 2;
				break;
			}

			ComponentInstance* cinst = ComponentInstance::FromHandler(hwnd);
			if(cinst)
				cinst->setLocation(rt.left, rt.top);

			::MoveWindow(hwnd, rt.left, rt.top, width, height,TRUE);
		}
	}

	flags |= UpdatePreviewWindow;
	InvalidateRect();

	pushMoveUndoRedoCommand(NULL);

}

struct SpreedOutInfo{
	HWND hwnd;
	RECT rt;
	int  key;
};
void EditUIPanel::spreedOut(BOOL bHorz)
{
	int count = selectedWnd.size();
	int min_value=0x7FFFFFFF, max_value=-99999999;
	int total_size = 0;
	if(count <= 2)
			return;

	SpreedOutInfo * infors = new SpreedOutInfo[count];
	SpreedOutInfo ** sort_infors = new SpreedOutInfo*[count];
	int idx = 0;

	_move_old_list.clear();

//	for(vector<HWND>::iterator it = selectedWnd.begin();
//		it != selectedWnd.end(); ++it)
	for(int j=0; j<(int)selectedWnd.size(); j++)
	{
		HWND hwnd = selectedWnd[j];
		infors[idx].hwnd = hwnd;
		::GetWindowRect(hwnd, &infors[idx].rt);
		_move_old_list.push_back(MAKELONG(infors[idx].rt.left, infors[idx].rt.top));
		if(bHorz){
			infors[idx].key = infors[idx].rt.left;
			if(min_value > infors[idx].rt.left)
				min_value = infors[idx].rt.left;
			if(max_value < infors[idx].rt.right)
				max_value = infors[idx].rt.right;
			total_size += RECTW(infors[idx].rt);
		}
		else
		{
			infors[idx].key = infors[idx].rt.top;
			if(min_value > infors[idx].rt.top)
				min_value = infors[idx].rt.top;
			if(max_value < infors[idx].rt.bottom)
				max_value = infors[idx].rt.bottom;
			total_size += RECTH(infors[idx].rt);
		}
		//insert sorted;
		SpreedOutInfo * tmp = &infors[idx];
		for(int i=0; i<idx; i++)
		{
			if(sort_infors[i]->key > tmp->key){
				SpreedOutInfo* ttmp = sort_infors[i];
				sort_infors[i] = tmp;
				tmp = ttmp;
			}
		}
		sort_infors[idx] = tmp;
		idx ++;
	}

	int interval = (max_value - min_value - total_size) / (count-1);

	if(interval <= 0)
		goto FAILED;

	//reset the pos
	for(int i=1; i<count-1; i++)
	{
		int width = RECTW(sort_infors[i]->rt);
		int height = RECTH(sort_infors[i]->rt);
		if(bHorz)
		{
			sort_infors[i]->rt.left = sort_infors[i-1]->rt.right + interval;
		}
		else
		{
			sort_infors[i]->rt.top = sort_infors[i-1]->rt.bottom + interval;
		}
		int x = sort_infors[i]->rt.left;
		int y = sort_infors[i]->rt.top;

		snapeGrid(x,y);

		ComponentInstance *cinst = ComponentInstance::FromHandler(sort_infors[i]->hwnd);
		if(cinst)
			cinst->setLocation(x, y);

		::MoveWindow(sort_infors[i]->hwnd,
				x, y,
				width, height, TRUE);
	}

	flags |= UpdatePreviewWindow;
	InvalidateRect();

	pushMoveUndoRedoCommand(NULL);

FAILED:

	delete [] infors;
	delete [] sort_infors;

}

void EditUIPanel::sameSize(BOOL bWidth)
{
	RECT rt;
	int width;
	int height;

	if(selectedWnd.size() <= 1)
			return;

	pushSizeUndoRedoCommand();

	::GetWindowRect(curWnd, &rt);
	width = RECTW(rt);
	height = RECTH(rt);

	//for(vector<HWND>::iterator it = selectedWnd.begin();
	//	it != selectedWnd.end(); ++it)
	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		HWND hwnd = selectedWnd[i];
		if(hwnd != curWnd)
		{
			::GetWindowRect(hwnd, &rt);
			if(bWidth)
				height = RECTH(rt);
			else
				width = RECTW(rt);

			ComponentInstance *cinst = ComponentInstance::FromHandler(hwnd);
			if(cinst)
				cinst->setSize(width, height);

			::MoveWindow(hwnd, rt.left, rt.top, width, height, TRUE);
		}
	}


	flags |= UpdatePreviewWindow;
	InvalidateRect();

}

////////////////////////////

#include "window-instance.h"

void EditUIPanel::draw_caption (HWND hWnd, HDC hdc, BOOL is_active)
{
	WindowInstance* win = (WindowInstance*)GetWindowAdditionalData(hWnd);
	//printf("hwnd=%p, win=%p, rdr=%p, old_win_rdr=%p, caption=%s, class=%s\n",hWnd, win, GetWindowInfo(hWnd)->we_rdr, win?win->old_win_rdr:NULL,GetWindowCaption(hWnd), GetClassName(hWnd));
	if(win && win->old_win_rdr)
		win->old_win_rdr->draw_caption(hWnd, hdc, TRUE);
}

void EditUIPanel::draw_border(HWND hWnd, HDC hdc, BOOL is_active)
{
	WindowInstance* win = (WindowInstance*)GetWindowAdditionalData(hWnd);
	if(win && win->old_win_rdr)
		win->old_win_rdr->draw_border(hWnd, hdc, TRUE);
}

void EditUIPanel::draw_caption_button(HWND hWnd, HDC hdc, int ht_code, int status)
{
	WindowInstance* win = (WindowInstance*)GetWindowAdditionalData(hWnd);
	if(win && win->old_win_rdr)
		win->old_win_rdr->draw_caption_button(hWnd, hdc, ht_code, status&~LFRDR_BTN_STATUS_INACTIVE);
}

void EditUIPanel::dumpWindowRenderer(WINDOW_ELEMENT_RENDERER *win_rdr)
{
	if(win_rdr == NULL)
		return;

	(static_cast<WindowInstance*>(baseInstance))->old_win_rdr = win_rdr;
	memcpy(editor_win_rdr, win_rdr, sizeof(WINDOW_ELEMENT_RENDERER));
	editor_win_rdr->draw_caption = draw_caption;
	editor_win_rdr->draw_border = draw_border;
	editor_win_rdr->draw_caption_button = draw_caption_button;
}

void EditUIPanel::setRendererEditor(FieldPanel* panelRdr)
{
	WindowInstance * winst = dynamic_cast<WindowInstance *>(ComponentInstance::FromHandler(curWnd));
	if(winst == NULL || panelRdr == NULL){
		if(panelRdr)
			panelRdr->setEventHandler(NULL);
		return ;
	}

	if(!isInstanceIn(winst)){
		panelRdr->setEventHandler(NULL);
		return ;
	}

	panelRdr->setInstance(winst->getRendererInstance());
	panelRdr->setEventHandler(this);
}

DWORD EditUIPanel::processEvent(Panel* sender, int event_id, DWORD param1/* = 0*/, DWORD param2/* = 0*/ )
{
	if(event_id == FIELDPANEL_INSTANCE_FIELD_CHANGED)
	{
		Instance * inst = (Instance*)param1;
		RendererInstance * rdrInst = dynamic_cast<RendererInstance*>(inst);
		if(rdrInst)
		{
			//update rdreditor's preview window
            int ids[2] = {(int)param2, 0};
			rdrInst->updatePreviewWindow(ids, curWnd);
			DWORD params[2] = {(DWORD)ids, (DWORD)rdrInst};
			
			RendererEditor* rdrEd =
				(RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
			rdrEd->setRdrXmlChanged();
			//update all the editors
			return sendEvent(EDITUIPANEL_UPDATE_SPECIAL_FIELD,(DWORD)ComponentInstance::PropRenderer, (DWORD)params);
		}
	}
    else if(event_id == FIELDPANEL_INSTANCE_FIELD_RESET)
	{
		Instance * inst = (Instance*)param1;
		RendererInstance * rdrInst = dynamic_cast<RendererInstance*>(inst);
		if(rdrInst)
		{
			//update rdreditor's preview window
            rdrInst->updatePreviewWindow((int*)param2, curWnd);

			//update rdreditor's preview window
			DWORD params[2] = {param2, (DWORD)rdrInst};
			//update all the editors
			RendererEditor* rdrEd =
				(RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
			rdrEd->setRdrXmlChanged();
			return sendEvent(EDITUIPANEL_UPDATE_SPECIAL_FIELD, (DWORD)ComponentInstance::PropRenderer, (DWORD)params);
        }
    }
	return 0;
}

HWND EditUIPanel::setPreview(BOOL bPreview)
{
	HWND hwnd = HWND_INVALID;
	if(bPreview){
		flags |= PreviewMode;

		if(baseInstance)
		{
			RECT rt;
			HDC hSecondaryDC = ::GetSecondaryDC(baseInstance->getPreviewHandler());
			hdcPreviewWnd = CreateCompatibleDC(hSecondaryDC);
			::GetWindowRect(hwndTopMost,&rt);
			BitBlt(hSecondaryDC, 0, 0, RECTW(rt), RECTH(rt), hdcPreviewWnd, 0, 0, 0);
		}
	}
	else{
		flags &= ~PreviewMode;
		DeleteCompatibleDC(hdcPreviewWnd);
		hdcPreviewWnd = HWND_INVALID;
	}

	if(baseInstance){
		baseInstance->previewWindow(bPreview);
		hwnd = baseInstance->getPreviewHandler();
	}

	//update window
	if(!bPreview)
	{
		flags |= UpdatePreviewWindow;
		InvalidateRect();
	}

	return hwnd;
}

void EditUIPanel::setDefRenderer(const char* defRdrName)
{
	if(baseInstance && baseInstance->setDefRenderer(defRdrName))
	{
		//set renderer
		WINDOWINFO *win_info = (WINDOWINFO*)::GetWindowInfo(hwndTopMost);
		dumpWindowRenderer(win_info->we_rdr);

		win_info->we_rdr = editor_win_rdr;

		flags |= UpdatePreviewWindow;
		InvalidateRect();
	}
}

//////////////////////////
void EditUIPanel::updateRefResValue(int res_id)
{
	ComponentInstance *cinst;
	int updatedFields[100];
	queue<ComponentInstance*> cinst_queue;

	cinst_queue.push(baseInstance);
	while(!cinst_queue.empty())
	{
		cinst = cinst_queue.front();
		cinst_queue.pop();

		//push all the children
		for(ComponentInstance* children = cinst->getChildren(); children; children = children->getNext())
			cinst_queue.push(children);

		int count = cinst->getReferencedFieldIds(res_id, updatedFields, sizeof(updatedFields)/sizeof(int));
		if(count > 0)
		{
			for(int i=0; i<count; i++)
			{
				updateInstanceField((Instance*)cinst,updatedFields[i]);
				sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)(Instance*)cinst,(DWORD)updatedFields[i]);
			}
		}
	}
}

void EditUIPanel::updateTexts()
{
	ComponentInstance *cinst;
	queue<ComponentInstance*> cinst_queue;

	cinst_queue.push(baseInstance);
	while(!cinst_queue.empty())
	{
		cinst = cinst_queue.front();
		cinst_queue.pop();

		//push all the children
		for(ComponentInstance* children = cinst->getChildren(); children; children = children->getNext())
			cinst_queue.push(children);

		updateInstanceField((Instance*)cinst, ComponentInstance::PropText);

	}
	ComponentInstance *sel = ComponentInstance::FromHandler(curWnd);
	if(sel)
		sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)(Instance*)sel,(DWORD)ComponentInstance::PropText);
}

BOOL EditUIPanel::updateRefResId(int res_old_id, int res_new_id)
{
	BOOL bupdated = FALSE;
	ComponentInstance *cinst;
	queue<ComponentInstance*> cinst_queue;

	cinst_queue.push(baseInstance);
	while(!cinst_queue.empty())
	{
		cinst = cinst_queue.front();
		cinst_queue.pop();

		//push all the children
		for(ComponentInstance* children = cinst->getChildren(); children; children = children->getNext())
			cinst_queue.push(children);

		if(cinst->onResIdChanged(res_old_id,res_new_id))
		{
			bupdated = TRUE;
		}
	}

	return bupdated;
}

BOOL EditUIPanel::deletedFieldFromInst(ComponentInstance *cinst, int id)
{
	BOOL bchanged = FALSE;
	if(!cinst)
		return FALSE;

	int ref_files [16];
	int count = cinst->getReferencedFieldIds(id, ref_files, sizeof(ref_files)/sizeof(int));
	if(count > 0)
	{
		HWND hold = cinst->getPreviewHandler();
		for(int i=0; i< count; i++){
			cinst->cleanField(ref_files[i]);
			bchanged = bchanged || cinst->syncPreviewWindow(ref_files[i]);
			if(hold == curWnd)
			{
				sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)(Instance*)cinst,(DWORD)ref_files[i]);
			}
		}
		syncSelectedWndFromInstance(cinst, hold);
	}

	for(cinst = cinst->getChildren(); cinst; cinst = cinst->getNext())
		/*ATTENTION, bchanged must be after the deleteFiledFromInst
		 *  Because Compiler may not call deleteFiledFromInst if bchanged is true
		 * */
		bchanged = deletedFieldFromInst(cinst, id) || bchanged;

	return bchanged;
}

void EditUIPanel::deleteRefRes(int id)
{
	//delete the res res
	if(deletedFieldFromInst(baseInstance, id))
	{
		flags |= UpdatePreviewWindow;
		InvalidateRect();
	}
}

extern "C" void VFBShowWindow(int show);
void EditUIPanel::gotoCode(Instance* inst, int event_id)
{
	ComponentInstance* cinst = dynamic_cast<ComponentInstance*>(inst);
	if(cinst == NULL)
		return ;

	if(!isInstanceIn(cinst))
		return ;

	if(event_id < ComponentInstance::PropEventBegin || event_id > ComponentInstance::PropEventEnd)
		return ;

	Value value = cinst->getField(event_id);
	if(value == 0) //null value
		return;

	FieldType* ft = cinst->getClass()->getFieldType(event_id);
	if(ft == NULL)
		return ;

	if(!saveXML(FALSE) && !saveSource())
		return ;
	g_env->saveAll();

	//get the instance file name
	string strFile = g_env->getSourcePath();
	char szBuff[256]="/";
	getSourceFileName(szBuff+1);
	strFile += szBuff;

	//make the function key
	sprintf(szBuff,"%u %s", (unsigned int)cinst->getSerialNumber(), ft->name.c_str());
	SocketClient *sock = SocketClient::getInstance();
	sock->goToCode(strFile.c_str(),szBuff);
	//hide
	VFBShowWindow(FALSE);
}

void EditUIPanel::updateRefResIdName(int res_id)
{
	ComponentInstance *cinst;
	int updatedFields[100];
	queue<ComponentInstance*> cinst_queue;

	cinst_queue.push(baseInstance);
	while(!cinst_queue.empty())
	{
		cinst = cinst_queue.front();
		cinst_queue.pop();

		//push all the children
		for(ComponentInstance* children = cinst->getChildren(); children; children = children->getNext())
			cinst_queue.push(children);

		int count = cinst->getReferencedFieldIds(res_id, updatedFields, sizeof(updatedFields)/sizeof(int));
		if(count > 0)
		{
			for(int i=0; i<count; i++)
			{
				updateInstanceField((Instance*)cinst,updatedFields[i]);
			}
		}
	}
}

void EditUIPanel::saveTemplates(const char* strFile)
{
	string strScriptFile = g_env->getConfigFile(g_env->getSysConfig("uieditor/trans/dlgtempl","uieditor/trans/dlgtmpl.lua"));

	ComponentInstance::saveTemplates(strFile, baseInstance, strScriptFile.c_str(), NULL);
}

void EditUIPanel::hide(BOOL bhide)
{
	if( (bhide && (flags&Hidden)) || (!bhide && (!(flags&Hidden))))
		return ;

	if(bhide) //hide instance
	{
		flags |= Hidden;
		hideInstances();
	}
	else
	{
		flags &= ~Hidden;
		if(baseInstance)
		{
			flags |=UpdatePreviewWindow;
			InvalidateRect();
			//update
			//sendEvent(EDITUIPANEL_INSTANCE_HIDE, FALSE, (DWORD)this);
		}
	}

}

void EditUIPanel::hideInstances()
{
	if(!baseInstance)
		return ;

	//update outline
	updateOutline(TRUE, FALSE);

	//sendEvent(EDITUIPANEL_INSTANCE_HIDE, TRUE, (DWORD)this);
}

BOOL EditUIPanel::onKeyDown(int scancode, DWORD key_status)
{
	switch(scancode)
	{
	case SCANCODE_REMOVE:
		if(key_status == 0)
		{
			deleteSelectedInstance();
			return TRUE;
		}
	case SCANCODE_CURSORBLOCKLEFT:
		moveSelections(-1,0);
		break;
	case SCANCODE_CURSORBLOCKRIGHT:
		moveSelections(1,0);
		break;
	case SCANCODE_CURSORBLOCKUP:
		moveSelections(0,-1);
		break;
	case SCANCODE_CURSORBLOCKDOWN:
		moveSelections(0,1);
		break;
	case SCANCODE_TAB:
		//find the next selectiong
		tabToNextSelection();
		break;
	case SCANCODE_SPACE:
		select_key_state = TRUE;
		break;
	}

	return FALSE;
}

BOOL EditUIPanel::onKeyUp(int scancode, DWORD key_status)
{
	if(scancode == SCANCODE_SPACE)
	{
		select_key_state = FALSE;
	}
	else if(scancode == SCANCODE_ESCAPE)
	{
		flags &= ~(CreateControlAlways|CreateControl);
		sendEvent(EDITUIPANEL_FINISH_CREATE);
		//change cursor
		SetCursor(GetSystemCursor(IDC_ARROW));
	}
	return FALSE;
}

void EditUIPanel::moveSelections(int xoffset, int yoffset)
{
	if(xoffset == 0 && yoffset == 0)
		return ;
	if(flags & AutoSnapeGrid)
	{
		if(xoffset < 0 && xoffset > -GRID_SIZE)
			xoffset = -GRID_SIZE;
		else if(xoffset > 0 && xoffset < GRID_SIZE)
			xoffset = GRID_SIZE;

		if(yoffset < 0 && yoffset > -GRID_SIZE)
			yoffset = -GRID_SIZE;
		else if(yoffset > 0 && yoffset < GRID_SIZE)
			yoffset = GRID_SIZE;

	}

	for(int i=0; i<(int)selectedWnd.size(); i++)
	{
		RECT rt;
		HWND hwnd = selectedWnd[i];
		int xtmp, ytmp;
		::GetWindowRect(hwnd, &rt);
		xtmp = rt.left + xoffset;
		ytmp = rt.top + yoffset;
		snapeGrid(xtmp, ytmp);
		if(xtmp != rt.left || ytmp != rt.top){
			ComponentInstance *cinst = ComponentInstance::FromHandler(hwnd);
			if(cinst){
				DWORD mask = cinst->getBoundMask();
				if(!(mask & (ComponentInstance::BOUND_MASK_X|ComponentInstance::BOUND_MASK_Y)))
					return ;

				if(!(mask & ComponentInstance::BOUND_MASK_X))
					xtmp = rt.left;
				if(!(mask & ComponentInstance::BOUND_MASK_Y))
					ytmp = rt.top;

				cinst->setLocation(xtmp, ytmp);
			::MoveWindow(hwnd, xtmp, ytmp, RECTW(rt), RECTH(rt), TRUE);
			flags |= UpdatePreviewWindow;
			}
		}
	}
	notifyBoundChanged(TRUE,FALSE);

	InvalidateRect();
}

void EditUIPanel::tabToNextSelection()
{
	ComponentInstance *cur_inst = NULL;
	if(!baseInstance)
		return;

	if(curWnd != hwndTopMost && ::IsWindow(curWnd))
	{
		//trad as first
		cur_inst = ComponentInstance::FromHandler(curWnd);
		cur_inst = cur_inst->getNext();
	}

	if(cur_inst == NULL)
	{
		cur_inst = baseInstance->getChildren();
	}

	if(!cur_inst)
		return;

	cancelSelectAll();
	insertSelect(cur_inst->getPreviewHandler(),TRUE);
	notifySelChanged();

	InvalidateRect();
}

//////////////////////////
//undo redo
void EditUIPanel::pushMoveUndoRedoCommand(ComponentInstance *old_container)
{
	int count = selectedWnd.size();
	if(count <= 0)
		return;

	ComponentInstanceLocationUndoRedoCommand * cmd =
		new	ComponentInstanceLocationUndoRedoCommand(old_container, count);

	for(int i=0; i<count; i++)
	{
		DWORD v;
		ComponentInstance * cinst = ComponentInstance::FromHandler(selectedWnd[i]);
		if(!cinst)
			continue;
		v = _move_old_list[i];
		cmd->setInstance(i,cinst,LOSWORD(v), HISWORD(v));
	}

	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_UNDO,TRUE);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_REDO,FALSE);
	pushUndoRedoCommand(cmd);
}

void EditUIPanel::pushSizeUndoRedoCommand(ComponentInstance * special)
{
	int count;
	if(special)
		count = 1;
	else
		count = selectedWnd.size();
	if(count <= 0)
		return;

	ComponentInstanceBoundUndoRedoCommand * cmd =
		new	ComponentInstanceBoundUndoRedoCommand(count);

	if(special)
		cmd->setInstance(0,special, &_size_old);
	else
	{
		for(int i=0; i<count; i++)
		{
			ComponentInstance * cinst = ComponentInstance::FromHandler(selectedWnd[i]);
			if(!cinst)
				continue;
			cmd->setInstance(i,cinst);
		}
	}
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_UNDO,TRUE);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_REDO,FALSE);
	pushUndoRedoCommand(cmd);
}

void EditUIPanel::pushCreateDeleteUndoRedoCommand(BOOL bCreate)
{
	int count = selectedWnd.size();
	if(!count)
		return;

	ComponentInstance *container = ComponentInstance::FromHandler(selectedWnd[0]);
	if(!container || !(container = container->getParent()))
		return;

	ComponentInstanceUndoRedoCommand *cmd =
		new ComponentInstanceUndoRedoCommand(container, count,
						bCreate
							?ComponentInstanceUndoRedoCommand::ADD
							:ComponentInstanceUndoRedoCommand::DELETE);

	for(int i=0; i<count; i++)
	{
		cmd->setInstance(i,ComponentInstance::FromHandler(selectedWnd[i]));
	}
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_UNDO,TRUE);
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_REDO,FALSE);
	pushUndoRedoCommand(cmd);
}

void EditUIPanel::undo()
{
	const UndoRedoCommand* cmd = NULL;
	if(observer)
		cmd = observer->undo();

	undoRedoUpdate(cmd);
}

void EditUIPanel::redo()
{
	const UndoRedoCommand* cmd = NULL;
	if(observer)
		cmd = observer->redo();

	undoRedoUpdate(cmd);
}

void EditUIPanel::undoRedoUpdate(const UndoRedoCommand *cmd)
{
	if(!cmd)
		return;

	if(instanceof(cmd, ComponentInstanceLocationUndoRedoCommand))
	{
		ComponentInstanceLocationUndoRedoCommand *lcmd = (ComponentInstanceLocationUndoRedoCommand*)cmd;
		ComponentInstanceLocationUndoRedoCommand::BoundInfo *bounds =
			lcmd->getBoundInfo();

		if(lcmd->getCount()<=0)
			return ;

		ComponentInstance *old_container = lcmd->getContainer();
		ComponentInstance *container = bounds[0].compinst->getParent();
		vector<HWND> lists;

		for(int i=0; i<lcmd->getCount(); i++){
			if(old_container!=container)
				lists.push_back(bounds[i].compinst->getPreviewHandler());
			notifyBoundChanged(TRUE,FALSE, bounds[i].compinst->getPreviewHandler());
		}

		if(container != old_container)
		{
			//notify container changed
			sendEvent(EDITUIPANEL_CHANGE_PARENT,(DWORD)&lists,(DWORD)old_container);
		}
	}
	else if(instanceof(cmd, ComponentInstanceBoundUndoRedoCommand))
	{
		ComponentInstanceBoundUndoRedoCommand::Bound * bounds =
			((ComponentInstanceBoundUndoRedoCommand*)cmd)->getBounds();
		for(int i=0; i<((ComponentInstanceBoundUndoRedoCommand*)cmd)->getCount(); i++)
		{
			notifyBoundChanged(TRUE, TRUE, bounds[i].cinst->getPreviewHandler());
		}
	}
	else if(instanceof(cmd, ComponentInstanceUndoRedoCommand))
	{
		ComponentInstanceUndoRedoCommand * instcmd = (ComponentInstanceUndoRedoCommand*)cmd;
		ComponentInstance ** insts = instcmd->getInsts();
		if(instcmd->getType() == ComponentInstanceUndoRedoCommand::ADD)//now add the instances
		{
			cancelSelectAll();
			for(int i=0; i<instcmd->getCount(); i++)
			{
				insertSelect(insts[i]->getPreviewHandler(),FALSE);
				sendEvent(EDITUIPANEL_INSTANCE_ADDED,(DWORD)insts[i]);
			}
			curWnd = selectedWnd[0];
			curContainer=instcmd->getParent();
			notifySelChanged();
		}
		else
		{
			cancelSelectAll();
			notifySelChanged();
			for(int i=0; i<instcmd->getCount(); i++)
			{
				insertSelect(insts[i]->getPreviewHandler(),FALSE);
				sendEvent(EDITUIPANEL_INSTANCE_DELETED,(DWORD)insts[i]);
			}
		}
	}
	else if(instanceof(cmd,InstancePropertyUndoRedoCommand))
	{
		InstancePropertyUndoRedoCommand *ipcmd = (InstancePropertyUndoRedoCommand*)cmd;
		//update control
		updateInstanceField(ipcmd->getInstance(),ipcmd->getProp());
		if(ipcmd->getInstance() == (Instance*)ComponentInstance::FromHandler(curWnd))
		{
			//update the filed
			sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)ipcmd->getInstance(), (DWORD)ipcmd->getProp());
		}
	}
	else if(instanceof(cmd,InstanceAllDefPropertiesRedoUndoCommand))
	{
		InstanceAllDefPropertiesRedoUndoCommand * defpcmd = (InstanceAllDefPropertiesRedoUndoCommand*)cmd;
		recreatePreviewWnd(defpcmd->getInstance());
		if(defpcmd->getInstance() == (Instance*)ComponentInstance::FromHandler(curWnd))
		{
			sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)defpcmd->getInstance());
		}
	}
	else if(instanceof(cmd,ComponentInstanceTextUndoRedoCommand))
	{
		ComponentInstanceTextUndoRedoCommand *ipcmd = (ComponentInstanceTextUndoRedoCommand*)cmd;
		//update control
		updateInstanceField(ipcmd->getInstance(),ComponentInstance::PropText);
		if(ipcmd->getInstance() == (Instance*)ComponentInstance::FromHandler(curWnd))
		{
			//update the filed
			sendEvent(EDITUIPANEL_FIELD_UPDATE,(DWORD)ipcmd->getInstance(), (DWORD)ComponentInstance::PropText);
		}
	}

	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_UNDO,canUndo());
	sendEvent(UIMENUITEM_ENABLE,ResEditor::GBC_REDO,canRedo());

	flags |= UpdatePreviewWindow;
	InvalidateRect();
}

/////////////////////////

#include "luahlp.h"
static int copy_file(const char* str_src, const char* str_dest)
{
	int total = 0;
	if(!isFileExist(str_src) || str_dest == NULL )
		return 0 ;

	//copy file
	FILE *fsrc = fopen(str_src,"rt");
	FILE *fdst = fopen(str_dest,"wt");
	char buff[1024];
	while(!feof(fsrc))
	{
		int len = fread(buff,1,sizeof(buff), fsrc);
		fwrite(buff, 1, len, fdst);
		total += len;
	}
	fclose(fsrc);
	fclose(fdst);
	return total;
}

void EditUIPanel::merge(const char* str_src, const char* str_dest)
{
	if(!isFileExist(str_src) || str_dest == NULL )
		return;

	if(!isFileExist(str_dest))
	{
		copy_file(str_src, str_dest);
		return;
	}

	//call lua scricpt "merge"
	//get script file
	const char* strFile = g_env->getSysConfig("uieditor/trans/merge","uieditor/trans/ncs-merge.lua");
	string strScriptFile = g_env->getConfigFile(strFile);

	string strtmpl = str_dest;
	strtmpl += ".tmp";

	int ret = call_lua_function(strScriptFile.c_str(),
					"merge",
					NULL,
					"sss",
					str_dest,
					str_src,
					strtmpl.c_str() //output file
					);

	if(ret == luar_ok)
	{
		//copy tmpl file to src_file
		copy_file(strtmpl.c_str(), str_dest);
	}
	else
	{
		fprintf(stderr,"call error :%s\n",lua_error_msg());
	}

	//remove
	remove(strtmpl.c_str());
	return ;
}


void EditUIPanel::connectEvents()
{
	//ComponentInstance *cinst;
	if(!baseInstance)
		return;
	//if(::IsWindow(curWnd) && (cinst=ComponentInstance::FromHandler(curWnd)))
	{
		try{
			ComponentInstance* cinst = ComponentInstance::FromHandler(curWnd);

			ConnectEventsWnd cew(GetHandle(), baseInstance, cinst, cinst?cinst->getID():0);
			if(cew.DoMode())
			{
				//set source changed
				setSourceChanged();
				setPropertyChanged(FALSE);
			}
            if (cew.toSource()) {
                LOG_WARNING("skip code. \n");
                string event = cew.getEventName();

                if(saveXML(FALSE) && !saveSource())
                    return;
                g_env->saveAll();

                //get the instance file name
                string strFile = g_env->getSourcePath();
                char szBuff[256]="/";
                getSourceFileName(szBuff+1);
                strFile += szBuff;

                SocketClient *sock = SocketClient::getInstance();
                sock->goToCode(strFile.c_str(), event.c_str());
				//hide
                VFBShowWindow(FALSE);
            }
		}catch(const char* err){
			LOG_WARNING("error when connect events:%s",err);
		}
	}
}

///////////////////////////////////////////////////////////

#define IDC_INPUT 		201

static CTRLDATA _input_ctrls [] =
{
	{
		"static", WS_VISIBLE,
		10, 10, 200, 30,
		0,
		"Template Name :",
		0,
        WS_EX_NONE,
        NULL,
        NULL
	},
	{
		"sledit", WS_VISIBLE|WS_BORDER,
		20, 40, 230, 30,
		IDC_INPUT,
		"",
		0,
        WS_EX_NONE,
        NULL,
        NULL
	},
	{
		"button", WS_VISIBLE|BS_PUSHBUTTON,
		110, 95, 70, 25,
		IDOK,
		"OK",
		0,
        WS_EX_NONE,
        NULL,
        NULL
	},
	{
		"button", WS_VISIBLE|BS_PUSHBUTTON,
		190, 95, 70, 25,
		IDCANCEL,
		"Cancel",
		0,
        WS_EX_NONE,
        NULL,
        NULL
	}
};

static DLGTEMPLATE _input_dlg =
{
	WS_BORDER|WS_CAPTION|WS_DLGFRAME,
	WS_EX_NONE,
	200, 150,  280, 160,
	"Export Template",
	0, 0,
	sizeof(_input_ctrls)/sizeof(CTRLDATA),
	_input_ctrls
};

static LRESULT _importProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit;
	char * str_id_name;
	switch(message)
	{
		case MSG_INITDIALOG:
			hEdit = GetDlgItem(hDlg,IDC_INPUT);
			str_id_name = (char*)GetWindowAdditionalData(hDlg);
			SetWindowText(hEdit, str_id_name);
			SetFocus(hEdit);
			SendMessage(hEdit, EM_SELECTALL, 0, 0);
			return 0;
		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
				hEdit = GetDlgItem(hDlg,IDC_INPUT);
				str_id_name = (char*)GetWindowAdditionalData(hDlg);
				if(GetWindowText(hEdit, str_id_name, 256)>0 && ValidIDName(str_id_name))
				{
					EndDialog(hDlg, 1);
					break;
				}
				MessageBox(hDlg,_("Invalid Name, ID name must start with a capital letter, and only contain capital letter, \'_\', digit, e.g. IDB_IMAGE1"),_("Error") ,MB_OK);
				SendMessage(hEdit, EM_SELECTALL, 0, 0);
				SetFocus(hEdit);
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
	}
	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

void EditUIPanel::exportTemplate(void)
{
	char szName[256] = {0};
	string bmpFile, tmplFile;
	string tmplPath = g_env->getConfigFile(USR_TEMPL_PATH);
	tmplPath += "/";

	_input_dlg.dwAddData = (DWORD)szName;
	if(DialogBoxIndirectParam(&_input_dlg, getHandler(), _importProc, 0))
	{
		bmpFile = tmplPath;
		bmpFile += szName;
		bmpFile += ".bmp";
		::SaveBitmapToFile(HDC_SCREEN, &bmpOutline, bmpFile.c_str());

		tmplFile = tmplPath;
		tmplFile += szName;
		tmplFile += ".tmpl";
        FileStreamStorage fss(tmplFile.c_str());
        TextStream stream(&fss);
        //baseInstance->saveXMLToStream(&stream);
        baseInstance->saveIndependXML(&stream, TRUE);
        fss.close();
	}
}
