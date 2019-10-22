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

#ifndef PROPPANEL_H_
#define PROPPANEL_H_

class PropertyPanel: public FieldPanel {
public:
	PropertyPanel(PanelEventHandler* handler);
	virtual ~PropertyPanel();

	void changeBounds(Instance *instance, DWORD bound);

protected:
	static IDValueType _idValueType;
	ValueType * getIDValueType() {
		return &_idValueType;
	}
	BOOL updatingValue(Value old_value, Value new_value,ValueType *vtype, DWORD mask);
	Value updateValue(Value value, ValueType *vtype,DWORD mask);
	BOOL updateEditorContent(HWND hEditor, ValueType *vtype, Value value, DWORD mask);

	HWND hrdr_editor;

	virtual PFNLVCOMPARE getSortFunc(){ return _prop_cmp; }
	//implement FieldEnumHandler
	BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user);
private:
	static int _prop_cmp(HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortData);
	virtual void cleanAll();
};

#endif /* PROPPANEL_H_ */
