/**
 * $Id$
 */

#ifndef RDRINSTANCE_H_
#define RDRINSTANCE_H_

class RendererInfo : public NCS_RDR_INFO
{
public:
	RendererInfo(){
		ctl_rdr = NULL;
		glb_rdr = NULL;
		elements = NULL;
	}
	~RendererInfo(){
		if(elements)
			delete[] elements;
	}

	inline NCS_RDR_ELEMENT& operator[](int idx)
	{
		if(elements == NULL)
			throw("elements is NULL");
		return elements[idx];
	}

	void resize(int count){
		if(count <= 0)
			return;

		if(elements)
			delete[] elements;

		elements = new NCS_RDR_ELEMENT[count+1];
		elements[count].id = -1;
		elements[count].value = (DWORD)-1;
	}
};

class RendererInstance: public Instance {
protected:
	string rdrName;
	string clsName;
	string typeName;
	HWND hWnd;
	mWidgetRenderer* rdr;
	WindowInstance *win_inst;

public:
	RendererInstance(Class *cls);
	RendererInstance(const RendererInstance &rdrst);

	BOOL getNcsRdrInfo(RendererInfo &rdrInfo);

	virtual ~RendererInstance();

	Instance * clone();
	string newName();

	static RendererInstance* createFromXmlNode(xmlNodePtr node);
	//static from className
	static RendererInstance* createFromClassName(const char* typeName,const char* className);
	Value getField(int id);

	BOOL loadFromXML(xmlNodePtr node);
	virtual void saveXMLToStream(TextStream *stream);

	//return size of total window
	virtual int saveBinToStream(BinStream *stream);
	virtual const char* getClassType() { return "renderer"; }

	const char* getRdrName() { return rdrName.c_str(); }
	const char* getClsName(){ return clsName.c_str(); }
    const char* getTypeName () { return typeName.c_str();}
    const char* getControlClass();
    HWND getHandler() { return hWnd; }
	HWND createPreviewWindow(HWND hParent);
	void destroyPreviewWindow();
	void updatePreviewWindow(int element_id, HWND hwndToUp = HWND_INVALID);

public:
	/*
	 * rdr_info = {}
	 * rdr_info.gbl_rdr = ""
	 * rdr_info.ctrl_rdr= ""
	 * rdr_info.elements={ id1=vale1, id2=value2 , ... }
	 *  ....
	 *
	 */
	void toLuaTable(void *luaState);

private:
	DWORD getRdrElementValue(int id, Value val, ValueType *vtype);
};


class RendererSet: public Reference {
private:
	//only for initialization, not maintain in runtime.
	set <int> rdrIds;

    //runtime info.
	set <RendererInstance*> insts;

	string rdrName;
	int id;

public:

	RendererSet(const char* clsName);

	virtual ~RendererSet();

	static RendererSet* createFromXmlNode(xmlNodePtr node);
	//static from className
	static RendererSet* createFromClassName(const char* typeName,const char* className);

	BOOL loadFromXML(xmlNodePtr node);
	virtual void saveXMLToStream(TextStream *stream);

	//return size of total window
	virtual int saveBinToStream(BinStream *stream);
	string newName();
	const char* getRdrName() { return rdrName.c_str(); }
	int getID () { return id; }

	int setID(int id){
		int old = this->id;
		this->id = id;
		return old;
	}

	virtual const char* getClassType() { return "rdrset"; }

    BOOL accept(RendererInstance *inst);
	BOOL insertInstance(RendererInstance *inst);
	BOOL removeInstance(RendererInstance *inst);
	BOOL hasRefInstance();
    //please call it after loading rdr.xml
    void initRdrSets();

    BOOL isExist(int rdr_id)
    {
        if (ID2TYPE(rdr_id) != NCSRT_RDR)
            return FALSE;

        for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
        {
            if (rdr_id == (*it)->getID())
                return TRUE;
        }

        return FALSE;
    }

    set<RendererInstance *>& getInsts() { return insts; }
    int release();
};

#endif /* RDRINSTANCE_H_*/
