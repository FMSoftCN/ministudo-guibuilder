/*
 * textlistpanel.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef TEXTLISTPANEL_H_
#define TEXTLISTPANEL_H_

#define MAX_ID_LEN 20
#define EVENT_UPDATE_VALUE 110
#define EVENT_SEL_CHANGED  111
#define DEFINABLE_ID_BASE  100
#define MAX_STRING_LEN 1024

class TextListPanel: public Panel,
							public EditableListView, EditableListViewHandler, ValueUpdator
{
	TBaseStringValueType<MutliStringEditor, VT_STRING> defaultValueType;
	TBaseStringValueType<MutliStringEditor, VT_STRING> curValueType;
	IntValueType * pIdValueType;
	StringValueType *pNameValueType;

	map<int, string> *pdefText;
	map<int, string> *pcurText;
	map<int, ResEditor::Resource*> *preses;

public:
	TextListPanel(PanelEventHandler* handler);
	virtual ~TextListPanel();

	HWND createPanel(HWND hPrent);
	virtual HWND getHandler(){ return GetHandle();}

	void setContents(map<int, ResEditor::Resource*> & reses, list<TeNode*> &configList);

	void onNewTextInserted(int id);
	void onTextUpdated(int id);
	void setSubText(TeNode *st_TeNode);
	void appendText(ResEditor::Resource* res);
	void deleteText();
	void updateList(list<TeNode*> &configList, map<int, ResEditor::Resource*> & reses);
	void updateHead(TeNode *def, TeNode *cur);
	int getSelection(void);
	void deleteItemById(int id);

	void updateId(int oldid, int newid);
	void updateIdName(int id);

	void updateEditors(){
		EditableListView::updateEditors();
		sendEvent(EVENT_SEL_CHANGED,hOldSelItem?1:0);
	}

private:
	//implement EditableListViewHandler
	HWND getEditor(HWND hParent, DWORD add_data, int sub_index);

	//implement ValueUpdator
	Value updateValue(Value value, ValueType *vtype, DWORD mask);
	BOOL updatingValue(Value old_value, Value new_value, ValueType *vtype, DWORD mask);

	void insertText(int item,ResEditor::Resource* res);
	void updateText(HLVITEM hlvItem, int item,ResEditor::Resource* res);

	BOOL WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret);
	void onRButtonUp(int x, int y, DWORD key_flag);
	void onPopupMenuCmd(int id);
};

#endif /* TEXTLISTPANEL_H_ */
