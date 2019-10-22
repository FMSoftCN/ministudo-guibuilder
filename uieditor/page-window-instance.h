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

#ifndef PAGEWINDOWINSTANCE_H_
#define PAGEWINDOWINSTANCE_H_

class PageWindowInstance: public WindowInstance {
public:
	PageWindowInstance(Class* cls);
	PageWindowInstance(const PageWindowInstance & pwi);
	virtual ~PageWindowInstance();
	virtual DWORD processMenuCommand(int cmdid);

	virtual const char* getClassType(){ return "paged-window"; }

	void setField(int id, Value value);

	int hittest(int x, int y);

	virtual BOOL insert(ComponentInstance* insert, BOOL bAutoCreate = TRUE);
	virtual BOOL remove(ComponentInstance* insert, BOOL bAutoDestroy = TRUE);

	void setPreviewWindowHandler(HWND hwnd);
	int getPageCount();

	Instance * clone();

	ComponentInstance * getActiveInstance();

	BOOL actviePage(ComponentInstance * inst);

	BOOL reAddPage(WindowInstance* page);
protected:

	int activePageIndexId;

	BOOL addPage(WindowInstance* page);

	int setActivePage(int idx);

	HWND getActivePage();

	int getInstancePageIdx(WindowInstance* page);
};

#endif /* PAGEWINDOWINSTANCE_H_ */
