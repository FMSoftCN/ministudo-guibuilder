

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <dirent.h>
#include <sys/stat.h>

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

#include "stream.h"
#include "resenv.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "dirtreepanel.h"
#include "imgres-listpanel.h"
#include "imageview.h"
#include "dirimgview.h"
#include "resimgview.h"
#include "resdescpanel.h"
#include "imgeditor.h"
#include "img-event-id.h"

static WNDPROC def_icon_proc;

#define IMAGE_WIDTH    120
#define IMAGE_HEIGHT   120

ImageView::ImageView(PanelEventHandler* handler) : Panel(handler)
{

}

ImageView::~ImageView()
{
	Reset();
}


HWND ImageView::createPanel(HWND hParent)
{
	RECT rt, margins = {10, 10, 10, 10};;

	GetClientRect(hParent, &rt);

	hIconView = CreateWindow(CTRL_ICONVIEW, "",
									WS_VISIBLE|WS_CHILD|WS_VSCROLL|WS_HSCROLL|IVS_LOOP, 3,
									0, 0, RECTW(rt), RECTH(rt),
									hParent, (DWORD)this);

	SendMessage (hIconView, IVM_SETITEMSIZE, (WPARAM)IMAGE_WIDTH, (LPARAM)IMAGE_HEIGHT);
	SendMessage (hIconView, IVM_SETMARGINS, 0, (LPARAM)&margins);

	SetNotificationCallback(hIconView, ImageView::_iconview_notification);

	def_icon_proc = GetWindowCallbackProc(hIconView);
	SetWindowCallbackProc(hIconView, ImageView::iconProc);

	return hIconView;
}

int ImageView::iconProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	ImageView *_this = (ImageView *)GetWindowAdditionalData(hwnd);
	if (message == MSG_RBUTTONDOWN)
	{
		int x = LOSWORD (lParam);
		int y = HISWORD (lParam);
		ClientToScreen(hwnd, &x, &y);
		_this->onRBtnUp(x, y, 0);
		return 0;
	} else if (message == MSG_COMMAND
            && wParam >= ImageEditor::IMG_MENUCMD_SORT
            && wParam <= ImageEditor::IMG_MENUCMD_IMPORTALL) {
        ImageEditor* resMgr = (ImageEditor*)(g_env->getResManager(NCSRT_IMAGE));
        resMgr->executeCommand((int)wParam, 0, (DWORD)_this);
		return 0;
	}

	return def_icon_proc(hwnd, message, wParam, lParam);
}

void ImageView::_iconview_notification(HWND hwnd, int id, int nc, DWORD add_data)
{
	ImageView* _this = (ImageView*)GetWindowAdditionalData(hwnd);

	if(_this)
		_this->onIconViewNotification(id, nc, add_data);
}

void ImageView::onIconViewNotification(int id, int nc, DWORD add_data)
{
	if (nc == IVN_SELCHANGED){
		ViewItem * item = (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, 0, (LPARAM)add_data);
		if(item)
		{
			sendEvent(IMAGE_VIEW_SEL_CHANGED, (DWORD)item->strFile.c_str(), item->getAddData());
		}
	 }
}

BOOL ImageView::AddFile(const char* img_file, DWORD addData)
{
	return InsertFile(img_file, -1, addData);
}

BOOL ImageView::InsertFile(const char* img_file, int at, DWORD addData)
{
	ViewItem *item = NULL;
	IVITEMINFO ivi;

	//is exist?
	item = findImage(img_file);
	if(item)
	{
		if(ViewItem::validateFile(img_file))
		{
			item->setFile(img_file);
			//refresh
			SendMessage(getHandler(), IVM_REFRESHITEM, 0, item->hIconItem);
			return TRUE;
		}

		return FALSE;
	}

	item = NewViewItem(img_file, addData);
	if(item == NULL || item->bmp.bmBits == NULL)
		return FALSE;

	ivi.nItem   = at;
	//ivi.label   = item->fileName;
	ivi.label   = item->label;
	ivi.bmp 	= &item->bmp;
	ivi.addData = (DWORD)item;

	SendMessage(hIconView, IVM_ADDITEM, (WPARAM)(&item->hIconItem), (LPARAM)&ivi);

	return TRUE;
}

BOOL ImageView::RemoveFile(const char* img_file, DWORD addData)
{
	ViewItem * item = findImage(img_file,addData);
	if(item)
	{
		SendMessage(hIconView, IVM_DELITEM, 0, item->hIconItem);
		delete item;
		return TRUE;
	}

	return FALSE;
}

BOOL ImageView::RefreshFile(const char* img, DWORD addData)
{
	ViewItem * item = findImage(img,addData);
	if(item)
	{
		SendMessage(hIconView, IVM_REFRESHITEM, 0, item->hIconItem);
	}

	return TRUE;
}

BOOL ImageView::SelectFile(const char* img_file, DWORD addData)
{
	int idx = -1;
	ViewItem* item = findImage(img_file, addData, &idx);
	if(item)
	{
		SendMessage(hIconView, IVM_SETCURSEL, idx, (LPARAM)TRUE);
		return TRUE;
	}
	return FALSE;
}

DWORD ImageView::GetImageAddData(int idx)
{
	ViewItem * item = (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	if(item)
		return item->getAddData();
	return 0L;
}

DWORD ImageView::GetImageAddData(const char* img_file)
{
	ViewItem * item = findImage(img_file);
	return item?item->getAddData():0L;
}

BOOL ImageView::SetImageAddData(int idx, DWORD add_data)
{
	ViewItem * item = (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, idx, 0);
	if(item){
		item->setAddData(add_data);
		return TRUE;
	}
	return FALSE;

}

BOOL ImageView::SetImageAddData(const char* img_file, DWORD add_data)
{
	ViewItem * item = findImage(img_file);
	if(item){
		item->setAddData(add_data);
		return TRUE;
	}
	return FALSE;
}

int ImageView::GetSelectIdx()
{
	return SendMessage(hIconView, IVM_GETCURSEL, 0, 0);
}

int ImageView::GetSelectImage(char* img_file_buff, int nMax)
{
	int sel_idx;
	ViewItem * item = getCurImage(&sel_idx);

	if(item == NULL)
		return sel_idx;

	strncpy(img_file_buff, item->strFile.c_str(), nMax);

	img_file_buff[nMax-1] = '\0';
	return sel_idx;

}

BOOL ImageView::SetItemSize(int w, int h)
{
	SendMessage(hIconView, IVM_SETITEMSIZE, (WPARAM)w, (LPARAM)h);
	return TRUE;
}

BOOL ImageView::Sort(BOOL bascend, int by)
{
	SVITEM_CMP cmp_fn = getImageItemSortFunc(bascend,by);

	if(!cmp_fn)
		return FALSE;

	SendMessage(hIconView, IVM_SORTITEMS, 0, (LPARAM)cmp_fn);

	return TRUE;

}

int ImageView::GetCount()
{
	//return view_list.size();
	return SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0);
}

void ImageView::Reset()
{
	for(int i=0; i<GetCount(); i++){
		ViewItem * item = (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, i, 0);
		if(item)
			delete item;
	}
	SendMessage(hIconView, IVM_RESETCONTENT, 0, 0);
	sendEvent(IMAGE_VIEW_SEL_CHANGED);
}

ImageView::ViewItem* ImageView::NewViewItem(const char* file, DWORD addData)
{
	if(!ViewItem::validateFile(file))
		return NULL;
	return new ImageView::ViewItem(file,addData);
}

//////////////////////////////////////////////

#define IS_THIS_FILE(fileextend, type)		(strcasecmp(fileextend, type) == 0)

//FIXME, add all the image format support by minigui
BOOL ImageView::ViewItem::validateFile(const char* file)
{
	if(file == NULL)
		return FALSE;

	const char* strextend = strrchr(file, '.');
	if(strextend == NULL)
		return FALSE;
	strextend ++;

	return (IS_THIS_FILE(strextend, "bmp")
#ifdef _MGIMAGE_PNG
						|| IS_THIS_FILE(strextend, "png")
#endif
#ifdef _MGIMAGE_JPG
						|| IS_THIS_FILE(strextend, "jpg")
						|| IS_THIS_FILE(strextend, "jpeg")
#endif
#ifdef _MGIMAGE_GIF
						|| IS_THIS_FILE(strextend, "gif")
#endif
#ifdef _MGIMAGE_LBM
						|| IS_THIS_FILE(strextend, "lbm")
#endif
#ifdef _MGIMAGE_PCX
						|| IS_THIS_FILE(strextend, "pcx")
#endif
#ifdef _MGIMAGE_TGA
						|| IS_THIS_FILE(strextend, "tga")
#endif
						);
}

ImageView::ViewItem::ViewItem(const char* file, DWORD addData)
{
	memset(&bmp, 0, sizeof(bmp));
	setFile(file);
	setAddData(addData);
}

void ImageView::ViewItem::setFile(const char* file)
{
	strFile = file;
	fileName = strrchr(strFile.c_str(),'/');
	if(fileName == NULL)
	{
		fileName = strrchr(strFile.c_str(), '\\');
		if(fileName == NULL)
			fileName = strFile.c_str();
	}
	fileName ++;

	strncpy(label, fileName, sizeof(label)-1);
	label[sizeof(label)-1] = '\0';

	UnloadBitmap(&bmp);

	struct stat statbuf;

	stat(file, &statbuf);

	if (statbuf.st_size < 1024 * 20)
	{
		if (ERR_BMP_OK != LoadBitmap(HDC_SCREEN, &bmp, file))
			return;
	}
	else
	{
		BITMAP tmp_bmp;
		if (ERR_BMP_OK !=LoadBitmap(HDC_SCREEN, &tmp_bmp, file))
			return;

		InitBitmap (HDC_SCREEN, 80, 80, 80 * GetGDCapability (HDC_SCREEN, GDCAP_BPP), NULL, &bmp);
		bmp.bmAlphaMask = tmp_bmp.bmAlphaMask;
		bmp.bmType = tmp_bmp.bmType;
		InitBitmapPixelFormat(HDC_SCREEN, &bmp);

		ScaleBitmap(&bmp, &tmp_bmp);
		UnloadBitmap(&tmp_bmp);
	}
}

ImageView::ViewItem * ImageView::findImage(const char* file, int *pidx)
{
	int count = GetCount();
	for(int i=0; i < count; i++){
		ImageView::ViewItem * item = (ImageView::ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, i,0);
		if(item)
		{
			if(strcmp(item->strFile.c_str(),file) == 0){
				if(pidx)
					*pidx = i;
				return item;
			}
		}
	}
	return NULL;
}
ImageView::ViewItem * ImageView::findImage(DWORD addData, int *pidx)
{
	int count = GetCount();
	for(int i=0; i < count; i++){
		ImageView::ViewItem * item = (ImageView::ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, i,0);
		if(item)
		{
			if(addData == item->getAddData()){
				if(pidx)
					*pidx = i;
				return item;
			}
		}
	}
	return NULL;
}

ImageView::ViewItem::~ViewItem()
{
	UnloadBitmap(&bmp);
}
