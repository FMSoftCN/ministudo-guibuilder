/*
 * page-window-instance.cpp
 *
 *  Created on: 2009-4-8
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include <mgncs/mgncs.h>

using namespace std;

#include "log.h"

#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"

#include "uiconfig.h"
#include "component-instance.h"
#include "window-instance.h"

#include "page-window-instance.h"

#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

PageWindowInstance::PageWindowInstance(Class* cls)
:WindowInstance(cls)
{
	// TODO Auto-generated constructor stub

	activePageIndexId = 0;

}

PageWindowInstance::PageWindowInstance(const PageWindowInstance &pwi)
:WindowInstance(pwi)
{

}

Instance * PageWindowInstance::clone()
{
	return (Instance*)(new PageWindowInstance(*this));
}

PageWindowInstance::~PageWindowInstance() {
	// TODO Auto-generated destructor stub
}

void PageWindowInstance::setField(int id, Value value)
{
	if(activePageIndexId == 0 && _class){
		FieldType *ft = _class->getFieldType("ActivePageIndex");
		if(ft){
			activePageIndexId = ft->id;
		}
	}

	if(id == activePageIndexId){
		value = (Value)setActivePage((int)value);
	}

	WindowInstance::setField(id, value);

}
int PageWindowInstance::getPageCount()
{
	if(useNewControlSet()){
		return ncsGetProperty(hwnd, NCSP_PRPSHT_PAGECOUNT);
	}
	else
	{
		return SendMessage(hwnd, PSM_GETPAGECOUNT, 0, 0);
	}
}

BOOL PageWindowInstance::addPage(WindowInstance* page)
{
	DLGTEMPLATE page_templ;

	if(!IsWindow(hwnd))
		return FALSE;

	if(!page->getDlgTemplate(&page_templ))
	{
		return FALSE;
	}
	//add Page
	if(useNewControlSet()){
		mWidget * widget = (mWidget*)ncsObjFromHandle(hwnd);
		if(ncsInstanceOf((mObject*)widget, (mObjectClass*)&g_stmPropSheetCls))
		{
			mPropSheet *prop = (mPropSheet*)widget;
			mPage * mpage = _c(prop)->addPage(prop, &page_templ, NULL);
			if(mpage == NULL){
				return FALSE;
			}
			//printf("page renderer=%p\n",GetWindowInfo(mpage->hwnd)->we_rdr);
			page->setPreviewWindowHandler(mpage->hwnd);
			page->resetNCSProps();
			return TRUE;
		}
		return FALSE;
	}
	//old
	{
		int idx = ::SendMessage(hwnd, PSM_ADDPAGE, (WPARAM)&page_templ, (LPARAM)DefaultControlProc);
		if(idx < 0){
			return FALSE;
		}
		page->setPreviewWindowHandler((HWND)::SendMessage(hwnd, PSM_GETPAGE, idx, 0));
	}
	return TRUE;
}

BOOL PageWindowInstance::reAddPage(WindowInstance* page)
{
	DLGTEMPLATE page_templ;

	if(!IsWindow(getPreviewHandler()) || !IsWindow(page->getPreviewHandler()))
		return FALSE;

	int idx = getInstancePageIdx(page);

	if(idx < 0)
		return FALSE;

	if(!page->getDlgTemplate(&page_templ))
	{
		return FALSE;
	}

	if(useNewControlSet()){
		mPropSheet *propsheet = (mPropSheet*)ncsObjFromHandle(getPreviewHandler());
		_c(propsheet)->removePageByIndex(propsheet, idx);
		mPage * mpage = _c(propsheet)->addPage(propsheet, &page_templ, NULL);
		if(mpage == NULL){
			return FALSE;
		}
		//printf("page renderer=%p\n",GetWindowInfo(mpage->hwnd)->we_rdr);
		page->setPreviewWindowHandler(mpage->hwnd);
		page->resetNCSProps();
	}
	else
	{
		int newidx = ::SendMessage(hwnd, PSM_ADDPAGE, (WPARAM)&page_templ, (LPARAM)DefaultControlProc);
		if(newidx < 0){
			return FALSE;
		}
		page->setPreviewWindowHandler((HWND)::SendMessage(hwnd, PSM_GETPAGE, newidx, 0));
	}

	SetWindowZOrder(page->getPreviewHandler(), idx);
	//setActivePage(idx);
	return TRUE;

}

void PageWindowInstance::setPreviewWindowHandler(HWND hwnd)
{
	if(!::IsWindow(hwnd))
		return;

	this->hwnd = hwnd;

	SetWindowAdditionalData(hwnd, (DWORD)this);

	//create children
	ComponentInstance * instance = children;
	while(instance)
	{
		addPage(dynamic_cast<WindowInstance*>(instance));
		instance = instance->getNext();
	}
}

BOOL PageWindowInstance::insert(ComponentInstance* insert, BOOL bAutoCreate)
{
	WindowInstance* page = dynamic_cast<WindowInstance*>(insert);
	if(page == NULL)
		return FALSE;

	if(!WindowInstance::insert(insert, FALSE))
		return FALSE;

	const char* str = page->getCaption();
	if(str == NULL || strlen(str) == 0)
	{
		char szText[100];
		int count = 0;
		ComponentInstance * inst = getChildren();
		while(inst)
		{
			inst = inst->getNext();
			count ++;
		}
		sprintf(szText,"%s%d", page->getClass()->getClassName(), count);
		page->setCaption(szText);
	}

	//insert
	if(bAutoCreate)
	{
		return addPage(page);
	}

	return TRUE;
}
BOOL PageWindowInstance::remove(ComponentInstance* insert, BOOL bAutoDestroy)
{
		if(!WindowInstance::remove(insert, FALSE))
			return FALSE;

		if(!bAutoDestroy)
			return TRUE;

		//destroy window
		HWND hPage = insert->getPreviewHandler();
		if(!::IsControl(hPage))
			return TRUE;

		//remove from propsheet
		if(useNewControlSet()){
			mWidget * widget = (mWidget*)ncsObjFromHandle(hwnd);

			if(ncsInstanceOf((mObject*)widget, (mObjectClass*)&g_stmPropSheetCls))
			{
				mPropSheet *prop = (mPropSheet*)widget;
				mPage * mpage = (mPage*)ncsObjFromHandle(hPage);
				if(mpage == NULL)
					return TRUE;

				_c(prop)->removePage(prop, mpage);

				insert->resetPreviewHandler();

				return TRUE;
			}
			return FALSE;
		}
		else //old
		{
			int idx = ::SendMessage(hwnd, PSM_GETPAGEINDEX, (WPARAM)hPage, 0);
			if(idx < 0)
				return TRUE;
			::SendMessage(hwnd, PSM_REMOVEPAGE, (WPARAM)idx, 0);
			insert->resetPreviewHandler();
			return TRUE;
		}

	return FALSE;
}

int PageWindowInstance::setActivePage(int idx)
{
	if(useNewControlSet()){
		mWidget * widget = (mWidget*)ncsObjFromHandle(hwnd);
		if(!widget)
			return 0;
		int max_id = (int)_c(widget)->getProperty(widget, NCSP_PRPSHT_PAGECOUNT);
		if(idx < 0)
			idx = 0;
		else if(idx >= max_id)
			idx = max_id - 1;
		_c(widget)->setProperty(widget,NCSP_PRPSHT_ACTIVEPAGEIDX,idx);
	}
	else
	{
		int max_id = ::SendMessage(hwnd, PSM_GETPAGECOUNT, 0, 0);
		if(idx < 0)
			idx = 0;
		else if(idx >= max_id)
			idx = max_id - 1;
		::SendMessage(hwnd,PSM_SETACTIVEINDEX, idx, 0);
	}
	return idx;
}

int PageWindowInstance::hittest(int x, int y)
{
	if(IsWindow(hwnd) && IsWindowVisible(hwnd))
	{
		RECT rt;
		GetWindowRect(hwnd, &rt);
		if(!PtInRect(&rt,x, y))
			return OUT;

		HWND hPage = getActivePage();

		if(!IsWindow(hPage))
			return REQ_MOUSE_AREA;

		x -= rt.left;
		y -= rt.top;
		WindowToClient(hwnd, &x, &y);

		GetWindowRect(hPage, &rt);

		//printf("x=%d,y=%d, rt.left=%d,rt.top=%d,rt.right=%d,rt.bottom=%d\n",
		//		x, y, rt.left, rt.top, rt.right, rt.bottom);
		if(PtInRect(&rt, x, y))
			return CONTAINER;

		return REQ_MOUSE_AREA;

	}
	return OUT;
}

HWND PageWindowInstance::getActivePage()
{
	if(useNewControlSet())
	{
		mWidget * widget = (mWidget*)ncsObjFromHandle(hwnd);
		if(!widget)
			return HWND_INVALID;
		mPage * page = NULL;
		page = (mPage*)_c(widget)->getProperty(widget,NCSP_PRPSHT_ACTIVEPAGE);
		return page? page->hwnd: HWND_INVALID;
	}
	else
	{
		return (HWND)::SendMessage(hwnd,PSM_GETPAGE, ::SendMessage(hwnd,PSM_GETACTIVEINDEX, 0, 0),0);
	}
}

ComponentInstance * PageWindowInstance::getActiveInstance()
{
	HWND hPage = getActivePage();
	if(::IsWindow(hPage))
		return ComponentInstance::FromHandler(hPage);
	return NULL;
}

DWORD PageWindowInstance::processMenuCommand(int cmdid)
{
	switch(cmdid)
	{
	case ResEditor::UI_MENUCMD_ADDPAGE: //Add Page
		//create a new Instance and remove it
		{
			FieldType * ft = _class->getFieldType("Page");
			if(ft == NULL)
				return DO_NOTHING;

			Value val = getField(ft->id);
			ComponentInstance * page = (ComponentInstance*)ComponentInstance::createFromClassName(NULL,(const char*)ft->vtype->toBinary(val));
			if(insert(page)){
				//TODO: undo redo support
				return NEED_UPDATE|INSTANCE_REFRESH;
			}
		}
		break;
	case ResEditor::UI_MENUCMD_DELPAGE: //Delete Page
		{
			HWND hPage = getActivePage();
			ComponentInstance * page = ComponentInstance::FromHandler(hPage);
			if(remove(page)){
				//TODO : undo redo support
				page->release();
				return NEED_UPDATE|INSTANCE_REFRESH;
			}
		}
		break;
	}
	return DO_NOTHING;
}

BOOL PageWindowInstance::actviePage(ComponentInstance * inst)
{
	int idx = 0;
	if(inst == NULL || inst == getActiveInstance())
		return FALSE;

	idx = getInstancePageIdx((WindowInstance*)inst);

	return setActivePage(idx) == idx;
}

int PageWindowInstance::getInstancePageIdx(WindowInstance* page)
{
	HWND hPage = page->getPreviewHandler();
	if(useNewControlSet() && IsWindow(hPage) && IsWindow(getPreviewHandler())){
		mPage *mpage = (mPage*)ncsObjFromHandle(hPage);
		mPropSheet *propsheet = (mPropSheet*)ncsObjFromHandle(getPreviewHandler());
		return _c(propsheet)->getPageIndex(propsheet, mpage);
	}
	else{
		int idx = 0;
		ComponentInstance* cinst = getChildren();
		while(cinst){
			if(cinst == page)
				break;
			cinst = cinst->getNext();
			idx ++;
		}
		if(cinst || cinst != page)
			return -1;
		return idx;
	}
}

