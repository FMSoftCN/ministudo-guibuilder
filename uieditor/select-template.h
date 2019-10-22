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
