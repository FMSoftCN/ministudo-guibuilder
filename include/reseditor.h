/*
 * reseditor.h
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#ifndef RESEDITOR_H_
#define RESEDITOR_H_

extern ResEditorEnv* g_env;

class ResEditor;

extern BOOL ValidIDName(const char* name);

enum ResEditorNotificationHandlerStatus{
	MSF_INCLUDE = 0,
	MSF_EXCLUDE,
	MSF_REPLACE
};
class ResEditorNotificationHandler
{
public:
	virtual ~ResEditorNotificationHandler(){}
	virtual void onNotification(ResEditor* editor, int nc, DWORD param) = 0;
	virtual void onMenuStatusChanged(ResEditor* editor, int id, DWORD newStatus, int type=MSF_INCLUDE) = 0;
    virtual int getEditorMenuStatus(ResEditor* editor, int id, UINT* status) = 0;

};

class ResEditor : public TMGStaticSubclass<MGStatic>,
				  public PanelEventHandler,
				  public PanelManager,
				  public ResManager
{
protected:
	ResEditorNotificationHandler * notification;

	void notify(int nc, DWORD param = 0){
		if(notification)
			notification->onNotification(this, nc, param);
	}

	void enableMenuItem(int id,BOOL bEnable=TRUE){
		notification->onMenuStatusChanged(this, id, MFS_DISABLED,bEnable?MSF_EXCLUDE:MSF_INCLUDE);
	}
	void checkMenuItem(int id, BOOL bChecked=TRUE){
		notification->onMenuStatusChanged(this, id, bChecked?MFS_CHECKED:MFS_UNCHECKED,MSF_REPLACE);
	}

	PanelLayout layout;

	BOOL WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret);

public:
	struct Resource{
		string name;
		int id;
		int source_id;
		int use_ref;
		Resource(const char* name, int id, const char* source)
		:use_ref(0)
		{
			if(name)
				this->name = name?name:"";
			this->id = id;
			this->source_id = g_env->addString(source);
		}
		~Resource(){
			g_env->removeString(this->source_id);
		}
	};

protected:
	mapex<string, Resource*> namedRes; //res defined by name
	mapex<int, Resource*> reses; //resource define by id

	virtual int newResId(int type, const char* name = NULL);

	virtual IDRangeManager* getIDRangeManager(int type, const char* name = NULL) { return NULL; }

	void unuseId(int id, Resource* resource = NULL)
	{
		IDRangeManager* idrm = getIDRangeManager(ID2TYPE(id), resource?resource->name.c_str():NULL);
		if(idrm)
			idrm->unuseId(id);
	}

	void useId(int id, Resource* resource = NULL)
	{
		IDRangeManager* idrm = getIDRangeManager(ID2TYPE(id), resource?resource->name.c_str():NULL);
		if(idrm)
			idrm->useId(id);
	}

	virtual BOOL loadXMLIDs(const char* xmlFile);
	virtual BOOL loadUserNodes(xmlNodePtr node) {
		return FALSE;
	}
	virtual BOOL saveUserNodes(TextStream *stream) {
		return TRUE;
	}


	virtual BOOL saveXMLIDs(const char* xmlFile);

	virtual Resource * getResourceBySourceId(int source_id){
		for(map<int, Resource*>::iterator it = reses.begin();
			it != reses.end(); ++it)
		{
			Resource* res = it->second;
			if(res && res->source_id == source_id)
				return res;
		}
		return NULL;
	}

	BOOL bIdChanged;

	virtual BOOL initEditor(){ /*Do nothing, drvier do*/ return TRUE; }

	virtual int typeFromId(int id){ return ID2TYPE(id); }

	virtual void onResUseChanged(int id, BOOL bAdd=TRUE){ }

public:

	static ResEditor* createResEditor(HWND hParent, const char* resEditorName, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);

	ResEditor();

	BOOL Create(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);

	BOOL loadTypeInfo(xmlNodePtr typeNode);

	~ResEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 ){
		return 0;
	}

	int getMenuStatus(int id, UINT* status) {
        return notification->getEditorMenuStatus(this, id, status);
    }
	enum commCommand {
        //File: global command < 50
		GBC_SAVE = 1, //(const char* file=param)
		GBC_SAVEAS, //(const char* file=param)
		GBC_EXIT, //(const char* file=param)

        //the following for resource editor.
		GBC_NEW = 50,
		GBC_OPEN, //(const char* file=param)
		GBC_CLOSE,
        //Edit:
		GBC_UNDO = 210,
		GBC_REDO,
		GBC_COPY,
		GBC_CUT,
		GBC_PASTE,
		GBC_DELETE,
        INCORE_RES_MGNCSCFG = 40,
        INCORE_RES_PACKAGE,
		GBC_IDRANGEMANAGER = 47,
		GBC_ABOUT = 48,
		GBC_HELP = 49,
	};

	enum menuCommand {
        //system id: save, exit, etc. 0~49
        MENUCMD_SYSID_BASE = 0,
        MENUCMD_SYSID_END = 49,

        //command id: 50~899
        MENUCMD_COMMID_BASE = MENUCMD_SYSID_END + 1,
        MENUCMD_COMMID_END  = 899,

        //user id: 900~999
        MENUCMD_USERID_BASE = MENUCMD_COMMID_END + 1,
        MENUCMD_USERID_END = MENUCMD_USERID_BASE + 99,

        //editor id
        MENUCMD_SELECTORID_BASE = 1001,
        MENUCMD_SELECTORID_END = MENUCMD_SELECTORID_BASE + 6,

        EDITOR_MENUID_BASE = 50,
        EDITOR_MENUID_MAX  = MENUCMD_USERID_END,


        //Pop Menu Header Id 800~900
		UI_PROP_POPMENU = 800,
		UI_TAB_POPMENU  = 850,
		UI_SET_POPMENU  = 880,
		RDR_POPMENU     = 800,

		IMG_POPMENU     = 700,
		TXT_POPMENU     = 600,

        //rdr: 300~500
		RDR_MENUCMD_NEWRDRSET = 301,
		RDR_MENUCMD_DELRDRSET = 302,
		RDR_MENUCMD_NEWRDR  = 401,
		RDR_MENUCMD_DELRDR  = 402,
		RDR_MENUCMD_ADDRDR  = 403,

        //Text: 600~700
		TXT_MENUCMD_NEW = 601,
		TXT_MENUCMD_DEL,
		TXT_MENUCMD_TRANS,
		TXT_MENUCMD_TRANSALL,
		TXT_MENUCMD_PROFILE,

        //Image: 700~800
		IMG_MENUCMD_SORT    = 701,
		IMG_MENUCMD_CLEAN   = 702,
		IMG_MENUCMD_REMOVE  = 703,
		IMG_MENUCMD_IMPORT  = 704,
		IMG_MENUCMD_IMPORTALL = 705,

        //ui:500~600
		UI_MENUCMD_ADDPAGE = 541,
		UI_MENUCMD_DELPAGE = 542,

		UI_MENUCMD_REMOVE_CUR = GBC_CLOSE + 1,
		UI_MEMUCMD_EXPORT_TEMPLATE,
		UI_MENUCMD_LEFT = 505,
		UI_MENUCMD_RIGHT,
		UI_MENUCMD_CENTER,
		UI_MENUCMD_TOP,
		UI_MENUCMD_BOTTOM,
		UI_MENUCMD_MIDDLE,
		UI_MENUCMD_HSPREED,
		UI_MENUCMD_VSPREED,
		UI_MENUCMD_SAMEWIDTH,
		UI_MENUCMD_SAMEHEIGHT,
        //ui menu item
        UI_MENUCMD_RULER,
        UI_MENUCMD_SETSTARTWIN,
        UI_MENUCMD_PREVIEW,
        UI_MENUCMD_SAVETMPLSOURCE,
        UI_MENUCMD_SETDEFRDR,
        UI_MENUCMD_SETSCREENSIZE,
        UI_MENUCMD_CONNECTEVENTS,
        UI_MENUCMD_FONTMANAGE,
        UI_MENUCMD_SNAPGRID,

        //ui tab menu command id
		UI_MENUCMD_MOVEUP = 551,
		UI_MENUCMD_MOVEDOWN,

        //ui set default value
        UI_MENUCMD_SETDEFVALUE = 581,
        UI_MENUCMD_SETDEFALL   = 582,
	};
	virtual void executeCommand(int cmd_id, int status, DWORD param){
	}

	virtual bool queryCommand(int cmd_id){
		return false;
	}

	enum GBActiveReason {
		USER_ACTIVE = 0,//active by mouse click
		REF_ACTIVE, //active by other editor
		SYS_ACTIVE, //active by system
	};
	virtual void active(bool bactive, int reason){
		if(bactive && reason == USER_ACTIVE){
			g_env->setMainCaption(getTypeName(getTypeMask()));
		}
	}

	enum GBMRefCommand {
		REF_OK = 1,
		REF_CANCEL,
		REF_USER = 1024
	};
	//when send by another editor
	virtual void executeRefCommand(int id, DWORD param){
	}

	const char* getTypeName(int type_mask){
		return NULL;
	}

	int getTypeMask(){
		return 0;
	}

	virtual int createRes(int type, const char* name, int id, const char* source, DWORD init_res);

	virtual DWORD getRes(int id){
		Resource* res = reses.at(id);
		return res?res->source_id:0;
	}

	virtual BOOL setRes(int id, DWORD dwres){
		Resource* res = reses.at(id);
		if(res == NULL)
			return FALSE;
		res->source_id = dwres;
		return TRUE;
	}

public:
	int setResName(int id, const char* name, DWORD res=0){
		if(name == NULL || strcmp(name, "") == 0)
			return -1;

		Resource *resource = reses.at(id);
		if(resource && strcmp(resource->name.c_str(), name)!=0
            && isValidName(name)){
			//remove name
			namedRes.erase(resource->name);
			resource->name = string(name);
			namedRes[resource->name] = resource;
			bIdChanged = TRUE;
			return id;
		}
		return -1;
	}

	BOOL setResId(int oldId, int newId, DWORD res = 0)
	{
		if(oldId == newId)
			return FALSE;

		if(ID2TYPE(oldId) != ID2TYPE(newId))
			return FALSE;

		if (reses.find(newId) != reses.end())
			return FALSE;

		useId(newId);
		unuseId(oldId);
		Resource *resource = reses.at(oldId);
		if(resource)
		{
			resource->id = newId;
			reses.erase(oldId);
			reses[newId] = resource;
			bIdChanged = TRUE;
			return TRUE;
		}
		return FALSE;
	}

	virtual int use(int id, BOOL auto_ref = FALSE)
	{
		Resource* resource = reses.at(id);
		if(resource)
		{
			int use_ref = resource->use_ref ++;
			if(use_ref == 0 && resource->use_ref>0)
			{
				bIdChanged = TRUE;
				onResUseChanged(id, TRUE);
			}
			return resource->use_ref;
		}
		return -1;
	}

	virtual int unuse(int id, BOOL bauto_ref=FALSE){
		Resource* resource = reses.at(id);
		if(resource && resource->use_ref > 0){
			if(--resource->use_ref <= 0)
			{
				bIdChanged = TRUE;
				onResUseChanged(id, FALSE);
			}
			return resource->use_ref;
		}
		return -1;
	}

	//remove a res
	virtual BOOL removeRes(int id, DWORD res = 0){
		Resource* resource = reses.at(id);
		if(resource == NULL || (resource->use_ref) > 0)
			return FALSE;
		reses.erase(id);
		if(resource->name.length() > 0)
			namedRes.erase(resource->name);
		unuseId(id, resource);
		delete resource;
		bIdChanged = TRUE;
        g_env->deleteRefRes(id);
		return TRUE;
	}

	virtual const char* idToName(int id){
		Resource * resource = reses.at(id);
		if(resource)
			return resource->name.c_str();
		return NULL;
	}
	virtual int nameToId(const char* name){
		if(name == NULL)
			return -1;
		Resource * resource = namedRes.at(name);
		if(resource)
			return resource->id;
		return -1;
	}
	//whether the name can be used as new resource name.
	virtual BOOL isValidName(const char* name)
	{
		if(!name)
			return FALSE;
		if (namedRes.find(name) == namedRes.end()){
			return ValidIDName(name);
		}
		return FALSE;
	}
	virtual void onResNameChanged(int id, const char* newName) { }
	virtual void onResIdChanged(int oldId, int newId){ }
	virtual void onResValueChanged(int id) { }
	virtual void onAllResUpdated(int type){ }
	virtual void onRefResDeleted(int id){ }

	void enumRes(ResEnumHandler *handler);

	virtual int getSelectedResID(){
		return -1;
	}

	virtual void updateRes(){ }
	virtual BOOL open(const char* xmlFile){ return FALSE; }
	virtual string save(BinStream* bin){ return ""; }

	virtual BOOL transIDs(TextStream *stream);
	virtual BOOL queryIDChanged(){ return  bIdChanged; }

	virtual BOOL loadConfig(xmlNodePtr root_node){
		return TRUE;
	}
	virtual BOOL saveConfig(TextStream* stream){
		return TRUE;
	}

	BOOL callSpecial(const char* strName, ...)
	{
		return FALSE;
	}

public:
	int getAllIDRangeManagers(vector<IDRangeManager*> & mngrlist){
		return 0;
	}

	BOOL updateIDRangeManager(IDRangeManager* idrm)
	{
		int res_type;
		if(!idrm)
			return FALSE;
		res_type = idrm->getType();
		IDRangeManager::IDNewRangeIterator idnri = idrm->getNewIDRanges();
		while(!idnri.isEnd())
		{
			IDRange& idrange = *idnri;

			int count = 0;
			for(map<int, Resource*>::iterator it = reses.lower_bound((idrange.min+1)|(res_type<<16));
					it != reses.upper_bound(idrange.max|(res_type<<16)); ++it)
				count ++;
		
			idrange.setUsed(count);	

			++idnri;
		}
		return TRUE;
	}

public:
    virtual void setChanged(BOOL bChanged=TRUE) { }

};


template<class TResEditor>
ResEditor* CreateTResEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification)
{
	ResEditor * resEditor = new TResEditor;
	if(resEditor)
	{
		if(!resEditor->Create(hParent, resEditorPanelLayout, notification))
		{
			delete resEditor;
			return NULL;
		}
	}
	return resEditor;
}

#define DECLARE_RESEDITOR(TResEditor) \
ResEditor* create##TResEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification) \
{ \
		return CreateTResEditor<TResEditor>(hParent, resEditorPanelLayout, notification); \
}

#endif /* RESEDITOR_H_ */
