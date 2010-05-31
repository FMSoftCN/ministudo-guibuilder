/*
 * page-window-instance.h
 *
 *  Created on: 2009-4-8
 *      Author: dongjunjie
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
