/*
 * textDlg.h
 *
 *  Created on: 2009-4-29
 *      Author: chp
 */

#ifndef TEXTDLG_H_
#define TEXTDLG_H_

#include "translater.h"

#define MAX_NAME_LEN 128

class NewItemDialog: public MGMainWnd {
private:
	list<TeNode*> *pConfigList;

	void onOK();
	void onCancel();
	void onDestroy();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	DECLARE_MSG_MAP;

public:
	string lang_country;

	NewItemDialog(HWND hParent, list<TeNode*> *pConfigList);
	~NewItemDialog();
};


class AddTextDialog: public MGMainWnd, EditableListViewHandler {
private:
	EditableListView listview;
	list<TeNode*> *pConfigList;

	void onOK();
	void onCancel();
	void onAdd();
	void onDel();
	void onSetDef();
	void onSetCur();
	void appendText(const char* plang_coutry);
	void updateText();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	void InitListView();

	DECLARE_MSG_MAP;

public:
	AddTextDialog(HWND hParent, list<TeNode*> &configList);
	~AddTextDialog();

	HWND getEditor(HWND hParent, DWORD add_data, int sub_index){return 0;}
};


#define IDCPROG 111
#define IDTITLE 112

class ProgressDialog: public MGMainWnd {
protected:
	int total;
	Translater *trans;
	HWND h_prog;
	void onCancel(void);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	DECLARE_MSG_MAP;

public:

	ProgressDialog(HWND hParent, const char* name, Translater *ts);
	~ProgressDialog();

	int setProg(int prog);
	void setTitle(const char *title);
	void setTotal(int to);
};

#endif /* TEXTDLG_H_ */
