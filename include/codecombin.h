
#ifndef CODE_COMBIN_H
#define CODE_COMBIN_H

#ifndef DEBUG
#define DEBUG 1
#endif

#define ET_USER 0
#define ET_AUTO 1
#define ET_DOC  2
#define ET_TEXT 3

class Element{
protected:
	int type;

public:
	Element * next;

	Element(int type){
		this->type = type;
		next = NULL;
	}

	virtual ~Element();

	int getType() { return type; }

	virtual bool loadFromFile(FILE *fp) = 0;

	virtual bool saveToFile(FILE* fp) = 0;

	virtual Element *clone() = 0;

#if DEBUG
	virtual void print(FILE *fp = stdout) = 0;
#endif
};

class TextElement : public Element {
protected:
	char * text;

public:
	TextElement():Element(ET_TEXT){ text = NULL; }
	TextElement(const TextElement & te);
	~TextElement();

	bool loadFromFile(FILE* fp);
	bool saveToFile(FILE* fp);

	void insertText(const char* str, int at);

	void appendText(const char * str){
		insertText(str, -1);
	}

	void setText(const char* str);

	const char* getText(){ return text; }

	Element* clone(){ return new TextElement(*this); }

#if DEBUG
	void print(FILE *fp = stdout) ;
#endif
};

class CompoundElement : public Element {
protected:
	Element * children;
	char    * name;

	void appendChild(Element * e);
public:
	CompoundElement(int type)
	:Element(type){ children = NULL; name = NULL; }
	CompoundElement(const CompoundElement& ce);

	~CompoundElement();

	bool loadFromFile(FILE *fp);
	bool saveToFile(FILE *fp);

	const char* getName(){ return name; }

	Element * getChild(int type, const char* name);

	Element* insertChild(Element *e, Element *at, bool before=false);

	bool combin(CompoundElement *ce);

	Element* clone(){ return new CompoundElement(*this); }


#if DEBUG
	void print(FILE *fp = stdout) ;
#endif

	static CompoundElement* LoadFromFile(const char* file);
};

class AutoGenElement : public CompoundElement {
public:
	AutoGenElement():CompoundElement(ET_AUTO){}

	~AutoGenElement();

	bool disableCode();

	bool replaceAutoGenPart(AutoGenElement *page);
};


int CodeCombinFiles(const char * fileFrom, const char* fileTo);

#endif

