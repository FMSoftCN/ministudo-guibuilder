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

#ifndef QVFB_H
#define QVFB_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <minigui/minigui.h>
MG_EXPORT void GUIAPI VFBSetCaption(const char* caption);
MG_EXPORT void GUIAPI VFBShowWindow(BOOL bshow);
MG_EXPORT void GUIAPI VFBAtExit(void (*callback)(void));

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
