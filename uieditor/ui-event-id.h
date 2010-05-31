/*
 * ui_event_id.h
 *
 *  Created on: 2009-4-1
 *      Author: dongjunjie
 */

#ifndef UI_EVENT_ID_H_
#define UI_EVENT_ID_H_

//define the event ui id
#define COMM_EVENT  0x0000
#define EDITUIPANEL_EVENT 0x10000
#define FIELDPANEL_EVENT  0x20000
#define PROPPANEL_EVENT (0x40000|FIELDPANEL_EVENT)
#define EVENT_EVENT          (0x50000|FIELDPANEL_EVENT)
#define RDR_EVENT              (0x60000|FIELDPANEL_EVENT)
#define STRUCT_EVENT 0x80000
#define NAVIGATOR_EVENT 0x90000
#define TOOLBOX_EVENT 0xA0000

#define UIMENUITEM_ENABLE (COMM_EVENT|1)
//param1 = id
//param2 = TRUE/FALSE
#define UIMENUITEM_CHECK  (COMM_EVENT|2)
//param1 = id
//param2 = TRUE/FALSE

#define EDITUIPANEL_FINISH_CREATE (EDITUIPANEL_EVENT|1)
#define EDITUIPANEL_SELCHANGE     (EDITUIPANEL_EVENT|2)
//Instance* instance = param1
//
#define EDITUIPANEL_BOUND_CHANGE  (EDITUIPANEL_EVENT|3)
//Instance * instance = parma1
//BoundMask boundmask = param2

#define EDITUIPANEL_INSTANCE_ADDED  (EDITUIPANEL_EVENT|4)
//ComponentInstance * instance = param1
//ComponentInstance * parent = param2
#define EDITUIPANEL_INSTANCE_DELETED (EDITUIPANEL_EVENT|5)
//ComponentInstance * instance = param1
#define EDITUIPANEL_CHANGE_PARENT (EDITUIPANEL_EVENT|6)
//vector<HWND> *lists = param1
//ComponentInstance * old_parent = param2
#define EDITUIPANEL_INSTANCE_REFRESH (EDITUIPANEL_EVENT|7)
//ComponentInstance * instance = param1
//BOOL refresh_children = param2
#define EDITUIPANEL_FIELD_UPDATE (EDITUIPANEL_EVENT|8)
//Instance * instance = param1
//int id = param2
#define EDITUIPANEL_MODIFIED  (EDITUIPANEL_EVENT|9)
//EditUIPanel *modified = param1
//BOOL bModified = TRUE|FALSE

#define EDITUIPANEL_OPEN (EDITUIPANEL_EVENT|9)
//EditUIPanel * panel = param1
#define EDITUIPANEL_CLOSE (EDITUIPANEL_EVENT|10)
//EditUIPanel * panel = param1
#define EDITUIPANEL_UPDATE (EDITUIPANEL_EVENT|11)
//EditUIPanel * panel = param1
#define EDITUIPANEL_UPDATE_SPECIAL_FIELD (EDITUIPANEL_EVENT|12)
//int field_id = param1
//DWORD param = param2
#define EDITUIPANEL_INSTANCE_HIDE (EDITUIPANEL_EVENT|13)
//BOOL bhide = param1
//EditUIPanel* panel = param1



#define TOOLBOX_SELCHANGE (TOOLBOX_EVENT|1)
//BOOL bAlways = param1

#define FIELDPANEL_INSTANCE_FIELD_CHANGED  (FIELDPANEL_EVENT|1)
//Instance* instance = param1
//int id = param2
#define FIELDPANEL_INSTANCE_FIELD_RESET (FIELDPANEL_EVENT|2)
//Instance* instance = param1

#define STRUCT_SELCHANGE   (STRUCT_EVENT|1)
//Instance = param1

#define STRUCT_SWITCHINSTANCE (STRUCT_EVENT|2)
//ComponentInstance *inst1 = param1, inst2 = param2

#define STRUCT_INSTANCE_DBLCLKED (STRUCT_EVENT|3)
//ComponentInstance *inst = param1

#define NAVIGATOR_SELCHANGE (NAVIGATOR_EVENT|1)
//EditUIPanel * panel = param1
#define NAVIGATOR_SHOWPANEL (NAVIGATOR_EVENT|2)
//const char* strName = param1

#define EVENTPANEL_GOTOCODE (EVENT_EVENT|1)
//Instance* instance = param1
//int event_id = param2

#endif /* UI_EVENT_ID_H_ */
