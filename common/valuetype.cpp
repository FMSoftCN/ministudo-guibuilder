/*
 * valuetype.cpp
 *
 *  Created on: 2009-3-24
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//#include <stdint.h>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include <mgutils/mgutils.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mapex.h"
using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"

#include "dlgtmpls.h"

#include "valuetype.h"
#include "class-instance.h"
#include "../uieditor/font-dialog.h"

const char* strNull = "";

static map<string, int> strPool;
static string stringNull = string("");

const string& getStringFromPool(const char* str){
	if(str == NULL || str == strNull)
		return stringNull;

	map<string, int>::iterator it = strPool.find(str);
	if(it == strPool.end())
	{
		string s = str;
		strPool[s] = 1;
		return strPool.find(s)->first;
	}

	return it->first;
}

const char* fromStringPool(const char* str)
{
	if(str == NULL || str == strNull)
		return str;

	map<string, int>::iterator it = strPool.find(str);
	if(it == strPool.end())
	{
		string s = str;
		strPool[s] = 1;
		//return s.c_str();
		it = strPool.find(str);
	}

	return it->first.c_str();
}

////////////////////////////////////////////////////////
// get Value instance

ValueType* ValueType::loadCompoundTypeFromXMLNode(xmlNodePtr node)
{
	if(xhIsNode(node,"enum"))
		return new EnumValueType(node);

	if(xhIsNode(node,"struct"))
		return new StructValueType(node);

	if(xhIsNode(node,"file"))
	{
		xmlChar* xname = xmlGetProp(node, (const xmlChar*)"name");
		if(xname == NULL)
			return NULL;
		xmlChar* xfilter = xmlGetProp(node, (const xmlChar*)"filter");
		ValueType *vtype = new FileValueType((const char*)xfilter, (const char*)xname);
		xmlFree(xname);
		if(xfilter)
			xmlFree(xfilter);
		return vtype;
	}

	return NULL;
}

BOOL ValueType::loadCompoundTypes(const char* xmlFile)
{
	if(xmlFile == NULL)
		return FALSE;

	xmlDocPtr doc = xmlParseFile(xmlFile);
	if(doc == NULL)
		return FALSE;

	xmlNodePtr node = xmlDocGetRootElement(doc);

	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;

		if(!loadCompoundTypeFromXMLNode(node))
		{
			//TODO
			LOG_WARNING("Load Node(\"%s\") Field", node->name);
		}

	}

	xmlFreeDoc(doc);

	return TRUE;
}



#define GET_BASE_TYPE(type_name, _class) \
	if(strcasecmp(type_name,type) == 0) return _class::getInstance()

#define GET_COMP_TYPE(type_name, _class) \
	if(strcasecmp(type_name, type) == 0) return _class::getInstance(name)

ValueType* ValueType::getValueType(const char* type_name)
{
	BOOL bcontructor = FALSE;
	if(type_name == NULL)
		return NULL;

	char type[32] = "\0";
	char name[32] = "\0";
	{
		const char* str = strchr(type_name,' ');
		if(str){
			strncpy(type, type_name, str - type_name);
			str += 1;
			while(*str == ' ' && *str) str++;
			strcpy(name, str);
		}
		else if((str = strchr(type_name, ':')) != NULL){ //constructor
			strncpy(type, type_name, str - type_name);
			strcpy(name, str+1);
			bcontructor = TRUE;
		}
		else{
			strcpy(type, type_name);
		}
	}

	GET_BASE_TYPE("int", IntValueType);
	GET_BASE_TYPE("char", CharValueType);
	GET_BASE_TYPE("string", StringValueType);
	GET_BASE_TYPE("color", ColorValueType);
	GET_BASE_TYPE("text", TextValueType);
	GET_BASE_TYPE("font", FontValueType);
	GET_BASE_TYPE("image", ImageValueType);
	GET_BASE_TYPE("renderer", RdrValueType);
	GET_BASE_TYPE("group", GroupValueType);
	//file
	if(strcmp(type, "file") == 0){
		if(bcontructor)
			return new FileValueType(name);
		else
			return FileValueType::getInstance(name);
	}

	//compiste type
	if(*name == 0)
		return NULL;
	GET_COMP_TYPE("enum", EnumValueType);
	GET_COMP_TYPE("struct", StructValueType);
	//GET_COMP_TYPE("array", ArrayValueType);
	//GET_COMP_TYPE("tree", TreeValueType);
	GET_COMP_TYPE("event", EventValueType);

	return NULL;
}

////////////////////////////////////////////////////////

//WNDPROC ValueEditor::_oldProc = NULL;

///////////////////////////
//File
void FileValueEditor::setValue(const char* text)
{
	bModified = FALSE;
	if(text == NULL)
		return;

	if(!updatingValue((Value)text))
		return;

	vtype->releaseValue(value);
	value = vtype->newValue(text);
	lastPath = text;
	updateValue();
}
void FileValueEditor::onExtend()
{
	FILEDLGDATA pfdd = {0};
	FileValueType* fvt = (FileValueType*)vtype;
	strcpy(pfdd.filepath, lastPath.c_str());
	strcpy(pfdd.filter, fvt->getFilter());
	pfdd.is_save = FALSE;
	if(FileOpenSaveDialog(GetDlgTemplate(ID_FONTSELECT), hwnd, NULL, &pfdd))
	{
		setValue(pfdd.filefullname);
		SetWindowText(hwnd, pfdd.filefullname);
		bModified = FALSE;
	}
}

mapex<string, FileValueType*,mystr_less> FileValueType::_instances;

///////////////////////////////////////////////////////////////////
//font

void FontValueEditor::onExtend()
{
	FONTDATA pfdd = {0};
	pfdd.min_size = 0;
	pfdd.max_size = 72;

	if (value)
		strcpy(pfdd.font_name, (const char *)value);

	if (TRUE == FontSelDlg (NULL, hwnd, NULL, &pfdd))
	{
		if (value && strcmp(pfdd.font_name, (const char *)value) == 0)
			return;
		setValue(pfdd.font_name);
		SetWindowText(hwnd, pfdd.font_name);
	}
	return;
}

void FontValueEditor::setValue(const char *font_name)
{
	bModified = FALSE;
	if(font_name == NULL)
		return;

	if(!updatingValue((Value)font_name))
		return;

	vtype->releaseValue(value);
	value = vtype->newValue(font_name);
	updateValue();
}


///////////////////////////
//color
int ColorValueEditor::wndProc(int message, WPARAM wParam, LPARAM lParam)
{
	if(message == MSG_PAINT){
		RECT rt;
		calcaRect(&rt,TRUE);
		HDC hdc = BeginPaint(hwnd);
		gal_pixel pixel = RGBA2Pixel(hdc, GetRValue(value),GetGValue(value), GetBValue(value),GetAValue(value));
		SetBrushColor(hdc, pixel);
		FillBox(hdc, rt.left+1, rt.top+1, RECTW(rt)-2, RECTH(rt)-2);
		EndPaint(hwnd, hdc);
		::InvalidateRect(::GetDlgItem(hwnd,EXTEND_ID), NULL, TRUE);
		return 0;
	}
	else if(message == MSG_GETTEXT)
	{
		char * str = (char*)lParam;
		int len = wParam;
		return snprintf(str,len,"0x%08X", (unsigned int)value);
	}

	return ExtendEditor::wndProc(message, wParam, lParam);
}

void ColorValueEditor::onExtend()
{
	//show color dialog
	COLORDLGDATA pcdd = {0, 0, 0, 0, 0, 0, 0, FALSE, NULL};
	memset (&pcdd, 0, sizeof(COLORDLGDATA));
	pcdd.b = GetBValue(value);
	pcdd.g = GetGValue(value);
	pcdd.r = GetRValue(value);
	if (ColorSelectDialog(NULL, ::GetMainWindowHandle(hwnd), NULL, &pcdd))
	{
		//set value
		value = MakeRGBA(pcdd.r, pcdd.g,pcdd.b,0xFF);
		updateValue();
	}
}

///////////////////////////////////////////////////////////////////////

Value TextValueType::newValue(const char* str)
{
	int id;
	if(str == NULL)
		return 0;

	id = strtol(str,NULL, 0);

	if(id < 0 || ID2TYPE(id)!=type) //create a new res
	{
		ResManager *resMgr = getResManager();
		if(resMgr)
		{
			id = resMgr->createRes(type, NULL, -1, str, 0);
		}
	}
	return (Value)id;
}

void MutliStringEditor::createEditor(int id, const RECT* rt)
{
	CreateWindow("sledit", "", ES_LEFT|WS_VISIBLE, id,
			rt->left, rt->top, RECTWP(rt), RECTHP(rt), hwnd, 0);
}

//show a text editor
#define IDC_MLTEXT 5
static CTRLDATA _multi_text_ctrls [] = {
		{
			CTRL_MLEDIT, WS_VSCROLL|/*WS_HSCROLL|*/WS_VISIBLE|ES_AUTOWRAP|WS_BORDER,
			10, 10, 300, 150,
			IDC_MLTEXT,
			"",
			0,
            WS_EX_NONE,
            NULL,
            NULL
		},
		{
			"button", WS_VISIBLE|BS_PUSHBUTTON,
			140, 170, 80, 30,
			IDOK,
			"OK",
			0,
            WS_EX_NONE,
            NULL,
            NULL
		},
		{
			"button", WS_VISIBLE|BS_PUSHBUTTON,
			230, 170, 80, 30,
			IDCANCEL,
			"Cancel",
			0,
            WS_EX_NONE,
            NULL,
            NULL
		}
};
static DLGTEMPLATE _multi_text_editor_dlg = {
		WS_BORDER|WS_CAPTION|WS_DLGFRAME|WS_VISIBLE,
		WS_EX_NONE,
		200,150, 330,230,
		"Input Text",
		0, 0,
		sizeof(_multi_text_ctrls)/sizeof(CTRLDATA),
		_multi_text_ctrls,
        0
};

int MutliStringEditor::_multiTextEditProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	MutliStringEditor *te;

	switch(message)
	{
	case MSG_INITDIALOG:
		SetWindowAdditionalData(hDlg, lParam);
		te = (TextResEditor*)GetWindowAdditionalData(hDlg);
		SetFocus(GetDlgItem(hDlg,IDC_MLTEXT));
		if(te){
			char szText[1024*8];
			if(GetWindowText(te->hwnd, szText, sizeof(szText)))
			{
				HWND hEdit = GetDlgItem(hDlg,IDC_MLTEXT );
				SetWindowText(hEdit, szText);
				SendMessage(hEdit, EM_SELECTALL, 0, 0);
			}
		}
		return 1;

	case MSG_COMMAND:
		if(LOWORD(wParam) == IDOK){
			//save value
			HWND hedit;
			int textlen;
			char *text;
			te = (MutliStringEditor*)GetWindowAdditionalData(hDlg);
			hedit = GetDlgItem(hDlg, IDC_MLTEXT);
			textlen = GetWindowTextLength(hedit);
			if(textlen <= 0){ //NULL text
				te->setValue(strNull);
			}
			else{
				text = new char[textlen+1];
				GetWindowText(hedit, text, textlen+1);
				te->setValue(text);
				delete[] text;
			}
			EndDialog(hDlg,1);
		}
		else if(LOWORD(wParam)== IDCANCEL){
			EndDialog(hDlg, 0);
		}
		return 0;
	}

	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

void MutliStringEditor::onExtend()
{
	DialogBoxIndirectParam(&_multi_text_editor_dlg, GetMainWindowHandle(hwnd),_multiTextEditProc, (LPARAM)this);
}

void TextResEditor::setValue(const char* text)
{
	TextValueType *txtvt = (TextValueType*)vtype;
	if(!updatingValue((DWORD)value))
		return;
	SetDlgItemText(hwnd, EDITOR_ID, text);
	value = txtvt->saveToRes(value,(DWORD)text);
	updateValue();
}

//////////////////////////////////////////
void RefResEditor::onExtend()
{
	//TODO open other resource editor
	//get a new value id

	int id = g_env->selectID(getResType());
	if(VALIDID(id,getResType()))
	{
		if(!setValue(id))
			return;

		//get and set name into editor
		ResManager* resMgr = getResManager();
		if(resMgr)
		{
			const char* strName = resMgr->idToName(id);
			if(strName){
				SetWindowText(hwnd, strName);
				return;
			}
		}
		char szText[100];
		sprintf(szText,"%d",id);
		SetWindowText(hwnd, szText);
	}
}

//////////////////////////////////////////
template<class TValue, class TEditor, class TSelf, int VType>
void TCompositeValueType<TValue, TEditor, TSelf, VType>::cleanInstances()
{
	typedef typename mapex<string, TSelf*, mystr_less>::iterator  Iterator;
	Iterator it;
	for(it = _instances.begin();
			it != _instances.end(); ++it)
	{
		if(it->second)
			it->second->release();
	}
	_instances.clear();

}

/////////////////////////////////////////
//EnumValueType
EnumValueType::EnumValueType(xmlNodePtr node)
{
	optionValueType = NULL;

	/*
	* <enum name="" option-type="">
	*       <option name="" value="">caption</option>
	*       ....
	* </enum>
	* e.g.
	* <enum name="Align" option-type="integer">
	*   <option name="Left" value="0">Left</option>
	*   <option name="Center" value="1">Center</option>
	*   <option name="Bottom" value="2">Bottom</option>
	* </enum>
	*  or for short:
	* <enum name="Align">
	*   <option name="Left"/>
	*   <option name="Center"/>
	*   <option name="Bottom"/>
	* </enum>
	*/
	if(node)
	{
		//get name
		xmlChar* xstr = xmlGetProp(node, (const xmlChar*)"name");
		if(xstr){
			name = (const char*) xstr;
			xmlFree(xstr);
		}

		DPRINT("Enum (%p) name=%s", this, name.c_str());
		//get value type
		xstr = xmlGetProp(node, (const xmlChar*)"option-type");
		if(xstr){
			optionValueType = ValueType::getValueType((const char*)xstr);
			xmlFree(xstr);
		}
		else{
			optionValueType = ValueType::getValueType("int");
		}

		int idx = 0;
		//load options
		for(xmlNodePtr child = node->xmlChildrenNode; child; child=child->next){
			if(!xhIsNode(child,"option"))
				continue;

			Option *option = new Option;
			//get name
			xstr = xmlGetProp(child,(const xmlChar*)"name");
			if(xstr){
				option->name = fromStringPool((const char*)xstr);
				xmlFree(xstr);
			}

			//get value
			xstr = xmlGetProp(child, (const xmlChar*)"value");
			if(xstr){
				option->value = optionValueType->newValue((const char*)xstr);
				xmlFree(xstr);
			}
			else
				option->value = idx;

			//get caption
			xstr = xhGetNodeText(child);
			if(xstr){
				option->caption = fromStringPool((const char*)xstr);
				xmlFree(xstr);
			}
			else{
				option->caption = option->name;
			}

			DPRINT("option name=%s,value=0x%08X", option->name, (unsigned int)option->value);

			//push into list
			options.push_back(option);

			idx ++;

		}
	}
	//try to insert
	insertInstance(this);
}

EnumValueType::~EnumValueType()
{
	list<Option*>::iterator it;
	for(it = options.begin(); it != options.end(); ++it)
	{
		if(optionValueType)
			optionValueType->releaseValue((*it)->value);
		delete *it;
	}
}


EnumValueType::Option * EnumValueType::getOptionByValue(Value value)
{
	list<Option*>::iterator it;

	for(it = options.begin();	it != options.end() ; ++it)
	{
		Option* option = *it;
		if(equal(value, option->value))
			return *it;
	}

	return NULL;
}

EnumValueType::Option * EnumValueType::getOptionByName(const char* strName)
{
	list<Option*>::iterator it;

	if(strName == NULL)
		return NULL;

	for(it = options.begin();it != options.end(); ++it)
	{
		if(strcasecmp((*it)->name, strName) == 0)
			return *it;
	}

	return NULL;
}

Value EnumValueType::getOptionValue(const char* name){

	Option* option = getOptionByName(name);
	if(option)
		return option->value;

	return (Value)-1;
}


DWORD EnumValueType::toMemo(Value value){
	return value;
}

Value EnumValueType::fromMemo(DWORD memo){
	return memo;
}

Value EnumValueType::newValue(xmlNodePtr node)
{
	Value value = (Value)-1;
	//get value
	xmlChar* xstr = xhGetNodeText(node);
	if(xstr){
		value = newValue((const char*)xstr);
		xmlFree(xstr);
	}

	return value;
}

Value EnumValueType::newValue(const char* str)
{
	Value value = getOptionValue(str);
	if(optionValueType)
		return optionValueType->newValue(value);
	return value;
}

mapex<string, EnumValueType*,mystr_less > EnumValueType::_instances;

// EnunValueEditor
void EnumValueEditor::onNotify(int id, int nc, DWORD add_data)
{
	if(nc == CBN_SELCHANGE){
		bModified = TRUE;
		onSaveValue();
	}
}

void EnumValueEditor::onSaveValue()
{
	if(!bModified)
		return;

	bModified = FALSE;

	int cur_sel = SendMessage(hwnd,CB_GETCURSEL, 0 , 0);
	if(cur_sel < 0)
		return ;
	EnumValueType::Option * option = (EnumValueType::Option *)SendMessage(hwnd, CB_GETITEMADDDATA, cur_sel, 0);
	if(option)
	{
		if(!updatingValue(option->value))
			return;

		//free old
		vtype->releaseValue(value);
		value = vtype->newValue(option->value);
		updateValue();
	}
}

BOOL EnumValueEditor::initEditor()
{
	//int sel_idx = -1;
	EnumValueType *evt = (EnumValueType*)vtype;
	list<EnumValueType::Option*>::iterator it;

	SendMessage (hwnd, CB_RESETCONTENT, 0, 0);

	if(evt){
		for(it = evt->options.begin();	it != evt->options.end(); ++it)
		{
			EnumValueType::Option* option = *it;
			//DP(0x20, "--- option caption=%s", option->caption);
			int idx = SendMessage (hwnd, CB_ADDSTRING, 0,
					(LPARAM)(option->caption?option->caption:option->name));
			SendMessage(hwnd, CB_SETITEMADDDATA, idx, (LPARAM)option);
			if(evt->equal(option->value, value)){
				//sel_idx = idx;
				SetWindowText(hwnd, option->caption?option->caption:option->name);
			}
		}
		//if(sel_idx >= 0 )
		//	SendMessage(hwnd, CB_SETCURSEL, sel_idx, 0);
	}

	return TRUE;
}

///////struct
//structvalue editor
void StructValueEditor::onExtend(){
	//show struct editor

}
////
//StructValueType
/*
 <struct name="">
	   <element name="" type="">[caption]</element>
	   .....
 </struct>
 */
StructValueType::StructValueType(xmlNodePtr node)
{
	if(node){
		//get name
		xmlChar* xstr = xmlGetProp(node, (const xmlChar*)"name");
		if(xstr){
			name = (const char*)xstr;
			xmlFree(xstr);
		}

		for(xmlNodePtr child=node->xmlChildrenNode; child; child = child->next)
		{
			if(!xhIsNode(child, "element"))
				continue;
			Element * element = new Element();
			xstr = xmlGetProp(child, (const xmlChar*)"name");
			if(xstr){
				element->name = fromStringPool((const char*)xstr);
				xmlFree(xstr);
			}

			xstr = xmlGetProp(child, (const xmlChar*)"type");
			if(xstr){
				element->vtype = ValueType::getValueType((const char*)xstr);
				xmlFree(xstr);
			}

			xstr = xhGetNodeText(node);
			if(xstr){
				element->caption = fromStringPool((const char*)xstr);
				xmlFree(xstr);
			}
			else
				element->caption = element->name;
			elements.push_back(element);
		}
	}

	insertInstance(this);
}

DWORD StructValueType::toMemo(Value value)
{
	if(value == 0)
		return (DWORD)-1;

	StructValue * sv = (StructValue*)value;

	if(sv== NULL || sv->values == NULL)
		return (DWORD) -1;

	DWORD *memo = new DWORD[elements.size()];

	int i = 0;
	for(list<Element*>::iterator it = elements.begin(); it != elements.end(); ++it){
		Element *e = *it;
		memo[i] = e->vtype->toMemo(sv->values[i]);
		i ++;
	}

	return (DWORD)(memo);
}

Value StructValueType::fromMemo(DWORD memo){
	if(memo == (DWORD)-1 || memo == 0)
		return 0;

	DWORD *memos = (DWORD*)memo;

	StructValue *sv = new StructValue(this);

	sv->values = new Value[elements.size()];
	sv->binValues = new DWORD[elements.size()];

	int i = 0;
	for(list<Element*>::iterator it = elements.begin(); it != elements.end(); ++it){
		Element *e = *it;
		sv->values[i] = e->vtype->fromMemo(memos[i]);
		sv->binValues[i] = e->vtype->toBinary(sv->values[i]);
		i ++;
	}

	return (DWORD)sv;
}

void StructValueType::freeMemo(DWORD memo){
	if(memo == (DWORD)-1 || memo == 0)
		return ;

	DWORD *memos = (DWORD*)memo;

	int i = 0;
	for(list<Element*>::iterator it = elements.begin(); it != elements.end(); ++it){
		Element *e = *it;
		e->vtype->freeMemo(memos[i]);
	}
	delete[] memos;
}

StructValueType::~StructValueType()
{
	for(list<Element*>::iterator it = elements.begin();
		it != elements.end(); ++it)
	{
		delete *it;
	}
}

ValueType* StructValueType::getElementType(const char* name, int *pidx){
	int idx = 0;
	if(name == NULL)
		return NULL;

	for(list<Element*>::iterator it = elements.begin();
		it != elements.end(); ++it, idx++)
	{
		Element * e = *it;
		if(strcmp(e->name, name) == 0){
			if(pidx)
				*pidx = idx;
			return e->vtype;
		}
	}
	return NULL;
}

const StructValueType::Element* StructValueType::getElement(int idx)
{
	list<Element*>::iterator it = elements.begin();
	for(int i=0;
		it != elements.end() && i < idx; ++it);
	return *it;
}

ValueType * StructValueType::getElementType(int idx){

	const Element* e = getElement(idx);
	return e?e->vtype:NULL;
}

BOOL StructValueType::compare(const StructValue & sv1, const StructValue & sv2) const
{
	if(sv1.values == sv2.values)
		return TRUE;
	if(sv1.values == NULL || sv2.values == NULL)
		return FALSE;

	int i = 0;
	for(list<Element*>::const_iterator it = elements.begin();
		it != elements.end(); ++it, i++)
	{
		if((*it)->vtype && !(*it)->vtype->equal(sv1.values[i], sv2.values[2]))
			return FALSE;
	}

	return TRUE;
}

void StructValueType::saveXMLStream(Value value, TextStream *stream)
{
	if(value == 0)
		return ;

	int i = 0;

	StructValue * sv = (StructValue*)value;

	stream->printf("\n");
	stream->indent();
	for(list<Element*>::iterator it = elements.begin(); it != elements.end(); ++it)
	{
		Element* e = *it;
		stream->printf("<%s>",e->name);
		e->vtype->saveXMLStream(sv->values[i], stream);
		stream->println("</%s>",e->name);
		i++;
	}
	stream->unindent();
}

//structvalue
StructValue::StructValue(StructValueType* type)
:TCompositeValue<StructValueType>(type)
{
	values = NULL;
	binValues = NULL;
}

StructValue::StructValue(StructValueType* type, xmlNodePtr node)
:TCompositeValue<StructValueType>(type)
{
	values = NULL;
	binValues = NULL;

	if(node){
		int count = vtype->getElementCount();
		values = new Value[count+1];
		binValues = new DWORD[count+1];
		memset(values, 0, sizeof(Value)*(count+1));
		memset(binValues, 0, sizeof(DWORD)*(count+1));
		values[0] = count;
		binValues[0] = count;

		for(xmlNodePtr child = node->xmlChildrenNode; child; child=child->next){
			if(node->type != XML_ELEMENT_NODE)
				continue;

			int idx;
			ValueType* evt = vtype->getElementType((const char*)node->name, &idx);
			if(evt){
				values[idx+1] = evt->newValue(child);
				binValues[idx+1] = evt->toBinary(values[idx+1]);
			}
		}
	}
}

StructValue::StructValue(StructValueType * type, const char* str)
:TCompositeValue<StructValueType>(type)
{
	values = NULL;
	binValues = NULL;
}

StructValue::StructValue(const StructValue& sv)
:TCompositeValue<StructValueType>(sv.vtype)
{
	int count = vtype->getElementCount();
	values = new Value[count+1];
	binValues = new DWORD[count+1];
	memset(values, 0, sizeof(Value)*(count+1));
	memset(binValues, 0, sizeof(DWORD)*(count+1));
	values[0] = count;
	binValues[0] = count;
	for(int i=0; i<count; i++){
		values[i+1] = sv.values[i+1];
		ValueType *vt = vtype->getElementType(i);
		if(vt)
		{
			vt->addValueRef(values[i+1]);
			binValues[i+1] = vt->toBinary(values[i+1]);
		}
	}
}

StructValue::~StructValue()
{
	if(values){
		for(int i=0; i<vtype->getElementCount(); i++){
			ValueType* vt = vtype->getElementType(i);
			if(vt)
				vt->releaseValue(values[i+1]);
		}
		delete[] values;
	}

    delete[] binValues;
}

string StructValue::toString(){
	char szText[100];
	sprintf(szText,"struct %s[%p]", vtype->getName(), this);
	return string(szText);
}


////////////////////
//Array


///////////////////////////////////////////////////////////////////
//
#define CREATE_COMPOSITE_VALUETYPE(node, strtype, Type) \
	if(xmlStrcmp(node->name, (const xmlChar*)(strtype)) == 0) \
		return new Type(node);
ValueType* createCompositeValueType(xmlNodePtr node)
{
	if(node == NULL)
		return NULL;

	CREATE_COMPOSITE_VALUETYPE(node, "enum", EnumValueType)

	return NULL;
}

///////////////////////////////////////
// Event Value editor
BOOL EventValueEditor::init(Value value, ValueType* vtype, ValueUpdator * updator,DWORD mask)
{
	if(this->vtype)
		this->vtype->releaseValue(this->value);

	this->value = value;
	this->updator = updator;
	this->vtype = vtype;
	this->mask = mask;
	string txt = vtype->toString(value);

	EventValueType * evtype = (EventValueType*)vtype;

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	for(list<string>::iterator it = evtype->historyNames.begin(); it != evtype->historyNames.end(); ++it)
	{
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)it->c_str());
	}

	//SendMessage(hwnd, CB_SETCURSEL,mvtype->idxOfName((const char*)value), 0);
	if(value)
		SetWindowText(hwnd, (const char*)value);

	return TRUE;
}

void EventValueEditor::onSaveValue()
{
	if(!bModified)
		return ;
	bModified = FALSE;
	//get context
	char szFuncName[256];
	if(GetWindowText(hwnd, szFuncName,sizeof(szFuncName)))
	{
		Value newvalue = vtype->newValue(szFuncName);
		if(vtype->equal(newvalue, value))
			return;
		if(!updatingValue(newvalue))
		{
			//rollback
			SetWindowText(hwnd,(const char*)value);
			return;
		}
		value = newvalue;
		updateValue();
	}
}

void EventValueEditor::insertFuncName(const char* szFuncName)
{
	if(szFuncName ==  NULL)
		return ;

	if(SendMessage(hwnd,CB_FINDSTRING, 0, (LPARAM)szFuncName) != CB_ERR){
		return;
	}

	SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szFuncName);
}

//message
//<event>
//  <handler> xxxx</handler>
//  <code>xxx </code>
//</event>
EventValueType::EventValueType(xmlNodePtr node, const char* default_prototype)
{
	if(node == NULL)
		throw("EventValueType, Node cannot be null");

	//get the handler
	for(node = node->xmlChildrenNode; node; node=node->next)
	{
		if(xhIsNode(node, "handler")){
			xmlChar* xhandler = xhGetNodeText(node);
			if(xhandler){
				parserHandler((const char*)xhandler);
				if(prototype.length() <= 0 && default_prototype)
					prototype = default_prototype;
				xmlFree(xhandler);
			}
		}
		else if(xhIsNode(node,"code")){
			xmlChar* xcode = xhGetNodeText(node);
			if(xcode)
			{
				code = (const char*)xcode;
				xmlFree(xcode);
			}
		}
		else if(xhIsNode(node, "content"))
		{
			xmlChar *xcontent = xhGetNodeText(node);
			if(xcontent){
				content = (char*)xcontent;
				xmlFree(xcontent);
			}
		}
	}
}

EventValueType::~EventValueType()
{

}

mapex<string, EventValueType*, mystr_less> EventValueType::_instances;

//check a func name is valid or not
BOOL EventValueType::checkFuncName(const char* funcName){

	if(funcName == NULL)
		return FALSE;

	if(*funcName != '_'
		&& !((*funcName >= 'A' && *funcName <= 'Z') || (*funcName >= 'a' && *funcName <= 'z')))
		return FALSE;

	funcName ++;
	while(*funcName){
		if(!( *funcName == '_'
				|| (*funcName >= 'A' && *funcName <= 'Z')
				|| (*funcName >= 'a' && *funcName <= 'z')
				|| (*funcName >='0' && *funcName <= '9')))
			return FALSE;

		funcName ++;
	}

	return TRUE;
}

void EventValueType::parserHandler(const char* handler)
{
	char szName[256];
	char szPropotype[512];

	int iproto = 0;

	//int iname = 0;

	if(handler == NULL)
		return;

	//find the first '('
	const char* str = strchr(handler, '(');
	if(str == NULL){
		name = handler;
		if(*handler)
			historyNames.push_back(name);
		return;
	}

	//find the space or head
	const char* str1;
	for(str1 = str - 1; str1 > handler && *str1!=' ' && *str1!='\t'; str1 --);

	if(str1 < handler)
		str1 = handler;
	else
		str1 ++;

	//copy the return type of fucntion property
	if(handler == str1){
		strcpy(szPropotype,"int @ ");
		iproto = strlen(szPropotype);
	}
	else {
		for(;handler < str1 && *handler!=' ' && *handler!='\t'; handler ++ )
			szPropotype[iproto++] = *handler;
		szPropotype[iproto++] = ' ';
		szPropotype[iproto++] = '@'; //function pos
		szPropotype[iproto++] = ' ';
	}


	//skip space
	while(*str1 && (*str1==' ' || *str1 == '\t')) str1 ++;

	//copy the name of function
	strncpy(szName, str1, str-str1);
	szName[str-str1] = '\0';

	//copy params info
	strcpy(szPropotype+iproto, str);

	//remove the ';' at the last
	iproto = strlen(szPropotype) - 1;

	while(szPropotype[iproto] == ';' || szPropotype[iproto]== ' ' || szPropotype[iproto] == '\t')
		iproto --;
	szPropotype[iproto+1] = '\0';

	name = szName;
	prototype = szPropotype;
	if(*szName)
	{
		//push into history
		historyNames.push_back(name);
	}
}

string EventValueType::toString(Value value)
{
	if(value == 0)
		return string(strNull);

	return string((const char*)value);
}

void EventValueType::saveXMLStream(Value value, TextStream *stream)
{
	//char szFunc[512];
	if(value == 0 || stream == NULL)
		return ;

	//save name
	/*stream->println("<message>",(const char*)value);
	stream->indent();
	stream->println("<handler>%s</handler>",name.c_str());
	stream->println("<map>%s</map>",mapper.c_str());
	stream->println("<prototype>%s</prototype>",getPropotype((const char*)value,szFunc));
	stream->unindent();
	stream->println("</message>");
*/
	stream->printf("%s",name.c_str());
}

char * EventValueType::genFunction(const char* name, char* szFuncName)
{
	int idx = 0;
	if(szFuncName == NULL || name == NULL)
		return NULL;

	const char* strproto = prototype.c_str();
	//copy return type
	for(idx = 0; strproto[idx] && strproto[idx] != '@'; idx ++){
		szFuncName[idx] = strproto[idx];
	}

	int iproto = idx + 1;
	//copy name
	strcpy(szFuncName + idx, name);
	idx += strlen(name);

	for(; strproto[iproto] ; iproto ++){
		szFuncName[idx ++] = strproto[iproto];
	}

	szFuncName[idx] = 0;

	return szFuncName;
}

string* EventValueType::idxOfName(const char* strName, int *pidx)
{
	int idx = 0;
	if(strName == NULL)
		return NULL;
	//str is in the list?
	for(list<string>::iterator it = historyNames.begin(); it != historyNames.end(); ++it)
	{
		if(strcmp(it->c_str(), strName)==0){
			if(pidx)
				 *pidx = idx;
			return &(*it);
		}
		idx ++;
	}

	return NULL;
}

Value EventValueType::newValue(const char* str)
{
	if(str == NULL)
		return 0;

	if(!checkFuncName(str))
		return 0;

	string* psstr = idxOfName(str, NULL);
	if(psstr)
		return (Value)psstr->c_str();

	//insert into
	string strname = str;
	historyNames.push_back(strname); //save into history
	return (Value)(historyNames.back().c_str());
}

Value EventValueType::newValue(xmlNodePtr node)
{
	if(node == NULL)
		return 0;
/*
	xmlChar* xstr = xhGetChildText(node,"prototype");
	if(!xstr)
		return 0;

	//get name from prototype
	const char* str = strchr((const char*)xstr, '(');
	const char* str1 ;
	for(str1 = str; str1 > (const char*)xstr && *str1 != ' ' && *str1 != '\t'; str1 ++);

	if(str1 <= (const char*)xstr)
		str1 = (const char*)xstr;
	else
		str1 ++;

	char szFuncName[256];
	strncpy(szFuncName,str1, str - str1);

	Value value = newValue(szFuncName);
	xmlFree(xstr);*/

	xmlChar* xstr = xhGetNodeText(node);
	if(xstr){
		Value value = newValue((const char*)xstr);
		xmlFree(xstr);
		return value;
	}

	return 0;
}

int RdrValueType::addValueRef(Value value)
{
    ResManager *resMgr = getResManager();
    if (resMgr) {
        Instance* inst = (Instance*) resMgr->getRes(value);
        if (inst)
            return inst->addRef();
    }
    return 1;
}

int RdrValueType::releaseValue(Value value)
{
    ResManager *resMgr = getResManager();
    if (resMgr) {
        Instance* inst = (Instance*) resMgr->getRes(value);
        if (inst)
            return inst->decRef();
    }
    return 1;
}

///////////////////////////////////////////////////////
void ClearValueTypes()
{
	FileValueType::cleanInstances();
	EnumValueType::cleanInstances();
	StructValueType::cleanInstances();
	EventValueType::cleanInstances();	
}

