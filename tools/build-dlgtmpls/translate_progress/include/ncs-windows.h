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

#ifndef NCS_MAINWNDS_H
#define NCS_MAINWNDS_H

#ifdef __cpluspus
extern "C" {
#endif

/**
 * If you want to export these ncs main window function
 * to other libraries, please predefined marco NCS_WND_EXPORT, 
 */
#ifndef NCS_WND_EXPORT
#define NCS_WND_EXPORT
#endif

/**
 * If you want to pass a speical structure pointer by 'user_data'
 * to a main window, please modify the ncs-window-types.h.
 *
 */
#include "ncs-window-types.h"



/** define the function for ID_TRANS_PROGRESS */
extern NCS_WND_EXPORT 
mMainWnd * ntCreateTransProgressEx(HPACKAGE package, HWND hParent, HICON h_icon, HMENU h_menu, DWORD user_dat);
#define ntCreateTransProgress(package, hParent, user_data) \
	ntCreateTransProgressEx(package, hParent, (HICON)0, (HMENU)0, (DWORD)(user_data))


/** define the start window */
#define ntStartWindowEx  ntCreateTransProgressEx
#define ntStartWindow(package, hParent, user_data) \
	ntStartWindowEx(package, hParent, (HICON)0, (HMENU)0, (DWORD)(user_data))


#ifdef __cplusplus
}
#endif

#endif /* end of window list */

