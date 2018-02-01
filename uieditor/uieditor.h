/*
 * uieditor.h
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#ifndef UIEDITOR_H_
#define UIEDITOR_H_

class UIEditor: public ResEditor {

protected:

	IDRangeManager  idrmUI;
	IDRangeManager  idrmControl;

	EditUIPanel * getEditUIByInstance(ComponentInstance * instance);

	IDRangeManager* getIDRangeManager(int type, const char* name){
		if(type == NCSRT_UI)
			return &idrmUI;
		else if(type == NCSRT_CONTRL)
			return &idrmControl;
		return NULL;
	}

	int getAllIDRangeManagers(vector<IDRangeManager*> & mngrlist){
		mngrlist.push_back(&idrmUI);
		mngrlist.push_back(&idrmControl);
		return 2;
	}

	void closeAll();

	void close(BOOL bremove_from_dist = FALSE);

	string def_cap_font;
	string def_clt_font;

	list<ExFont*> exFontList;

	mapex<string, EditUIPanel*> editors;
	EditUIPanel *curEdit;
	//EditUIPanel *startWnd;
	int startWndId;

	ToolboxPanel *toolboxPanel;
	EventPanel * eventPanel;
	RendererPanel* rdrPanel;
	PropertyPanel* propPanel;
	NavigatorPanel *navigatorPanel;
	StructPanel *structPanel;

	static UIEditor * _instance;

	int typeFromId(int id){
		if(id >= -1 && id <= 7) //IDC_STATIC
			return NCSRT_CONTRL;

		return ID2TYPE(id);
	}

	struct UIResource : public ResEditor::Resource
	{
		set<Instance*> insts;
		UIResource(const char* name, int id, const char* source)
		:ResEditor::Resource(name, id, source)
		{
			use_ref = 1;
		}

        ~UIResource(){
            insts.clear();
        }

		void insert(Instance* inst){
			if(inst == NULL)
				return;
			insts.insert(inst);
		}

		BOOL isset(Instance* inst){
			if(inst == NULL)
				return FALSE;
			return insts.count(inst)>0;
		}

		Instance * remove(Instance *inst){
			insts.erase(inst);
			return inst;
		}

		int instsCount(){ return insts.size(); }

		/*int removeOtherIds(set<Instance*> removedInsts){
			int i=0;
			set<Instance*>::iterator it = insts.begin();
			while(it!=insts.end())
			{
				Instance* inst = *it;
				if(inst->getID() != id)
				{
					set<Instance*>::iterator itt = it;
					++it;
					insts.erase(itt);
					continue;
				}
				i ++;
				++it;
			}
			return i;
		}*/
	};

public:
	UIEditor();
	virtual ~UIEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );
	Panel* createPanel(const char* name, const char* caption, const mapex<string,string>* params);

	virtual void updateRes();
	virtual BOOL open(const char* xmlFile, BOOL bHide = FALSE);
	virtual string save(BinStream* bin);
	BOOL transIDs(TextStream* stream);

	BOOL isPredefinedID(int id);
	const char* getPredefineName(int id);
	int getPredefinedId(const char* name);

    int editorsCount(){ return editors.size(); }

	const char *getDefaultClientFont(){
		return def_clt_font.c_str();
	}

	void executeCommand(int cmd_id, int status, DWORD param);

	int createRes(int type, const char* name, int id,const char*source,DWORD init_res);

	DWORD getRes(int id){
		UIResource* res = (UIResource*)reses.at(id);
		return (DWORD)(res?*(res->insts.begin()):NULL);
	}

	BOOL setRes(int id, DWORD dwres){
		if(dwres == 0)
			return FALSE;

		UIResource* res = (UIResource*)reses.at(id);
		if(res)
		{
			res->insert((Instance*)dwres);
			return TRUE;
		}
		return FALSE;
	}

	BOOL removeRes(ComponentInstance *cinst);
	DWORD getConfig(){ return configFlags; }

	static UIEditor* getInstance(){ return _instance; }

	int getTypeMask(){ return NCSRT_UI|NCSRT_CONTRL; }

	const char* getTypeName(int type_mask)
	{
		if(type_mask == NCSRT_UI)
			return "UI";
		else if (type_mask == NCSRT_CONTRL)
			return "Control";
		return "";
	}

	const char* idToName(int id);
	int nameToId(const char* name);

	BOOL loadConfig(xmlNodePtr root_node);
	BOOL saveConfig(TextStream* stream);

	void updateAllSpecialFields(int field_id, DWORD param);

	virtual void onResNameChanged(int id, const char* newName);
	virtual void onResIdChanged(int oldId, int newId);
	virtual void onResValueChanged(int id);
	virtual void onAllResUpdated(int type);
	virtual void onRefResDeleted(int id);

	int setResName(int id, const char* name, DWORD res=0);

	BOOL setResId(int old_id, int new_id, DWORD res=0);

	BOOL removeRes(int id, DWORD res=0);

	void active(bool bactive, int reason)
	{
		if(bactive && reason == USER_ACTIVE)
			updateMainWindowCaption();
	}

	BOOL callSpecial(const char* strName, ...);

    void setChanged(BOOL bChanged = TRUE);

private:
	static void _propsheet_notifi(HWND hwnd, LINT id, int nc, DWORD add_data);

	void changeCurEditor(BOOL bSendEvent = FALSE);

	void switchCurEditor(EditUIPanel *eui);

	BOOL initEditor();

	HWND hPropSheet;

	/*void onStartWndChanged()
	{
		EditUIPanel * startWnd = getStartWnd();
		if(structPanel && startWnd){
			startWnd->setStartWnd(TRUE);
			structPanel->setStartWnd(startWnd->getBaseInstance());
		}
	}*/

	int selectStartWnd();

	EditUIPanel * getEditUIPanel(int id){
		if(id == -1)
			return NULL;
		for(mapex<string,EditUIPanel*>::iterator it = editors.begin();
			it != editors.end(); ++it)
		{
			EditUIPanel *eui = it->second;
			if(eui && eui->getBaseInstance() && eui->getBaseInstance()->getID() == id)
				return eui;
		}
		return NULL;
	}

	void preview();

	void updateInstanceRes(ComponentInstance* cinst);

  void getButtonGroupList(map<int, string> *pidList);

	void showPanel(EditUIPanel* panel, BOOL bshow = TRUE);

	void removeInstanceRes(ComponentInstance *cinst);

	void setStartWindow(int start_wnd_id, BOOL avoid_source_changed = FALSE);
	BOOL selectDefRdrWnd(string &strName);

	int screen_width;
	int screen_height;
	int screen_depth;

	void showScreenSetting();
public:
	SIZE getScreenSize(){
		SIZE sz={screen_width, screen_height};
		return sz;
	}

private:
	void saveWindowHeaders();

	void saveMgConfig(void);

	void updateMainWindowCaption();

	void updateRdrElements(Instance *inst, int* ele_ids);

	void onEditUIModified(EditUIPanel *euip, BOOL bModified);
	void setTabPageTitle(EditUIPanel *euip);

	char * getEditUITitle(EditUIPanel *euip, char *szCaption, const char* fileName=NULL)
	{
		szCaption[0] = '\0';
		if(euip->isSourceChanged() || euip->isPropertyChanged())
			strcat(szCaption,"*");
		if(euip->isStartWnd())
			strcat(szCaption, "[StartWnd]");
		strcat(szCaption,fileName?fileName:euip->getFileName());
		return szCaption;
	}

	const char* parseFileName(const char* file)
	{
		const char* fileName = strrchr(file, '/');
		if(fileName == NULL)
			fileName = strrchr(file,'\\');
		if(fileName == NULL)
			fileName = file;
		else
			fileName ++;
		return fileName;
	}

protected:
	BOOL isFileOpend(const char* file){
		if(!file)
			return FALSE;
		return editors.at(file)!=NULL;
	}

	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);

/*******************/
//config
protected:
	DWORD configFlags;
	enum UIConfigFlags{
		UICF_GRID = 0x1
	};

	void loadExtends(xmlNodePtr node);
	void saveExtends(TextStream *stream);

	string dir_extend_templates;
	vector<string> extend_mainwnds;
};

#endif /* UIEDITOR_H_ */
