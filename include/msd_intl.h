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

#ifndef MSD_INTL_H_
#define MSD_INTL_H_

//#include "gbconfig.h"
#include "guibuilder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSTUDIO_LOCALE

const char* msd_gettext(const char *msgid);

int msd_locale_init(const char*str_cfg);

void free_mo_info();

#define _ msd_gettext

#else

#define _

#endif

#ifdef __cplusplus
}
#endif

#endif /* MSD_INTL_H_ */
