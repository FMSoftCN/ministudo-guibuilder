/*
 * select-template.h
 *
 *  Created on: 2009-4-7
 *      Author: dongjunjie
 */

#ifndef SELECTTEMPLATE_H_
#define SELECTTEMPLATE_H_


extern BOOL copyfile(const char* filesrc, const char* filedst);

#define MSG_SLT_ISOPENED  MSG_USER + 200

class SelectTemplate: public MGMainWnd {
public:
	SelectTemplate(HWND hParent, const char* strTemplPaths);
	virtual ~SelectTemplate();

	const char* getNewFileName(){ return newFile.c_str(); }

	DECLARE_MSG_MAP;

protected:
	void onOK();
	void onCancel();

	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	BOOL updateNewFile();

	//void onDestroy();
	HWND hOwner;

	string newFile;

	void loadTemplFromDir(const char* path);

	HWND insertNewPage(HWND hPropSheet, const char* szPath);

	void insertTemplate(const char* file, HWND hIconView);

};

#endif /* SELECTTEMPLATE_H_ */
