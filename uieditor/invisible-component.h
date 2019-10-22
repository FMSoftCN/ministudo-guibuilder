/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
