/*
 * select-project-type-dlg.h
 *
 *  Created on: 2009-4-26
 *      Author: dongjunjie
 */

#ifndef SELECTPROJECTTYPEDLG_H_
#define SELECTPROJECTTYPEDLG_H_

class SelectProjectTypeDlg: public MGMainWnd {
	char* cfgPath;

	BOOL getAndInitGUIBuilderCfg();
	BOOL initProjectInfo(const char* cfgPath);

public:
	SelectProjectTypeDlg(HWND hParent,const char* cfgPath);
	virtual ~SelectProjectTypeDlg();

	DECLARE_MSG_MAP;

protected:
	void onOK();
	void onCancel();
	void onDestroy();
};

#endif /* SELECTPROJECTTYPEDLG_H_ */
