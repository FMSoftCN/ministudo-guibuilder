/**
 * $Id$
 */

#ifndef RDRTREEPANEL_H_
#define RDRTREEPANEL_H_

class RendererTreePanel: public Panel, public MGCtrlNotification, public TMGStaticSubclass<MGTreeView> {

protected:
	DECLARE_MSG_MAP;

	void onRButtonUp(int x, int y, DWORD key_flag);
	void onPopupMenuCmd(int id);
	BOOL getIDName (int id, char* idName);

public:
	RendererTreePanel(PanelEventHandler* handler);
	virtual ~RendererTreePanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return GetHandle();
	}

	BOOL insertItem(int id, GHANDLE parent);
//	BOOL removeItem(int id, GHANDLE parent);
	BOOL removeItem(int id, GHANDLE parent, BOOL delAllRef=FALSE);
	BOOL selectItem(int id, GHANDLE parent);
	//find first occur item
	GHANDLE findItem(int id, GHANDLE parent);

	GHANDLE getSelItem();
	DWORD getSelItemAddData();
	DWORD getItemAddData(GHANDLE hitem);
	BOOL updateIDName (int id, const char* name);
	BOOL updateIDValue (int oldId, int newId);

	void updateInstance();

	void OnCtrlNotified(MGWnd* ctrl, int id, int code, DWORD add_data);
	BOOL isValidIdHandle(GHANDLE handle);
	GHANDLE getParentHandle(GHANDLE handle);
	GHANDLE setSelItem(GHANDLE selHandle);
};

#endif /* RDRTREEPANEL_H_ */
