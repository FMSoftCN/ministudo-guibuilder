
#ifndef IDRANGE_EIDITOR_H
#define IDRANGE_EIDITOR_H

class IDRangeEditor : public MGMainWnd {
public:
	IDRangeEditor(HWND hParent);

	DECLARE_MSG_MAP;

protected:
	BOOL onInitDialog(HWND hFocus, LPARAM lParam);

	void onNewRange();
	void onExtendRange();
	void onExit();

	void onResTypeChanged(HWND hCBResType);

	int bChanged;
};


#endif


