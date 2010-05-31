/*
 * fieldpanel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
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

	BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret);

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
