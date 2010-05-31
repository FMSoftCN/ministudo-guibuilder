/*
 * proppanel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
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
