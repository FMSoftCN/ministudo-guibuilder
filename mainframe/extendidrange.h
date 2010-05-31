#ifndef EXTEND_IDRANGE_H
#define EXTEND_IDRANGE_H

class ExtendIDRange : public MGMainWnd
{
public:
	ExtendIDRange(HWND hParent, IDRange * idrange);

	DECLARE_MSG_MAP;

protected:
	void onOK();

	void onCancel();

	BOOL onInitDialog(HWND hFcous, LPARAM lParam);

	IDRange*        idrange;
};


#endif


