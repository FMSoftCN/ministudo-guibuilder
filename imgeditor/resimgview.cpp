/*
 * resimgview.cpp
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <map>
#include "mapex.h"
#include <list>
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "panel.h"
#include "resenv.h"
#include "imageview.h"
#include "resimgview.h"
#include "dirtreepanel.h"
#include "imgres-listpanel.h"
#include "imageview.h"
#include "dirimgview.h"
#include "resimgview.h"
#include "resdescpanel.h"
#include "stream.h"
#include "panellayout.h"
#include "reseditor.h"
#include "imgeditor.h"
#include "img-event-id.h"

ResImageView::ResImageView(PanelEventHandler* handler):ImageView(handler)
{

}

ResImageView::~ResImageView()
{

}

int ResImageView::removeSelectedImage()
{
	int sel_idx = SendMessage(hIconView, IVM_GETCURSEL, 0, 0);
	if(sel_idx < 0)
		return -1;

	ResViewItem * item = (ResViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, sel_idx, 0);
	if(!item || !sendEvent(IMAGE_VIEW_REMOVED,(DWORD)item->strFile.c_str(), (DWORD)item->id)){
		return -1;
	}
	lockLoading();
	delete item;
	SendMessage(hIconView, IVM_DELITEM, sel_idx, 0);
	unlockLoading(TRUE);
	int count = GetCount();
	if(count > 0){
		if(sel_idx >= count)
			sel_idx = 0;
		SendMessage(hIconView, IVM_SETCURSEL, sel_idx, 0);
		item = (ResViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, sel_idx, 0);
		if(item)
			sendEvent(IMAGE_VIEW_SEL_CHANGED, (DWORD)item->strFile.c_str(), item->getAddData());
		return sel_idx;
	}
	else
	{
		sendEvent(IMAGE_VIEW_SEL_CHANGED);
	}
	return 0;
}

void ResImageView::onRBtnUp(int x, int y, DWORD key_flag)
{
	mapex<int, int>idsets;

	idsets[ImageEditor::IMG_MENUCMD_SORT] = MAP_MASK_STATE(0, 0);
	//idsets[ImageEditor::IMG_MENUCMD_CLEAN] = MAP_MASK_STATE(0, 0);
	idsets[ImageEditor::IMG_MENUCMD_REMOVE] = MAP_MASK_STATE(0, 0);

	hPopupMenu = g_env->createPopMenuFromConfig(ImageEditor::IMG_POPMENU, idsets);
	if(hPopupMenu != 0 && -1 != GetSelectIdx())
	{
		TrackPopupMenu (hPopupMenu, TPM_DEFAULT, x, y, getHandler());
	}
}

HWND ResImageView::_hIconView;


ImageView::ViewItem* ResImageView::checkAndResetItem(ViewItem* viewItem, DWORD addData, int *pidx)
{
	ResImageView::ResViewItem *vi = (ResImageView::ResViewItem*)findImage(addData, pidx);
	if(vi)
	{
		vi->InitFrom(*viewItem);
	}
	return (ViewItem*)vi;
}

