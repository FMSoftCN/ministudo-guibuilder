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
#ifndef DLG_TEMPLATE_H
#define DLG_TEMPLATE_H


#define ID_FONTMANAGER  10000
#define ID_IMPORTIMAGE  10001
#define ID_NEWFILE  10002
#define ID_NEWRENDERER  10003
#define ID_NEWRENDERERSET  10004
#define ID_ADDRENDERER  10005
#define ID_SETDEFAULTRENDERER  10006
#define ID_SETSCREENSIZE  10007
#define ID_ADDLANG  10008
#define ID_TEXTPROFILE  10009
#define ID_CONNECTEVENT  10010
#define ID_SELECTEVENT  10011
#define ID_INPUTEVENTNAME  10012
#define ID_FONTSELECT  10013
#define ID_IDRANGEEDITOR  10014
#define ID_NEWIDRANGE  10015
#define ID_EXTENDIDRANGE  10016
#define ID_ABOUT  10017
#define ID_SETSTARTWINDOW  10018
#define ID_TRANSLATEPROGRESS  10019
#define ID_SELECTPROJECTTYPE  10020

#ifdef __cplusplus
extern "C" {
#endif

extern DLGTEMPLATE * GetDlgTemplate(int id);

extern LRESULT AutoCenterDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

extern PCTRLDATA GetControlData(DLGTEMPLATE *tmpl, int id);

#ifdef __cplusplus
}
#endif

#endif /*DLG_TEMPLATE_H*/

