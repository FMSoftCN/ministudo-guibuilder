/*
 * imageview.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef IMAGEVIEW_H_
#define IMAGEVIEW_H_

#define IS_DEFAULT_IMAGE(bmp) \
	(bmp.bmBits == default_img.bmBits)

class ImageView: public Panel {

	static int iconProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam);
	static void _iconview_notification(HWND hwnd, int id, int nc, DWORD add_data);
	static int _inputProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam);
	void onIconViewNotification(int id, int nc, DWORD add_data);

public:
	class ViewItem
	{
	public:
		ViewItem(){
			fileName = NULL;
			label[0] = 0;
			memset(&bmp, 0, sizeof(bmp));
			hIconItem = 0;
		}
		ViewItem(const char* file, DWORD addData, BOOL bDelayLoadImage = FALSE);
		static BOOL validateFile(const char* file);
		virtual ~ViewItem();
		string strFile;
		const char* fileName;
		char label[10];
		BITMAP bmp;
		GHANDLE hIconItem;
		virtual void setAddData(DWORD addData){}
		virtual DWORD getAddData(){return 0L;}

		void InitFrom(const ViewItem& vi) {
			strFile = vi.strFile;
			fileName = vi.fileName;
			strcpy(label, vi.label);
			hIconItem = 0;
			//copy bitmap
			if(!IS_DEFAULT_IMAGE(bmp))
				UnloadBitmap(&bmp);
			memcpy(&bmp, &vi.bmp, sizeof(bmp));
			if(!IS_DEFAULT_IMAGE(vi.bmp))
			{
				int size = bmp.bmWidth * bmp.bmHeight * bmp.bmBytesPerPixel;
				bmp.bmBits = (unsigned char*)malloc(size);
				if(vi.bmp.bmBits)
					memcpy(bmp.bmBits, vi.bmp.bmBits, size);
				else
					memset(bmp.bmBits, 0xFF, size);

				if(vi.bmp.bmAlphaMask) {
					int size = vi.bmp.bmHeight * ((vi.bmp.bmWidth + 3) & (~3));
					bmp.bmAlphaMask = (Uint8*)calloc(1, size);
					memcpy(bmp.bmAlphaMask, vi.bmp.bmAlphaMask, size);
				}
			}
		}

		void setFile(const char* file);
		void reloadImage(BOOL bForce = TRUE);

		//public
		static int cmp_view_item_desc(const ViewItem& vi1, const ViewItem& vi2)
		{
			return strcmp(vi1.strFile.c_str(), vi2.strFile.c_str());
		}

		static int cmp_view_item_aesc(const ViewItem& vi1, const ViewItem& vi2)
		{
			return -strcmp(vi1.strFile.c_str(), vi2.strFile.c_str());
		}

	};

	class ViewItemIterator {
		friend class ImageView;
		HWND hIconView;
		int count;
		int cur_idx;

		ViewItemIterator(HWND hIconView){
			this->hIconView = hIconView;
			count = SendMessage(hIconView, IVM_GETITEMCOUNT, 0, 0);
			cur_idx = 0;
		}
	public:
		ViewItemIterator() {
			hIconView = HWND_INVALID;
			count = 0;
			cur_idx = 0;
		}
		ViewItemIterator & operator = (const ViewItemIterator& vii){
			hIconView = vii.hIconView;
			count = vii.count;
			cur_idx = vii.cur_idx;
			return *this;
		}
		ViewItemIterator(const ViewItemIterator& vii){
			*this = vii;
		}

		ViewItemIterator& operator ++(){
			cur_idx ++;
			return *this;
		}

		BOOL isEnd(){
			return cur_idx >= count;
		}

		ViewItem* operator*(){
			WNDPROC proc = GetWindowCallbackProc(hIconView);
			if(!proc)
				return NULL;
			return (ViewItem*)proc(hIconView,IVM_GETITEMADDDATA, cur_idx, 0);
		}
		ViewItem* operator->(){
			WNDPROC proc = GetWindowCallbackProc(hIconView);
			if(!proc)
				return NULL;
			return (ViewItem*)proc(hIconView,IVM_GETITEMADDDATA, cur_idx, 0);
		}

		HWND iconView() { return hIconView; }
	};

	ViewItemIterator begin(){
		return ViewItemIterator(hIconView);
	}

	ViewItemIterator at(int idx){
		ViewItemIterator vii(hIconView);
		vii.cur_idx = idx;
		return vii;
	}

	ViewItem * getCurImage(int *pidx = NULL)
	{
		if(pidx)
			*pidx = -1;
		int sel_idx = GetSelectIdx();
		if(sel_idx < 0)
			return NULL;
		if(pidx)
			*pidx = sel_idx;
		return (ViewItem*)SendMessage(hIconView, IVM_GETITEMADDDATA, sel_idx, 0);
	}

protected:

	HWND hIconView;

	HMENU hPopupMenu;

	ViewItem * findImage(const char* file, int *pidx = NULL);
	ViewItem * findImage(DWORD addData, int *pidx = NULL);
	ViewItem * findImage(const char* file, DWORD addData, int *pidx=NULL){
		return file?findImage(file,pidx):findImage(addData,pidx);
	}


	template<int (*cmp_fn)(const ViewItem&,const ViewItem&), class TImageView>
	static int _file_desc_cmp(HSVITEM hsvi1, HSVITEM hsvi2){
		ViewItem *v1 = (ViewItem *)SendMessage(TImageView::_hIconView, IVM_GETITEMADDDATA, 0, hsvi1);
		ViewItem *v2 = (ViewItem *)SendMessage(TImageView::_hIconView, IVM_GETITEMADDDATA, 0, hsvi2);
		if(v1 == NULL)
			return -1;
		else if(v2 == NULL)
			return -2;
		return (*cmp_fn)(*v1, *v2);
	}
	virtual SVITEM_CMP getImageItemSortFunc(BOOL bascend, int by) = 0;

	virtual ViewItem* NewViewItem(const char* file, DWORD addData, BOOL bDelayLoadImage = FALSE);
	virtual ViewItem* NewViewItem(const ViewItem* vi, DWORD addData);

public:

	ImageView(PanelEventHandler* handler);
	virtual ~ImageView();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return hIconView;
	}

	virtual void onRBtnUp(int x, int y, DWORD key_flag) = 0;


	BOOL AddFile(const char* img_file, DWORD addData, BOOL bDelayLoadImage = FALSE);
	BOOL Add(ViewItem* viewitem, DWORD addData, BOOL bDelayLoadImage = FALSE){ return Insert(viewitem, -1, addData, bDelayLoadImage); }
	BOOL InsertFile(const char* img_file, int at, DWORD addData, BOOL bDelayLoadImage = FALSE);
	BOOL Insert(ViewItem * viewitem, int at, DWORD addData, BOOL bDelayLoadImage = FALSE);
	BOOL SelectFile(const char *img_file, DWORD addData=0);
	BOOL RemoveFile(const char* img_file, DWORD addData=0);
	BOOL RefreshFile(const char* img, DWORD addData=0);

	DWORD GetImageAddData(int idx);
	DWORD GetImageAddData(const char* img_file);

	BOOL SetImageAddData(int idx, DWORD add_data);
	BOOL SetImageAddData(const char* img_file, DWORD add_data);

	int GetSelectIdx();
	int GetSelectImage(char* img_file_buff, int nMax);

	BOOL SetItemSize(int w, int h);

	enum sortBy{
			sort_by_name,
			sort_by_date
	};

	BOOL Sort(BOOL bascend, int by);

	int GetCount();

	void Reset();

	//call this when call AddFile or InsertFile with bDelayLoadImage = TRUE
	void SyncLoadImages();

private:
	static pthread_mutex_t   mt_sync_loading;
	static void*  sync_loading_images_proc(void*);
	static pthread_t pt_sync_loading;
	static sem_t     st_sync_loading;
	static int    sync_loading_flags;
	static ImageView * sync_loading_first;
	static BITMAP default_img;

	int           cur_loading_idx;
	ImageView     *sync_loading_next;

protected:
	void lockLoading() { if(cur_loading_idx >= 0) pthread_mutex_lock(&mt_sync_loading); }
	void unlockLoading(BOOL bRevert = FALSE) { 
		if(cur_loading_idx >= 0) 
		{
			if(bRevert)
				cur_loading_idx = 0;
			pthread_mutex_unlock(&mt_sync_loading); 
		}
	}
public:
	static void InitImageView();
	static void UnitImageView();

protected:
	virtual ViewItem* checkAndResetItem(ViewItem* viewItem, DWORD addData, int *pidx = NULL) {
		return NULL;
	}
};


#endif /* IMAGEVIEW_H_ */
