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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmlheads.h"
#include "xmlhelp.h"

#include <string>
#include <map>
#include <list>

using namespace std;


#include "trans.h"

class IntVariable;
class StrVariable;

class IntVariable : public Variable {
	DWORD value;
public:
	IntVariable(DWORD value){
		type = VT_INT;
		this->value = value;
	}
	
	void combin(const Variable *pv)
	{
		if(pv->getType() == VT_INT)
			value |= ((IntVariable*)pv)->value;
	}

	string toString(){
		char szText[64];
		sprintf(szText,"0x%08X", value,value);
		return szText;
	}

	unsigned long getValue(){ return value; }
};

class StrVariable : public Variable {
	string strValue;
public:
	StrVariable(const char* str){
		type = VT_STR;
		strValue = str;
	}

	StrVariable(string& str){
		type = VT_STR;
		strValue = str;
	}

	void combin(const Variable *pv)
	{
		if(pv->getType() == VT_STR)
			strValue += ((StrVariable*)pv)->strValue;
	}

	string toString(){
		return strValue;
	}

	unsigned long getValue(){ return (unsigned long)strValue.c_str(); }
};

static int name2type(const char* str)
{
	static const char* names[]={
		"int",
		"string",
		"text",
		"enum",
		NULL
	};

	for(int i=0; names[i]; i++)
		if(strcmp(str, names[i]) == 0)
			return i;
	return -1;
}


Translator::Translator()
{
	super = NULL;
}

Translator::~Translator()
{
	for(map<string,Setter*>::iterator it = sets.begin();
		it != sets.end(); ++it)
	{
		delete it->second;
	}
	
	for(map<string,Setter*>::iterator it = source_defaults.begin();
		it != source_defaults.end(); ++it)
	{
		delete it->second;
	}

	for(map<string,Translate*>::iterator it = translates.begin();
		it != translates.end(); ++it)
	{
		delete it->second;
	}
}


TransedObject* Translator::translate(xmlNodePtr source_node)
{
	TransedObject* tobj, *childobj;
	list<string> seted_name;

	tobj = new TransedObject;
	tobj->setName(name);

	//add setters
	for(map<string,Setter*>::iterator it = sets.begin();
		it != sets.end(); ++it)
	{
		//insert mapstring
		Setter* s = it->second;
		Variable* v = s->getVariable();
		if(v)
			tobj->insertProp(s->getName(), v);
	}

	for(xmlNodePtr node = source_node->xmlChildrenNode;
		node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;

		//get node name
		if(xhIsNode(node, "window") || xhIsNode(node, "paged-window"))
		{
			//find name 
			xmlChar* xclass = xmlGetProp(node, (const xmlChar*)"class");
			if(!xclass)
				continue;

			Translator * t = GetTranslatorByName((const char*)xclass);
			xmlFree(xclass);
			if(!t)
				continue;

			childobj = t->translate(node);
			if(childobj)
			{
				tobj->appendChild(childobj);
			}
		}
		else
		{
			Translate * t = getTranslate((const char*)(node->name));
			if(t == NULL)
				continue;
			
			xmlChar* xvalue = xhGetNodeText(node);
			if(!xvalue)
				continue;

			Variable *newv = t->getVariable((const char*)xvalue);
			tobj->insertProp(t->dest_name, newv);
			seted_name.push_back(t->source_name);
		}
	}

	//set defaults
	
	setSourceDefaults(tobj,seted_name);	
	return tobj;
}

void Translator::setSourceDefaults(TransedObject* tobj, list<string>& props_name_have_setted, Translator* pthis/*=NULL*/)
{
	if(!tobj)
		return ;

	if(pthis == NULL)
		pthis = this;

	for(map<string, Setter*>::iterator it = source_defaults.begin();
		it != source_defaults.end(); ++it)
	{
		Setter* s = it->second;
		bool seted = false;

		for(list<string>::iterator lit = props_name_have_setted.begin();
			props_name_have_setted.end() != lit; ++lit)
		{
			if(strcmp(s->name.c_str(), lit->c_str()) == 0)
			{
				seted = true;
				break;
			}
		}

		if(!seted)
		{
			Translate * t = pthis->getTranslate(s->getName().c_str());
			//fprintf(stderr, "---- %s-%s: t:%p, %s\n",name.c_str(),pthis->name.c_str(), t, s->getName().c_str());
			if(t)
			{
				Variable * v = t->getVariable(s->value.c_str());
				if(v)
				{
					tobj->insertProp(t->dest_name,v);
					props_name_have_setted.push_back(s->getName());
				}
			}
		}
	}

	if(super)
		super->setSourceDefaults(tobj, props_name_have_setted, pthis);
}

Setter* Setter::createFromXML(xmlNodePtr node)
{
	xmlChar* x = xmlGetProp(node,(const xmlChar*) "name");
	Setter * s = NULL;
	if(x)
	{
		s = new Setter();
		s->name = (const char*)x;

		xmlFree(x);

		x = xmlGetProp(node, (const xmlChar*)"type");
		if(x){
			s->type = name2type((const char*)x);
			xmlFree(x);
		}

		x = xmlGetProp(node, (const xmlChar*)"value");
		if(x)
		{
			s->value = (const char*)x;
			xmlFree(x);
		}
	}
	return s;
}

Translator * Translator::createFromXML(xmlNodePtr rnode)
{
	if(rnode == NULL)
		return NULL;

	//get Name
	xmlChar* xname = xmlGetProp(rnode, (const xmlChar*)"class");
	if(!xname)
		return NULL;

	Translator * t = new Translator();

	t->name = (const char*)xname;

	xmlFree(xname);

	//get extends
	xmlChar* xextends = xmlGetProp(rnode, (const xmlChar*)"extends");

	if(xextends){
		t->super = GetTranslatorByName((const char*)xextends);
		xmlFree(xextends);
	}

	
	for(xmlNodePtr node = rnode->xmlChildrenNode;
		node;
		node = node->next)
	{
		if(xhIsNode(node, "set"))
		{
			Setter* s = Setter::createFromXML(node);
			if(s)
			{
				//printf("-- setter = %s:%s,%d\n", s->name.c_str(), s->value.c_str(), s->type);
				t->sets[s->name] = s;
			}
		}
		else if(xhIsNode(node, "source-default"))
		{
			Setter* s = Setter::createFromXML(node);
			if(s)
			{
				t->source_defaults[s->name] = s;
			}
		}
		else if(xhIsNode(node, "translate"))
		{
			Translate* tt = new Translate();
			for(xmlNodePtr tn = node->xmlChildrenNode; tn; tn = tn->next)
			{
				xmlChar* x1;
				if(xhIsNode(tn, "source"))
				{
					x1 = xmlGetProp(tn, (const xmlChar*)"name");
					if(x1){
						tt->source_name = (const char*)x1;
						xmlFree(x1);
					}
					x1 = xmlGetProp(tn, (const xmlChar*)"type");
					if(x1){
						tt->source_type = name2type((const char*)x1);
						xmlFree(x1);
					}
				}
				else if(xhIsNode(tn, "dest"))
				{
					x1 = xmlGetProp(tn, (const xmlChar*)"name");
					if(x1){
						tt->dest_name = (const char*)x1;
						xmlFree(x1);
					}
					x1 = xmlGetProp(tn, (const xmlChar*)"type");
					if(x1){
						tt->dest_type = name2type((const char*)x1);
						xmlFree(x1);
					}

				}
				else if(xhIsNode(tn,"enum-map"))
				{
					for(xmlNodePtr c = tn->xmlChildrenNode; c; c=c->next)
					{
						if(c->type != XML_ELEMENT_NODE)
							continue;
						Translate::EnumMapNode mn;
						mn.name = (const char*)c->name;
						x1 = xhGetNodeText(c);
						mn.value =(const char*)x1;
						if(x1)
							xmlFree(x1);

						tt->enumMapper.push_back(mn);
					}
				}
			}
			t->translates[tt->source_name] = tt;
		}
	}

	return t;

}

////////////////////////////////////////////////////
static map<string, Translator*> translators;
bool LoadTranslators(const char* list_file)
{
	if(!list_file)
		return false;

	FILE *fp = fopen(list_file, "rt");
	if(!fp)
		return false;
	
	char szLine[1024];
	while(fgets(szLine, sizeof(szLine), fp))
	{
		int len = strlen(szLine);
		szLine[len-1] = 0;
		xmlDocPtr doc = xmlParseFile(szLine);
		if(!doc)
			continue;
		Translator * t = Translator::createFromXML(xmlDocGetRootElement(doc));

		xmlFreeDoc(doc);

		if(t)
		{
			translators[t->getName()] = t;
		}
	}

    fclose(fp);
	return true;
}


Translator* GetTranslatorByName(const char* name)
{
	if(!name)
		return NULL;

	if(translators.count(name) > 0)
		return translators[name];

	return NULL;
}


TransedObject* TranslateFile(const char*file)
{
	if(!file)
		return NULL;

	xmlDocPtr doc = xmlParseFile(file);
	if(!doc)
		return NULL;

	xmlNodePtr node = xmlDocGetRootElement(doc);

	if(!node)
		return NULL;

	xmlChar* x = xmlGetProp(node, (const xmlChar*)"class");
	if(!x){
		xmlFreeDoc(doc);
		return NULL;
	}


	Translator * t = GetTranslatorByName((const char*)x);

	xmlFree(x);
	
	if(!t){
		xmlFreeDoc(doc);
		return NULL;
	}

	TransedObject* tobj = t->translate(node);

	xmlFreeDoc(doc);

	return tobj;
}

////////////////////////////////////////////////
Variable* Translate::getVariable(const char* strvalue)
{
	if(!strvalue)
		return NULL;

	if(dest_type == VT_INT){
		if(source_type == VT_ENUM)
		{
			for(list<EnumMapNode>::iterator it = enumMapper.begin();
				it != enumMapper.end(); ++it)
			{
				EnumMapNode &node = *it;
				if(strcmp(node.name.c_str(), strvalue) == 0)
					return new IntVariable(strtol(node.value.c_str(), NULL, 0));
			}
			return new IntVariable(0);
		}
		else
			return new IntVariable(strtol(strvalue,NULL,0));
	}
	else if(dest_type == VT_STR)
	{
		if(source_type == VT_STR || source_type == VT_INT)
			return new StrVariable(strvalue);
		else if(source_type == VT_TEXT)
		{
			string &str = GetTextById(strtol(strvalue, NULL, 0));
			return new StrVariable(str);	
		}
		else if(source_type == VT_ENUM)
		{
			for(list<EnumMapNode>::iterator it = enumMapper.begin();
				it != enumMapper.end(); ++it)
			{
				EnumMapNode &node = *it;
				if(strcmp(node.name.c_str(), strvalue) == 0)
					return new StrVariable(node.value);
			}
			return new StrVariable(strvalue);
		}
	}
	else
		return NULL;
}

//////////////////////////////////////////

Variable* Setter::getVariable()
{
	if(type == VT_INT)
		return new IntVariable(strtol(value.c_str(),NULL,0));

	else if(type == VT_STR)
		return new StrVariable(value);
	else if(type == VT_TEXT)
		return new StrVariable(GetTextById(strtol(value.c_str(),NULL,0)));

	return NULL;
}


//////////////////////////////////////////////////
//

static map<int,string> texts;

string str_null = "";

string & GetTextById(int id)
{
	if(texts.count(id) > 0)
		return texts[id];
	return str_null;
}


string getTextInfo(xmlNodePtr node)
{
	string str = "";
	xmlChar* xlang = NULL, *xcountry = NULL;
	for(node = node->xmlChildrenNode; node && (!xlang || !xcountry); node = node->next)
	{
		if(xhIsNode(node, "lang")){
			xlang = xhGetNodeText(node);
		}
		else if(xhIsNode(node,"country")){
			xcountry = xhGetNodeText(node);
		}
	}

	if(!xlang || !xcountry)
	{
		if(xlang) xmlFree(xlang);
		if(xcountry) xmlFree(xcountry);
		return str_null;
	}

	str = (const char*)xlang;
	str += "_";
	str += (const char*)xcountry;

	if(xlang) xmlFree(xlang);
	if(xcountry) xmlFree(xcountry);
	
	return str;
}

string getTextFile(const char* dir)
{
	char szBuf[1024];
	sprintf(szBuf, "%s/text-config.xml", dir);
	
	xmlDocPtr doc = xmlParseFile(szBuf);
	if(!doc)
		return str_null;
	
	xmlNodePtr node = xmlDocGetRootElement(doc);

	szBuf[0] = 0;

	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(!xhIsNode(node, "text"))
			continue;

		xmlChar* x = xmlGetProp(node, (const xmlChar*)"status");
		if(x  && strcmp((const char*)x, "default") == 0)
			sprintf(szBuf, "%s/%s.txt", dir, getTextInfo(node).c_str());
		else if(x && strcmp((const char*)x, "current") == 0)
		{
			sprintf(szBuf,"%s/%s.txt",dir, getTextInfo(node).c_str());
			xmlFree(x);
			xmlFreeDoc(doc);
			return string(szBuf);
		}
	}

	xmlFreeDoc(doc);

	if(szBuf[0] == 0)
		return str_null;

	return string(szBuf);

}


static bool readText(const char* strFile);

bool LoadTextSource(const char* dir)
{
	string text_file;
	if(!dir)
		return false;

	//load 	
	text_file = getTextFile(dir);
	
	return readText(text_file.c_str());
}

////////////////////////////////////////////
	
static inline bool isNumber(int ch)
{
	return ((ch>='0' && ch<='9')
				|| ch=='X'
				|| ch=='x'
				|| (ch>='A' && ch<='F')
				|| (ch>='a' && ch<='f'));
}

static int getId(FILE* fp)
{
	int i = 0;
	int ch;
	char szNumber[100];

	while(!feof(fp) && isNumber((ch=fgetc(fp))))
	{
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;

	return strtol(szNumber,NULL, 0);
}

static inline bool isSpace(int ch)
{
	return (ch==' ' || ch=='\r' || ch=='\n' || ch=='\0');
}

static inline int getOct(FILE *fp)
{
	char szNumber[100];
	int i = 0;
	int ch;
	while(!feof(fp))
	{
		ch = fgetc(fp);
		if(!(ch>='0' && ch<='7'))
			break;
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;
	ungetc(ch,fp);
	return strtol(szNumber,NULL, 8);
}

static inline int getHex(FILE* fp)
{
	char szNumber[100];
	int i = 0;
	int ch;
	while(!feof(fp) && isNumber((ch=fgetc(fp))))
	{
		szNumber[i++] = ch;
	}
	szNumber[i] = 0;
	ungetc(ch,fp);
	return strtol(szNumber,NULL, 16);
}

static string getString(FILE* fp)
{
	//skip space
	char szBuff[1024];
	int idx = 0;
	int ch = 0;
	int chend = 0;
	string str;
	while(!feof(fp) && isSpace((ch=fgetc(fp))));

	if(ch == '\"') //end by "
	{
		chend = ch;
		ch = fgetc(fp);
	}

	do{
		if(ch == '\\') //
		{
			ch = fgetc(fp);
			if(ch == '\r' || ch == '\n') //Mutli line changed
			{
				ch = fgetc(fp);
				if(chend && ch == chend)
					break;
			}
			else if(ch == 'n')
				ch = 10;
			else if(ch == 'r')
				ch = 13;
			else if(ch == 't')
				ch = 9;//tab
			else if(ch == 'b') //back
			{
				idx --;
				ch = fgetc(fp);
				continue;
			}
			else if(ch=='0') //translate number
			{
				ch = fgetc(fp);
				if(ch == 'X' || ch == 'x') //hex
				{
					ch = getHex(fp);
				}
				else
				{
					ungetc(ch,fp);
					ch = getOct(fp);
				}
			}
		}
		else
		{
			if((chend && ch == chend) || ( !chend && (ch == '\r' || ch == '\n'))) //end
				break;
		}

		szBuff[idx ++] = ch;

		if(idx >= (int)sizeof(szBuff))
		{
			szBuff[idx] = '\0';
			str += szBuff;
			idx = 0;
		}

		if(feof(fp))
			break;

		ch = fgetc(fp);

	}while(1);

	if(chend)
	{
		//To Line end
		while(!feof(fp))
		{
			ch = fgetc(fp);
			if(ch == '\r' || ch == '\n')
				break;
		}
	}

	szBuff[idx] = '\0';
	str += szBuff;
	return str;
}

static void nextLine(FILE *fp)
{
	int ch;
	if(fp == NULL)
		return;

	do{
		ch = fgetc(fp);
	}while(!feof(fp) && ch!='\r' && ch!='\n');
}

static bool readText(const char* strFile)
{
	int id = 0;

	if(strFile == NULL)
		return false;

	FILE *fp = fopen(strFile, "rt");
	if(fp == NULL)
		return false;

	while(!feof(fp))
	{
		int ch = fgetc(fp);
		if(ch == '#') //comment
		{
			nextLine(fp);
			continue;
		}
		ungetc(ch,fp);

		id = getId(fp);
		if(id <= 0 || ((id&0xFFFF0000)>>16) != 0x0002)
		{
			nextLine(fp);
			continue;
		}

		texts[id] = getString(fp);
	}

	return true;
}


//////////////////////////////////////////////
bool TranslateProject(const char* prj_dir, PProcessTransedObj process_transedObj, void *param)
{
	char szbuf[1024];
	if(prj_dir == NULL)
		return false;

	sprintf(szbuf,"%s/res/text",prj_dir);
	LoadTextSource(szbuf);

	//load every 
	sprintf(szbuf,"%s/%s", prj_dir, "res/ui/id.xml");
	
	xmlDocPtr doc = xmlParseFile(szbuf);
	if(!doc)
		return false;

	xmlNodePtr node = xmlDocGetRootElement(doc);
	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(xhIsNode(node, "res-id"))
		{
			char* source = (char*)xhGetChildText(node, "source");
			if(source && strstr(source, ".xml"))
			{
				sprintf(szbuf, "%s/%s",prj_dir, source);
				TransedObject* tobj = TranslateFile(szbuf);
				process_transedObj(tobj,source,param);
				xmlFree(source);
			}
		}
	}

	xmlFreeDoc(doc);
	
	return true;
}

static void dump_transed_obj(TransedObject* tobj, FILE* fp, char* prefix)
{
	if(!tobj)
		return ;
	if(fp == NULL)
		fp = stdout;

	//printf name
	fprintf(fp,"%sName: %s\n", prefix?prefix:"", tobj->getName().c_str());
	//printf props
	map<string,Variable*> & props = tobj->getProps();
	for(map<string, Variable*>::iterator it = props.begin();
		it != props.end(); ++it)
	{
		Variable * v = it->second;
		fprintf(fp,"%s\t%s : %s\n", prefix?prefix:"",it->first.c_str(), v->toString().c_str());
	}

	//printf children
	TransedObject* child = tobj->getChildren();
	if(child)
	{
		if(prefix)
			strcat(prefix, "\t");
		for(; child; child = child->getNext())
		{
			dump_transed_obj(child, fp, prefix);
		}
		if(prefix)
		{
			int len = strlen(prefix);
			if(len > 0)
				prefix[len-1] = 0;
		}
	}
}

void DumpTransedObj(TransedObject* tobj, const char* source_file, FILE*  fp)
{
	char szPrefix[100] = "\0";
	dump_transed_obj(tobj, fp, szPrefix);
}


