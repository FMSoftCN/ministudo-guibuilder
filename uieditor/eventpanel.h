/*
 * eventpanel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef EVENTPANEL_H_
#define EVENTPANEL_H_


class EventPanel: public FieldPanel {
public:
	EventPanel(PanelEventHandler* handler);
	virtual ~EventPanel();

protected:
	virtual PFNLVCOMPARE getSortFunc(){ return _event_cmp; }
	//implement FieldEnumHandler
	BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user);

	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
	{
		if(iMsg == MSG_LBUTTONDBLCLK){
			DWORD event_id;
			if(getCurData(&event_id))
			{
				//hide value
				for(int i=0; i<count; i++)
				{
					if(editors[i] == (HWND) 0 )
						continue;
					//save value
					if(!::SendMessage(editors[i],VTM_SAVEVALUE, 0, 0))
						return FALSE;
				}
				sendEvent(EVENTPANEL_GOTOCODE, (DWORD)instance,event_id);
				return TRUE;
			}
		}
		return FieldPanel::WndProc(iMsg, wParam, lParam, pret);
	}

private:
	static int _event_cmp(HLVITEM nItem1, HLVITEM nItem2, PLVSORTDATA sortData);
	virtual void cleanAll();
};

#endif /* EVENTPANEL_H_ */
