/*
 * class_instance.h
 *
 *  Created on: 2009-3-25
 *      Author: dongjunjie
 */

#ifndef CLASS_INSTANCE_H_
#define CLASS_INSTANCE_H_

#include "clipboard.h"

class Reference
{
	int ref;
public:
	Reference():ref(1){}
	virtual ~Reference(){}
	int addRef(){
		return ++ref;
	}

    int decRef(){
        return --ref;
    }

    int getRef() {
        return ref;
    }
	int release(){
		if(--ref == 0) {
			delete this;
			return 0;
		}
		return ref;
	}
};

struct Field{
	int id:24;
	int attr:8;
	Value value;
};

struct FieldType{
	int id;
	string name;
	ValueType * vtype;
	string error_tip;

	FieldType(int id, const char* name, ValueType* vtype, const char* error_tip){
		this->id = id;
		this->name = name?name:"";
		this->vtype = vtype;
		if(error_tip)
			this->error_tip = error_tip;
		this->vtype->addRef();
	}

	~FieldType(){
		if(vtype)
			vtype->release();
	}
};

#ifdef Class
#undef Class
#endif

class Class;

class Instance;

class FieldEnumHandler
{
public:
	virtual ~FieldEnumHandler(){}
	virtual BOOL setField(Class* cls, int id, const char* name, ValueType* vtype, DWORD user) = 0;
};

class Class : public Reference
{
protected:
	Class * super;

	mapex<int, FieldType*> fieldTypes;
	mapex<string, FieldType*> namedFieldTypes;

	mapex<int, Field*> defaultFields; //default Field;

	string className;


	static mapex<string, Class*> _classMaper;

	BOOL bContainer;
    BOOL bControl;

	void addFieldType(const char* name,int id, ValueType* vtype, const char* error_tip = NULL);

private:
	static BOOL loadClassProperty(xmlNodePtr node, Class *_class, int& prop_id);
	static BOOL loadClassEvent(xmlNodePtr node, Class *_class, int& event_id, char **default_prototype);
	static BOOL loadClassDefProperty(xmlNodePtr node, Class *_class, uint8_t attr);

public:
	static Class* loadFromXML(xmlNodePtr node); //load from xml
	Class* getSuper() { return super; }
	static Class* getClassByName(const char* rootNodeName, const char* className);
    static void unloadAllClasses();

	//create a new name auto
	static string newClassName(Class* cls);

	int classRefCount();

    static const mapex<string, Class*> getClassMaper(void) { return _classMaper; }
    BOOL isControl(void) { return bControl; }
    //call when reading config file, such as control.cfg
    void initAsControl (BOOL bCtrl) { bControl = bCtrl; }

	Class(const char* name, Class* super = NULL){
		this->super = super;
		if(super)
			super->addRef();
		//fieldTypeBlocks = NULL;
		if(name)
			className = name;

        bControl = FALSE;
	}

	bool isClass(const char* str){
		if(!str)
			return false;
		if(strcmp(className.c_str(),str) == 0)
			return true;
		if(super)
			return super->isClass(str);
		return false;
	}

	virtual ~Class();

	const char* getClassName(){ return className.c_str(); }

	FieldType * getFieldType(int id);
	FieldType * getFieldType(const char* name);
	//get the Value of Field
	ValueType * getFieldValueType(int id);
	//get the type of ID value.
	int getIDValueType(int id);
	ValueType * getFieldValueType(const char* name);
	//get Default Value
	Value getFieldDefault(int id);
	//get FieldDefaultAttr
	enum{ FIELD_ATTR_FIXED = 1, FIELD_ATTR_HIDE = 2 };
	DWORD getFieldDefaultAttr(int id);
	BOOL isDefaultFieldExist(int id);

	void getRangedDefaultValue(int begin_id, int end_id,map<int,Field*> & values);

	//Id to Name
	const char* getFieldName(int id);
	int getFieldId(const char* name);

	BOOL isContainer(){ return bContainer; }

	//for ResEditor, to enum fields
	virtual BOOL enumFields(FieldEnumHandler *handler, BOOL enumSupper = TRUE, DWORD dwUser=0);

	const mapex<int, Field*> getDefaultFiles() const { return defaultFields; }
};

//////
class InstanceNotificationHandler
{
public:
	virtual ~InstanceNotificationHandler(){}
	virtual void onFieldChanged(int fieldId, Instance *instance) = 0;
};

class Instance;
struct ListenEntry
{
	int sender_id;
	int event_id;
	string prototype;
};

class Instance : public Reference  //, public UndoRedoObject
{
protected:
	Class * _class; //the class of Instance
	int id;
	InstanceNotificationHandler * notification;

	mapex<int, Field*> fields;

	void notifyFieldChanged(int fieldId){
		if(notification)
			notification->onFieldChanged(fieldId, this);
	}

	BOOL bLocked;

	BOOL loadProperty(xmlNodePtr node);
	BOOL loadMessage(xmlNodePtr node);
	BOOL loadListen(xmlNodePtr node);

	DWORD serialNum; //serialNumber, the only one id

	static DWORD genSerialNum(Instance *inst);

	void loadSerial(xmlNodePtr node);
	void saveSerial(TextStream* stream);

	struct RefResFieldItem{
		Field* field;
		RefResFieldItem *next;
	};

	RefResFieldItem * ref_list;//reference list

	list<ListenEntry*> listens;

public:

	Instance(Class* cls);
	Instance(InstanceNotificationHandler* notification, Class* cls, int id);
    virtual ~Instance();

	Class * getClass(){ return _class;}

	int getID(){ return id; }

	DWORD getSerialNumber(){ return serialNum; }

	int setID(int id){
		int old = this->id;
		this->id = id;
		return old;
	}

	bool isInstance(const char* strClss){
		return _class->isClass(strClss);
	}

	virtual BOOL loadFromXML(xmlNodePtr node) = 0;

	//get Field value
	virtual Value getField(int id);
	//set Field value
	virtual void setField(int id, Value value, BOOL bAutoUse=FALSE);
	//clean Field, set field as default
	virtual void cleanField(int id, BOOL bforce_clean=FALSE);
	//clean all between begin_id and end_id. if default, delete all.
	virtual void cleanAll(int begin_id = -1, int end_id = -1,BOOL bforce_clean=FALSE);

	virtual void removeRefReses();

	int getIDValueType(int id);
	int getFieldsSize() {return fields.size(); }

	virtual BOOL isFieldExist(int id);

	virtual BOOL isSettedField(int id){
		return fields.find(id) != fields.end();
	}

	//get Attribute
	virtual DWORD getFieldAttr(int id){
		Field *field = fields.at(id);
		if(field)
				return field->attr;
		return _class->getFieldDefaultAttr(id);
	}

	virtual void getRangedFields(int begin_id, int end_id, map<int, Field*> & values);

	virtual void saveXMLToStream(TextStream *stream) = 0;

	virtual int saveBinToStream(BinStream *stream) = 0;

	virtual BOOL lock(){ bLocked = TRUE; return TRUE; } //lock instance, cannot edit it
	virtual BOOL unlock(){ bLocked = FALSE; return TRUE; } //unlock instance, can edit

	virtual Instance* clone() = 0;

	const mapex<int, Field*>& getFields() const { return fields; }

	void copyFields(mapex<int, Field*> &newFields) const;

	void copyFieldValues(mapex<int, DWORD> &newFieldvalues, int begin=-1, int end=-1) const;

	void setNotification(InstanceNotificationHandler *notification) {
        this->notification = notification;
	}

	virtual string newName(){
		return Class::newClassName(_class);
	}

	virtual BOOL onResIdChanged(int res_old_id, int res_new_id, int* updateFields=NULL, int max = 0);
	virtual int getReferencedFieldIds(int res_id, int* fieldIds, int max);
	virtual BOOL isInRefList(Field *field);
	virtual BOOL addRefList(Field *field);
	virtual BOOL removeRefList(Field* field);
	virtual BOOL enableClean(int id){
		return enableClean(_class->getFieldType(id));
	}
	virtual BOOL enableClean(FieldType* ft){
		return ft && ft->vtype && ft->vtype->getType()!=VT_TEXT;
	}

	RefResFieldItem * copyRefFieldList(mapex<int, Field*> &newFields) const;

	ListenEntry* addListen(int sender, int event_id, const char* prototype);

	//if event_id == -1, mean clear all the events
	//if sender == -1, mean clear all the events named event_id
	BOOL removeListen(int sender = -1, int event_id=-1);

	const char* getListenPrototype(int sender, int event_id);

	BOOL saveListens(TextStream *stream);

	BOOL clearListens();

	list<ListenEntry*>& getListens(){
		return listens;
	};

	template<typename TField>
	const bool getFieldErrorTip(TField field, string& str) {
		str = "";
		FieldType * ft = _class->getFieldType(field);
		if(!ft)
			return false;
		str = ft->error_tip;
		return true;
	}


	virtual void incUseOfRefReses();

	virtual void decUseOfRefReses();

public: //copy cut, paste support

	static BOOL copy(Instance** instances, int count);
	static BOOL cut(Instance** instances, int count);
	static ObjectClipBoard<Instance*>::Array paste();

	/*
	 *  prop_table = {}
	 *  prop_table.id1 = value1
	 *  prop_table.id2 = value2
	 *  	...
	 *
	 */
	void setPropsToLuaTable(void* luaState, int prop_begin, int prop_end);
	static int inst_push_value_type(void* luaState, int table_idx, Value value, ValueType *vt);


	//support undoredo
public:
	virtual void autoStoreIdName();
	virtual void autoRegisterID();
protected:
	string strIDName;
};

#define SET_TABLE(L,t, key, value) do{ \
	value ; \
    lua_setfield(L, t, #key); \
}while(0)

typedef ObjectClipBoard<Instance*>::Array InstanceArray;


template<class TInstance>
TInstance* createInstanceByName(const char* typeName, const char* className)
{
	Class * _class = Class::getClassByName(typeName, className);
	if(_class == NULL)
		return NULL;
	TInstance* instance = (TInstance*)(new TInstance(_class));
	if(instance == NULL)
		return NULL;
	return instance;
}

///////////////////////////////////////////
class InstancePropertyUndoRedoCommand : public UndoRedoCommand
{
public:
	InstancePropertyUndoRedoCommand(Instance * inst, int prop_id, Value value, BOOL bIsDefault);
	~InstancePropertyUndoRedoCommand();

	void execute();

	int getProp(){ return prop_id; }
	Instance* getInstance(){ return inst;}

protected:
	Instance *inst;
	int       prop_id;
	DWORD     old_value;
	BOOL      old_is_default;
};

class InstanceAllDefPropertiesRedoUndoCommand : public UndoRedoCommand
{
public:
	InstanceAllDefPropertiesRedoUndoCommand(Instance *inst, int begin=-1, int end=-1);
	~InstanceAllDefPropertiesRedoUndoCommand();

	void execute();

	Instance * getInstance(){ return inst;}

protected:
	BOOL is_undo; //restorage the value before set default;
	mapex<int, DWORD> field_values;
	Instance *inst;
};

/////////////////////////////////////////////////////////////////////

#endif /* CLASS_INSTANCE_H_ */
