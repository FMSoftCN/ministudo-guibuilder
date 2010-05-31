
#ifndef NEWIDRANGE_H
#define NEWIDRANGE_H

class NewIDRange : public MGMainWnd {
	IDRangeManager* idrm;

public:
	NewIDRange(HWND hParent, IDRangeManager* idrm = NULL,BOOL bEnableCancel=FALSE);

	DECLARE_MSG_MAP;

protected:
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	void onOK();
	void onCancel();

	void onFreeRangeChanged(HWND hlv);

	void onLearnMore();
};


#endif


