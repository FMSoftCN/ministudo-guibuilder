/**
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include <set>

#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "../uieditor/component-instance.h"
#include "../uieditor/window-instance.h"
#include "rdr-instance.h"
#include "rdr-event-id.h"

#include "panel.h"

#include "rdr-preview-panel.h"

RendererPreviewPanel::RendererPreviewPanel(PanelEventHandler* handler)
:Panel(handler)
{
	inst = NULL;
}

RendererPreviewPanel::~RendererPreviewPanel() {
}

HWND RendererPreviewPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);
	TMGStaticSubclass<MGStatic>::Create("", hParent, 0, 0, RECTW(rt),RECTH(rt), WS_VISIBLE|WS_BORDER, 0, -1);
	::SetWindowBkColor(m_hWnd, PIXEL_lightwhite);
	//::UpdateWindow(m_hWnd, TRUE);
	InvalidateRect();
	Subclass();

	_oldProc = DefaultControlProc;

	return getHandler();
}

BEGIN_MSG_MAP(RendererPreviewPanel)
	MAP_PAINT(onPaint)
END_MSG_MAP

void RendererPreviewPanel::updateInstanceField(RendererInstance *instance, int* ids)
{
	if (!instance || instance != inst)
		return;

	inst->updatePreviewWindow(ids);

	if(::IsMainWindow(inst->getHandler()))
		InvalidateRect();
}

// ComponetInstance Interface Implements
static int DoubleBufferProc(HWND hwnd, HDC private_dc, HDC real_dc,  const RECT* update_rc, const RECT* real_rc, const RECT* main_rc)
{
	BitBlt(private_dc,update_rc->left, update_rc->top, RECTWP(update_rc), RECTHP(update_rc), real_dc, real_rc->left, real_rc->top,0);
	return 0;
}

static void draw_caption (HWND hWnd, HDC hdc, BOOL is_active)
{
	WindowInstance* win = (WindowInstance*)GetWindowAdditionalData(hWnd);
	if(win && win->old_win_rdr)
		return win->old_win_rdr->draw_caption(hWnd, hdc, TRUE);
}

void draw_border(HWND hWnd, HDC hdc, BOOL is_active)
{
	WindowInstance* win = (WindowInstance*)GetWindowAdditionalData(hWnd);
	if(win && win->old_win_rdr)
		return win->old_win_rdr->draw_border(hWnd, hdc, TRUE);
}

static void dumpWindowRenderer(HWND hwnd)
{
	static WINDOW_ELEMENT_RENDERER editor_win_rdr = {{0}, };
	ComponentInstance *inst = ComponentInstance::FromHandler(hwnd);
	WINDOWINFO *win_info;
	if(!inst)
		return;

	win_info = (WINDOWINFO*)GetWindowInfo(hwnd);
	if(!win_info)
		return ;

	WINDOW_ELEMENT_RENDERER *win_rdr = (WINDOW_ELEMENT_RENDERER*)(win_info->we_rdr);
	if(win_rdr == NULL)
		return;

	(static_cast<WindowInstance*>(inst))->old_win_rdr = win_rdr;
	memcpy(&editor_win_rdr, win_rdr, sizeof(editor_win_rdr));
	editor_win_rdr.draw_caption = draw_caption;
	editor_win_rdr.draw_border  = draw_border;
	win_info->we_rdr = &editor_win_rdr;
}

HWND RendererPreviewPanel::createPreviewWindow(RendererInstance *instance)
{
	//no changed
	if (inst && (inst == instance))
		return HWND_NULL;

	//changed
	destroyPreviewWindow();

	if (instance) {
		HWND hwnd;
		ExcludeWindowStyle(getHandler(), WS_VISIBLE);
		if (HWND_NULL != (hwnd = instance->createPreviewWindow(getHandler()))) {
			inst = instance;
			if(::IsMainWindow(hwnd))
			{
				RECT rt;
				dumpWindowRenderer(hwnd);
				::SetSecondaryDC(hwnd,::GetSecondaryDC(hwnd),DoubleBufferProc);
				::GetWindowRect(hwnd,&rt);
				::MoveWindow(hwnd, -RECTW(rt), -RECTH(rt), RECTW(rt), RECTH(rt), FALSE);
			}
			IncludeWindowStyle(getHandler(), WS_VISIBLE);
			InvalidateRect();
			::ShowWindow(hwnd, SW_SHOW);
			return inst->getHandler();
		}
		IncludeWindowStyle(getHandler(), WS_VISIBLE);

	}
	return HWND_NULL;
}

void RendererPreviewPanel::onPaint(HDC hdc)
{
	if(inst && ::IsMainWindow(inst->getHandler())){
		HWND hMain = inst->getHandler();
		ComponentInstance * cinst = ComponentInstance::FromHandler(hMain);
		if(!cinst)
			return ;
		cinst->previewWindow(TRUE);
		cinst->updatePrevWindow();
		HDC hdc_main = ::GetSecondaryDC(hMain);
		if(hdc_main == HWND_NULL)
			return;
		int x, y, w, h;
		cinst->getLocation(x,y);
		cinst->getSize(w,h);

		BitBlt(hdc_main,0, 0, w, h , hdc, x, y, 0);
	}
}

void RendererPreviewPanel::destroyPreviewWindow()
{
	if (inst) {
		inst->destroyPreviewWindow();
		inst = NULL;
	}
}
