/*
 * editable-listview.h
 *
 *  Created on: 2009-3-23
 *      Author: dongjunjie
 */

#ifndef EDITABLELISTVIEW_H_
#define EDITABLELISTVIEW_H_

class EditableListView;

class EditableListViewHandler
{
public:
	virtual ~EditableListViewHandler(){}
	virtual HWND getEditor(HWND hParent, DWORD add_data, int sub_index) = 0;
};

class EditableListView : public TMGStaticSubclass<MGListView>
{
	static void editable_listview_notifi(HWND hwnd, int id, int nc, DWORD add_data);

	EditableListViewHandler * handler ;

	HWND getEditor(DWORD add_data, int sub_index){
		if(handler)
			return handler->getEditor(m_hWnd, add_data, sub_index);
		return HWND_INVALID;
	}

protected:
	BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret);

public:
	EditableListView(EditableListViewHandler * handler=NULL);
	virtual ~EditableListView();

	BOOL Create(HWND hParent, int x, int y, int w, int h, DWORD dwStyle, DWORD dwExStyle);

	void setHandler(EditableListViewHandler * handler){
		this->handler = handler;
	}

	void clearAll();

	BOOL getCurData(DWORD* pdata){
		if(pdata && (hOldSelItem != (HLVITEM)0))
		{
			*pdata = GetItemData(hOldSelItem);
			return TRUE;
		}
		return FALSE;
	}

	BOOL setCurData(DWORD data){
		if(hOldSelItem != (HLVITEM)0){
			return SetItemAddData(hOldSelItem,data) == 0;
		}
		return FALSE;
	}

	HLVITEM findByAddData(DWORD data,HLVITEM pi=(HLVITEM)0);

	HWND getCurrentEditor(int sub_idx){
		if(sub_idx < 0 || sub_idx >= count)
			return HWND_INVALID;
		return editors[sub_idx];
	}

	void deleteItem(HLVITEM hlv, BOOL autoSelect = TRUE);

protected:

	HWND* editors;
	int count;
	HLVITEM hOldSelItem;

	inline void checkEditorArray(){
		int newcount = GetColumnCount();
		if(editors==NULL || count != newcount){
			HWND *htmp = new HWND[newcount];
			if(editors)
				memcpy(htmp, editors, sizeof(HWND)*(count<newcount)?count:newcount);

			if(count < newcount)
				memset(htmp+count, 0, sizeof(HWND)*(newcount-count));

			if(editors)
				delete[] editors;
			editors = htmp;
			count = newcount;
		}
	}

	void hideAllEditors(BOOL updateItem = TRUE);
	void showEditors(int show_cmd);
	void offsetEditors(int xoff, int yoff, BOOL bUpdate);

public:
	virtual void updateEditors();
  
private:
	static int my_strcmp (const char* s1, const char* s2, size_t n)
	{
		if(s1 == NULL && s2 == NULL)
			return 0;
		else if(s1 == NULL)
			return -1;
		else if(s2 == NULL)
			return 1;
		else
			return strncmp(s1, s2, n);
	}

};

#endif /* EDITABLELISTVIEW_H_ */
