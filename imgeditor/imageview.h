/*
 * imageview.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef IMAGEVIEW_H_
#define IMAGEVIEW_H_

class ImageView: public Panel {

	static int iconProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam);
	static void _iconview_notification(HWND hwnd, int id, int nc, DWORD add_data);
	static int _inputProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam);
	void onIconViewNotification(int id, int nc, DWORD add_data);

public:
	class ViewItem
	{
	public:
		ViewItem(const char* file, DWORD addData);
		static BOOL validateFile(const char* file);
		virtual ~ViewItem();
		string strFile;
		const char* fileName;
		char label[10];
		BITMAP bmp;
		GHANDLE hIconItem;
		virtual void setAddData(DWORD addData){}
		virtual DWORD getAddData(){return 0L;}

		void setFile(const char* file);

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
			return (ViewItem*)SendMessage(hIconView,IVM_GETITEMADDDATA, cur_idx, 0);
		}
		ViewItem* operator->(){
			return (ViewItem*)SendMessage(hIconView,IVM_GETITEMADDDATA, cur_idx, 0);
		}
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

	virtual ViewItem* NewViewItem(const char* file, DWORD addData);

public:

	ImageView(PanelEventHandler* handler);
	virtual ~ImageView();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return hIconView;
	}

	virtual void onRBtnUp(int x, int y, DWORD key_flag) = 0;


	BOOL AddFile(const char* img_file, DWORD addData);
	BOOL InsertFile(const char* img_file, int at, DWORD addData);
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

private:
};

#endif /* IMAGEVIEW_H_ */
