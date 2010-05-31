/*
 * imgres-listpanel.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef IMGRESLISTPANEL_H_
#define IMGRESLISTPANEL_H_

class ImageResListPanel: public Panel, MGCtrlNotification
{
	MGListBox listbox;

public:

	ImageResListPanel(PanelEventHandler* handler);
	virtual ~ImageResListPanel();

	HWND createPanel(HWND hPrent);
	virtual HWND getHandler(){
		return listbox.GetHandle();
	}

	void OnCtrlNotified(MGWnd* ctrl, int id, int code, DWORD add_data);

	BOOL AddRes(const char *idName, int id);
	BOOL RemoveRes(int id);
	BOOL SelectRes(int id);
	int GetSelect();
	int GetIndex(int id);
	int GetCount();

	BOOL updateIDName (int newid, int oldid);
	BOOL updateIDValue (int oldId, int newId);

};

#endif /* IMGRESLISTPANEL_H_ */
