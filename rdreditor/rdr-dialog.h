/**
 * $Id$
 */

#ifndef _RDR_NEWDIALOG_H_
#define _RDR_NEWDIALOG_H_

#define MAX_NAME_LEN 128

class NewRdrSetDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onKeyDown(int scancode, DWORD key_status);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

    string rdrName;
    string idName;

	DECLARE_MSG_MAP;

public:
	NewRdrSetDialog(HWND hParent);
	~NewRdrSetDialog(){}

	const char* getRdrName() {return rdrName.c_str();}
	const char* getIdName() {return idName.c_str();}
};

class NewRdrDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onKeyDown(int scancode, DWORD key_status);
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
    void onRdrSelChange (int code, HWND hWnd);

    string rdrName;
    string clsName;
    string idName;
    BOOL visibleCls;

    BOOL isRdrSetDlg;
    BOOL isDisabledCls;

	DECLARE_MSG_MAP;

public:
	NewRdrDialog(HWND hParent, const char* rdrname, 
            const char* clsname, BOOL visibleCls);

	~NewRdrDialog();

	const char* getRdrName() {return rdrName.c_str();}
	const char* getClsName() {return clsName.c_str();}
	const char* getIdName() {return idName.c_str();}
};


class AddRdrDialog: public MGDialog {
protected:
	void onOK();
	void onCancel();
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);
	BOOL onKeyDown(int scancode, DWORD key_status);

    string rdrName;
	set<int> idList;
	DWORD addData;
	DECLARE_MSG_MAP;

public:
	//addData is the id of RendererSet. Not used, please use 0.
	AddRdrDialog(HWND hParent, const char* rdrName, DWORD addData);
	~AddRdrDialog();

	set<int> getIDList() {return idList; }
};
#endif /*_RDR_NEWDIALOG_H_*/
