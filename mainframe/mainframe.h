/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAINFRAME_H_
#define MAINFRAME_H_
#include <string>
using namespace std;

struct CharStream
{
	virtual ~CharStream(){}
	const inline bool isSpace(char ch){
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

	static BOOL auto_save_timer(HWND hwnd, LINT id, DWORD count);

	BITMAP bmp_logo;

public:
	HMENU createPopMenuFromConfig(int popmenu_id, mapex<int, int> id_state);

	MenuManager* getCommMenuManager(){ return &commMenu;}

	void onExit();

	DECLARE_MSG_MAP;

protected:

	int resNumber;
	ResManager** resManagers;
    struct Version {
        unsigned int major:16;
        unsigned int minor:8;
        unsigned int micro:8;
        char szVersion[16];

        Version(){
            major = 0; 
            minor = 0; 
            micro = 0;
            szVersion[0] = '\0';
        }

        Version(int major, int minor, int micro) {
            setVersion(major, minor, micro);
        }
        friend bool operator == (const Version &v1, const Version &v2) {
            return (v1.major == v2.major
                    && v1.minor == v2.minor
                    && v1.micro == v2.micro);
        }

        friend bool operator < (const Version &v1, const Version &v2) {
            return (v1.major < v2.major
                    || (v1.major == v2.major && v1.minor < v2.minor)
                    || (v1.major == v2.major && v1.minor == v2.minor && v1.micro < v2.micro));
        }

        void setVersion(int major, int minor, int micro)
        {
            this->major = major;
            this->micro = micro;
            this->minor = minor;
            sprintf(szVersion, "%d.%d.%d",major, minor, micro);
        }
    };

    static Version gbVersion;
    Version prjVersion;

	void onResSwitch(int editorId, int code, HWND hwnd);
	void onEditorMenuCmd(int ctrlid, int code, HWND hwnd);
	void onSysMenuCmd(int ctrlid, int code, HWND hwnd);

	void onMenuStatusChanged(ResEditor* editor, LINT id, DWORD newStatus, int type=MSF_INCLUDE);
    int getEditorMenuStatus(ResEditor* editor, LINT id, UINT* status);
	void setEditorMenuStatus(ResEditorInfo * rei, LINT id, DWORD newStatus, int type=MSF_INCLUDE);

	void onCSizeChanged(int cx, int cy);
	//BOOL OnKeyDown(int scancode,DWORD key_status);
	BOOL onEraseBkgnd(HDC hdc, const PRECT pclip);
	void onDestroy();

	void onSave();

	//////////////////config
	const char* getVersion() { return gbVersion.szVersion; }
	const char* getFormatVersion(){ return gbVersion.szVersion; }
	const char* getEncoding(){ return "utf-8\0\0\0\0\0\0\0\0\0\0";} //len is 16byte
	const char* getVendor(){ return "FM-miniStudiio\0\0";} //len is 16byte

    const char getPathSeparator()
    {
#ifdef WIN32
        return '\\';
#else
        return '/';
#endif
    }

    const char* getResPackageSuffix()
    {
        return ".res";
    }
	
	string getResPackageName() 
    {
		/**
		 * in windows, readdir would change the current word dir, so, we
		 * must use the absolut path
		*/
        const char sep = getPathSeparator();
        if (configuration.resPackName && configuration.resPackName[0] != '\0')
		{
			/*
            const char* pack_name = NULL;
			pack_name = strrchr(configuration.resPackName, sep);
			if(pack_name == NULL)
				pack_name = configuration.resPackName;
				*/
			string sep_linux = "/";
			string sep_windows = "\\";
			string pack_name;
			pack_name = string(configuration.resPackName);
			string::size_type pos;
			pos = pack_name.find_last_of(sep_linux);
			if (pos != string::npos)
				pack_name = pack_name.substr(pos + 1);
			
			//pack_name = replace_all(string(pack_name), sep_linux, sep_windows).c_str();
			
			LOG_WARNING("--------pack_name=%s----:%s----\n",pack_name.c_str(),string(strResPath + sep + pack_name.c_str()).c_str());
			return string(strResPath + sep + pack_name.c_str());
			
			//return string(strResPath + sep + getPrjName() + getResPackageSuffix());
		}
        else {
			LOG_WARNING("+++++++++++++++++++++++++++++++++++pack_name=-------------peizhi:%s--+++++++++++++\n",string(strResPath + sep + getPrjName() + getResPackageSuffix()).c_str());
            return string(strResPath + sep + getPrjName() + getResPackageSuffix());
        }
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
    string  sn;
    string  licenseMode;
#endif

public:
	MainFrame();
	virtual ~MainFrame();

	BOOL Create();

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    virtual BOOL isAuthMode() { return authMode; }
    virtual BOOL checkSoftDog(int* remainDay);
    virtual BOOL checkLimit();
    virtual string checkSNInfo();

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
        //make sure project name is not null.
		if (strPrjName.compare("") == 0) {
            string pwd(getenv("PWD"));
            size_t found = pwd.find_last_of(getPathSeparator());
            if (found != string::npos) {
                strPrjName = pwd.substr(found + 1);
            }
            else 
                strPrjName = pwd;
        }
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

    //generatre res
    BOOL generateIncoreRes(int type);
};

#endif /* MAINFRAME_H_ */
