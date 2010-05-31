/*
 * dirimgview.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef DIRIMGVIEW_H_
#define DIRIMGVIEW_H_

#include "imageview.h"

class DirImageView: public ImageView
{
	char *path;   //the path to be shown...

public:
	DirImageView(PanelEventHandler* handler);
	virtual ~DirImageView();

	BOOL SetPath(const char *newpath);
	void Refresh(void);

	void onRBtnUp(int x, int y, DWORD key_flag);

protected:
	virtual SVITEM_CMP getImageItemSortFunc(BOOL bascend, int by)
	{
		_hIconView = hIconView;
		if(by == sort_by_name){
			if(bascend)
				return (SVITEM_CMP)(ImageView::_file_desc_cmp<
						ImageView::ViewItem::cmp_view_item_aesc,
						DirImageView>);
			else
				return (SVITEM_CMP)(ImageView::_file_desc_cmp<
						ImageView::ViewItem::cmp_view_item_desc,
						DirImageView>);
		}
		return NULL;
	}
public:
	static HWND _hIconView;
};

#endif /* DIRIMGVIEW_H_ */
