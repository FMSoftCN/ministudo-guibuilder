

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
extern int usleep(unsigned long);
#else
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
#ifdef WIN32
#include "func-win.h"
#endif

static WNDPROC def_icon_proc;

#define IMAGE_WIDTH    120
#define IMAGE_HEIGHT   120

ImageView::ImageView(PanelEventHandler* handler) : Panel(handler)
{
	cur_loading_idx = -1;
	sync_loading_next = NULL;
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

LRESULT ImageView::iconProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ImageView *_this = (ImageView *)GetWindowAdditionalData(hwnd);
	if (message == MSG_RBUTTONDOWN)
	{
		SendMessage(hwnd, MSG_LBUTTONDOWN, wParam, lParam);
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

BOOL ImageView::AddFile(const char* img_file, DWORD addData, BOOL bDelayLoadImage)
{
	return InsertFile(img_file, -1, addData, bDelayLoadImage);
}

BOOL ImageView::InsertFile(const char* img_file, int at, DWORD addData,BOOL bDelayLoadImage)
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
			SendMessage(getHandler(), IVM_REFRESHITEM, 0, (LPARAM)item->hIconItem);
			return TRUE;
		}

		return FALSE;
	}

	item = NewViewItem(img_file, addData, bDelayLoadImage);
	if(!item)
		return FALSE;

	ivi.nItem   = at;
	//ivi.label   = item->fileName;
	ivi.label   = item->label;
	ivi.bmp 	= &item->bmp;
	ivi.addData = (DWORD)item;

	lockLoading();
	SendMessage(hIconView, IVM_ADDITEM, (WPARAM)(&item->hIconItem), (LPARAM)&ivi);
	unlockLoading(TRUE);

	return TRUE;
}

BOOL ImageView::Insert(ViewItem* viewitem, int at, DWORD addData, BOOL bDelayLoadImage)
{
	IVITEMINFO ivi;
	BOOL bNewItem = FALSE;
	int idx = -1;

	ViewItem* item = checkAndResetItem(viewitem, addData, &idx);
		
	if(!item)
	{
		item = NewViewItem(viewitem, addData);
	}

	if(!item)
		return FALSE;

	if(!bDelayLoadImage)
		item->reloadImage();

	if(idx >= 0)
	{
		//refresh the item
		SendMessage(hIconView, IVM_REFRESHITEM, idx, 0);
	}
	else
	{
		ivi.nItem   = at;
		//ivi.label   = item->fileName;
		ivi.label   = item->label;
		ivi.bmp 	= &item->bmp;
		ivi.addData = (DWORD)item;

		lockLoading();
		SendMessage(hIconView, IVM_ADDITEM, (WPARAM)(&item->hIconItem), (LPARAM)&ivi);
		unlockLoading(TRUE);
	}

	return TRUE;
}

BOOL ImageView::RemoveFile(const char* img_file, DWORD addData)
{
	ViewItem * item = findImage(img_file,addData);
	if(item)
	{
		lockLoading();
		SendMessage(hIconView, IVM_DELITEM, 0, (LPARAM)item->hIconItem);
		delete item;
		unlockLoading(TRUE);
		return TRUE;
	}

	return FALSE;
}

BOOL ImageView::RefreshFile(const char* img, DWORD addData)
{
	ViewItem * item = findImage(img,addData);
	if(item)
	{
		SendMessage(hIconView, IVM_REFRESHITEM, 0, (LPARAM)item->hIconItem);
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

	lockLoading();
	SendMessage(hIconView, IVM_SORTITEMS, 0, (LPARAM)cmp_fn);
	unlockLoading(TRUE);

	return TRUE;

}

int ImageView::GetCount()
{
	//return view_list.size();
	return SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0);
}

void ImageView::Reset()
{
	lockLoading();
	for(int i=0; i<GetCount(); i++){
		ViewItem * item = (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, i, 0);
        delete item;
	}
	SendMessage(hIconView, IVM_RESETCONTENT, 0, 0);
	unlockLoading(TRUE);
	sendEvent(IMAGE_VIEW_SEL_CHANGED);
}

ImageView::ViewItem* ImageView::NewViewItem(const char* file, DWORD addData, BOOL bDelayLoadImage)
{
	if(!ViewItem::validateFile(file))
		return NULL;
	return new ImageView::ViewItem(file,addData, bDelayLoadImage);
}

ImageView::ViewItem* ImageView::NewViewItem(const ViewItem* vi, DWORD addData)
{
	if(!vi)
		return NULL;
	ViewItem * newvi = new ViewItem;
	newvi->InitFrom(*vi);
	return newvi;
}


////////////////////////////////////////////////////
//
pthread_mutex_t ImageView::mt_sync_loading;
pthread_t ImageView::pt_sync_loading;
sem_t ImageView::st_sync_loading;
int ImageView::sync_loading_flags;
ImageView* ImageView::sync_loading_first;
BITMAP ImageView::default_img;


void ImageView::SyncLoadImages()
{
	//insert me into the waiting list
	pthread_mutex_lock(&mt_sync_loading);
	cur_loading_idx = 0;
	ImageView* tmp = sync_loading_first;
	while(tmp && tmp != this) 
		tmp = tmp->sync_loading_next;
	if(!tmp)
	{
		sync_loading_next = sync_loading_first;
		sync_loading_first = this;
		sem_post(&st_sync_loading);
	}
	pthread_mutex_unlock(&mt_sync_loading);
}

void* ImageView::sync_loading_images_proc(void* param)
{

	while(sync_loading_flags)
	{
		if(sync_loading_first == NULL)
			sem_wait(&st_sync_loading);

		pthread_mutex_lock(&mt_sync_loading);
		ImageView * view = sync_loading_first;
		ImageView * prev = NULL;
		while(view)
		{
			WNDPROC view_proc = GetWindowCallbackProc(view->hIconView);
			int count = view_proc(view->hIconView, IVM_GETITEMCOUNT, 0, 0);
			//printf("-- view=%p, count=%d\n", view, count);
			if(view->cur_loading_idx >= count) {
				ImageView * tmp = view;
				//Remove this 
				if(prev == NULL)
					sync_loading_first = view->sync_loading_next;
				else
				{
					prev->sync_loading_next = view->sync_loading_next;
				}
				view = view->sync_loading_next;
				tmp->sync_loading_next = NULL;
				tmp->cur_loading_idx = -1;
				//printf("-- remove view=%p, new view=%p\n",tmp, view);
				continue;
			}

			while(view->cur_loading_idx < count)
			{
				//printf("view=%p cur_loading_idx=%d\n",view, view->cur_loading_idx);
				ViewItem * item = (ViewItem*)view_proc(view->hIconView, IVM_GETITEMADDDATA, view->cur_loading_idx++, 0);
				if(!item || !IS_DEFAULT_IMAGE(item->bmp))
					continue;
				item->reloadImage(FALSE);
				//update the function
				PostMessage(view->hIconView, IVM_REFRESHITEM, 0, (LPARAM)item->hIconItem);
				break;
			}
			prev = view;
			view = view->sync_loading_next;
		}
		pthread_mutex_unlock(&mt_sync_loading);
		usleep(1000);
	}

	pthread_mutex_destroy(&mt_sync_loading);
	sem_destroy(&st_sync_loading);

	return NULL;
}

void ImageView::InitImageView()
{
	//LoadImage
	string str = g_env->getConfigFile("imgeditor/default_img.png");
	LoadBitmapFromFile(HDC_SCREEN, &default_img, str.c_str());
	pthread_mutex_init(&mt_sync_loading, NULL);
	sem_init(&st_sync_loading, 0, 0);
	sync_loading_flags = 1;
	pthread_create(&pt_sync_loading, NULL, sync_loading_images_proc, NULL);
}

void ImageView::UnitImageView()
{
	sync_loading_flags = 0;
	sem_post(&st_sync_loading);
	//UnloadImage
	UnloadBitmap(&default_img);
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

ImageView::ViewItem::ViewItem(const char* file, DWORD addData, BOOL bDelayLoadImage)
{
	memset(&bmp, 0, sizeof(bmp));
	setFile(file);
	if(!bDelayLoadImage)
		reloadImage();
	else
		memcpy(&bmp, &default_img, sizeof(bmp));
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

#ifdef WIN32
	memset(label,0, sizeof(label));
	asciitoutf8(fileName, label, sizeof(label)-1);
#else
	strncpy(label, fileName, sizeof(label)-1);
	label[sizeof(label)-1] = '\0';
#endif
}

void ImageView::ViewItem::reloadImage(BOOL bForce)
{
	if(!bForce && !IS_DEFAULT_IMAGE(bmp))
		return;
	else
	{
		if(!IS_DEFAULT_IMAGE(bmp))
			UnloadBitmap(&bmp);
	}

	struct stat statbuf;

	stat(strFile.c_str(), &statbuf);

	if (statbuf.st_size < 1024 * 20)
	{
		if (ERR_BMP_OK != LoadBitmap(HDC_SCREEN, &bmp, strFile.c_str()))
			return;
	}
	else
	{
		BITMAP tmp_bmp;
		if (ERR_BMP_OK !=LoadBitmap(HDC_SCREEN, &tmp_bmp, strFile.c_str()))
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
	if(!IS_DEFAULT_IMAGE(bmp))
		UnloadBitmap(&bmp);
}
