/**
 * $Id$
 */

#ifndef RDRPREVIEWPANEL_H_
#define RDRPREVIEWPANEL_H_

class RendererPreviewPanel: public Panel, public TMGStaticSubclass<MGStatic> {

protected:
	DECLARE_MSG_MAP;
	RendererInstance *inst;

	void onPaint(HDC hdc);

public:
	RendererPreviewPanel(PanelEventHandler* handler);
	virtual ~RendererPreviewPanel();

	HWND createPanel(HWND hParent);

	HWND getHandler(){
		return GetHandle();
	}

	HWND createPreviewWindow(RendererInstance *instance);
	void destroyPreviewWindow();
	RendererInstance* getPreviewInstance() {return inst; }
	void updateInstanceField(RendererInstance *instance, int* ids);
};

#endif /* RDRPREVIEWPANEL_H_ */
