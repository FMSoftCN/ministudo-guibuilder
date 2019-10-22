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
#include <stdarg.h>
#include <string.h>

#include "mgheads.h"
#include "mgfcheads.h"

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mapex.h"
using namespace std;

#include "xmlheads.h"
#include "xmlhelp.h"

#include "log.h"
#include "undo-redo.h"

#include "stream.h"
#include "resenv.h"
#include "valuetype.h"

#include "class-instance.h"
#include "clipboard.h"

//static name
static mapex<Class*, int> _class_map_ref;

//////////////////////
//Class

FieldType * Class::getFieldType(int id)
{
	/*
	for(FieldTypeBlock * block = fieldTypeBlocks; block; block = block->next)
	{
		if(block->startId >= id && block->endId<id){
			return &block->fieldstypes[id-block->startId];
		}
	}*/

	FieldType * ft = fieldTypes.at(id);

	if(ft)
		return ft;

	return super?super->getFieldType(id):NULL;

}

FieldType * Class::getFieldType(const char* name)
{
	if(name == NULL)
		return NULL;
/*
	for(FieldTypeBlock * block = fieldTypeBlocks; block; block = block->next)
	{
		for(int i=0; i<block->endId - block->startId; i++){
			if(block->fieldstypes[i].name)
				continue;
			if(strcmp(block->fieldstypes[i].name, name) == 0)
				return &block->fieldstypes[i];
		}
	}
	*/

	FieldType * ft = namedFieldTypes.at(name);
	if(ft)
		return ft;

	return super?super->getFieldType(name):NULL;
}

//get the Value of Field
ValueType * Class::getFieldValueType(int id)
{
	FieldType * ft = getFieldType(id);
	if(ft)
		return ft->vtype;
	return NULL;
}

int Class::getIDValueType(int id)
{
	ValueType *vt = getFieldValueType(id);
	if (vt)
		return vt->getType();

	return VT_UNKNOWN;
}

void Class::getRangedDefaultValue(int begin_id, int end_id,map<int,Field*> & values)
{
	if(begin_id > end_id)
		return ;

	if(super)
		super->getRangedDefaultValue(begin_id, end_id,values);

	mapex<int,Field*>::iterator it = defaultFields.lower_bound(begin_id);
	while(it != defaultFields.end())
	{
		if(it->first > end_id)
			break;
		values[it->first] = it->second;
		++it;
	}
}

ValueType * Class::getFieldValueType(const char* name)
{
	FieldType * ft = getFieldType(name);
	if(ft)
		return ft->vtype;
	return NULL;
}
//get Default Value
Value Class::getFieldDefault(int id)
{
	Field * field = defaultFields.at(id);
	if(field)
		return field->value;
	return super?super->getFieldDefault(id): 0;
}

BOOL Class::isDefaultFieldExist(int id)
{
	if(defaultFields.find(id) != defaultFields.end())
		return TRUE;
	return super?super->isDefaultFieldExist(id):FALSE;
}
//get FieldDefaultAttr
DWORD Class::getFieldDefaultAttr(int id)
{
	Field * field = defaultFields.at(id);
	if(field)
		return field->attr;
	return super?super->getFieldDefaultAttr(id): 0;
}

//Id to Name
const char* Class::getFieldName(int id)
{
	FieldType * ft = getFieldType(id);
	if(ft)
		return ft->name.c_str();
	return NULL;
}

int Class::getFieldId(const char* name)
{
	FieldType * ft = getFieldType(name);
	if(ft)
		return ft->id;
	return -1;
}

//for ResEditor, to enum fields
BOOL Class::enumFields( FieldEnumHandler *handler, BOOL enumSupper, DWORD dwUser)
{
	if(handler == NULL)
		return FALSE;

	for(map<int, FieldType*>::iterator it=fieldTypes.begin(); it != fieldTypes.end(); ++ it)
	{
		FieldType* ft = it->second;
		DP("id=%d, ft=%p",it->first,ft);
		if(ft == NULL)
			continue;
		handler->setField(this, ft->id, ft->name.c_str(), ft->vtype, dwUser);
	}

	if(enumSupper && super)
		super->enumFields(handler, TRUE,dwUser);

	return TRUE;
}

Class::~Class()
{
	for(map<int, Field*>::iterator it = defaultFields.begin(); it != defaultFields.end(); ++it)
	{
		Field *f = it->second;
		if(f){
			FieldType *ft = fieldTypes.at(f->id);
            if (ft) {
                ft->vtype->releaseValue(f->value);
            }
			delete f;
		}
	}

	for(map<int, FieldType*>::iterator it = fieldTypes.begin(); it!=fieldTypes.end(); ++it)
	{
		FieldType *ft = it->second;
        delete ft;
	}

	if(super)
		super->release();
}

Class* Class::loadFromXML(xmlNodePtr node)
{
	int prop_id = 0;
	int event_id = 0;
	char* def_prototype = NULL;
	string full_name;

	if(node == NULL)
		return NULL;

	//get rootNodeName
	const xmlChar* xrootNodeName = node->name;
	//get class
	xmlChar* xclass = xmlGetProp(node,(const xmlChar*)"class");
	if(xclass == NULL)
	{
		fprintf(stderr, "ERROR:Load Class>> cannot find \"class\" property in root node\n");
		return NULL;
	}

	//get extend
	xmlChar* xextends = xmlGetProp(node,(const xmlChar*)"extends");

	//get accept
	xmlChar* xaccept = xmlGetProp(node, (const xmlChar*)"accept");

	DPRINT("--- load class:%s : %s\n",xclass, xextends);

	Class* super = NULL;

	if(xextends)
		super = Class::getClassByName((const char*)xrootNodeName, (const char*)xextends);

	Class *_class = new Class((const char*)xclass,super);

	if(_class == NULL){
		fprintf(stderr, "ERROR:Load Class>> cannot create class object \"%s\" extends \"%s\" \n", (const char*)(xclass), (xextends?(const char*)xextends:"<Object>"));
		goto FAILED;
	}

	if(xaccept){
		_class->bContainer = xmlStrcmp(xaccept,(const xmlChar*)"true")==0?TRUE:FALSE;
		xmlFree(xaccept);
	}
	else
	{
		_class->bContainer = super?super->isContainer():FALSE;
	}

	//get property of Class
	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp(node->name, (const xmlChar*)"property") == 0){
			loadClassProperty(node, _class, prop_id);
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"event") == 0){
			loadClassEvent(node, _class, event_id, &def_prototype);
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"set-event") == 0) {
			loadClassDefEvent(node, _class, FIELD_ATTR_FIXED) ;
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"set-property") == 0){
			loadClassDefProperty(node, _class, FIELD_ATTR_FIXED);
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"default-property") == 0){
			loadClassDefProperty(node, _class, 0);
		}
	}

	//insert into classMaper

	full_name = (const char*)xrootNodeName;
	full_name += "::";
	full_name += (const char*)xclass;
	//printf("---- Load class:%s\n", full_name.c_str());
	_classMaper[full_name] = _class;

	goto RETURN;

FAILED:
	delete _class;

RETURN:
	if(xextends)
		xmlFree(xextends);

	if(xclass)
		xmlFree(xclass);

	return _class;
}

void Class::addFieldType(const char* name,int id, ValueType* vtype, const char* error_tip /*= NULL*/)
{
	//create new Field Type
	FieldType * ft = new FieldType(id, name, vtype, error_tip);

	DP( "id=%d, ft=%p", id, ft);
	//insert into
	fieldTypes[id] = ft;
	namedFieldTypes[ft->name] = ft;
}

BOOL Class::loadClassProperty(xmlNodePtr node, Class *_class, int& prop_id)
{
	//get id
	DWORD defValue = 0;
	BOOL haveDef = FALSE;
	int id = xhGetIntProp(node, "id", -1);
	if(id == -1)
	{
		prop_id ++;
	}
	else
		prop_id = id;

	//get Name
	xmlChar* xname = xmlGetProp(node, (const xmlChar*)"name");
	if(xname == NULL)
		return FALSE;

	//get type
	xmlChar* xtype = xmlGetProp(node, (const xmlChar*)"type");
	if(xtype == NULL)
		return FALSE;

	ValueType * vtype = ValueType::getValueType((const char*)xtype);

	if(vtype == NULL)
	{
		//try load as composite type
		xmlNodePtr child = xhGetChild(node, (const char*)xtype);
		if(child)
			vtype = createCompositeValueType(child);
	}

	xmlFree(xtype);

	if(vtype == NULL){
		xmlFree(xname);
		return FALSE;
	}

	//get default value
	xmlChar* xdefault = xmlGetProp(node, (const xmlChar*)"default");
	if(xdefault){
		defValue = vtype->newValue((const char*)xdefault);
		haveDef = TRUE;
		xmlFree(xdefault);
	}
	else {
		xmlNodePtr nodeDef = xhGetChild(node, "default");
		if(nodeDef){
			defValue = vtype->newValue(nodeDef);
			haveDef = TRUE;
		}
	}

	xmlChar * error_tip = xhGetChildText(node,  "error-tip");

	_class->addFieldType((const char*)xname,prop_id, vtype, (const char*)error_tip);

	if(error_tip)
		xmlFree(error_tip);

	if(haveDef)
	{
		//insert defvalue
		Field *field = new Field;
		field->id = prop_id;
		field->attr = 0;
		field->value = defValue;
		_class->defaultFields[prop_id] = field;
	}

	xmlFree(xname);

	return TRUE;
}
/*
BOOL Class::loadClassMessage(xmlNodePtr node, Class *_class, int& msg_id)
{
	//get id of msg
	//TODO message
	int id;
	//get the id of msg
	id = xhGetIntProp(node, "id", -1);

	if(id == -1)
		id = ++msg_id;
	else
		msg_id = id;

	//get MessageValueType
	MessageValueType * vtype = new MessageValueType(node);

	_class->addFieldType(vtype->getName(),id, vtype);

	return TRUE;
}*/

/*
BOOL Class::loadClassNotify(xmlNodePtr node, Class *_class, int& msg_id)
{
	//TODO Notify
	int id;
	//get the id of msg
	id = xhGetIntProp(node, "id", -1);

	if(id == -1)
		id = ++msg_id;
	else
		msg_id = id;

	//get MessageValueType
	NotifyValueType * vtype = new NotifyValueType(node);

	_class->addFieldType(vtype->getName(),id, vtype);

	return TRUE;
}*/
BOOL Class::loadClassEvent(xmlNodePtr node, Class *_class, int& event_id, char **default_prototype)
{
	int id;
	//get the id of msg
	id = xhGetIntProp(node, "id", -1);

	if(id == -1)
		id = ++event_id;
	else
		event_id = id;

	//get EventValueType
	EventValueType * vtype = new EventValueType(node, *default_prototype);

	if(!vtype)
		return FALSE;

	*default_prototype = (char*)vtype->getPrototype();

	xmlChar* error_tip = xhGetChildText(node, "error-tip");
	_class->addFieldType(vtype->getName(),id, vtype, (const char*)error_tip);
	if(error_tip)
		xmlFree(error_tip);

	return TRUE;
}

BOOL Class::loadClassDefProperty(xmlNodePtr node, Class *_class, uint8_t attr)
{
	//process set-property
	//get Name
	DWORD value = 0;
	xmlChar* xname = xmlGetProp(node, (const xmlChar*)"name");
	if(xname == NULL)
		return FALSE;

	FieldType * ft = _class->getFieldType((const char*)xname);

	xmlFree(xname);

	if(ft == NULL)
		return FALSE;

	//get Value
	xmlChar *xvalue = xmlGetProp(node, (const xmlChar*)"value");
	if(xvalue){
		value = ft->vtype->newValue((const char*)xvalue);
		xmlFree(xvalue);
	}
	else
	{
		xmlNodePtr nodeValue = xhGetChild(node, "value");
		if(nodeValue == NULL)
			return FALSE;
		value = ft->vtype->newValue(nodeValue);
	}

	Field *field = _class->defaultFields.at(ft->id);

	if(field){
		ft->vtype->releaseValue(field->value);
		field->value = value;
		field->attr = attr;
	}
	else
	{
		field = new Field;
		field->id = ft->id;
		field->value = value;
		field->attr = attr;
		_class->defaultFields[ft->id] = field;
	}

	return TRUE;

}

BOOL Class::loadClassDefEvent(xmlNodePtr node, Class* _class, int attr)
{
	int id = xhGetIntProp(node, "id");
	
	FieldType *ft = _class->getFieldType(id);

	if(!ft)
		return FALSE;

	Field *field = _class->defaultFields.at(ft->id);

	if(field){
		field->attr = attr;
	}
	else
	{
		field = new Field;
		field->id = ft->id;
		field->value = 0;
		field->attr = attr;
		_class->defaultFields[ft->id] = field;
	}

	return TRUE;

}

void Class::unloadAllClasses()
{
    for (map<string, Class*>::iterator it = Class::_classMaper.begin();
            it != Class::_classMaper.end(); ++it)
    {
		Class *clss = it->second;
		if(clss)
			clss->release();
    }
}

//the name of class must be "rootNode::className"
//e.g "winodw::widget", "render::classic" and so on
mapex<string, Class*> Class::_classMaper;
Class* Class::getClassByName(const char* rootNodeName, const char* className)
{
	string full_class_name;
	if(className == NULL)
		return NULL;

	if(strstr(className,"::"))
	{
		full_class_name = className;
	}
	else
	{
		if(rootNodeName == NULL)
			return NULL;

		full_class_name = rootNodeName;
		full_class_name += "::";
		full_class_name += className;
	}

	return _classMaper.at(full_class_name);
}

string Class::newClassName(Class* cls)
{
	if(cls == NULL)
		return string("");

	char szName[100];
	const char* strName = cls->getClassName();
	if(strName == NULL)
		return string("");

	const char* str = strchr(strName,':');
	if(str){
		while(*str == ':') str++;
		strName = str;
	}

	_class_map_ref[cls] ++;
	int ref = _class_map_ref.at(cls);

	sprintf(szName, "ID_%s%d", strName, ref);
	//upercase it
	for(int i=0; szName[i]; i++)
		szName[i] = toupper(szName[i]);

	return string(szName);
}

int Class::classRefCount()
{
	return _class_map_ref.at(this);
}

////////////////
//Instance

Instance::Instance(Class* cls)
{
	notification = NULL;
	id = -1;
	_class = cls;
	bLocked = FALSE;
	//gen a default serial
	serialNum = genSerialNum(this);
	ref_list = NULL;

	_class->addRef();
}

Instance::Instance(InstanceNotificationHandler* notification, Class* cls, int id)
{
	this->notification = notification;
	_class = cls;
	id = id;
	bLocked = FALSE;
	//TODO release ref_list
	ref_list = NULL;
}

int Instance::getIDValueType(int id)
{
	return _class->getIDValueType(id);
}

//get Field value
Value Instance::getField(int id)
{
	Field *field = fields.at(id);
	if(field)
		return field->value;
	return _class->getFieldDefault(id);
}

BOOL Instance::isFieldExist(int id)
{
	if(fields.find(id) != fields.end())
		return TRUE;

	return _class->isDefaultFieldExist(id);
}
//set Field value
void Instance::setField(int id, Value value, BOOL bAutoUse/*=FALSE*/)
{
	Value old_value = 0;
	if(bLocked)
		return ;

	FieldType *ft = _class->getFieldType(id);
	if(ft == NULL)
		return ;

	Field *field = fields.at(id);
	if(field){
		if(field->value == value) //do not set
			return;

		old_value = field->value;
		ft->vtype->releaseValue(field->value);
		field->value = ft->vtype->newValue(value);
	}
	else
	{
		field = new Field;
		field->id = ft->id;
		field->attr = 0;
		field->value = value;
		ft->vtype->addValueRef(value);
		fields[id] = field;
	}

	//TODO Add into Ref_list
	int ivt = ft->vtype->getType();
	if(ivt == VT_RDR || ivt== VT_IMAGE || ivt == VT_TEXT)
	{
		//auto use ref
		if(bAutoUse && ivt != VT_TEXT) //Instance always use one text
		{
			ResManager* resMgr = g_env->getResManager(ID2TYPE(value));
			if(resMgr){
				if(old_value > 0)
					resMgr->unuse((int)old_value);
				resMgr->use((int)value);
			}
		}
		//TODO Is in the ref_list
		if(isInRefList(field))
			return;
		//TODO add into ref_list
		addRefList(field);
	}
}

void Instance::cleanField(int id, BOOL focuse_clean)
{
	if(!focuse_clean && !enableClean(id))
		return ;

	mapex<int, Field*>::iterator it = fields.find(id);
	if(it != fields.end())
	{
		Field* f = it->second;
		removeRefList(f);
		//release value
		ValueType* vt = _class->getFieldValueType(id);
		if(vt)
			vt->releaseValue(f->value);
		delete f;
		fields.erase(it);
	}
}

BOOL Instance::removeRefList(Field* field)
{
	if(!ref_list)
		return FALSE;

	RefResFieldItem *res_ref = ref_list;
	RefResFieldItem* rtmp = NULL;
	ResManager* resMgr = NULL ;
	FieldType* ft;

	if(field == ref_list->field)
	{
		ref_list = ref_list->next;
		rtmp = res_ref;
		goto DELETE;
	}

	while(res_ref->next && res_ref->next->field != field)
		res_ref = res_ref->next;

	if(res_ref->next)
	{
		rtmp = res_ref->next;
		res_ref->next = rtmp->next;
		goto DELETE;
	}
	return FALSE;

DELETE:
	if(!rtmp)
		return FALSE;

	ft = _class->getFieldType(rtmp->field->id);
	if(ft && ft->vtype->getType()!=VT_TEXT)
	{
		resMgr = g_env->getResManager(ID2TYPE(rtmp->field->id));
		if(resMgr)
		{
			resMgr->unuse(id);
		}
	}

	delete rtmp;

	return TRUE;
}

BOOL Instance::isInRefList(Field *field)
{
	BOOL bexist = FALSE;
	RefResFieldItem *res_ref = ref_list;

	while(res_ref)
	{
		if(res_ref->field == field)
			bexist = TRUE;
		res_ref = res_ref->next;
	}

	return bexist;
}

BOOL Instance::addRefList(Field *field)
{
	RefResFieldItem *res_ref;

	res_ref = new RefResFieldItem;
	res_ref->field = field;
	res_ref->next = ref_list;
	ref_list = res_ref;

	return TRUE;
}

BOOL Instance::onResIdChanged(int res_old_id, int res_new_id,int* updateFields, int max)
{
	BOOL bchanged = FALSE;
	RefResFieldItem * res_ref = ref_list;
	int idx = 0;

	while(res_ref)
	{
		if((int)res_ref->field->value == (res_old_id))
		{
			res_ref->field->value = res_new_id;
			bchanged = TRUE;
			if(updateFields && idx < max)
				updateFields[idx++] = res_ref->field->id;
		}
		res_ref = res_ref->next;
	}

	if(updateFields)
		updateFields[idx] = 0; //exist

	return bchanged;
}

int Instance::getReferencedFieldIds(int res_id, int* fieldIds, int max)
{
	int idx = 0;
	RefResFieldItem * res_ref = ref_list;

	if(!fieldIds || max <= 0)
		return 0;

	while(res_ref && idx < max)
	{
		if(res_ref->field && (int)res_ref->field->value == res_id)
			fieldIds[idx++] = res_ref->field->id;
		res_ref = res_ref->next;
	}

	return idx;
}

BOOL Instance::loadProperty(xmlNodePtr node)
{
	//get property type
	FieldType * ft = NULL;

	if(xmlStrcmp(node->name,(const xmlChar*)"message") == 0
			||  xmlStrcmp(node->name, (const xmlChar*)"notify") == 0)
	{
		//get by handler
		xmlChar* xhandler = xhGetChildText(node,"handler");
		ft = _class->getFieldType((const char*)xhandler);
		xmlFree(xhandler);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"ID") == 0){
		xmlChar* xstr = xhGetNodeText(node);
		if(xstr){
			id = strtoul((const char*)xstr,NULL, 0);
			xmlFree(xstr);
		}
		else {
			id = -1;
		}

		if(id != -1){
			//get name
			xstr = xmlGetProp(node, (const xmlChar*)"name"); //get Id Name
			//Add ID Name to Res
			ResManager* resMgr = g_env->getResManager(ID2TYPE(id));
			if(resMgr){
				id = resMgr->createRes(ID2TYPE(id),(const char*)xstr, id, NULL, (DWORD)this);
			}
			if(xstr)
				xmlFree(xstr);
		}
		else
		{
			//get name
			int type = xhGetIntProp(node, "type");
			if(type > 0)
			{
				xstr = xmlGetProp(node, (const xmlChar*)"name"); //get Id Name
				ResManager* resMgr = g_env->getResManager(type);
				if(resMgr){
					char newname [256];
					strcpy(newname, xstr ?(const char *)xstr:"ID_");
					while(!resMgr->isValidName(newname)) //exist
					{
						//new a name
						int len = strlen(newname);
						if (isdigit(newname[len]))
							newname[len]++;
						else
							strcat(newname, "0");
					}
					id = resMgr->createRes(NCSRT_CONTRL, newname, id, NULL, (DWORD)this);
				}
				if(xstr)
					xmlFree(xstr);
			}
		}
		return TRUE;
	}
	else
		ft = _class->getFieldType((const char*)node->name);
	if(ft == NULL)
		return FALSE;

	//get attribute
	//is read only
	uint8_t attr = (uint8_t)xhGetBoolProp(node, "hide", Class::FIELD_ATTR_HIDE);
	attr |= (uint8_t)xhGetBoolProp(node, "readonly", Class::FIELD_ATTR_FIXED);

	DWORD value = ft->vtype->newValue(node);

	//new field
	Field *field = new Field;
	field->id = ft->id;
	field->attr = attr;
	field->value = value;

	fields[ft->id] = field;

	//TODO Add into Ref_list
	int ivt = ft->vtype->getType();
	if(ivt == VT_RDR || ivt== VT_IMAGE || ivt == VT_TEXT)
	{
		//TODO Is in the ref_list
		if(isInRefList(field))
			return TRUE;
		//TODO add into ref_list
		addRefList(field);
	}

	DPRINT("ValueType=%p", ft->vtype);
	DPRINT("Property Name=%s value=%s", ft->name.c_str(), ft->vtype->toString(value).c_str());

	return TRUE;
}

BOOL Instance::loadMessage(xmlNodePtr node)
{
	//TODO: load message
	//get the id of
	return TRUE;
}

BOOL Instance::loadListen(xmlNodePtr node)
{
	if(node == NULL)
		return FALSE;

	int sender = 0;
	int event = 0;
	xmlChar *prototype = NULL;
	/*
	 * <listen>
	 * 	<sender>[sender-id]</sender>
	 * 	<event>[event-id]</event>
	 * 	<prototype>[C Function property Name]</prototype>
	 * </listen>
	 *
	 */
	for(node=node->xmlChildrenNode; node; node = node->next)
	{
		if(xhIsNode(node,"sender"))
		{
			sender = xhGetNodeInt(node);
		}
		else if(xhIsNode(node,"event"))
		{
			event = xhGetNodeInt(node);
		}
		else if(xhIsNode(node,"prototype"))
		{
			prototype = xhGetNodeText(node);
		}
	}

	if(!addListen(sender, event, (const char*)prototype))
	{
		xmlFree(prototype);
		return TRUE;
	}

	if(prototype)
		xmlFree(prototype);
	return FALSE;
}

void Instance::loadSerial(xmlNodePtr node)
{
	if(node == NULL)
		return ;

	//get from prop
	xmlChar* xstr = xmlGetProp(node, (const xmlChar*)"serial-number");
	if(xstr){
		serialNum = (DWORD)strtoul((const char*)xstr, NULL, 10);
		xmlFree(xstr);
	}
}

void Instance::saveSerial(TextStream* stream)
{
	if(stream)
	{
		stream->printf(" serial-number=\"%u\"",serialNum);
	}
}

void Instance::copyFields(mapex<int, Field*> &newFields) const
{
	//copy Fields

	for(map<int, Field*>::const_iterator it = fields.begin(); it != fields.end(); ++it)
	{
		Field * field = new Field;
		*field = *(it->second);
		newFields[field->id] = field;
		ValueType *vt = _class->getFieldValueType(it->first);
		if(vt)
		{
			field->value = vt->newValue(field->value);
		}
	}

}

void Instance::copyFieldValues(mapex<int, DWORD> &newFieldvalues, int begin/*=-1*/, int end/*=-1*/) const
{
	if(begin <= 0)
		begin = 0;
	if(end <= 0)
		end = 0x7FFFFFFF;

	for(map<int,Field*>::const_iterator it = fields.begin(); it != fields.end() && begin < end; ++it, ++begin )
	{
		const Field * ft = it->second;
		ValueType *vt = _class->getFieldValueType(it->first);
		newFieldvalues[it->first] = ft->value;
		if(vt)
			 vt->addValueRef(ft->value);
	}
}

Instance::RefResFieldItem *Instance::copyRefFieldList(mapex<int, Field*> &newFields) const
{
	//TODO copy ref_list
	RefResFieldItem * res_ref = ref_list;
	RefResFieldItem * new_res_list = NULL;

	while(res_ref)
	{
		Field *field = newFields.at(res_ref->field->id);
		if(field)
		{
			RefResFieldItem * ritem = new RefResFieldItem;
			ritem->field = field;
			ritem->next = new_res_list;
			new_res_list = ritem;
		}
		res_ref = res_ref->next;
	}
	return new_res_list;
}

Instance::~Instance()
{
	//removeRefReses();
	clearListens();
	cleanAll(-1,-1, TRUE);
	if(_class)
		_class->release();
}

void Instance::cleanAll(int begin_id, int end_id, BOOL force_clean/*=FALSE*/)
{
	BOOL delAll = FALSE;
	if ((begin_id == -1) && (end_id == -1)) {
		delAll = TRUE;
	}

	if(_class){
		for(map<int, Field*>::iterator it = fields.begin(); it!=fields.end(); ++it)
		{
			Field *f = it->second;
			if(f){
				if (delAll) {
					FieldType *ft = _class->getFieldType(f->id);
					if(ft && (force_clean || enableClean(ft)))
						ft->vtype->releaseValue(f->value);
					delete f;
				}
				else {
					//delete specified property
					if ((f->id >= begin_id) && (f->id <= end_id)) {
						cleanField(it->first);
					}
				}
			}
		}
	}
	if (delAll) {
		fields.clear();
		while(ref_list)
		{
			RefResFieldItem* res_ref = ref_list;
			ref_list = ref_list->next;
			delete res_ref;
		}
		ref_list = NULL;
	}
}

void Instance::removeRefReses()
{
	RefResFieldItem* res_ref = ref_list;
	int old_id = -1;
	ResManager* resMgr = NULL;
	for(;res_ref;res_ref = res_ref->next)
	{
		if(res_ref->field->value != old_id){
			ResManager* resMgrtmp = g_env->getResManager(ID2TYPE(res_ref->field->value));
			if(!resMgrtmp)
				continue;
			resMgr = resMgrtmp;
			old_id = (int)res_ref->field->value;
		}
		if(resMgr)
			resMgr->removeRes(res_ref->field->value);
	}
}

void Instance::getRangedFields(int begin_id, int end_id, map<int, Field*> & values)
{
	if(begin_id > end_id)
		return;

	_class->getRangedDefaultValue(begin_id, end_id, values);

	mapex<int,Field*>::iterator it = fields.lower_bound(begin_id);
	while(it != fields.end())
	{
		if(it->first > end_id)
			break;
		values[it->first] = it->second;
		++it;
	}
}

DWORD Instance::genSerialNum(Instance *inst)
{
	DWORD magic = 0x34A5B10E;

	srand(time(NULL));

	if(inst)
	{
		magic = ((DWORD)inst) ^ ((DWORD)(inst->getClass()));
	}

	DWORD r = rand();

	DWORD seria = 0;

	for(int i=0; i<4; i++)
	{
		int rp = r&0xFF;
		int mp = (magic&0xFF000000)>>24;
		if(rp == 0)
			rp = 1;
		if(mp == 0)
			mp = rand()%255;

		seria |= (mp*rp);
		seria <<= 8;
		r >>= 8;
		magic <<= 8;
	}

	return seria;

}

//listen support
ListenEntry* Instance::addListen(int sender, int event_id, const char* prototype)
{
	if(sender <= 0 || event_id < 0 || prototype == NULL)
		return NULL;

	for(list<ListenEntry*>::iterator it = listens.begin(); it != listens.end(); it++)
	{
		ListenEntry* le = *it;
		if(le==NULL)
			continue;
		if(le->event_id == event_id && le->sender_id == sender){
			le->prototype = prototype;
			return le;
		}
	}

	ListenEntry * le = new ListenEntry;
	le->sender_id = sender;
	le->event_id = event_id;
	le->prototype = prototype;

	listens.push_back(le);

	return le;
}

BOOL Instance::removeListen(int sender, int event_id)
{
	//remove all
	if(sender==-1){
		return clearListens();
	}

	list<ListenEntry*>::iterator it = listens.begin();
	while(it!=listens.end())
	{
		ListenEntry* le = *it;
		if(le && le->sender_id == sender && (event_id==-1 || event_id==le->event_id))
		{
			list<ListenEntry*>::iterator itt = it;
			it ++;
			delete le;
			listens.erase(itt);
			continue;
		}
		it ++;
	}
	return TRUE;
}

const char* Instance::getListenPrototype(int sender, int event_id)
{
	for(list<ListenEntry*>::iterator it = listens.begin(); it != listens.end(); ++it)
	{
		ListenEntry* le = *it;
		if(!le)
			continue;

		if(le->sender_id == sender && le->event_id == event_id)
			return le->prototype.c_str();
	}
	return NULL;
}

BOOL Instance::saveListens(TextStream *stream)
{
	for(list<ListenEntry*>::iterator it = listens.begin(); it != listens.end(); ++it)
	{
		ListenEntry* le = *it;
		if(!le)
			continue;
		stream->println("<listen>");
		stream->indent();
			stream->println("<sender>%d</sender>", le->sender_id);
			stream->println("<event>%d</event>",le->event_id);
			stream->println("<prototype>%s</prototype>",le->prototype.c_str());
		stream->unindent();
		stream->println("</listen>");
	}
	return TRUE;
}

BOOL Instance::clearListens()
{
	for(list<ListenEntry*>::iterator it = listens.begin(); it != listens.end(); it++)
	{
		ListenEntry* le = *it;
		if(le==NULL)
			continue;
		delete le;
	}
	return TRUE;
}

void Instance::incUseOfRefReses()
{
	ResManager *resMgr = NULL;
	for(RefResFieldItem* res_ref = ref_list; res_ref; res_ref = res_ref->next)
	{
		resMgr = g_env->getResManager(ID2TYPE(res_ref->field->value));
		if(!resMgr)
			continue;
		resMgr->use(res_ref->field->value,FALSE);
	}
}

void Instance::decUseOfRefReses()
{
	ResManager *resMgr = NULL;
	for(RefResFieldItem* res_ref = ref_list; res_ref; res_ref = res_ref->next)
	{
		resMgr = g_env->getResManager(ID2TYPE(res_ref->field->value));
		if(!resMgr)
			continue;
		resMgr->unuse(res_ref->field->value,FALSE);
	}
}

///////////////////////////////////////////
//Instance support

static ObjectClipBoard<Instance*> _instance_clipboard;

BOOL Instance::copy(Instance** instances, int count)
{
	return _instance_clipboard.push(instances, count) > 0;
}

BOOL Instance::cut(Instance** instances, int count)
{
	return copy(instances, count);
}

ObjectClipBoard<Instance*>::Array Instance::paste()
{
	return _instance_clipboard.top(0);
}


void Instance::autoStoreIdName()
{
	if(id > 0){
		ResManager* resMgr = g_env->getResManager(ID2TYPE(id));
		if(resMgr)
		{
			strIDName = resMgr->idToName(id);
			return ;
		}
	}

	strIDName.clear();

}

void Instance::autoRegisterID()
{
	if(id <= 0)
		return ;

	ResManager* resMgr = g_env->getResManager(ID2TYPE(id));
	if(resMgr)
	{
		if(strIDName.size() <= 0)
			strIDName = newName();
		id = resMgr->createRes(ID2TYPE(id), strIDName.c_str(),id, NULL, (DWORD)this);
	}
}

////////////////
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

void Instance::setPropsToLuaTable(void* luaState, int prop_begin, int prop_end)
{
	lua_State* L = (lua_State*)luaState;

	lua_newtable(L);
	int prop_table = lua_gettop(L);
	//save props
	for(map<int,Field*>::iterator it = fields.lower_bound(prop_begin);
		it != fields.end() && it->first<=prop_end; ++it)
	{
		Field* f = it->second;
		FieldType *ft = _class->getFieldType(it->first);

		//prop_table.id = value
		lua_pushinteger(L, it->first);
		if(!inst_push_value_type(L,prop_table, f->value, ft->vtype))
			lua_pushinteger(L, 0);
		lua_settable(L, prop_table);
	}
}

int Instance::inst_push_value_type(void* luaState, int table_idx, Value value, ValueType *vt)
{
	if(!vt)
		return 0 ;

	lua_State* L = (lua_State*)luaState;

	switch(vt->getType())
	{
	case VT_INT:
	case VT_COLOR:
		//SET_TABLE(L, table_idx, value, lua_pushinteger(L, (lua_Integer)value));
		lua_pushinteger(L, (lua_Integer)value);
		break;
	case VT_STRING:
	case VT_TEXT:
	case VT_IMAGE:
	case VT_FILE:
		//SET_TABLE(L, table_idx, value, lua_pushstring(L,(const char*)vt->toBinary(value)));
		lua_pushstring(L,(const char*)vt->toBinary(value));
		break;
	case VT_ENUM:
		{
			EnumValueType *evt =  (EnumValueType*)vt;
			ValueType* opt_vt = evt->getOptionValueType();
			if(opt_vt)
				return inst_push_value_type(L, table_idx, value, opt_vt);
			return 0;
		}
	case VT_RDR://save id
		//SET_TABLE(L, table_idx, value, lua_pushinteger(L,(lua_Integer)value));
		lua_pushinteger(L,(lua_Integer)value);
		break;
	case VT_STRUCT:
	{
		StructValueType *svt = (StructValueType*)vt;
		StructValue *sv = (StructValue*)value;
		//new table
		lua_newtable(L);
		int struct_table = lua_gettop(L);
		Value *values = sv->getValues();
		for(int i=0; i<svt->getElementCount(); i++)
		{
			//set name of element
			const StructValueType::Element *e = svt->getElement(i);
			if(!e)
				continue;
			lua_pushstring(L, (const char*)e->name);
			inst_push_value_type(L, struct_table,values[i], e->vtype);
			lua_settable(L, struct_table);
		}
		break;
	}
	case VT_ARRAY:
	case VT_FONT:
	case VT_BINARY:
		return 0;
	}
	return 1;
}


//////////////////////////////////////////////////////////////
//InstancePropertyUndoRedoCommand
InstancePropertyUndoRedoCommand::InstancePropertyUndoRedoCommand(Instance * inst, int prop_id, Value value,BOOL bIsDefault)
{
	if(!inst)
		throw("InstancePropertyUndoRedoCommand cannot accept inst as NULL");
	this->inst = inst;
	this->prop_id = prop_id;
	old_value = 0;
	old_is_default = bIsDefault;
	if(!old_is_default){
		ValueType *vtype = inst->getClass()->getFieldValueType(prop_id);
		if(vtype)
			old_value = vtype->toMemo(value);
	}
}

InstancePropertyUndoRedoCommand::~InstancePropertyUndoRedoCommand()
{
	ValueType * vtype = inst->getClass()->getFieldValueType(prop_id);
	if(vtype)
		vtype->freeMemo(old_value);
}

void InstancePropertyUndoRedoCommand::execute()
{
	ValueType * vtype = inst->getClass()->getFieldValueType(prop_id);
	if(!vtype)
		return;

	BOOL is_default = !inst->isSettedField(prop_id);
	DWORD now_memo;
	if(!is_default){
		DWORD now_value = inst->getField(prop_id);
		now_memo = vtype->toMemo(now_value);
	}

	if(old_is_default)
	{
		inst->cleanField(prop_id); //cleanField auto unuse
	}
	else
	{
		DWORD value = vtype->fromMemo(old_value);
		inst->setField(prop_id, value,TRUE); //add use ref
	}
	old_is_default = is_default;
	old_value = now_memo;
}

/////////////////////////////////////////////////////////////
InstanceAllDefPropertiesRedoUndoCommand::InstanceAllDefPropertiesRedoUndoCommand(Instance *inst, int begin/*=-1*/, int end/*=-1*/)
{
	if(!inst)
		throw("InstanceAllDefPropertiesRedoUndoCommand cannot accept inst as NULL");
	this->inst = inst;
	is_undo = TRUE;
	inst->copyFieldValues(field_values, begin, end);
}

InstanceAllDefPropertiesRedoUndoCommand::~InstanceAllDefPropertiesRedoUndoCommand()
{
	Class *cls = inst->getClass();
	for(mapex<int, DWORD>::iterator it = field_values.begin();
		it != field_values.end();
		++ it)
	{
		ValueType *vtype = cls->getFieldValueType(it->first);
		if(vtype)
			vtype->releaseValue(it->second);
	}
}

void InstanceAllDefPropertiesRedoUndoCommand::execute()
{
	if(is_undo) //reset old values
	{
		for(mapex<int, DWORD>::iterator it = field_values.begin();
			it != field_values.end();
			++ it)
		{
			inst->setField(it->first, it->second);
		}
		is_undo = FALSE;
	}
	else
	{
		inst->cleanAll();
		is_undo = TRUE;
	}
}
////////////////////////////////////////////////////////////
