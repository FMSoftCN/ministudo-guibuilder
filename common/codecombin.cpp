
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codecombin.h"

static int fgetnstr(char * str, int n, FILE *fp)
{
	int i;
	for(i=0; i<n && !feof(fp); i++){
	   str[i] = fgetc(fp);
	   if(str[i] == -1)
		   break;
	   if(str[i] == '\n' || str[i] == '\r'){
		   ungetc(str[i],fp);
		   break;
	   }
	}
	str[i] = 0;
	return i;
}

static int fungetnstr(const char *str, int n, FILE *fp)
{
	if(n<=0)
		n = strlen(str);

    while(--n >= 0)
    {
        ungetc(str[n], fp);
    }
    return n;
}

static int fgetword(char *str, const char *spliter, FILE *fp,int nMax)
{
	int i;
	if(spliter == NULL)
		spliter = " \t\r\n,;";

	//skip space
	char ch = 0;
	while(!feof(fp)){
		ch = fgetc(fp);
		if(!(ch==' ' || ch=='\t'))
			break;
	}
	ungetc(ch, fp);
	for(i=0; i<nMax && !feof(fp); i++)
	{
		str[i] = fgetc(fp);
		if(str[i] == -1)
			break;

		if(str[i] == '\n' || str[i] == '\r'){
		  ungetc(str[i],fp);
		  break;
	   }

		int j;
		for(j=0; spliter[j] && spliter[j] != str[i]; j++);
		if(spliter[j]) //exit
			break;
	}

	str[i] = '\0';
	return i;
}

////////////////////////////////////////////

Element::~Element()
{
}
///////////////////////////////////////////

TextElement::TextElement(const TextElement& te)
:Element(ET_TEXT)
{
	text = NULL;

	if(te.text)
		text = strdup(te.text);
}

TextElement::~TextElement()
{
    free(text);
}

bool TextElement::loadFromFile(FILE *fp)
{
	char szText[1024*4];
	char sztmp[16];
	int len = 0;
	int max = sizeof(szText) - 6;
	int stat = 0; //0 -- normal 1 -- at newline
	if(fp == NULL)
		return false;

	while(!feof(fp))
	{
		if(stat == 1) // try test is //<[[ or //]]>
		{
			int n = fgetnstr(sztmp, 5, fp);
			if(n<5 || (strcmp(sztmp, "//<[[") != 0
				&& strcmp(sztmp, "//]]>") != 0))
			{
				stat = 0;
				strcpy(szText + len, sztmp);
				len += n;
			}
			else{
				//find end, exit
				fungetnstr(sztmp, n, fp);
				break;
			}
		}
		else
		{
			stat = 0;
			szText[len] = fgetc(fp);
			if(szText[len] == '\r' || szText[len] == '\n')
				stat = 1;
			len ++;
		}
		if(len >= max){
			szText[len] = '\0';
			appendText(szText);
			len = 0;
		}
	}

	if(len>0)
	{
		szText[len] = '\0';
		appendText(szText);
	}

	return true;
}

void TextElement::insertText(const char* str, int at)
{
	if(str == NULL || *str == '\0')
		return ;

	if(text == NULL)
		text = strdup(str);
	else{
		int lenold = strlen(text);
		int strl = strlen(str);
		int len = lenold + strl + 1;
		char* newtext = (char*)malloc(len);
		//find place
		if(at < 0)
			at = lenold;
		else if(at > lenold)
			at = lenold;

		if(at >0)
			strncpy(newtext, text, at);
		strcpy(newtext+at, str);

		if(at < lenold)
			strcpy(newtext+at+strl, text + at);

		newtext[len-1] = '\0';
		free(text);
		text = newtext;
	}
}

void TextElement::setText(const char* str)
{
	if(str == NULL)
		return;

    free(text);
	text = strdup(str);
}

#if DEBUG
void  TextElement::print(FILE *fp)
{
	fprintf(fp, "Text Element: \n -----------------------\n");
	if(text)
		fprintf(fp, "%s", text);
	fprintf(fp, "--------------------------------\n");
}

#endif

bool TextElement::saveToFile(FILE *fp)
{
	if(text)
		fprintf(fp, "%s", text);
	return true;
}


/////////////////////////////////////////////

CompoundElement::CompoundElement(const CompoundElement& ce)
:Element(ce.type)
{
	name = NULL;
	children = NULL;
	if(ce.name)
		name = strdup(ce.name);

	Element * e = ce.children;
	Element * mye = NULL;
	while(e)
	{
		if(mye == NULL){
			children = e->clone();
			mye = children;
		}
		else{
			mye->next = e->clone();
			mye = mye->next;
		}
		//printf("mye=%p(%p)\n", mye,this);
		e = e->next;
	}
	mye->next = NULL;
}

CompoundElement::~CompoundElement()
{
    free(name);

	while(children){
		Element * e = children;
		children = children->next;
		delete e;
	}
}

bool CompoundElement::loadFromFile(FILE *fp)
{
	char szWord[256];
	Element * e = NULL;
	if(fp == NULL)
		return false;

    free(name);
	name = NULL;
	//try to load id

	if(!feof(fp)){
		int n = fgetword(szWord, "= \t\n\r", fp, sizeof(szWord)-1);
		if(strcmp(szWord, "id") == 0)
		{
			//find name
			char end;
			while(!feof(fp)){
				end = fgetc(fp);
				if(end == '\"' || end=='\'')
					break;
			}
			unsigned int idx = 0;
			while(!feof(fp) && idx < sizeof(szWord)-1)
			{
				szWord[idx] = fgetc(fp);
				if(szWord[idx] == end)
					break;
				idx ++;
			}
		   // printf("idx=%d, szWord=%s\n",idx, szWord);
			if(idx>0){
				szWord[idx] = '\0';
				name = strdup(szWord);
				//printf("----name=%p, name=%s(this=%p\n", name,name,this);
			}
		}
		else{
			if(n > 0)
				fungetnstr(szWord, n, fp);
		}
	}

	while(!feof(fp)){
		int type;
		int n;
		n = fgetnstr(szWord, 5, fp);
		if(n == 5 && strcmp(szWord, "//]]>") == 0) // get end of this element
		{
			return true;
		}
		else if(n == 5 && strcmp(szWord, "//<[[") == 0){
			//get type
			n = fgetword(szWord, NULL, fp, sizeof(szWord)-1);

			if(n<=0)
				return true;

			if(strcmp(szWord, "AutoGenCode") == 0)
				type = ET_AUTO;
			else if(strcmp(szWord, "UserCode") == 0)
				type = ET_USER;
			else{
				fprintf(stderr,"unkown word:%s\n", szWord);
				return false;
			}
		}
		else
		{
			type = ET_TEXT;
			fungetnstr(szWord, n, fp);
		}


		switch(type){
		case ET_TEXT:
			e = new TextElement;
			break;
		case ET_AUTO:
		case ET_USER:
			e = new CompoundElement(type);
			break;
		}

		//printf("e=%p, type=%d\n",e, type);
		if(e){
			if(e->loadFromFile(fp))
				 appendChild(e);
		}
		e = NULL;
	}

	return true;
}

Element* CompoundElement::getChild(int type, const char* childname)
{
	Element * e = children;

	//find by type
	//printf("name =%s\n",childname);
	while(e && type != -1 && type != e->getType())
		e = e->next;

	if(e == NULL)
		return NULL;

	//by name
	while(e && childname){
		if(e->getType() == ET_AUTO ||
			e->getType() == ET_USER ||
			e->getType() == ET_DOC)
		{
			CompoundElement * ce = (CompoundElement*)e;
			if(ce->getName() && strcmp(ce->getName(), childname)==0)
				return e;
		}

		e = e->next;
		while(e && type!=-1 && type != e->getType())
			e = e->next;
	}

	return e;
}

void CompoundElement::appendChild(Element * e)
{
	if(e == NULL)
		return;

	if(children == NULL){
		children = e;
		e->next = NULL;
		return ;
	}

	Element* eprev = children;

	while(eprev->next)
		eprev = eprev->next;

	eprev->next = e;
	e->next = NULL;
}

CompoundElement * CompoundElement::LoadFromFile(const char* file)
{
	FILE *fp;
	if(file == NULL)
		return NULL;
	fp = fopen(file, "rt");
	if(fp == NULL)
		return NULL;

	//new doc compound
	CompoundElement * pce = new CompoundElement(ET_DOC);

	if(!pce->loadFromFile(fp)){
		delete pce;
		pce = NULL;
	}

	fclose(fp);
	return pce;
}

#define print_list(header)  do{ Element* _e=header; printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"); \
	while(_e){ printf("%p,",_e); _e = _e->next; } printf("\n");}while(0)
Element* CompoundElement::insertChild(Element *e, Element *at, bool before)
{
	if(e == NULL)
		return NULL;

	//print_list(children);
	//printf("insert e=%p\n",e);
	if(children == NULL || at == NULL) //insert  last
	{
		appendChild(e);
		return e;
	}

	Element * afe = children;
	if(before){
		if(at == children)
			afe = NULL;
		else{
			while(afe->next && afe->next != at)
				afe = afe->next;
		}
	}
	else{
		while(afe != at && afe->next)
			afe = afe->next;
	}

	//printf("insert At=%p\n", afe);

	if(afe == NULL){
		e->next = children;
		children = e;
	}
	else
	{
		e->next = afe->next;
		afe->next = e;
	}
	//print_list(children);

	return e;
}

#if DEBUG
void CompoundElement::print(FILE *fp)
{
	if(type == ET_DOC)
		fprintf(fp,"Doc Element");
	else if(type == ET_AUTO)
		fprintf(fp, "AutoGenCode Element");
	else if(type == ET_USER)
		fprintf(fp, "UserCode Element");
	else{
		fprintf(fp, ">>> Error: Unkown Element\n");
		return;
	}

	//printf("----name=%p, (this=%p)\n", name,this);
	if(name){
		fprintf(fp, " id=\"%s\"", name);
	}

	fprintf(fp, "\n----------------------------\n");
	Element * e = children;
	while(e){
		e->print(fp);
		e = e->next;
	}
	fprintf(fp, "-------------------------------\n");
}
#endif

bool CompoundElement::combin(CompoundElement *ce)
{
	Element * elast = NULL;
	if(ce == NULL)
		return false;

	//only process autogen code which have a name
	Element * e = ce->getChild(ET_AUTO, NULL);

	while(e)
	{
		if(e->getType()==ET_AUTO && ((CompoundElement*)e)->getName()){
			AutoGenElement * te = (AutoGenElement*)getChild(ET_AUTO, ((CompoundElement*)e)->getName());
			if(te)
			{
				te->replaceAutoGenPart((AutoGenElement*)e);
				elast = te;
			}
			else{
				elast =  insertChild(e->clone(), elast);
			}
		}
		//FIX BUG 3646, when insert a new Element , stride over the non-AutoGen-Element ...
		while(elast && elast->next && elast->next->getType() != ET_AUTO)
			elast = elast->next;

		e = e->next;
	}

	e = children;
	while(e)
	{
		if(e->getType() == ET_AUTO && ((CompoundElement*)e)->getName())
		{
			if(!ce->getChild(ET_AUTO, ((CompoundElement*)e)->getName()))
			{
				AutoGenElement * te = (AutoGenElement*)e;
				te->disableCode();
			}
		}
		e = e->next;
	}

	return true;

}

bool CompoundElement::saveToFile(FILE *fp)
{
	//fp = stdout;
	switch(type)
	{
	case ET_AUTO:
		fprintf(fp, "//<[[AutoGenCode ");
		break;
	case ET_USER:
		fprintf(fp, "//<[[UserCode ");
		break;
	}

	//printf("name=%p (this=%p)\n",name, this);
	//printf("name=%s\n, name");
	if(name)
		fprintf(fp, "id=\"%s\" ", name);

	//printf("enter:%p.........................(%p)\n", this, children);
	//print children
	Element * e = children;
	while(e){
		e->saveToFile(fp);
		e = e->next;
	}
	//printf("leave:%p....................................\n", this);

	if(type == ET_AUTO || type == ET_USER)
		fprintf(fp, "//]]>");
	return true;
}

/////////////////////////////////////////////////////////
//AutoGenElement

AutoGenElement::~AutoGenElement()
{

}

bool AutoGenElement::disableCode()
{
	TextElement * te ;
	if(children && children->getType() == ET_TEXT){
		te = (TextElement * ) children;
	}
	else{
		te = new TextElement;
		te->next = children;
		children = te;
	}

	te->insertText("\n#if 0 //",0);

	//find last one
	Element* e = children;
	while(e->next) e = e->next;

	if(e && e->getType() == ET_TEXT)
		te = (TextElement*) e;
	else{
		te = new TextElement;
		e->next = te;
		te->next = NULL;
	}

	te->appendText("\n#endif\n");

	return true;
}

bool AutoGenElement::replaceAutoGenPart(AutoGenElement *page)
{
	if(page == NULL)
		return false;

	//test the age is same
	if(page->getName() == name
		|| (page->getName() && name && strcmp(name, page->getName()) == 0))
	{
		//replace all the text element
		Element * mye = children;
		Element * e = page->getChild(ET_TEXT, NULL);

		while(mye && e){
			while(mye && mye->getType() != ET_TEXT)
				mye = mye->next;
			if(mye == NULL)
				break;
			((TextElement*)mye)->setText(((TextElement*)e)->getText());

			mye = mye->next;
			e = e->next;
			while(e && e->getType() != ET_TEXT)
				e = e->next;
		}
	}


	return true;
}




////////////////////////////////////////////////////////
//CodeCombinFiles

int CodeCombinFiles(const char * fileFrom, const char* fileTo)
{
	int ret = 0;
	FILE *fp = NULL;
	CompoundElement * ceFrom = NULL;
	CompoundElement * ceTo = NULL;


	if(fileFrom == NULL || fileTo == NULL)
		return 0;

	ceFrom = CompoundElement::LoadFromFile(fileFrom);
	ceTo = CompoundElement::LoadFromFile(fileTo);

	if(ceFrom == NULL )
		goto FAILED;

	if(ceTo != NULL)
	{
		ceTo->combin(ceFrom);
		//ceTo->print();
	}

	fp = fopen(fileTo, "wt");
	if(ceTo)
		ceTo->saveToFile(fp);
	else
		ceFrom->saveToFile(fp);
	fflush(fp);
	fclose(fp);

	ret = 1;

FAILED:
    delete ceFrom;
    delete ceTo;

	return ret;
}


