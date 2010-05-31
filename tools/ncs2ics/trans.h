
#ifndef TRANS_H
#define TRAMS_H

typedef unsigned int DWORD;

enum  {
	VT_INT = 0, //int
	VT_STR,   //string
	VT_TEXT, //text
	VT_ENUM //enum
};

class Translator;

class Variable {
protected:
	int type;
public:

	virtual void combin(const Variable *pv) = 0;
	int getType() const { return type;}

	virtual string toString() = 0;

	virtual unsigned long getValue() = 0;

};

///////////////////////////////
// object
class TransedObject {
protected:
	string name;
	TransedObject * next;
	TransedObject * children;
	map<string, Variable*> props; 

public:
	TransedObject(){
		next = children = NULL;
	}

	~TransedObject(){
		for(map<string, Variable*>::iterator it = props.begin();
			it != props.end(); ++it)
		{
			delete it->second;
		}

		TransedObject* child = children;
		while(child)
		{
			TransedObject * t = child;
			child = child->next;
			delete t;
		}
	}

	string& getName(){ return name; }
	void setName(string &str){ name = str; }
	void setName(const char* str) { name = str; }

	map<string, Variable*>& getProps(){ return props;}

	void insertProp(string n, Variable *newv)
	{
		Variable *v = props[n];
		if(v == NULL)
			props[n] = newv;
		else
		{
			v->combin(newv);
			delete newv;
		}
	}


	void addNext(TransedObject* next){
		next->next = this->next->next;
		this->next = next;
	}

	TransedObject* getNext() { return next; }

	TransedObject* getChildren() { return children; }

	void appendChild(TransedObject* c){
		if(children == NULL){
			children = c;
			c->next = NULL;
			return ;
		}
	
		TransedObject* child;

		for(child = children; child->next; child = child->next);
		child->next = c;
		c->next = NULL;
	}

};

//////////////////////////////
class Translate
{
	friend class Translator;
protected:
	string source_name;
	int source_type;
	string dest_name;
	int dest_type;

	struct EnumMapNode {
		string name;
		string value;
	};
	list<EnumMapNode> enumMapper;
	
public:
	Translate()
	{
		source_type = dest_type = 0;
	}
	Variable * getVariable(const char* strvalue);
};

class Setter
{
	friend class Translator;
protected:
	string name;
	int type;
	string value;

public:
	string& getName(){ return name;}
	virtual Variable* getVariable();

	static Setter* createFromXML(xmlNodePtr node);
};

class Translator
{
protected:
	string name;
	
	map<string,Setter*> sets;
	map<string,Setter*> source_defaults;

	map<string,Translate*> translates;

	Translator *super;

public:
	Translator();
	~Translator();

	string & getName(){return name;}

	static Translator * createFromXML(xmlNodePtr node);

	TransedObject* translate(xmlNodePtr source);

protected:
	
	virtual Translate * getTranslate(const char* tname)
	{
		if(!tname)
			return NULL;
		
		if(translates.count(tname) != 0)
			return translates[tname];

		return super?super->getTranslate(tname):NULL;
	}

	void setSourceDefaults(TransedObject* tobj, list<string>& props_name_have_setted, Translator* pthis=NULL);
	
};

Translator* GetTranslatorByName(const char* name);




bool LoadTranslators(const char* list_file);

TransedObject* TranslateFile(const char*file);


string & GetTextById(int id);

bool LoadTextSource(const char* dir);

/////////////////

typedef void (*PProcessTransedObj)(TransedObject* tobj, const char* source_file, void* param);

bool TranslateProject(const char* prj_dir, PProcessTransedObj process_transedObj, void *param);


void DumpTransedObj(TransedObject* tobj, const char* source_file, FILE*  fp);

#endif


