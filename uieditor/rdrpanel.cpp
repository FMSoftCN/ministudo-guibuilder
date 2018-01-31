/*
 * rdrpanel.cpp
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"
#include "window-instance.h"
#include "../rdreditor/rdr-instance.h"

#include "panel.h"
#include "editable-listview.h"
#include "fieldpanel.h"

#include "rdrpanel.h"
#include "../rdreditor/rdr-event-id.h"

RendererPanel::RendererPanel(PanelEventHandler* handler)
:FieldPanel(handler)
{

}

RendererPanel::~RendererPanel() {
	// TODO Auto-generated destructor stub
}

int RendererPanel::_rdr_cmp(HLVITEM nItem1,
		HLVITEM nItem2, PLVSORTDATA sortData)
{
	/*FieldPanel::InstanceEditor * pinst_editor = (FieldPanel::InstanceEditor *) GetWindowAdditionalData(sortData->hLV);*/
	RendererPanel* rdrPanel =  (RendererPanel*)fromHandle(sortData->hLV);
	Instance * inst = /*pinst_editor->fieldPanel->*/rdrPanel->getInstance();

	int id1 = (int)::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem1);
	int id2 = (int)::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem2);

	int id1type = id1 ? (inst->getIDValueType(id1)) : 0;
	int id2type = id2 ? (inst->getIDValueType(id2)) : 0;

	if(id1type == id2type)
		return id1 - id2;
	else if(id1type < id2type)
		return -1;
	else
		return 1;
}
IDValueType RendererPanel::_idValueType(NCSRT_RDR |NCSRT_RDRSET);

