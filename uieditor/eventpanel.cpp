/*
 * eventpanel.cpp
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

#include "ui-event-id.h"

#include "panel.h"
#include "editable-listview.h"
#include "fieldpanel.h"

#include "eventpanel.h"

#include "component-instance.h"

EventPanel::EventPanel(PanelEventHandler* handler)
:FieldPanel(handler)
{
	// TODO Auto-generated constructor stub

}

EventPanel::~EventPanel() {
	// TODO Auto-generated destructor stub
}

BOOL EventPanel::setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user)
{
	if(id >= ComponentInstance::PropEventBegin && id <= ComponentInstance::PropEventEnd)
		return FieldPanel::setField(cls, id, name, vtype, user);
	return TRUE;
}

int EventPanel::_event_cmp(HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortData)
{
	int id1 = ::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem1);
	int id2 = ::SendMessage(sortData->hLV,LVM_GETITEMADDDATA, 0, (LPARAM) nItem2);

	int clsId1 = id1 / 50;
	int clsId2 = id2 / 50;

	//printf("id1=%d, id2=%d\n",id1, id2);

	if(clsId1 == clsId2)
		return id1 - id2;

	return clsId2 - clsId1;
}

void EventPanel::cleanAll()
{
	if(!instance)
		return;

	//undo redo
	InstanceAllDefPropertiesRedoUndoCommand *cmd =
		new InstanceAllDefPropertiesRedoUndoCommand(instance,ComponentInstance::PropEventBegin,
				ComponentInstance::PropEventEnd+1);

	pushUndoRedoCommand(cmd);
	//end undo redo

	instance->cleanAll(ComponentInstance::PropEventBegin,
			ComponentInstance::PropEventEnd);
}
