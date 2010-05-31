/*
 * ResIdEditor.cpp
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#ifndef WIN32
#include <unistd.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include "mapex.h"
#include <vector>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include "msd_intl.h"
using namespace std;

#include "log.h"
#include "stream.h"
#include "resenv.h"

#include "valuetype.h"
#include "editable-listview.h"
#include "panel.h"
#include "panellayout.h"

#include "reseditor.h"
#include "resid-editor.h"
#include "resid-list-panel.h"

ResIdEditor::ResIdEditor() {
}

ResIdEditor::~ResIdEditor() {
    delete res_id_panel;
}

Panel* ResIdEditor::createPanel(const char* name, const char* caption, const mapex<string,string>*params)
{
	if(strcasecmp(name, "ResIdListPanel") == 0)
		return res_id_panel = new ResIdListPanel(this);
	return NULL;
}

DWORD ResIdEditor::processEvent(Panel* sender, int event_id, DWORD param1, DWORD param2)
{
	return 0;
}

void ResIdEditor::executeCommand(int cmd_id, int status, DWORD param)
{
	switch(cmd_id)
	{
		case GBC_OPEN:
		{
			//TODO, maybe open a selection dialog ...
			//res_id_panel->setContents();
			break;
		}
		//TODO , for other command ......
		default:
			break;
	}
}

void ResIdEditor::active(bool bactive, int reason)
{
	if(bactive == true && (reason == 0 || reason == 1)){
		g_env->setMainCaption(_("Resource ID Editor"));
		updateRes();
	}
}

void ResIdEditor::updateRes()
{
	 res_id_panel->setContents();
}

///////////////////////
//Create ResIdEditor
DECLARE_RESEDITOR(ResIdEditor)

