/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
