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

#ifndef FIELDPANEL_H_
#define FIELDPANEL_H_

class FieldPanel: public Panel,
					public EditableListView,
					EditableListViewHandler,
					FieldEnumHandler,
					ValueUpdator,
					protected UndoRedoObject
{
	friend class InstanceEditor;
protected:
/*	class InstanceEditor : public EditableListView
	{
	public:
		InstanceEditor(EditableListViewHandler * handler=NULL)
		:EditableListView(handler){}

		FieldPanel *fieldPanel;

	};

	InstanceEditor listview;*/
public:
	FieldPanel(PanelEventHandler* handler);
	virtual ~FieldPanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return /*listview.*/GetHandle();
	}

	void setInstance(Instance* instance, UndoRedoObserver* observer=NULL);

	Instance *getInstance(){ return instance; }

	void updateEditingField();

	static FieldPanel* fromHandle(HWND hwnd){
		return (FieldPanel*)(EditableListView*)GetWindowAdditionalData(hwnd);
	}

	void refreshFields(Instance *inst, int *ele_ids = NULL);
	void refreshField(Instance* inst, int id);

	void refreshInstanceIdName(int inst_id, const char* name);

protected:
	Instance * instance;
	//implement EditableListViewHandler
	HWND getEditor(HWND hParent, DWORD add_data, int sub_index);
	//implement FieldEnumHandler
	BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user);
	//implement ValueUpdator
	Value updateValue(Value value, ValueType *vtype, DWORD mask);
	BOOL updatingValue(Value old_value, Value new_value,ValueType *vtype, DWORD mask);

	void appendField(int id, const char* name,Value value,ValueType* vtype,int nItem = 0);

	void updateFieldItem(int id, ValueType* vtype = NULL);

	virtual PFNLVCOMPARE getSortFunc(){ return NULL; }

	ValueType *getFieldValueType(int id){
		if(instance == NULL)
			return NULL;
		FieldType * ft = instance->getClass()->getFieldType(id);
		if(ft == NULL)
			return NULL;
		return ft->vtype;
	}

	virtual ValueType * getIDValueType() { return NULL; }

	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);

	void onRButtonUp(int x, int y);

	void setDefValue();
	void setAllDefValue();
	virtual void cleanAll()
	{
		if (instance){
			//undo redo
			InstanceAllDefPropertiesRedoUndoCommand *cmd =
				new InstanceAllDefPropertiesRedoUndoCommand(instance);

			pushUndoRedoCommand(cmd);
			//end undo redo

			instance->cleanAll();
		}
	}
};

#endif /* FIELDPANEL_H_ */
