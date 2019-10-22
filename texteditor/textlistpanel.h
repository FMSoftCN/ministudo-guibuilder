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

	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);
	void onRButtonUp(int x, int y, DWORD key_flag);
	void onPopupMenuCmd(int id);
};

#endif /* TEXTLISTPANEL_H_ */
