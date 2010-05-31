/*
 * MainFrame.h
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#ifndef MAINFRAME_H_
#define MAINFRAME_H_

struct CharStream
{
	virtual ~CharStream(){}
	inline bool isSpace(char ch){
		return ch==' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\0';
	}
	virtual int getc() = 0;
	virtual int getWord(char* buf, int max) = 0;
	virtual int getPath(char* buf, int max);
};

class MainFrame : public MGMainWnd, ResEditorNotificationHandler, ResEditorEnv
{

protected:

	MenuManager commMenu;

	struct ResEditorInfo {
		ResEditorInfo();
		ResEditorInfo(xmlNodePtr node, MainFrame *mainframe);
		~ResEditorInfo();
		HMENU hMenu;
		HWND  hToolbar;
		string strCaption;
		string editorFileExt;
		HICON  hIcon;
		ResEditor * editor;
		int type;
		int editorId;
		BITMAP ntb_bmp;
		HACCEL hAccel;
		MenuManager menuManager;

		BOOL isSupportFile(const char* ext);
	};

	list<ResEditorInfo*> resEditors;
	list<ResEditorInfo*> stackManager; //stacked the res editor when one editor reference another

	ResEditorInfo* curEditor;
	BITMAP  bmpSelector;
    HWND    hResSelector;
    mapex<string, int>editorIDs;
    int 	selectorSize;
    void loadSelector(xmlNodePtr node);

   void onNotification(ResEditor* editor, int nc, DWORD param) {};

	BOOL isRefMode(){ return stackManager.size()>0; }

	void loadConfig(const char* cfgFile);

	BOOL initEditorManager();

	void setCurResEditor(ResEditorInfo *newcur);

	ResManager* getResManager(int type);

	int getAllResManager(ResManager*** rms)
	{
		*rms = resManagers;
		return resNumber;
	}

	int selectID(int type);

	HWND hRefPanel; //the panel window who reference other mode

	//auto save
	BOOL isModified;
	int  save_counter;

	static BOOL auto_save_timer(HWND hwnd, int id, DWORD count);

	BITMAP bmp_logo;

public:
	HMENU createPopMenuFromConfig(int popmenu_id, mapex<int, int> id_state);

	MenuManager* getCommMenuManager(){ return &commMenu;}

	void onExit();

	DECLARE_MSG_MAP;

protected:

	int resNumber;
	ResManager** resManagers;

	void onResSwitch(int editorId, int code, HWND hwnd);
	void onEditorMenuCmd(int ctrlid, int code, HWND hwnd);
	void onSysMenuCmd(int ctrlid, int code, HWND hwnd);

	void onMenuStatusChanged(ResEditor* editor, int id, DWORD newStatus, int type=MSF_INCLUDE);
	void setEditorMenuStatus(ResEditorInfo * rei, int id, DWORD newStatus, int type=MSF_INCLUDE);

	void onCSizeChanged(int cx, int cy);
	//BOOL OnKeyDown(int scancode,DWORD key_status);
	BOOL onEraseBkgnd(HDC hdc, const PRECT pclip);
	void onDestroy();

	void onSave();

	//////////////////config
	const char* getVersion() { return "0.4"; }
	const char* getFormatVersion(){ return "0.4"; }
	const char* getEncoding(){ return "utf-8\0\0\0\0\0\0\0\0\0\0";}
	const char* getVendor(){ return "feynman-mstudio\0\0\0\0\0\0";}
	string getResPackageName() {
		/**
		 * in windows, readdir would change the current word dir, so, we
		 * must use the absolut path
		*/
        if (configuration.resPackName)
		{
			const char* pack_name = NULL;
            pack_name = strrchr(configuration.resPackName, '/');
			if(pack_name == NULL)
				pack_name = strchr(configuration.resPackName,'\\');
			if(pack_name == NULL)
				pack_name = configuration.resPackName;
			return string(strResPath + "/" + pack_name);
		}
        else
            return string(strResPath+"/"+getPrjName()+".res");
	}
	void writeStringResource(BinStream* bin);

	//////////////////////////////////////////

	//define the paths
	string strHeaderFile;
	string strPrjName;
	string strPrjPath;
	string strResPath;
	string strSrcPath;

	void setProjectPath(const char* path);
	void openFile(const char* file);
	void updateFile(const char* updateFile);
	const char* getResFileName(const char* file);

private:
	void initWindowMenu(HMENU hMenu, ResEditorInfo* who);
	string defRdr;
#ifdef _MSTUDIO_OFFICIAL_RELEASE
    BOOL    authMode;
    int     validDay;
#endif

public:
	MainFrame();
	virtual ~MainFrame();

	BOOL Create();

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    virtual BOOL isAuthMode() { return authMode; }
    virtual BOOL checkSoftDog(int* remainDay);
    virtual BOOL checkLimit();
#endif

    virtual const char* getDefRdrName()
    {
        return defRdr.c_str();
    }
    virtual BOOL setDefRdrName(const char* rdrName)
    {
        if (rdrName && GetWindowRendererFromName(rdrName)) {
			defRdr = rdrName;
            ncsSetSystemRenderer(rdrName);
			return TRUE;
        }
        return FALSE;
    }
	virtual const char* getResourcePath()
	{
		return strResPath.c_str();
	}
	virtual const char* getHeaderFile()
	{
		return strHeaderFile.c_str();
	}
	virtual const char* getSourcePath()
	{
		return strSrcPath.c_str();
	}

	virtual const char* getProjectPath()
	{
		return strPrjPath.c_str();
	}

	virtual const char* getPrjName()
	{
		return strPrjName.c_str();
	}

	void saveAll() {
        onSave();
    }
	int getEndian() { return LITTLE_ENDIAN; }
	void processArgs(CharStream * cs);
	void processArgs(int argc, const char** argv);

	void updateResName(int id, const char* newName);
	void updateResId(int oldId, int newId);
	void updateResValue(int id);
	void updateAllRes(int type);
	void deleteRefRes(int id);
	//string pool
private:
	//string pool struct
	struct StringEntry{
		string str;
		int id;
		int ref;
	};

	mapex<int, StringEntry*> stringPool;
	mapex<string, StringEntry*> namedStringPool;
	int str_id_begin;
	int newStrId()
	{
		if(stringPool.size() >= 65535){
			return -1;
		}

		if(str_id_begin >= 65535){
			str_id_begin = NCSRM_SYSSTR_MAXID;
		}

		while(stringPool.at(++str_id_begin) == NULL)
			return str_id_begin|(NCSRT_STRING<<16);
		return -1;
	}
public:
	virtual const char* getString (int id)
	{
		StringEntry * se;

		if(id < 0) 	return NULL;

		if((se = stringPool.at(id))!=NULL)
			return se->str.c_str();

		return NULL;
	}

	virtual int stringToId(const char* str){
		StringEntry * se;

		if(str == NULL || *str == 0
			|| strcmp(str, "res/") == 0)  return -1;

		if(NULL == (se = namedStringPool.at(str)))
			return -1;
		return se->id;
	}

	virtual int addString(const char*str)
	{
		int id;
		StringEntry * se;

		if(str == NULL || *str == 0
			|| strcmp(str, "res/") == 0)  return -1;

		if(NULL == (se = namedStringPool.at(str)))
		{
			if((id = newStrId()) < 0)  return -1;

			se = new StringEntry;
			se->id = id;
			se->str = str;
			se->ref = 1;

			stringPool[id] = se;
			namedStringPool[se->str] = se;
		} else {
			se->ref ++;
		}

		return se->id;
	}

	virtual int removeString(int id)
	{
		StringEntry * se;
		if((se = stringPool.at(id))!=NULL){
			if(--se->ref == 0){
				stringPool.erase(id);
				namedStringPool.erase(se->str);
				delete se;
			}
		}
		return 1;
	}

	//system config support
private:
	mapex<string,string> sysConfig;

	BOOL loadSysConfig(){
		//TODO Add SysConfig support
		return TRUE;
	}
public:
	const char* getSysConfig(const char* configName, const char* defaultValue=NULL){
		if(configName == NULL)
			return NULL;
		if(sysConfig.count(configName)>1)
			return sysConfig[configName].c_str();
		return defaultValue;
	}

public:
	void setMainCaption(const char* caption);

	//configuration
protected:
	struct ConfigurationInfo{
		char* name;
		char* caption;
		char* mgVersion;
		char* controlSet;
		string cfgPath;
		char* resPackName;

		struct LibraryInfo{
			void *handler;
			int  (*init)(void);
			void (*uninit)(void);
			char* libPath;
			char* symInit;
			char* symUninit;
			LibraryInfo(char* path, char* symInit, char* sysUninit);
			~LibraryInfo();

			void save(TextStream *stream);
		};
		list<LibraryInfo*> libraries;

		BOOL load(xmlNodePtr info);
		void save(TextStream *stream);

		ConfigurationInfo();
		~ConfigurationInfo();
	};

	string configFile;
	ConfigurationInfo configuration;

	BOOL loadConfig();
	BOOL saveConfig();
	BOOL newGUIBuilderConfig();

	string getConfigFile(const char* cfgFileName)
	{
		if(cfgFileName == NULL)
			return string("");
		string str = configuration.cfgPath;
		str += "/";
		str += cfgFileName;
		return str;
	}

public:
	BOOL newIDRange(IDRangeManager* idrm);

	IDRangeOwnerManager* getIDRangeOwnerManager(){
		return &idrange_owner_manager;
	};

protected:
	class IDRangeOwnerManagerImpl : public IDRangeOwnerManager
	{
		map<string, IDRangeOwner>  idrangOwners;
		IDRangeOwner* cur_owner;
	public:
		
		IDRangeOwnerManagerImpl();
		~IDRangeOwnerManagerImpl();

		IDRangeOwner* getOwner(const char* name);

		IDRangeOwner* getCurOwner() ;

		int getAllOwners(IDRangeOwner*** owners);

		IDRangeOwner * newOwner(const char* name);
	};

	IDRangeOwnerManagerImpl idrange_owner_manager;

	BOOL loadIDRanageInfo(xmlNodePtr node);
	BOOL saveIDRangeInfo(TextStream &txtStream);
};

#endif /* MAINFRAME_H_ */
