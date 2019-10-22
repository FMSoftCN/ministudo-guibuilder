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
#ifndef PANEL_H
#define PANEL_H

class Panel;

class PanelEventHandler
{
public:
	virtual ~PanelEventHandler(){}
	virtual DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 ) = 0;
};

class Panel
{
protected:
	PanelEventHandler * handler;
	DWORD sendEvent(int event_id, DWORD param1=0, DWORD param2=0) {
		if(handler)
			return handler->processEvent(this, event_id, param1, param2);

        return 0;
	}

public:
	Panel():handler(NULL){}
	Panel(PanelEventHandler* handler):handler(handler){}
	virtual ~Panel(){}

	void setEventHandler(PanelEventHandler* handler){
		this->handler = handler;
	}

	virtual HWND createPanel(HWND hPrent) = 0;
	virtual HWND getHandler() = 0;
};

#endif

