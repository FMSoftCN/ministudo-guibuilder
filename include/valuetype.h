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

#ifndef VALUETYPE_H_
#define VALUETYPE_H_

extern ResEditorEnv * g_env;

#define VTM_SAVEVALUE MSG_USER+555
#define VTM_RESETVALUE MSG_USER + 556
//wparam = 0
// Value newvalue = lParam;

extern const char* strNull;

extern const char* fromStringPool(const char* str);

typedef SDWORD Value;

void ClearValueTypes();

class ValueType;
/****************************************************************************************
 *   ATTENTUON !!!!!!!
 *  Change ValueTypeType When enumResPackRdrType changed
 *  in New Control Set : <mgncs/mgncs.h>
 ***************************************************************************************/

enum ValueTypeType{
	VT_UNKNOWN = -1, //
	VT_COLOR = 0, // NCSRM_RDRTYPE_COLOR
	VT_INT, // NCSRM_RDRTYPE_METRIC
	VT_IMAGE, //NCSRM_RDRTYPE_IMAGE
	VT_FONT, //NCSRM_RDRTYPE_FONT
	VT_BINARY, //NCSRM_RDRTYPE_BINARY
	VT_STRING,
	VT_FILE,
	VT_TEXT,
	VT_NCS_MAX, // NCSRM_RDRTYPE_MAX, no use
	VT_RDR,
	VT_GROUP,
	VT_ENUM,
	VT_STRUCT,
	VT_ARRAY,
	VT_EVENT,
	VT_MAX
};

#define VT_METRIC VT_INT

class ValueUpdator
{
public:
	virtual ~ValueUpdator() {}
	//beging updating value
	virtual Value updateValue(Value value, ValueType* vtype,DWORD mask) = 0;
	//value has been updated
	virtual BOOL updatingValue(Value old_value, Value new_value, ValueType *vtype,DWORD mask) = 0;

	//updator the Editor content
	virtual BOOL updateEditorContent(HWND hEditor, ValueType *vtype, Value value, DWORD mask)
	{
		return FALSE;
	}
};

struct mystr_less
{
	bool operator()(const string& str1, const string&str2) const
	{
		return strcasecmp(str1.c_str(), str2.c_str()) < 0;
	}
};

class ValueType
{
	int ref;
public:
	ValueType():ref(1){}

	virtual ~ValueType(){}

	virtual int getType() = 0;

	virtual string toString(Value value) = 0;
	virtual HWND retrieveEditor(HWND hParent, Value value, ValueUpdator * updator, DWORD mask = 0) = 0;
	virtual DWORD toBinary(Value value) = 0;
	virtual DWORD toRes(Value value) = 0;
	virtual void saveXMLStream(Value value, TextStream* stream) = 0;

	virtual Value newValue(Value value) = 0;
	virtual Value newValue(xmlNodePtr node) = 0;
	virtual Value newValue(const char* str) = 0;

	//for UndoRedo
	virtual DWORD toMemo(Value value) = 0;
	virtual Value fromMemo(DWORD memo) = 0;
	virtual void freeMemo(DWORD memo) = 0;

	virtual int addValueRef(Value value) = 0;
	virtual int releaseValue(Value value) = 0;

	virtual BOOL equal(Value value1, Value value2) = 0;

	int addRef(){ return ++ref; }
	int release(){
		if(--ref == 0){
			delete this;
			return 0;
		}
		return ref;
	}

	static ValueType* getValueType(const char* type_name);
	static ValueType* loadCompoundTypeFromXMLNode(xmlNodePtr node);
	static BOOL loadCompoundTypes(const char* xmlFile);
};

ValueType* createCompositeValueType(xmlNodePtr node);

//define the basic value type
template<class TEditor, int VType= VT_UNKNOWN>
class TValueType : public ValueType
{
public:

	int getType(){ return VType; }

	HWND retrieveEditor(HWND hParent, Value value, ValueUpdator * updator,DWORD mask=0)
	{
		TEditor * editor;
		if(!IsWindow(hParent))
			return HWND_INVALID;

		HWND hwnd = GetDlgItem(hParent, (LINT)this);
		if(!IsControl(hwnd))
		{
			editor = new TEditor();
			hwnd = editor->create(hParent, (LINT)this);
		}
		else{
			editor = (TEditor*)(TEditor::fromHandle(hwnd));
		}
		if(editor == NULL)
			return HWND_INVALID;

		editor->init(value,(ValueType*)this, updator, mask);
		return hwnd;
	}
};

//define the TEditor
class ValueEditor
{
protected:
	HWND hwnd;
	ValueUpdator * updator;
	ValueType *vtype;
	Value value;
	BOOL bModified;
	DWORD mask;

	WNDPROC oldProc;

	inline Value updateValue(){
		if(updator)
			return updator->updateValue(value, vtype,mask);
		return 0;
	}
	inline BOOL updatingValue(Value new_value){
		if(updator)
			return updator->updatingValue(value,new_value, vtype,mask);
		return FALSE;
	}
	inline BOOL updateEditorContent(){
		if(updator)
			return updator->updateEditorContent(hwnd,vtype,value,mask);
		return TRUE;
	}

	virtual void onSaveValue() {	}

	//subclass the proc
	static LRESULT editor_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ValueEditor * _this = fromHandle(hwnd);
		if(_this)
			return _this->wndProc(message, wParam, lParam);

		return DefaultControlProc(hwnd, message, wParam, lParam);
	}

	//Notification
	static void editor_notification(HWND hwnd, LINT id, int nc, DWORD add_data){
		ValueEditor * _this = fromHandle(hwnd);
		if(_this)
			_this->onNotify(id, nc, add_data);
	}

	LRESULT callOldProc(UINT message,WPARAM wParam, LPARAM lParam){
		if(oldProc)
			return (oldProc)(hwnd, message, wParam, lParam);
		return DefaultControlProc(hwnd, message, wParam, lParam);
	}

	virtual LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam) {
		if(MSG_DESTROY == message){
			WNDPROC _old_proc = oldProc;
			SetWindowAdditionalData(hwnd, 0);
			if(_old_proc)
				_old_proc(hwnd, message, wParam, lParam);
			delete this;
			return 0;
		}
		else if(VTM_SAVEVALUE == message){
			onSaveValue();
			return TRUE;
		}
		else if(VTM_RESETVALUE == message){
			onResetValue((Value)lParam);
			return 0;
		}
		return callOldProc(message, wParam, lParam);
	}

	virtual void onNotify(LINT id, int nc, DWORD add_data){
	}

	virtual void onResetValue(Value newValue)
	{
		if(newValue != value)
		{
			Value oldValue = value;
			value = newValue;
			vtype->addValueRef(newValue);
			vtype->releaseValue(oldValue);
		}
		SetWindowText(hwnd,vtype->toString(value).c_str());
	}


public:
	HWND getHandler(){ return hwnd; }

	static ValueEditor* fromHandle(HWND hwnd){
		return (ValueEditor*)GetWindowAdditionalData(hwnd);
	}

	ValueEditor()
        :hwnd(HWND_INVALID)
         ,updator(NULL)
         ,vtype(NULL)
         ,value(0)
         ,bModified(FALSE)
         ,mask(0)
         ,oldProc(NULL)
    {
	}

	HWND createWindow(HWND hParent, LINT id,
			const char* className, DWORD style, DWORD ex_style, DWORD dwData = 0)
	{
		hwnd = CreateWindowEx(className, "",
				style|WS_CHILD|WS_BORDER, ex_style,
				id, 0, 0, 0, 0, hParent, dwData);
		SetWindowAdditionalData(hwnd, (DWORD)this);
		SetNotificationCallback(hwnd, editor_notification);
		oldProc = SetWindowCallbackProc(hwnd, editor_proc);
		return hwnd;
	}

	virtual HWND create(HWND hParent, LINT id) = 0;

	BOOL init(Value value, ValueType* vtype, ValueUpdator * updator, DWORD mask=0){
		//if(this->vtype)
		//	this->vtype->releaseValue(this->value);
		this->value = value;
		this->updator = updator;
		this->vtype = vtype;
		this->mask = mask;
		if(updateEditorContent())
		{
			return TRUE;
		}
		string txt = vtype->toString(value);
		vtype->addValueRef(value);
		SetWindowText(hwnd, txt.c_str());
		bModified = FALSE;
		if(this->vtype)
			this->vtype->addRef();
		return TRUE;
	}

	virtual ~ValueEditor(){
		if(vtype) {
			vtype->releaseValue(value);
        }
	}
};

///////////////////////////////
//base  value type
template<typename TMeta, class TEditor, class TSelf, int VType = VT_UNKNOWN>
class TBaseValueType : public TValueType<TEditor, VType>
{
public:
	DWORD toBinary(Value value){
		return (DWORD)value;
	}

	DWORD toRes(Value value){
		return (DWORD)value;
	}

	void saveXMLStream(Value value, TextStream *stream){
		stream->printf("%s", static_cast<TSelf*>(this)->toString(value).c_str());
	}

	Value newValue(Value value){
		addValueRef(value);
		return value;
	}

	Value newValue(xmlNodePtr node){
		if(node == NULL)
			return (Value)0;
		//get string
		xmlChar* txt = xhGetNodeText(node);
		Value value = static_cast<TSelf*>(this)->newValue((const char*)txt);
		xmlFree(txt);
		return value;
	}

	//for UndoRedo
	DWORD toMemo(Value value){
		return value;
	}
	Value fromMemo(DWORD memo){
		return (Value)(memo);
	}
	void freeMemo(DWORD memo){
		return ;
	}

	BOOL equal(Value value1, Value value2){
		return (value1 == value2);
	}

	int addValueRef(Value value){
		return 1;
	}

	int releaseValue(Value value){
		return 1;
	}

	static ValueType* getInstance(){
		static TSelf * _psingleton = new TSelf();
		return _psingleton;
	}
};


//interger value
class IntValueEditor;

class IntValueType : public TBaseValueType<int, IntValueEditor, IntValueType, VT_INT>
{
public:
	string toString(Value value){
		char szText[32];
		sprintf(szText, "%d",(int)value);
		return string(szText);
	}

	//from str
	Value newValue(const char* str){
		if(str == NULL)
			return (Value)0;
		return (Value)strtol(str, NULL, 0);
	}
};

class IntValueEditor : public ValueEditor
{
protected:
	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam){
		switch (message) {
			case MSG_CHAR:
				if((wParam >='0' && wParam <= '9') || wParam == 127/*back space*/ || wParam == '-'){
					if(wParam == '-'){
						int line_pos = 0, char_pos = 0;
						if(!(SendMessage(hwnd, EM_GETCARETPOS, (WPARAM)&line_pos, (LPARAM)&char_pos) == 0
								&& line_pos == 0 && char_pos == 0)){
							return 0;
						}
					}
					bModified = TRUE;
					break;
				}

				return 0;
			case MSG_KEYDOWN:
				if(wParam == SCANCODE_ENTER){
				onSaveValue();
				return 0;
				}
				break;

			default:
				break;
		}
		return ValueEditor::wndProc(message, wParam, lParam);
	}

	void onSaveValue(){
		if(!bModified)
			return ;
		bModified = FALSE;
		char szText[64];
		char *text = szText;
		int len = GetWindowText(hwnd, szText, sizeof(szText)-1);
		if(len > 1) {
			while(text[0] && text[0] == '0') //strip the 0 in the first
				text ++;
		}
		Value newValue = vtype->newValue(text);
		if(!updatingValue(newValue))
		{
			//rollback
			vtype->releaseValue(newValue);
            SetWindowText(hwnd, vtype->toString(value).c_str());
			return;
		}
		vtype->releaseValue(value);
		value = newValue;
        value = updateValue();
		SetWindowText(hwnd, vtype->toString(value).c_str());
	}

public:
	HWND create(HWND hParent, LINT id){
		HWND sleWnd = createWindow(hParent, id, "sledit", ES_LEFT, 0);
        SendMessage(sleWnd, EM_LIMITTEXT, 63, 0);
        return sleWnd;
	}

};

//char value type
class CharValueEditor;
class CharValueType : public TBaseValueType<char, CharValueEditor, CharValueType, VT_INT>
{
public:
	string toString(Value value){
		char szText[32];
		if((value >=0 && value <9)|| value>127)
			sprintf(szText,"0x%lx",value);
		else if(value == 9)
			strcpy(szText,"\\t");
		else if(value == 10)
			strcpy(szText, "\\r");
		else if(value == 13)
			strcpy(szText, "\\n");
		else if(value == '\\')
			strcpy(szText, "\\\\");
		else
			sprintf(szText, "%c",(int)value);
		return string(szText);
	}
	//from str
	Value newValue(const char* str){
		if(str == NULL)
			return (Value)0;
		if(str[0]=='0' || str[1]=='x' || str[1]=='X')
		{
			return strtol(str, NULL, 0);
		}
		else if(str[0] =='\\')
		{
			switch(str[1])
			{
			case 'r': return 10;
			case 'n': return 13;
			case 't': return 9;
			case 'a': return '\a';
			case 'b': return '\b';
			case '\\': return '\\';
			}
		}
		return str[0];
	}
};

class CharValueEditor : public ValueEditor
{
protected:
	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam){
   switch (message) {
   case MSG_CHAR:
	   bModified = TRUE;
	   break;
		case MSG_KEYDOWN:
			if(wParam == SCANCODE_ENTER){
				onSaveValue();
				return 0;
			}
			break;
		}
		return ValueEditor::wndProc(message, wParam, lParam);
	}

	void onSaveValue(){
		if(!bModified)
			return ;
		bModified = FALSE;
		char szText[8];
		GetWindowText(hwnd, szText, sizeof(szText)-1);
		Value newValue = vtype->newValue(szText);
		if(!updatingValue(newValue))
		{
			//rollback
			vtype->releaseValue(newValue);
            SetWindowText(hwnd, vtype->toString(value).c_str());
			return;
		}
		vtype->releaseValue(value);
		value = newValue;
		updateValue();
	}
public:
	HWND create(HWND hParent, LINT id){
			HWND hwnd = createWindow(hParent, id, "sledit", ES_LEFT, 0);
			SendMessage(hwnd, EM_LIMITTEXT, 8,0);
			return hwnd;
		}

};


template<class TEditor,int VType = VT_STRING>
class TBaseStringValueType : public TBaseValueType<char*, TEditor, TBaseStringValueType<TEditor, VType>,VType >
{
public:
	string toString(Value value){
		if(value != 0)
			return string((const char*)value);
		return string("");
	}

	void saveXMLStream(Value value, TextStream *stream){
		if(value != 0)
			stream->printf("%s",(const char*)value);
	}

	Value newValue(const char* str){
		if(str == NULL)
			return (Value)0;

		char* newstr = (char*)malloc(strlen(str)+2) + 1;
		newstr[-1] = 1;
		strcpy(newstr, str);
		return (Value)newstr;
	}

	//for UndoRedo
	DWORD toMemo(Value value){
		if(value == 0)
			return 0;
		return (DWORD)strdup((const char*)value);
	}

	Value fromMemo(DWORD memo){
		Value v = newValue((char*)memo);
		freeMemo(memo);
		return v;
	}

	void freeMemo(DWORD memo){
		if(memo){
			free((char*)memo);
		}
	}

	BOOL equal(Value value1, Value value2){
		if(value1 == value2)
			return TRUE;
		if(value1 == 0 || value2 == 0)
			return FALSE;
		return strcmp((const char*)value1, (const char*)value2) == 0;
	}

	int addValueRef(Value value){
		if(value){
			char * str = (char*)value;
			return ++str[-1];
		}
		return 0;
	}

	int releaseValue(Value value){
		if(value){
			char * str = (char*)value;
			if(-- str[-1] == 0){
				free(str-1);
				return 0;
			}
			return str[-1];
		}
		return 0;
	}
};

class StringValueEditor;
typedef TBaseStringValueType<StringValueEditor, VT_STRING> StringValueType;

//string value type
class StringValueEditor : public ValueEditor
{
protected:
	void onNotify(LINT id, int nc, DWORD add_data){
		if(nc == EN_UPDATE || nc == EN_ENTER)
			onSaveValue();
		else if(nc == EN_CHANGE)
			bModified = TRUE;
	}

	void onSaveValue()
	{
		if(!bModified)
			return;
        bModified = FALSE;
		//get string
		char szText[1024];

		GetWindowText(hwnd, szText,sizeof(szText));
		Value newValue = vtype->newValue(szText);
		if(!updatingValue(newValue)) //don't changed
		{
			//revert
			vtype->releaseValue(newValue);
			SetWindowText(hwnd, vtype->toString(value).c_str());
			return;
		}
        vtype->releaseValue(value);
        value = newValue;
        updateValue();
	}

public:
	HWND create(HWND hParent, LINT id){
		return createWindow(hParent, id, "sledit", ES_LEFT, 0);
	}
};


///compund editor
#define EXTEND_WIDTH 30
class ExtendEditor : public ValueEditor
{
protected:
	enum { EXTEND_ID = 100, EDITOR_ID=101};
	virtual void onEditor(int nc, HWND heditor) {}
	virtual void onExtend() {}
	virtual void createEditor(LINT id, const RECT *rt) {}
	bool extend_dlg;


	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == MSG_COMMAND)
		{
			int id = LOWORD (wParam);
			if(id == EXTEND_ID){
				extend_dlg = true;
				onExtend();
				extend_dlg = false;
			}
			else if(id == EDITOR_ID)
				onEditor(HIWORD(wParam), (HWND)lParam);
			return 0;
		}
		else if(message == MSG_SIZECHANGED){
			RECT *prt = (RECT*)lParam;
			//calc editor
			HWND heditor = GetDlgItem(hwnd, EDITOR_ID);

			HWND hextend = GetDlgItem(hwnd, EXTEND_ID);
			int width = RECTWP(prt);
			int height = RECTHP(prt);
			::MoveWindow(heditor, 0, 0, width-EXTEND_WIDTH, height, FALSE);
			::MoveWindow(hextend, width - EXTEND_WIDTH, 0,EXTEND_WIDTH, height, FALSE);
			return 0;
		}
		else if(message == MSG_SETTEXT){
			HWND heditor = GetDlgItem(hwnd, EDITOR_ID);
			if(IsControl(heditor))
				return SetWindowText(heditor,(const char*)lParam);
			return 0;
		}
		else if (message == MSG_GETTEXT) {
			HWND heditor = GetDlgItem(hwnd, EDITOR_ID);
			if(IsControl(heditor))
				return GetWindowText(heditor, (char*)lParam, (int)wParam);
			return 0;
		}
		else if (message == MSG_PAINT) {
			::InvalidateRect(GetDlgItem(hwnd, EDITOR_ID), NULL, TRUE);
			::InvalidateRect(GetDlgItem(hwnd, EXTEND_ID), NULL, FALSE);
		}
		else if (message == VTM_SAVEVALUE){
			if(extend_dlg)
				return 0;
			onSaveValue();
			return TRUE;
		}

		//return DefaultControlProc(hwnd, message, wParam, lParam);
		return ValueEditor::wndProc(message, wParam, lParam);
	}

	void calcaRect(RECT *prt, BOOL bEditor=TRUE){
		GetClientRect(hwnd, prt);
		if(bEditor){
			prt->right -= EXTEND_WIDTH;
		}
		else{
			prt->left = prt->right - EXTEND_WIDTH;
		}
	}
public:

	ExtendEditor()
	:extend_dlg(false)
	{
	}

	HWND create(HWND hParent, LINT id){
		createWindow(hParent, id, "static", 0,0);
		oldProc = DefaultControlProc;
		RECT rt;
		calcaRect(&rt, TRUE);
		createEditor(EDITOR_ID, &rt);
		//create extend button
		calcaRect(&rt, FALSE);
		CreateWindow("button","...", BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE,EXTEND_ID,
				rt.left, rt.top, RECTW(rt), RECTH(rt), hwnd, 0);
		return hwnd;
	}
};

////////////////////////////////////////////////////////
// FileValueEditor
class FileValueEditor : public ExtendEditor
{
protected:
	string lastPath;
	void createEditor(LINT id, const RECT *rt){
		//create a editor
		CreateWindowEx("sledit", "", ES_LEFT|WS_CHILD|WS_VISIBLE, 0, EDITOR_ID, rt->left, rt->top, RECTWP(rt), RECTHP(rt), hwnd, 0);
	}
	void onExtend();

	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == MSG_COMMAND)
		{
			int id = LOWORD(wParam);
			int code = HIWORD(wParam);
			if(id == EDITOR_ID){
				if(code == EN_CHANGE)
					bModified = TRUE;
				else if(code == EN_ENTER)
				{
					onSaveValue();
				}
				return 0;
			}
		}
		return ExtendEditor::wndProc(message, wParam, lParam);
	}

	void onSaveValue()
	{
		if(bModified){
			char szText[1024];
			::GetWindowText(GetDlgItem(hwnd, EDITOR_ID), szText, sizeof(szText)-1);
			setValue(szText);
			bModified = FALSE;
		}
	}

	void setValue(const char* text);

public:
	FileValueEditor(){
#ifdef WIN32
		lastPath = "C:\\";
#else
		lastPath = getenv("HOME");
#endif
	}
};

class FileValueType : public TBaseStringValueType<FileValueEditor, VT_FILE>
{
protected:
	string filter;
	typedef mapex<string, FileValueType*, mystr_less>  FileValueTypeMap;

	static FileValueTypeMap _instances;
public:
	FileValueType(const char* filter = NULL, const char* name = NULL){
		if(filter && *filter!='\0'){
			this->filter = filter;
		}
		else
		{
			this->filter = "AllFile(*.*)";
		}

		if(name){
			_instances[name] = this;
		}
	}
	const char* getFilter(){ return filter.c_str(); }

	static ValueType* getInstance(const char* strname){
		if(strname == NULL)
			return NULL; //return a new object
		return _instances.at(strname);
	}

	static void cleanInstances(){
		for(FileValueTypeMap::iterator it = _instances.begin();
				it != _instances.end(); ++it)
		{
			if(it->second)
				it->second->release();
		}
		_instances.clear();
	}

	DWORD toRes(Value value)
	{
		if(value != 0)
		{
			return (DWORD)g_env->addString((const char*)value);
		}
		return 0;
	}

	int releaseValue(Value value){
		if(value)
		{
			int id = g_env->stringToId((const char*)value);
			g_env->removeString(id);
		}
		return TBaseStringValueType<FileValueEditor, VT_FILE>::releaseValue(value);
	}
};

///////////////////////////////////////////////////////////

class FontValueEditor : public ExtendEditor
{
public:
	FontValueEditor(){}

protected:
	void onExtend();
	void setValue(const char* font_name);

	void createEditor(LINT id, const RECT *rt)
	{
		CreateWindowEx("static", "", ES_LEFT|WS_CHILD|WS_VISIBLE, 0, EDITOR_ID, rt->left, rt->top, RECTWP(rt), RECTHP(rt), hwnd, 0);
	}
};

typedef TBaseStringValueType<FontValueEditor, VT_FONT> FontValueType;

///////////////////////////////////////////////////////////

class ColorValueEditor : public ExtendEditor
{
protected:
	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void onExtend();

	void createEditor(LINT id, const RECT *rt){
		//create a editor
		//CreateWindowEx("sledit", "", ES_LEFT|WS_CHILD|WS_VISIBLE, 0, EDITOR_ID, rt->left, rt->top, RECTWP(rt), RECTHP(rt), hwnd, 0);
	}

public:
	BOOL init(Value value, ValueType* vtype, ValueUpdator * updator, DWORD mask=0){
		if(this->vtype)
			this->vtype->releaseValue(this->value);
		this->value = value;
		this->updator = updator;
		this->vtype = vtype;
		this->mask = mask;
		updateEditorContent();
		return TRUE;
	}
};

class ColorValueType : public TBaseValueType<DWORD, ColorValueEditor, ColorValueType, VT_COLOR>
{
public:
	string toString(Value value){
		char szText[16];
		sprintf(szText, "0x%08X", (unsigned int)value);
		return string(szText);
	}

	Value newValue(const char* str){
		if(str == NULL)
			return 0;

		return (Value)strtoul(str, NULL, 0);
	}
};

///////////////////////////////////////////////////////////
//res id editor
template <class TEditor, int VType = VT_UNKNOWN>
class TBaseResValueType : public TValueType<TEditor, VType>
{
protected:
	int type;
public:
	virtual ResManager * getResManager(){
		return g_env->getResManager(type);
	}

	TBaseResValueType(int type){
		this->type = type;
	}

	int getResType(){ return type; }

	string toString(Value value){
		ResManager *resMgr = getResManager();
		if(resMgr){
			const char* str = resMgr->idToName((int)value);
			if(str != NULL)
				return string(str);
		}
		char szText[32];
		sprintf(szText, "%d",(int)value);
		return string(szText);
	}

	DWORD toBinary(Value value){
		// TODO return (DWORD)RetrieveResource((int)value);
		if(value == 0)
			return 0;
		ResManager *resmgr = getResManager();
		if(resmgr)
			return resmgr->getRes((int)value);
		return 0;
	}

	DWORD toRes(Value value){
		return (DWORD)value;
	}

	//for UndoRedo
	DWORD toMemo(Value value){
		return value;
	}
	Value fromMemo(DWORD memo){
		return (Value)(memo);
	}
	void freeMemo(DWORD memo){
		return ;
	}

	void saveXMLStream(Value value, TextStream *stream){
		stream->printf("0x%08X",(int)value);
	}

	Value newValue(Value value){
		addValueRef(value);
		return value;
	}

	Value newValue(xmlNodePtr node){
		if(node == NULL)
			return (Value)-1; //invalid key
		//get string
		xmlChar* txt = xhGetNodeText(node);
		Value value = newValue((const char*)txt);
		xmlFree(txt);
		return value;
	}

	Value newValue(const char* str){
		if(str == NULL)
			return (Value) -1;
		return (Value)strtol(str,NULL,0);
	}

	BOOL equal(Value value1, Value value2)
	{
		return value1 == value2;
	}

	int addValueRef(Value value){
		return 1;
	}

	int releaseValue(Value value){
		return 1;
	}

	virtual int saveToRes(int id, DWORD binaryValue)
	{
		// TODO Save binaryValue to res
		ResManager *resmgr = getResManager();
		if(resmgr == NULL)
			return -1;
		resmgr->setRes(id, binaryValue);
		return id;
	}

	virtual int changeIdName(Value value, const char* newName){
		ResManager *resmgr = getResManager();
		if(resmgr){
			return resmgr->setResName((int)value,newName);
		}
		return -1;
	}
};

///////////////////////////////
//BaseResValueType
template<class TParent>
class TBaseResValueEditor : public TParent
{
protected:
	virtual int getResType() = 0;
	virtual ResManager* getResManager(){
		return g_env->getResManager(getResType());
	}
	virtual BOOL setValue(const char* txt)
	{
		if(!txt)
			return FALSE;
		int id = strtol(txt, NULL, 0);
		if(!VALIDID(id, getResType()))
		{
			ResManager* resMgr = getResManager();
			if(!resMgr)
				return FALSE;
			id = resMgr->nameToId(txt);
			if(!VALIDID(id,getResType()))
				return FALSE;
		}
		if(id == (int)TParent::value)
			return FALSE;
		return setValue(id);
	}

	virtual BOOL setValue(int id)
	{
		if(!TParent::updatingValue(id))
			return FALSE;
		TParent::value = (Value)id;
		TParent::updateValue();
		return TRUE;
	}

	void onSaveValue(){
		if(!TParent::bModified)
			return;
		TParent::bModified = FALSE;
		//get Value id Name
		char szText[256];
		if(GetWindowText(TParent::hwnd, szText,sizeof(szText))>0)
			setValue(szText);
	}
};

//ID Value
class IDValueEditor;
typedef TBaseResValueType<IDValueEditor> IDValueType;

class IDValueEditor: public ValueEditor
{
protected:
	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message) 
        {
            case MSG_CHAR:
                //only support ascii character.
                if (!((0x0ff0000 & wParam) >> 16) && !HIBYTE(wParam)) {
                    unsigned char ch = LOBYTE (wParam);
                    if(ch == 8 || ch == 127 || ch == '\b' || ch == '_' || isalnum(ch)) {
                        bModified = TRUE;
                        break;
                    }
                }

                return 0;

            case MSG_KEYDOWN:
                if(wParam == SCANCODE_ENTER) {
                    onSaveValue();
                    return 0;
                }
                break;

            default:
                break;
		}
		return ValueEditor::wndProc(message, wParam, lParam);
	}

	void onSaveValue()
    {
		if(!bModified)
			return;
		bModified = FALSE;

		char szText[64];
		GetWindowText(hwnd, szText, sizeof(szText)-1);

		ResManager* resMgr = g_env->getResManager(ID2TYPE(value));

		if(!updatingValue(resMgr->nameToId(szText))) {
			SetWindowText(hwnd, resMgr->idToName(value));
			return;
		}
		int newid = resMgr->setResName(value, szText, mask);
		if(newid != -1){
			value = newid;
			updateValue();
		}
		else
			SetWindowText(hwnd, resMgr->idToName(value));
	}

public:
	HWND create(HWND hParent, LINT id){
		HWND sleWnd = createWindow(hParent, id, "sledit", ES_LEFT, 0);
        SendMessage(sleWnd, EM_LIMITTEXT, 63, 0);
        return sleWnd;
	}
};


///////////////////
class BaseResValueEditor;

typedef TBaseResValueType<BaseResValueEditor> BaseResValueType;

class BaseResValueEditor : public TBaseResValueEditor<StringValueEditor>
{
protected:
	int getResType(){ return ((BaseResValueType*)vtype)->getResType();}
};

template <class TEditor,class TSelf,int VType = VT_UNKNOWN>
class TResValueType : public TBaseResValueType<TEditor, VType>
{
public:
	static ValueType* getInstance(){
		static TSelf * _psingleton = new TSelf();
		return _psingleton;
	}

public:
	TResValueType(int type)
	:TBaseResValueType<TEditor, VType>(type)
	{

	}
};

///////
///text value

class MutliStringEditor : public ExtendEditor
{
	static LRESULT _multiTextEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
public:
	MutliStringEditor(){}
protected:
	void createEditor(LINT id, const RECT *rt);
	void onExtend(); //show a multi text edit dialog

	void onSaveValue()
	{
		if(bModified){
			char szText[1024];
			::GetWindowText(GetDlgItem(hwnd, EDITOR_ID), szText, sizeof(szText)-1);
			setValue(szText);
			bModified = FALSE;
		}
	}
	virtual void setValue(const char* str)
	{
		Value newValue = vtype->newValue(str);
		if(!updatingValue(newValue)) //don't changed
		{
			//revert
			vtype->releaseValue(newValue);
			SetWindowText(hwnd, vtype->toString(value).c_str());
			return ;
		}
		vtype->releaseValue(value);
		value = newValue;
		SetDlgItemText(hwnd, EDITOR_ID, str);
		updateValue();
	}

	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(message == MSG_COMMAND)
		{
			LINT id = LOWORD(wParam);
			int code = HIWORD(wParam);
			if(id == EDITOR_ID){
				if(code == EN_CHANGE)
					bModified = TRUE;
				else if(code == EN_ENTER)
					onSaveValue();
				return 0;
			}
		}
		return ExtendEditor::wndProc(message, wParam, lParam);
	}
};

class TextResEditor : public MutliStringEditor
{
protected:

	void setValue(const char* text);
};

class TextValueType : public TResValueType<TextResEditor, TextValueType, VT_TEXT>
{
public:
	TextValueType():
	TResValueType<TextResEditor, TextValueType, VT_TEXT>(NCSRT_TEXT){}

	string toString(Value value){
		//get resource
		const char* strvalue = (const char*)toBinary(value);
		if(strvalue)
			return string(strvalue);
		return string("");
	}

	Value newValue(const char* str);

	Value newValue(Value value)
	{
		const char* strvalue = (const char*)toBinary(value);
		return newValue(strvalue?strvalue:"");
	}

	int saveToRes(int id, DWORD binValue){
		ResManager *resMgr = getResManager();
		if(resMgr){
			if(!resMgr->setRes(id, binValue))
				return resMgr->createRes(type,NULL, id,(const char*) binValue, 0);
		}
		return id;
	}
};

//the editor to reference other resource
class RefResEditor : public TBaseResValueEditor<ExtendEditor>
{
protected:
	void onExtend();
	void createEditor(LINT id, const RECT *rt){
		//create a editor
		CreateWindowEx("sledit", "", ES_LEFT|WS_CHILD|WS_VISIBLE, 0, EDITOR_ID, rt->left, rt->top, RECTWP(rt), RECTHP(rt), hwnd, 0);
	}
};
/////////////////
//image res

class ImageResEditor : public RefResEditor
{
protected:
	int getResType(){
		return NCSRT_IMAGE; //TODO return Image res type
	}

/*	LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam){
		return RefResEditor::wndProc(message, wParam, lParam);
	}*/
};

class ImageValueType : public TResValueType<ImageResEditor, ImageValueType, VT_IMAGE>
{
public:
	ImageValueType()
	:TResValueType<ImageResEditor, ImageValueType, VT_IMAGE>(NCSRT_IMAGE)
	{}
	DWORD toBinary(Value value)
	{
		int source_id = TResValueType<ImageResEditor, ImageValueType, VT_IMAGE>::toBinary(value);
		return (DWORD)(g_env->getString(source_id));
	}
};

class GroupResEditor : public TBaseResValueEditor<ValueEditor>
{
protected:
	int getResType(){
		return NCSRT_CONTRL; //TODO return Renderer res type
	}

	void onNotify(LINT id, int nc, DWORD add_data)
	{
		if(nc == CBN_SELCHANGE){
			bModified = TRUE;
			onSaveValue();
		}
	}

	void onSaveValue()
	{
		if(!bModified)
			return ;
		bModified = FALSE;
		//get ADD data
		LRESULT sel_idx = ::SendMessage(hwnd,CB_GETCURSEL, 0, 0);
		if(sel_idx < 0)
			return ;
		LRESULT id = ::SendMessage(hwnd, CB_GETITEMADDDATA, sel_idx, 0);
		if(id == (LRESULT)value)
			return;
		//update
		if(!updatingValue(id))
		{
            //reset old name
            ResManager* resMgr = g_env->getResManager(ID2TYPE(value));
            SetWindowText(hwnd, resMgr->idToName(value));
			return;
		}
		value = (Value)id;
		updateValue();
	}

public:
	HWND create(HWND hParent, LINT id)
	{
		return createWindow(hParent, id, "combobox",CBS_DROPDOWNLIST|CBS_READONLY|CBS_SORT|CBS_NOTIFY, 100);
	}

};

class GroupValueType : public TResValueType<GroupResEditor, GroupValueType, VT_GROUP>
{
public:
	GroupValueType()
	:TResValueType<GroupResEditor, GroupValueType, VT_GROUP>(NCSRT_CONTRL)
	{}

	DWORD toBinary(Value value){ return (DWORD)value; }
	DWORD toRes(Value value){
		return (DWORD)(value&0xFFFF);
	}
};

///renderer type
//TODO declear RdrValueType and RdrResEditor'
class RdrResEditor : public TBaseResValueEditor<ValueEditor>
{
protected:
	int getResType(){
		return NCSRT_RDR; //TODO return Renderer res type
	}

	void onNotify(LINT id, int nc, DWORD add_data)
	{
		if(nc == CBN_SELECTOK){
			bModified = TRUE;
			onSaveValue();
		}
	}

	void onSaveValue()
	{
		if(!bModified)
			return ;
		bModified = FALSE;
		//get ADD data
		LRESULT sel_idx = ::SendMessage(hwnd,CB_GETCURSEL, 0, 0);
		if(sel_idx < 0)
			return ;
		LRESULT id = ::SendMessage(hwnd, CB_GETITEMADDDATA, sel_idx, 0);
		if(id == (LRESULT)value && id != -1)
			return;
		//update
		if(!updatingValue(id))
		{
			return;
		}
		value = (Value)id;
		updateValue();
	}

public:
	HWND create(HWND hParent, LINT id)
	{
		return createWindow(hParent, id, "combobox",CBS_DROPDOWNLIST|CBS_READONLY|CBS_SORT|CBS_NOTIFY, 0);
	}

};

class RdrValueType : public TResValueType<RdrResEditor, RdrValueType, VT_RDR>
{
public:
	RdrValueType()
	:TResValueType<RdrResEditor, RdrValueType, VT_RDR>(NCSRT_RDR|NCSRT_RDRSET)
	{
    }

	int addValueRef(Value value);
	int releaseValue(Value value);
};


///////////////////////////////////////////////////////////////////////////////////////////
template<class TValue, class TEditor, class TSelf, int VType = VT_UNKNOWN>
class TCompositeValueType : public TValueType<TEditor, VType>
{
protected:
	string name;
public:

	const char* getName(){ return name.c_str(); }

	string toString(Value value){
		if(value){
			return ((TValue*)(value))->toString();
		}
		return string("");
	}

	DWORD toBinary(Value value){
		if(value){
			return ((TValue*)(value))->toBinary();
		}
		return 0;
	}

	DWORD toRes(Value value){
		if(value){
			return ((TValue*)(value))->toRes();
		}
		return 0;
	}

	void saveXMLStream(Value value, TextStream *stream){
		if(value){
			((TValue*)(value))->saveXMLStream(stream);
		}
	}

	Value newValue(Value value){
		if(value == 0)
			return (Value)(new TValue(static_cast<TSelf*>(this)));
		else
			return (Value)(new TValue(*((TValue*)(value))));
	}

	Value newValue(xmlNodePtr node){
		return (Value)(new TValue(static_cast<TSelf*>(this),node));
	}

	Value newValue(const char* str){
		return (Value)(new TValue(static_cast<TSelf*>(this),str));
	}

	BOOL equal(Value value1, Value value2){
		if(value1 == value2)
			return TRUE;
		if(value1 == 0 || value2 == 0)
			return FALSE;

		return *((TValue*)value1) == (*((TValue*)(value2)));
	}

	int addValueRef(Value value){
		if(value){
			((TValue*)(value))->addRef();
		}
		return 0;
	}

	int releaseValue(Value value){
		if(value){
			((TValue*)(value))->release();
		}
		return 0;
	}

	static ValueType* getInstance(const char* strname){
		if(strname == NULL)
			return NULL; //return a new object
		return _instances.at(strname);
	}


protected:
	typedef mapex<string, TSelf*, mystr_less>  InstanceMap;
	static InstanceMap _instances;
	static void insertInstance(TSelf* instance){
		if(instance && instance->name.length()>0){
			_instances[instance->name] = instance;
		}
	}
public:
	static void cleanInstances();
};

template<class TValue, class TEditor, class TSelf, int VType>
mapex<string, TSelf*,mystr_less> TCompositeValueType<TValue, TEditor, TSelf, VType>::_instances;

///////////
template<class TValueType>
class TCompositeValue
{
	int ref;

protected:
	TValueType *vtype;

public:

	int addRef(){
		return ++ref;
	}

	int release(){
		if(--ref == 0)
			delete this;
		return ref;
	}

	TCompositeValue(TValueType *vtype):ref(1){
		this->vtype = vtype;
	}

};

template<class TValue, class TBaseEditor = ValueEditor >
class TCompositeValueEditor : public TBaseEditor
{
protected:
	virtual BOOL initEditor() {return TRUE;}

public:
	BOOL init(Value value, ValueType* vtype, ValueUpdator * updator, DWORD mask=0){
		this->value = value;
		this->updator = updator;
		this->vtype = vtype;
		this->mask = mask;
		if(initEditor())
		{
			TBaseEditor::updateEditorContent();
			return TRUE;
		}
		return FALSE;
	}
};

////////
//enum type

class EnumValue ;

class EnumValueEditor : public TCompositeValueEditor<EnumValue>
{
protected:

	void onNotify(LINT id, int nc, DWORD add_data);
	BOOL initEditor();

	void onSaveValue();

public:
	HWND create(HWND hParent, LINT id){
		return createWindow(hParent, id, "combobox",
				CBS_DROPDOWNLIST|CBS_READONLY|CBS_NOTIFY, 0);
	}
};

class EnumValueType : //public TCompositeValueType<EnumValue, EnumValueEditor, EnumValueType, VT_ENUM>
	public TValueType<EnumValueEditor, VT_ENUM>
{
	friend class EnumValueEditor;
protected:
	ValueType * optionValueType;
	string name;
	struct Option{
		const char* name;
		const char* caption;
		Value value;
		Option(){
			name = caption = (char*)strNull;
			value = 0;
		}
		~Option(){
		}
	};

	list<Option*> options;

	Option * getOptionByValue(Value value);
	Option * getOptionByName(const char* strName);
public:

	const char* getName(){ return name.c_str(); }

	EnumValueType(xmlNodePtr node);
	~EnumValueType();

	ValueType* getOptionValueType(){
		return optionValueType;
	}


	Value getOptionValue(const char* name);

	DWORD toMemo(Value value);

	Value fromMemo(DWORD memo);

	void freeMemo(DWORD memo){
		return;
	}

	string toString(Value value){
		Option * option = getOptionByValue(value);
		if(option)
			return option->caption;
		return "";
	}

	DWORD toBinary(Value value)
	{
		ValueType *vt = getOptionValueType();
		if(vt)
			return vt->toBinary(value);
		return (DWORD)-1;
	}

	DWORD toRes(Value value)
	{
		ValueType *vt = getOptionValueType();
		if(vt)
			return vt->toRes(value);
		return (DWORD)-1;
	}

	void saveXMLStream(Value value, TextStream* stream)
	{
		Option * option = getOptionByValue(value);
		if(option)
		{
			stream->printf("%s",option->name);
		}
	}

	Value newValue(Value value){
		if(optionValueType)
			return optionValueType->newValue(value);
		return value;
	}

	Value newValue(xmlNodePtr node);

	Value newValue(const char* str);

	int addValueRef(Value value)
	{
		if(optionValueType)
			return optionValueType->addValueRef(value);
		return 0;
	}

	int releaseValue(Value value)
	{
		if(optionValueType)
			return optionValueType->releaseValue(value);
		return 0;
	}

	BOOL equal(Value value1, Value value2)
	{
		if(optionValueType)
			return optionValueType->equal(value1, value2);
		return value1 == value2;
	}

	int optionCount(){
		return options.size();
	}

	static ValueType* getInstance(const char* strname){
		if(strname == NULL)
			return NULL; //return a new object
		return _instances.at(strname);
	}

private:
	static mapex<string, EnumValueType*,mystr_less > _instances;

protected:
	static void insertInstance(EnumValueType* instance){
		if(instance && instance->name.length()>0){
			_instances[instance->name] = instance;
		}
	}

public:
	static void cleanInstances()
	{
		for(mapex<string, EnumValueType*, mystr_less> :: iterator it = _instances.begin();
				it != _instances.end(); ++it)
		{
			if(it->second)
				it->second->release();
		}
		_instances.clear();
	}
};

//////////struct

class StructValue;

class StructValueEditor : public TCompositeValueEditor<StructValue, ExtendEditor>
{
protected:
	//void initEditor();
	void onExtend();
};

class StructValueType : public TCompositeValueType<StructValue, StructValueEditor, StructValueType,VT_STRUCT>
{
	friend class StructValueEditor;
protected:

public:
	struct Element{
		const char * name;
		const char * caption;
		ValueType *vtype;
		Element(){
			name = caption = strNull;
			vtype = NULL;
		}
		~Element(){
		}
	};
protected:
	list<Element*> elements;
public:

	int getElementCount(){ return elements.size(); }

	ValueType * getElementType(const char* name, int *pidx=NULL);
	ValueType * getElementType(int idx);
	const Element* getElement(int idx);

	StructValueType(xmlNodePtr node);
	~StructValueType();

	void saveXMLStream(Value value, TextStream* stream);

	BOOL compare(const StructValue& sv1, const StructValue& sv2) const;

	DWORD toMemo(Value value);

	Value fromMemo(DWORD memo);

	void freeMemo(DWORD memo);

	void memoToValues(const DWORD* memos, Value* values);
	DWORD valuesToMemo(const Value* values);
};

class StructValue : public TCompositeValue<StructValueType>
{
	friend class StructValueType;

	Value * values;
	DWORD * binValues;
public:
	StructValue(StructValueType * type);

	StructValue(StructValueType * type, xmlNodePtr node);

	StructValue(StructValueType * type, const char* str);

	StructValue(const StructValue& sv);

	Value * getValues(){ return values;}
	DWORD * getBins(){ return binValues;}

	~StructValue();

	string toString();

	DWORD toRes(){
		return (DWORD)values;
	}

	DWORD toBinary(){
		return (DWORD)binValues;
	}

	void saveXMLStream(TextStream* stream){};

	friend BOOL operator==(const StructValue& sv1, const StructValue& sv2){
		if(sv1.vtype == sv2.vtype){
			return sv1.vtype->compare(sv1, sv2);
		}
		return FALSE;
	}
};

/////////////////////
//Array
/*
class ArrayValue;

class ArrayValueEditor : public TCompositeValueEditor<ArrayValue, ExtendEditor>
{
protected:

	void onExtend();
};

class ArrayValuetype : public TCompositeValueEditor<ArrayValue, ArrayValueEditor, ArrayValueType>
{
	friend class ArrayValueEditor;

protected:
	ValueType* elementValueType;
};
*/

///////////////////////////////////////////////////////////////////////
//EventValueType
typedef const char* PCSTR;
class EventValueEditor : public ValueEditor
{
protected:
	void onSaveValue();

	int wndProc(int message, WPARAM wParam, LPARAM lParam)
	{
		if(message == MSG_KEYDOWN){
			if(wParam == SCANCODE_ENTER){
				bModified = TRUE;
				onSaveValue();
				return 0;
			}
		}
		return ValueEditor::wndProc(message, wParam, lParam);
	}

	void onNotify(LINT id, int nc, DWORD add_data){
		if(nc == CBN_SELCHANGE){
			bModified = TRUE;
			onSaveValue();
		}
		else if(nc == CBN_EDITCHANGE){
			bModified = TRUE;
		}
	}

	void insertFuncName(const char* szFuncName);

public:
	HWND create(HWND hParent, LINT id){
		return createWindow(hParent, id, "combobox", CBS_DROPDOWNLIST|CBS_NOTIFY|CBS_READONLY, 0);
	}
	BOOL init(Value value, ValueType* vtype, ValueUpdator * updator, DWORD mask=0);
};
class EventValueType : public TValueType<EventValueEditor, VT_EVENT>
{
	friend class EventValueEditor;
protected:
	string prototype; //the property of string
	string name;
	string code;
	string content;

	static mapex<string, EventValueType*,mystr_less> _instances;

	//check a func name is valid or not
	BOOL checkFuncName(const char* funcName);

	void parserHandler(const char* handler);

	string* idxOfName(const char* strName,int *pidx = NULL);

	list<string> historyNames; //the history names of function

	char* genFunction(const char* name, char* szFuncName);

public:

	EventValueType(xmlNodePtr node, const char* default_prototype = NULL);
	~EventValueType();

	const char* getName(){ return name.c_str(); }
	const char* getPrototype(){ return prototype.c_str(); }
	const char* getCode(){ return code.c_str(); }
	const char* getContent(){ return content.c_str();}

	string toString(Value value);

	DWORD toBinary(Value value){
		return (DWORD)value;
	}

	DWORD toRes(Value value){
		return (DWORD)value;
	}

	int addValueRef(Value value){
		return 1;
	}

	int releaseValue(Value value){
		return 1;
	}

	BOOL equal(Value value1, Value value2){
		return (value1 == value2 &&  value1!=0);
	}

	void saveXMLStream(Value value, TextStream *stream);

	DWORD toMemo(Value value){
		return (DWORD)value;
	}

	Value fromMemo(DWORD memo){
		return memo;
	}

	void freeMemo(DWORD memo){
	}

	Value newValue(Value value){
		return value;
	}

	Value newValue(const char* str);

	Value newValue(xmlNodePtr node);

	static ValueType* getInstance(const char* typeName){
		if(typeName == NULL)
			return NULL;
		return _instances.at(typeName);
	}

	static void cleanInstances()
	{
		for(mapex<string, EventValueType*, mystr_less>::iterator it = _instances.begin();
				it != _instances.end(); ++it)
		{
			if(it->second)
				it->second->release();
		}
		_instances.clear();

	}

};
#endif /* VALUETYPE_H_ */
