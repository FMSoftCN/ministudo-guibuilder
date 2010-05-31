/*
 * toolbox-panel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef TOOLBOXPANEL_H_
#define TOOLBOXPANEL_H_


class ToolboxPanel: public Panel, MGCtrlNotification {

	MGListView listview;

	BOOL loadConfig(const char* strConfig);
	BOOL loadControl(xmlNodePtr node, HLVITEM hlvItem, int nItem);
	BOOL loadControls(xmlNodePtr node, HLVITEM hlvParent, int nItem);

	void OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data);

	DWORD sendBySystem;

	string ctrlistName;
    set<PBITMAP> bmps;
    set<char*> items;

public:
	ToolboxPanel(PanelEventHandler *handler, const mapex<string,string>*params);
	virtual ~ToolboxPanel();

	HWND createPanel(HWND hPrent);
	virtual HWND getHandler(){
		return listview.GetHandle();
	}

	void cancelSelect();

};

#endif /* TOOLBOXPANEL_H_ */
