/*
 * navigator-panel.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef NAVIGATORPANEL_H_
#define NAVIGATORPANEL_H_

class NavigatorPanel: public Panel {

	HWND hIconView;

	static void _iconview_notifi(HWND hIconView, int id, int nc, DWORD add_data);

public:
	NavigatorPanel(PanelEventHandler* handler);
	virtual ~NavigatorPanel();

	HWND createPanel(HWND hParent);
	virtual HWND getHandler(){
		return hIconView;
	}

	void insertEditUIPanel(const char* strName, EditUIPanel* eui);

	void removeEditUIPanel(EditUIPanel *eui);

	void setCurPanel(EditUIPanel* eui);

	void updateEditUIPanel(EditUIPanel* eui);

	void closePanel(EditUIPanel *eui);

protected:
	const char* parserName(const char* strName, char* szBuff);

	int findByEditUIPanel(EditUIPanel* eui);

	/*struct PanelInfo{
		enum PanelInfoType{
			INFO_NAME = 0,
			INFO_OBJECT
		};
		int type;
		union {
			char * strName;
			EditUIPanel *eui;
		}infos;

		PanelInfo(const char* strName, EditUIPanel* eui)
		{
			if(eui == NULL && strName == NULL)
				throw("error, Navigator don\'t accept NULL name and object");
			if(eui == NULL)
			{
				type = INFO_NAME;
				infos.strName = strdup(strName);
			}
			else
			{
				type= INFO_OBJECT;
				infos.eui = eui;
			}
		}

		~PanelInfo()
		{
			if(type == INFO_NAME && infos.strName)
				free(infos.strName);
		}

		inline BOOL isObject(){
			return type == INFO_OBJECT;
		}

		inline const char* getName(){
			if(type == INFO_NAME)
				return infos.strName;
			return infos.eui->getXMLFile();
		}

		EditUIPanel* closePanel(){
			EditUIPanel * panel = NULL;
			if(type == INFO_OBJECT){
				type = INFO_NAME;
				panel = infos.eui;
				const char* strName = infos.eui->getXMLFile();
				infos.strName = strdup(strName);
			}
			return panel;
		}

		void setPanel(EditUIPanel* eui){
			if(eui == NULL)
				return;
			if(type == INFO_NAME)
				free(infos.strName);
			type = INFO_OBJECT;
			infos.eui = eui;
		}

		inline BOOL equal(EditUIPanel* eui){
			return (type == INFO_OBJECT && infos.eui == eui);
		}

		inline BOOL equal(const char* str){
			return (str && strcmp(str, getName()) == 0);
		}
	};*/

//	PanelInfo* getPanelInfo(const char* strName, int& idx);
	EditUIPanel * getPanelInfo(const char* strName, int &idx);

	//BITMAP bmpClosed;

	DWORD flags;

	void showPanel();

	static int _new_icon_view_proc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam);
	WNDPROC oldIconViewProc;
};

#endif /* NAVIGATORPANEL_H_ */
