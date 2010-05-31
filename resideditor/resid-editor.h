/*
 * ResIdEditor.h
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#ifndef RESIDEDITOR_H_
#define RESIDEDITOR_H_

#include "resid-list-panel.h"

class ResIdEditor: public ResEditor
{
	ResIdListPanel* res_id_panel;
public:
	ResIdEditor();
	virtual ~ResIdEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );
	Panel* createPanel(const char* name, const char* caption, const mapex<string,string>*params);
	void executeCommand(int cmd_id, int status, DWORD param);

//	virtual BOOL open(const char* xmlFile);
//	virtual string save();

	void updateRes();
	void active(bool bactive, int reason);
};

#endif /* RESIDEDITOR_H_ */
