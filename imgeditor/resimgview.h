/*
 * resimgview.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef RESIMGVIEW_H_
#define RESIMGVIEW_H_


class ResImageView: public ImageView {

public:

	class ResViewItem : public ImageView::ViewItem
	{
	public:
		ResViewItem() {
			id = 0;
		}
		ResViewItem(const char* file, DWORD add_data, BOOL bDelayLoadImage)
		:ImageView::ViewItem(file, add_data, bDelayLoadImage){
			id = (int)add_data;
		}

		int id;
		void setAddData(DWORD addData){
			id = (int)addData;
		}
		DWORD getAddData(){return (DWORD)id;}
	};

	ViewItem* NewViewItem(const char* file, DWORD addData, BOOL bDelayLoadImage=FALSE){
		if(!ImageView::ViewItem::validateFile(file))
			return NULL;
		return new ResViewItem(file, addData, bDelayLoadImage);
	}

	ViewItem * NewViewItem(const ViewItem *vi, DWORD addData) {
		if(!vi)
			return NULL;
		ResViewItem * rvi = new ResViewItem();
		rvi->InitFrom(*vi);
		rvi->setAddData(addData);
		return (ViewItem*)rvi;
	}

	ResImageView(PanelEventHandler* handler);
	virtual ~ResImageView();

	void onRBtnUp(int x, int y, DWORD key_flag);

	//return nexted selected id
	int removeSelectedImage();

	int getSelectedId(){
		ResViewItem * item = (ResViewItem*)getCurImage();
		if(item)
			return item->id;
		return -1;
	}

protected:
	virtual SVITEM_CMP getImageItemSortFunc(BOOL bascend, int by)
	{
		_hIconView = hIconView;
		if(by == sort_by_name){
			if(bascend)
				return (SVITEM_CMP)(ImageView::_file_desc_cmp<
						ImageView::ViewItem::cmp_view_item_aesc,
						ResImageView>);
			else
				return (SVITEM_CMP)(ImageView::_file_desc_cmp<
						ImageView::ViewItem::cmp_view_item_desc,
						ResImageView>);
		}
		return NULL;
	}

	ViewItem* checkAndResetItem(ViewItem* viewItem, DWORD addData, int *pidx);

public:
	static HWND _hIconView;

};

#endif /* RESIMGVIEW_H_ */
