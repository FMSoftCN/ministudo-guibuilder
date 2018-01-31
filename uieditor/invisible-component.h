/*
 * invisible-component.h
 *
 *  Created on: 2009-4-30
 *      Author: dongjunjie
 */

#ifndef INVISIBLECOMPONENT_H_
#define INVISIBLECOMPONENT_H_

class InvisibleComponent: public ComponentInstance {
public:
	InvisibleComponent(Class *cls);
	InvisibleComponent(const InvisibleComponent& invcomp);
	virtual ~InvisibleComponent();

	virtual HWND createPreviewWindow(HWND hParent = HWND_INVALID);
	virtual void destroyPreviewWindow();
	virtual int syncPreviewWindow(int field_id);

	Instance * clone();

	BOOL loadFromXML(xmlNodePtr node);

	virtual void saveXMLToStream(TextStream *stream);

	//return size of total window
	virtual int saveBinToStream(BinStream *stream);

	const char* getControlClass();

	void previewWindow(BOOL bPreview = TRUE){
		if(IsWindow(hwnd))
			ShowWindow(hwnd, bPreview?SW_HIDE:SW_SHOW);
	}

	void toLuaTable(void *luaState, int table_idx);

private:
	static LRESULT _invcomp_preview_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static PBITMAP getBitmapByInstance(Class *cls);
	static map<DWORD, BITMAP> _comp_bmps;

	PBITMAP pshow_bmp;

protected:
	char* getClassName(char* szBuff);
};

#endif /* INVISIBLECOMPONENT_H_ */
