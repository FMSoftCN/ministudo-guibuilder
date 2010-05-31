/*
 * resdescpanel.h
	*
 * Created on: 2009-4-7
 **/

#ifndef RESDESCPANEL_H_
#define RESDESCPANEL_H_


class ResDescPanel: public Panel {
	MGStatic desc;
public:
	ResDescPanel(PanelEventHandler* handler);
	virtual ~ResDescPanel();
	HWND createPanel(HWND hParent);
	virtual HWND getHandler()
	{
		return desc.GetHandle();
	}
	void setText(const char *text);
};

#endif /* RESDESCPANEL_H_ */
