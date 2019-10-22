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

#ifndef IDRANGE_EIDITOR_H
#define IDRANGE_EIDITOR_H

class IDRangeEditor : public MGMainWnd {
public:
	IDRangeEditor(HWND hParent);

	DECLARE_MSG_MAP;

protected:
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	void onNewRange();
	void onExtendRange();
	void onExit();

	void onResTypeChanged(HWND hCBResType);

	int bChanged;
};


#endif


