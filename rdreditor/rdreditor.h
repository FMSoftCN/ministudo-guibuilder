/**
 * $Id$
 */

#ifndef RDREDITOR_H_
#define RDREDITOR_H_

#include "reseditor.h"

class RdrResource: public ResEditor::Resource {
public:
	union {
		RendererSet			*rdrSet;
		RendererInstance 	*rdr;
	}instance;

	RdrResource(const char* name, int id, const char* source)
	:ResEditor::Resource(name, id, source)
	{
		use_ref = 0;
		if (ID2TYPE(id) == NCSRT_RDR)
			instance.rdr = NULL;
		else
			instance.rdrSet = NULL;
	}
	~RdrResource(){
	}

	BOOL setRes(DWORD dwres) {
		if (ID2TYPE(id) == NCSRT_RDR) {
			RendererInstance* inst = (RendererInstance *)dwres;
			if (id != inst->getID())
				return FALSE;
			instance.rdr = inst;
			return TRUE;
		}
		else if (ID2TYPE(id) == NCSRT_RDRSET) {
			RendererSet* set = (RendererSet *)dwres;
			if (id != set->getID())
				return FALSE;
			instance.rdrSet = set;
			return TRUE;
		}

		return FALSE;
	}

	DWORD getRes() {
		if (ID2TYPE(id) == NCSRT_RDR)
			return (DWORD)instance.rdr;
		else if (ID2TYPE(id) == NCSRT_RDRSET)
			return (DWORD)instance.rdrSet;
		return 0;
	}
};

class RendererEditor: public ResEditor {
    struct RdrCopyPasteInfo {
        BOOL isCut;
        string name;

        RdrCopyPasteInfo() { 
            isCut = FALSE;
            name = "";
        }

        ~RdrCopyPasteInfo() {}
    };
protected:
	RendererPreviewPanel	*rdrPreviewPanel;
	RendererTreePanel		*rdrTreePanel;
	RendererPanel			*rdrPanel;

	string xmlRdrFile;
	Uint16 status;

	IDRangeManager idrmRdr;
	IDRangeManager idrmRdrSet;

	void load_classes(const char* ctrllist);

public:

	IDRangeManager* getIDRangeManager(int type, const char* name)
	{
		if(type == NCSRT_RDR)
			return &idrmRdr;
		else if(type == NCSRT_RDRSET)
			return &idrmRdrSet;
		return NULL;
	}

	int getAllIDRangeManagers(vector<IDRangeManager*> & mngrlist)
	{
		mngrlist.push_back(&idrmRdr);
		mngrlist.push_back(&idrmRdrSet);
		return 2;
	}

private:
    RdrCopyPasteInfo copyPasteInfo;

	BOOL loadXmlRdrFile(const char* xmlFile);
	template<class TInstance>
	int addResource(TInstance *instance, const char* idName);

    void saveXML();
    string saveBin(BinStream* bin);

public:
	mapex <string, string> classTypeList;	//classname=>typename
	set <string> ctrls_config;	//control config set
	map <string, set<string> > renderers; //classic: static, button ...
	map <string, set<string> > classes;	  //static: classic, skin

	RendererEditor();
	virtual ~RendererEditor();

	DWORD processEvent(Panel* sender, int event_id, DWORD param1 = 0, DWORD param2 = 0 );
	Panel* createPanel(const char* name, const char* caption,const mapex<string,string>*params);
	void executeCommand(int cmd_id, int status, DWORD param);
    void refreshRdrPanel(RendererInstance *inst);

	virtual int createRes(int type, const char* name, int id, const char* source, DWORD init_res);
	virtual void updateRes();
	virtual BOOL open(const char* xmlFile);
	virtual string save(BinStream* bin);
	virtual void copy(BOOL bremove = FALSE);
	virtual void paste();
	virtual BOOL initEditor();
	void getName(Class *cls, char* rdrName, char* clsName, char* typeName);

	virtual BOOL loadUserNodes(xmlNodePtr node) {
		if(xhIsNode(node, "file")) {
			xmlChar* xfilename = xhGetNodeText(node);
			if(xfilename) {
				xmlRdrFile =  (const char*)xfilename;
				xmlFree(xfilename);
			}

			xfilename = NULL;
			return TRUE;
		}
		return FALSE;
	}
	virtual BOOL saveUserNodes(TextStream *stream) {
		//save <file> node
		stream->printf("<file>");
		stream->printf("%s", xmlRdrFile.c_str());
		stream->println("</file>");
		return TRUE;
	}

	int getTypeMask(){ return NCSRT_RDR | NCSRT_RDRSET; }

	const char* getTypeName(int type_mask)
	{
		if(type_mask == NCSRT_RDR)
			return "Renderer";
		else if(type_mask == NCSRT_RDRSET)
			return "Renderer-set";
		return "";
	}

	void active(bool bactive, int reason){
			if(bactive && reason == USER_ACTIVE){
				g_env->setMainCaption("Renderer Editor");
			}
		}

	const char* getClsType(const char* clsname);

	int getUseRefValue(int id)
	{
		RdrResource *resource = (RdrResource *)reses.at(id);
		if (resource)
			return resource->use_ref;

		return -1;
	}
	virtual DWORD getRes(int id){
		RdrResource *resource = (RdrResource *)reses.at(id);
		if (resource)
			return resource->getRes();

		return 0;
	}

	virtual BOOL setRes(int id, DWORD dwres){
		RdrResource* resource = (RdrResource *)reses.at(id);
		if(resource == NULL)
			return FALSE;

		return resource->setRes(dwres);

		return FALSE;
	}

	int getSelectedResID();
	BOOL setResId(int oldId, int newId, DWORD res=0);
	int setResName(int id, const char* name, DWORD res=0);
	virtual BOOL removeRes(int id, DWORD res = 0);

	void getRendererList(const char* clsname, set<string> *pRdrList);
	void getClassList(const char* rdrname, set<string> &clsList);
	void getIDListByCtrlClsName(const char* clsName, map<int, string> *pidList);
	int newClsRenderer(HWND hParent, const char* clsName, char* strIdName=NULL, BOOL visibleCls=TRUE, GHANDLE parent=0);
	BOOL callSpecial(const char* strName, ...);

	enum enumIdListFlag {
		IDLIST_RDRSET,
		IDLSIT_RDR,
		IDLIST_ALL
	};
	void getIDListByRdrName(const char* rdrName, set<int> &idList, int idListFlag);

	//success return id, otherwise -1.
	int newRendererInstance(const char* rdrName,
			const char* clsName, const char* idName);
	//success return id, otherwise -1.
	int newRendererSet(const char* rdrName, const char* idName);

	virtual void onResNameChanged(int id, const char* newName);
	virtual void onResIdChanged(int oldId, int newId);
	virtual void onResValueChanged(int res_id);

	enum RdrEditorStatus{
		IDXML_CHANGED = 0x01,
		RDRXML_CHANGED = 0x02,
	};
	void setAllChanged() {
		status = IDXML_CHANGED | RDRXML_CHANGED;
	}
	void setIdXmlChanged() {
		if (!isIdXmlChanged())
		    status |= IDXML_CHANGED;
	}
	void setRdrXmlChanged() {
		if (!isRdrXmlChanged())
		    status |= RDRXML_CHANGED;
	}
	void clearIdXmlChanged() {
		if (isIdXmlChanged())
		    status &= ~IDXML_CHANGED;
	}
	void clearRdrXmlChanged() {
		if (isRdrXmlChanged())
		    status &= ~RDRXML_CHANGED;
	}

	BOOL isIdXmlChanged() {
		return status & IDXML_CHANGED;
	}
	BOOL isRdrXmlChanged() {
		return status & RDRXML_CHANGED;
	}
};

#endif /* RDREDITOR_H_ */
