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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#define DUMP_SKIN  "dump-skin"

BOOL RegisterDumpSkinRenderer()
{
	static WINDOW_ELEMENT_RENDERER dump_skin_rdr;
	const WINDOW_ELEMENT_RENDERER * skin_rdr = GetWindowRendererFromName("skin");
	if(skin_rdr == NULL)
		return FALSE;

	memcpy(&dump_skin_rdr, skin_rdr, sizeof(WINDOW_ELEMENT_RENDERER));
	strcpy(dump_skin_rdr.name, DUMP_SKIN);

	if(!AddWindowElementRenderer(DUMP_SKIN, &dump_skin_rdr))
		return FALSE;

	return TRUE;

}

