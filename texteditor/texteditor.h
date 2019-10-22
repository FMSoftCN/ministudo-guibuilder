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

#ifndef TEXTEDITOR_H_
#define TEXTEDITOR_H_

#define TEXT_LEN 20
struct TeNode
{
	enum TeNodeStatus{ DEF_LANG = 1, CUR_LANG = 2};
	int idx;
	int status;
	char langType[TEXT_LEN];
	char country[TEXT_LEN];
	char charSet[TEXT_LEN];
	map<int, string> langMap;
	char *fontName;

	TeNode()
	{
		idx = 0;
		status = 0;
		memset(langType, 0, sizeof(langType));
		memset(country, 0, sizeof(country));
		memset(charSet, 0, sizeof(charSet));
		fontName = NULL;
	}

	const char* getText(int id){
		map<int, string>::iterator it = langMap.find(id);
		if(it != langMap.end())
			return it->second.c_str();
		return NULL;
	}

	BOOL setText(int id, const char* szText)
	{
		if(szText == NULL)
			return FALSE;
		langMap[id] = szText;
		return TRUE;
	}

	BOOL changeId(int oldId, int newId) {
		map<int, string>::iterator it = langMap.find(oldId);
		if(it != langMap.end()) {
			langMap[newId] = it->second;
			langMap.erase(it);
			return TRUE;
		}
		return FALSE;
	}

	BOOL readText(const char* str_file);

	BOOL saveText();

	int saveTextBin(BinStream* bin);
};


#define LOCALE_SAVEFILE "text.res"

class TextListPanel;
class Translater;
class ProgressDialog;

class TextEditor: public ResEditor
{
	HWND hCombobox;
	TextListPanel *listPanel;
	TeNode *pDefTeNode;
	TeNode *pCurTeNode;

	Translater *ts;
	ProgressDialog *progDlg;
	pthread_t pt_trans;

public:
	list<TeNode*> configList;
	TextEditor();
	virtual ~TextEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );
	Panel* createPanel(const char* name, const char* caption, const mapex<string, string>*param);

	string save(BinStream* bin);
	virtual void updateRes();

	void executeCommand(int cmd_id, int status, DWORD param);

	int getTypeMask(){ return NCSRT_TEXT;}
	const char* getTypeName(int type_mask){return "Text";}

	BOOL setResId(int oldId, int newId, DWORD res=0);
	int setResName(int id, const char* name, DWORD res=0);

	DWORD getRes(int id);
	BOOL setRes(int id, DWORD dwres);
	BOOL removeRes(int id, DWORD res=0);
	int createRes(int type, const char* name, int id, const char* source,DWORD init_res);

private:
	BOOL setRes(int id, const char* str, BOOL check_id_exist = TRUE);

	BOOL readXmlConfig(const char *xmlFile);
	void saveXmlConfig(const char *xmlFile);
	void updateLangList(const char *strDir);
	void updateCombobox();
	void createTextFile(const char *szFileName);
	BOOL open(const char* xmlFile);
	void onTextComboboxNotification(HWND hwnd, LINT id, int nc, DWORD add_data);
	static void _textCombobox_notifi(HWND hwnd, LINT id, int nc, DWORD add_data);
	BOOL initEditor();

	void onNewText();
	void onDeleteText();
	void onTrans();
	static void *_trans(void *arg);
	void onTransAll();
	static void *_transAll(void *arg);
	const char* newTextName(char* strTextName);

protected:
	void onResUseChanged(int id, BOOL bAdd=TRUE);

	IDRangeManager idrmUser;
	IDRangeManager idrmAnonymous;

	IDRangeManager* getIDRangeManager(int type, const char* id_name)
	{
		if(type != NCSRT_TEXT)
			return NULL;

		return (id_name && id_name[0]!=0)? &idrmUser: &idrmAnonymous;
	}

public:
	int getAllIDRangeManagers(vector<IDRangeManager*> &mngrlist)
	{
		mngrlist.push_back(&idrmUser);
		mngrlist.push_back(&idrmAnonymous);
		return 2;
	}
};


#endif /* TEXTEDITOR_H_ */
