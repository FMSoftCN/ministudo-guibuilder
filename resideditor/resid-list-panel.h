/*
 * ResIDListPanel.h
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#ifndef RESIDLISTPANEL_H_
#define RESIDLISTPANEL_H_

#include "valuetype.h"
#include "editable-listview.h"

#define RESIDLISTPANEL_CHANGED  1

class ResIdListPanel:public Panel,ResEnumHandler,EditableListViewHandler, ValueUpdator
{
	EditableListView listview;

public:
	ResIdListPanel(PanelEventHandler* handler);
	virtual ~ResIdListPanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return listview.GetHandle();
	}

	void setContents();

	void setRes(ResManager* resMgr, int type, int id, const char* name, DWORD res);

protected:

	IntValueType *pIntValueType;

	//implement EditableListViewHandler
	HWND getEditor(HWND hParent, DWORD add_data, int sub_index);

	//implement ValueUpdator
	Value updateValue(Value value, ValueType *vtype, DWORD mask);
	BOOL updatingValue(Value old_value, Value new_value, ValueType *vtype, DWORD mask);
};

#endif /* RESIDLISTPANEL_H_ */
