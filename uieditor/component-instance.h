/*
 * component-instance.h
 *
 *  Created on: 2009-3-27
 *      Author: dongjunjie
 */

#ifndef COMPONENTINSTANCE_H_
#define COMPONENTINSTANCE_H_

class ComponentInstance;

#define IS_EXTEND_PROP(id)   ((id)>= ComponentInstance::PropExtendBegin && (id)< ComponentInstance::PropExtendEnd)

class ComponentNotificationHandler: public InstanceNotificationHandler
{
public:
	virtual void onSetParent(ComponentInstance* instance) = 0;
	virtual void onRemoveFromParent(ComponentInstance* instance) = 0;
	virtual void onInsertInstance(ComponentInstance* instance, ComponentInstance* insert) = 0;
	virtual void onRemoveInstance(ComponentInstance* instance, ComponentInstance* remove) = 0;
};


class ComponentInstance: public Instance {
public:
	ComponentInstance(Class* cls);
	virtual ~ComponentInstance();

protected:
	HWND hwnd;
	ComponentInstance * parent, *children, *next, *prev;

	inline ComponentNotificationHandler* handler(){
		return dynamic_cast<ComponentNotificationHandler*>(notification);
	}

	void notifyInsertInstance(ComponentInstance* insert){
		if(handler())
			handler()->onInsertInstance(this, insert);
	}
	void notifyRemoveInstance(ComponentInstance* remove){
		if(handler())
			handler()->onRemoveInstance(this, remove);
	}
	void notifySetParent(){
		if(handler())
			handler()->onSetParent(this);
	}

	void notifyRemoveFromParent(){
		if(handler())
			handler()->onRemoveFromParent(this);
	}

	enum PreviewWndState {
		PreWndUpdating = 0x1,
		PreWndPreviewing = 0x02,
		PreWndSnapGrid = 0x04
	};

	DWORD prevWndState;

public:
	virtual HWND createPreviewWindow(HWND hParent = HWND_INVALID) = 0;
	virtual void destroyPreviewWindow();
	enum enumSyncPrevWindowRet{
		SPWE_OK = 0,
		SPWE_NOHANDLE,
		SPWE_NOFIELD,
		SPWE_REJECT,
		SPWE_IGNORED,
		SPWE_NEWVALUE,
	};
	virtual int syncPreviewWindow(int field_id) = 0;
	virtual BOOL updateSpecialField(int field_id, DWORD param){
		BOOL bRet = FALSE;
		for(ComponentInstance * child = children; child; child = child->next){
			if(child->updateSpecialField(field_id,param))
				bRet = TRUE;
		}
		return bRet;
	}
	virtual BOOL setDefRenderer(const char* defRdrName);
	virtual HWND recreatePreviewWindow(HWND hParent = HWND_INVALID){
		destroyPreviewWindow();
		return createPreviewWindow(hParent);
	}

	HWND getPreviewHandler(){ return hwnd; }
	virtual void resetPreviewHandler();
	virtual void setPreviewWindowHandler(HWND hwnd){ this->hwnd = hwnd;};

	virtual void updatePrevWindow(BOOL updateFlag=TRUE);

	virtual void previewWindow(BOOL bPreview = TRUE){
		if(bPreview)
			prevWndState |= PreWndPreviewing;
		else
			prevWndState &= ~PreWndPreviewing;
		//do nothing for it self;
		ComponentInstance* cinst = children;
		while(cinst)
		{
			cinst->previewWindow(bPreview);
			cinst = cinst->next;
		}
	}

	virtual void setSnapGrid(BOOL bSnape = TRUE){
		if(bSnape)
			prevWndState |= PreWndSnapGrid;
		else
			prevWndState &= ~PreWndSnapGrid;
		//do nothing for it self;
		ComponentInstance* cinst = children;
		while(cinst)
		{
			cinst->setSnapGrid(bSnape);
			cinst = cinst->next;
		}
	}

	virtual BOOL getDlgTemplate(DLGTEMPLATE* pteml){ return FALSE; }
	virtual BOOL getCtrlData(CTRLDATA *pctrl){ return FALSE; }
	virtual BOOL saveIndependXML(TextStream *textStream, BOOL saveTmplXML = FALSE) {
		saveXMLToStream(textStream);
		return TRUE;
	}

	//static instance
	static ComponentInstance* FromHandler(HWND hwnd){ return (ComponentInstance*)GetWindowAdditionalData(hwnd); }

	//static form xmlNode
	static ComponentInstance* createFromXmlNode(xmlNodePtr node);
	//static from className
	static ComponentInstance* createFromClassName(const char* typeName,const char* className);

	void setLocation(int x, int y);
	void getLocation(int&x, int &y);
	void setSize(int cx, int cy);
	void getSize(int &cx, int &cy);
	virtual void setCaption(const char* str){}

	//getBoundMask: to tell caller, location or size can changed or not
	enum BoundMask{BOUND_MASK_X=1, BOUND_MASK_Y=2, BOUND_MASK_WIDTH=4, BOUND_MASK_HEIGHT=8};
	virtual DWORD getBoundMask();

	virtual BOOL isContainer(){ return _class&&(_class->isContainer()); }

	virtual BOOL insert(ComponentInstance* insert, BOOL bAutoCreate = TRUE);
	virtual BOOL remove(ComponentInstance* insert, BOOL bAutoDestroy = TRUE);

	ComponentInstance * getParent() const { return parent; }
	ComponentInstance * getChildren() const { return children; }
	ComponentInstance * getPrev() const { return prev; }
	ComponentInstance * getNext() const { return next; }

	virtual void incUseOfRefReses(){
		Instance::incUseOfRefReses();
		for(ComponentInstance* cinst = children; cinst; cinst = cinst->getNext())
			cinst->incUseOfRefReses();
	}

	virtual void decUseOfRefReses(){
		Instance::decUseOfRefReses();
		for(ComponentInstance* cinst = children; cinst; cinst = cinst->getNext())
			cinst->decUseOfRefReses();
	}

	virtual DWORD processMenuCommand(int cmdid) { return 0; }
	enum meunCommandRet{
		DO_NOTHING = 0,
		NEED_UPDATE = 0x1,
		INSTANCE_REFRESH = 0x2,
	};

	//set or reset the new component Instance
	BOOL setParent(ComponentInstance* parent){
		if(getParent())
			getParent()->remove(this);
		this->parent = parent;
		if(parent)
			parent->insert(this);
		notifySetParent();
		return TRUE;
	}

	enum componentInstanceHitCode {
			OUT = 0, //out of instance
			IN, //Mouse is in the instance
			CONTAINER, // container area, can accept other instance
			REQ_MOUSE_AREA, //鼠标在需要鼠标消息的区域
	};
	virtual int hittest(int x, int y);
	virtual int processMessage(int message, WPARAM wParam, LPARAM lParam);

	enum enumPorp{
		PropClass = 1000,
		PropX ,
		PropY,
		PropWidth,
		PropHeight,
		PropText,
		PropRenderer,
		PropBkColor,
		PropFont,
		PropBkImage,
		PropMax
	};

	virtual BOOL enableClean(FieldType* ft){
		if(ft->id >= PropClass && ft->id <= PropText  && ft->id != PropRenderer)
			return FALSE;
		return Instance::enableClean(ft);
	}

	enum enumPropStyle{ PropStyleBegin = 2000, PropStyleEnd=2050};
	enum enumPropExStyle{ PropExStyleBegin = 3000, PropExStyleEnd=3020 };
	enum enumPropEvent { PropEventBegin = 4000, PropEventEnd = 5000 };
	enum enumPropExtend { PropExtendBegin = 10000, PropExtendEnd = 20000};

	ComponentInstance* cloneChildren(ComponentInstance * newParent) const;

	void setPreviewWindowUpdateFlag(BOOL bUpdate=TRUE){
		if(bUpdate)
			prevWndState |= PreWndUpdating;
		else
			prevWndState &= ~PreWndUpdating;
		ComponentInstance * instance = children;
		while(instance)
		{
			if(instance->isContainer())
			{
				instance->setPreviewWindowUpdateFlag(bUpdate);
			}
			instance = instance->next;
		}
	}

	/*
	 * template_table = {}
	 * template_table.name = instance_name
	 * template_table.id = instance->getID()
	 * template_table.serial = serial_num
	 * template_table.ctrlClass =
	 * template_table.x =
	 * template_table.y =
	 * template_table.w =
	 * template_table.h =
	 * template_table.style =
	 * template_table.exstyle =
	 * template_table.caption =
	 * template_table.props = { {id, value},{id,value}, {...} }
	 * template_table.rdr_info = { gbl_rdr = "classic", ctrl_rdr="classic", ctrl_class="", {id, value}, {id,value}}
	 * template_table.bk_color =
	 */
	virtual void toLuaTable(void *luaState, int table_idx) = 0;

	static void saveSrouce(const char* fileName, ComponentInstance *cinst, const char* scriptFile, const char* scripFunc);
	static void saveTemplates(const char* fileName, ComponentInstance* cinst, const char* scriptFile, const char* args=NULL);

	void switchComponentInstances(ComponentInstance* cinst1, ComponentInstance* cinst2);

	string newName();

	//undo redo support
public:
	void autoStoreIdName(){
		Instance::autoStoreIdName();
		for(ComponentInstance*child = children; child; child=child->getNext())
			child->autoStoreIdName();
	}
	void autoRegisterID(){
		Instance::autoRegisterID();
		for(ComponentInstance*child = children; child; child=child->getNext())
			child->autoRegisterID();
	}
};

////////////////////////////////////////
//Add Delete Instance command
class ComponentInstanceUndoRedoCommand : public UndoRedoCommand
{
public:
	ComponentInstanceUndoRedoCommand(ComponentInstance *parent, int count, int type);
	~ComponentInstanceUndoRedoCommand();

	void setInstance(int idx, ComponentInstance *inst);

	enum Type{
		ADD = 0,
		DELETE
	};
	void execute();

	int getCount(){ return count;}
	ComponentInstance ** getInsts() { return insts;}
	int getType() { return type; }
	ComponentInstance *getParent(){ return parent;}
private:
	ComponentInstance *parent;
	ComponentInstance **insts;
	int count;
	int type;

	//void deleteResId(ResManager* resMgr,Instance *inst);
};

class ComponentInstanceLocationUndoRedoCommand : public UndoRedoCommand
{
public:
	ComponentInstanceLocationUndoRedoCommand(ComponentInstance *oldContainer, int count);
	~ComponentInstanceLocationUndoRedoCommand();

	void setInstance(int idx, ComponentInstance *inst, int x, int y);

	void execute();

	struct BoundInfo {
		ComponentInstance *compinst;
		int x;
		int y;
	};

	int getCount(){ return count;}
	BoundInfo * getBoundInfo(){return boundInfos;}

	ComponentInstance *getContainer(){ return old_container; }

protected:
	BoundInfo * boundInfos;
	int count;
	ComponentInstance *old_container;

};


class ComponentInstanceBoundUndoRedoCommand : public UndoRedoCommand
{
public:
	ComponentInstanceBoundUndoRedoCommand(int count);
	~ComponentInstanceBoundUndoRedoCommand();

	void setInstance(int idx, ComponentInstance *cinst);
	void setInstance(int idx, ComponentInstance *cinst, const RECT *rc);

	void execute();

	struct Bound{
		ComponentInstance * cinst;
		RECT rc;
	};

	int getCount(){ return count; }
	Bound * getBounds(){ return bounds;}

private:
	Bound *bounds;
	int    count;
};

//////caption
class ComponentInstanceTextUndoRedoCommand: public UndoRedoCommand
{
public:
	ComponentInstanceTextUndoRedoCommand(ComponentInstance *cinst);
	~ComponentInstanceTextUndoRedoCommand();

	void execute();

	ComponentInstance *getInstance(){ return cinst;}
private:
	char * oldstr;
	ComponentInstance *cinst;
};

///////////////////////////////////////

#endif /* COMPONENTINSTANCE_H_ */
