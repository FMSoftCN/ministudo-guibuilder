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

