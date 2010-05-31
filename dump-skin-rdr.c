/*
 * dump-skin-rdr.c
 *
 *  Created on: 2009-7-27
 *      Author: dongjunjie
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

