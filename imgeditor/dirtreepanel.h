/*
 * dirtreepanel.h
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#ifndef DIRTREEPANEL_H_
#define DIRTREEPANEL_H_


class DirTreePanel: public Panel, MGCtrlNotification, TMGStaticSubclass<MGTreeView> {

	//MGTreeView tree;

public:
	DirTreePanel(PanelEventHandler* handler);
	virtual ~DirTreePanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return /*tree.*/GetHandle();
	}

	void setPath(const char* path);

	int getCurPath(char* path, int nMax);

	void OnCtrlNotified(MGWnd* wnd, int id, int code, DWORD add_data);

	DECLARE_MSG_MAP;
private:
	void loadSubDirs(GHANDLE hitem);
};

#endif /* DIRTREEPANEL_H_ */
