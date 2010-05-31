/*
 * connect-events-wnd.h
 *
 *  Created on: 2009-7-1
 *      Author: dongjunjie
 */

#ifndef CONNECTEVENTSWND_H_
#define CONNECTEVENTSWND_H_

class ConnectEventsWnd: public MGMainWnd{
public:
	ConnectEventsWnd(HWND hParent, ComponentInstance *inst, int default_id=-1);
	virtual ~ConnectEventsWnd();
    BOOL toSource() { return bSkipCode; }
    string getEventName() {return eventName; }

protected:
	DECLARE_MSG_MAP;
	ComponentInstance * inst;

	BOOL onInitDialog(WPARAM wParam,LPARAM lParam);
	void onAddEvent();
	void onDeleteEvent();
	void onModify();
	void onSkip();
	void onOK();
	void onCancel();
	void onListenerSelChanged(HWND hCtrl);

	ComponentInstance * getListener();

	ListenEntry* getCurListen();

	void addListenEntry(HWND hlv, ResManager *resMgr,ListenEntry* le, ComponentInstance *listener, int idx=-1);

	BOOL bChanged;
	BOOL bSkipCode;
	string eventName;
};

#endif /* CONNECTEVENTSWND_H_ */
