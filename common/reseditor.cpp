/*
 * reseditor.cpp
 *
 *  Created on: 2009-3-20
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <string>
#include <map>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "stream.h"
#include "resenv.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"
#ifdef WIN32
#include "func-win.h"
#endif

ResEditor::ResEditor()
{
	notification = NULL;
	bIdChanged = FALSE;
}

BOOL ResEditor::Create(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification)
{
	//create Window
	if(!TMGStaticSubclass<MGStatic>::Create(NULL, hParent, 0, 0, RECTW(g_rcScr), RECTH(g_rcScr), 0))
		return FALSE;

	Subclass();

	this->notification = notification;

	layout.SetAttachedWnd(m_hWnd);
	layout.load(resEditorPanelLayout,this);

	return initEditor();
}

ResEditor::~ResEditor()
{
	Resource *res;
	map<int, Resource*>::iterator it;

	for(it = reses.begin(); it != reses.end(); ++it){
		res = it->second;
        delete res;
	}
}
/*
 *  <support-type>
 *    <type name="" value=""/>
 *     ...
 *  </support-type>
 */
BOOL ResEditor::loadTypeInfo(xmlNodePtr typeNode)
{
#if 0
	//get child count
	resTypeCount = xhGetChildCount(typeNode, "type");
	if(resTypeCount == 0)
		return TRUE;

	resTypes = new int[resTypeCount];
	resNames = new char*[resTypeCount];

	int idx = 0;
	for(xmlNodePtr child = typeNode->xmlChildrenNode; child; child = child->next){
		if(!xhIsNode(child, "type"))
			continue;
		resTypes[idx] = xhGetIntProp(child, "value");
		xmlChar* xstr = xmlGetProp(child, (const xmlChar*)"name");
		if(xstr){
			resNames[idx] = strdup((const char*)xstr);
			xmlFree(xstr);
		}
		idx ++;
	}
#endif
	return TRUE;

}

int ResEditor::createRes(int type, const char* name, int id, const char* source, DWORD init_res)
{
	Resource * res;

	//get id
	if(id <= 0 || ID2TYPE(id) != type) {
        //first, check name.
        if (name && isValidName(name) == FALSE) {
            char note[256];
            sprintf (note, "Resource name[%s] have been used.\n", name);
            MessageBox("Warning", note, MB_OK);
            return -1;
        }

		id = newResId(type, name);
    }

	if(id == -1)
		return -1; //not support

	if(NULL == (res = new Resource(name, id, (const char *)source)))
		return -1;

	reses[id] = res;
	if(name)
		namedRes[res->name] = res;

	bIdChanged = TRUE;

	useId(id, res);

	return id;
}

void ResEditor::enumRes(ResEnumHandler *handler)
{
	Resource* res;
	map<int, Resource*>::iterator it;

	if(handler == NULL)
		return ;

	for(it = reses.begin(); it != reses.end(); ++it){
		res = it->second;
		if(res)
			handler->setRes(this,typeFromId(res->id), res->id, res->name.c_str(), res->source_id);
	}
}

BOOL ResEditor::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{
	if(iMsg == MSG_DESTROY)
	{
		Detach();
	}
	else if(MSG_CSIZECHANGED == iMsg)
	{
		layout.SetGridLayoutRect(0,0, (int)wParam, (int)lParam);
	}
	else
	{
		layout.GridLayoutHookProc(m_hWnd, iMsg, wParam, lParam);
	}

	*pret = DefaultControlProc(m_hWnd, iMsg, wParam, lParam);
	return TRUE;
}

BOOL ResEditor::loadXMLIDs(const char* xmlFile)
{
	xmlDocPtr doc;
	xmlNodePtr node, child;
	xmlChar* xname = NULL;
	xmlChar* xsource = NULL;
	int type = -1;
	int id = -1;
	//Resource * resource;

	if(xmlFile == NULL)
		return FALSE;

	doc = xmlParseFile(xmlFile);
	if(doc == NULL)
		return FALSE;

	node = xmlDocGetRootElement(doc);
	if(node == NULL || !xhIsNode(node,"res-ids"))
	{
		xmlFreeDoc(doc);
		return FALSE;
	}

	//load a res-id
	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if (loadUserNodes(node)) {
			continue;
		}
		if(!xhIsNode(node, "res-id"))
			continue;

		for(child = node->xmlChildrenNode; child; child=child->next)
		{
			if(child->type != XML_ELEMENT_NODE)
				continue;

			if(xmlStrcmp(child->name, (const xmlChar*)"id") == 0){
				xname = xmlGetProp(child, (const xmlChar*)"name");
				id = xhGetIntProp(child, "value", -1);
				//if(id == -1) //invalid id
				//	break;
			}
			else if(xmlStrcmp(child->name, (const xmlChar*)"type") == 0){
				xmlChar* xtype = xhGetNodeText(child);
				if(xtype == NULL)
					break;
				type = strtol((const char*)xtype,NULL,0);
				xmlFree(xtype);
			}
			else if(xmlStrcmp(child->name, (const xmlChar*)"source") == 0){
				xsource = xhGetNodeText(child);
			}
		}

		if(getTypeMask()&type)
		{
			/*resource = new Resource((const char*)xname, id, (const char*)xsource);
			if(resource)
			{
				reses[id] = resource;
				if(xname)
					namedRes[resource->name] = resource;
			}*/
#ifdef WIN32
			if(xsource)
			{
				char szSource[1024];
				utf8toascii((const char*)xsource, szSource, sizeof(szSource));
				createRes(type, (const char*)xname,id, szSource, 0);
			}
			else
#endif
				createRes(type, (const char*)xname,id, (const char*)xsource, 0);
		}

		if(xname)
			xmlFree(xname);
		if(xsource)
			xmlFree(xsource);

		xname = NULL;
		xsource = NULL;
		id = -1;
		type = -1;
	}

	xmlFreeDoc(doc);
	return TRUE;
}

BOOL ResEditor::saveXMLIDs(const char* xmlFile)
{
	Resource* resource;
	map<int, Resource*>::iterator it;

	if(!bIdChanged)
		return FALSE;

	if(xmlFile == NULL)
		return FALSE;

	FileStreamStorage fss(xmlFile);
	TextStream stream(&fss);

	stream.println("<res-ids>");

	stream.indent();
	saveUserNodes(&stream);

	for(it=reses.begin(); it != reses.end(); ++it)
	{
	//	if(isPredefinedID(it->first))
	//		continue;
		resource = it->second;
		if(resource){
			stream.println("<res-id>");
			stream.indent();
			stream.printf("<id ");
			if(resource->name.length() > 0)
				stream.printf("name=\"%s\" ", _ERT(resource->name.c_str()));
			stream.println(" value=\"0x%x\"/>", resource->id);
			stream.println("<type>%d</type>", typeFromId(resource->id));
			stream.printf("<source>");
			const char* source = g_env->getString(resource->source_id);
			if(source)
			{
#ifdef WIN32
				char szText[1024];
				asciitoutf8(source, szText, sizeof(szText));
				stream.printf("%s",szText);
#else
				stream.printf("%s",source);
#endif
			}
			stream.println("</source>");
			stream.unindent();
			stream.println("</res-id>");
		}
	}
	stream.unindent();

	stream.println("</res-ids>");

	bIdChanged = FALSE;

	return TRUE;
}

BOOL ResEditor::transIDs(TextStream* stream)
{
	Resource* resource;
	map<int, Resource*>::iterator it;

	if(stream == NULL || reses.size() <= 0)
		return FALSE;

	for(it=reses.begin(); it != reses.end(); ++it)
	{
		resource = it->second;
		if(resource && resource->name.length()>0){
			stream->println("#define %s\t\t0x%0x",resource->name.c_str(), resource->id);
		}
	}

	return TRUE;
}

int ResEditor::newResId(int type, const char* name)
{
	BOOL extend_sucess = TRUE;
	IDRangeManager* idrm = getIDRangeManager(type, name);
	IDRangeOwner*   owner = g_env->getIDRangeOwnerManager()->getCurOwner();

	if(!idrm)
		return -1;

	do{
		int i = 0;
		for(int id = idrm->nextId(owner); id != -1 && i < 65535; id = idrm->nextId(owner) , i ++)
		{
			if(reses.count(id) <= 0)
			{
				return id;
			}
		}

		extend_sucess = g_env->newIDRange(idrm);
	}while(extend_sucess);

	return -1;
}

///////////////////////////////
//
extern ResEditor* createResIdEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);
extern ResEditor* createTextEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);
extern ResEditor* createImageEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);
extern ResEditor* createUIEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);
extern ResEditor* createRendererEditor(HWND hParent, const char* resEditorPanelLayout, ResEditorNotificationHandler* notification);

ResEditor* ResEditor::createResEditor(HWND hParent, const char* resEditorName,
		const char* resEditorPanelLayout, ResEditorNotificationHandler* notification)
{
	if(strcasecmp("ResIdEditor", resEditorName) == 0)
		return createResIdEditor(hParent, resEditorPanelLayout, notification);
	else if(strcasecmp("TextEditor", resEditorName) == 0)
		return createTextEditor(hParent, resEditorPanelLayout, notification);
	else if(strcasecmp("ImageEditor", resEditorName) == 0)
		return createImageEditor(hParent, resEditorPanelLayout,notification);
	else if(strcasecmp("UIEditor", resEditorName) == 0)
		return createUIEditor(hParent, resEditorPanelLayout, notification);
	else if(strcasecmp("RendererEditor", resEditorName) == 0)
		return createRendererEditor(hParent, resEditorPanelLayout, notification);

	return NULL;
}

/////////////////////////////////////
//check name
BOOL ValidIDName(const char* name)
{
	if(!isalpha(name[0]))
		return FALSE;

	for(int i=1; name[i]; i++)
	{
		if(!(name[i] == '_' || isdigit(name[i]) || isalpha(name[i])))
			return FALSE;
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////
IDRangeOwner::IDRangeOwner()
{
	header = NULL;
}

BOOL IDRangeOwner::isInRange(int id)
{
	int type = ID2TYPE(id);
	id &= 0xFFFF;
	for(IDRange* idrange = header; idrange; idrange = idrange->next_slibing)
	{
		if(idrange->manager->getType() == type && idrange->isInRange(id))
			return TRUE;
	}
	return FALSE;
}

void IDRangeOwner::addIDRange(IDRange* idrange)
{
	if(!idrange)
		return;

	idrange->owner = this;
	if(!header)
	{
		header = idrange;
		idrange->next_slibing = idrange->prev_slibing = NULL;
	}
	else
	{
		idrange->prev_slibing = header->prev_slibing;
		idrange->next_slibing = header;
		header->prev_slibing  = idrange;
		header = idrange;
	}

}

////////////////

IDRange::IDRange(int min, int max)
{
	next = prev = NULL;
	next_slibing = prev_slibing = NULL;
	used = 0xFFFF;
	flags = NEWIDRANGE;
	owner = NULL;
	this->min = min;
	this->max = max;
}

BOOL IDRange::extend(int min, int max)
{
	BOOL changed = FALSE;
	BOOL bOK = FALSE;

	if((min > 0 &&  (prev && min < prev->max))
		|| (max >0 && (next && max > next->min)))
		return FALSE;

	if((min > 0 && ( min >= (max > 0 ? max : this->max)))
		|| (max > 0 && (max <= (min > 0 ? min : this->min))))
		return FALSE;

	if(min > 0)
	{
		if(min != this->min)
		{
			changed = TRUE;
			this->min = min;
		}
		bOK = TRUE;
	}

	if(max > 0 )
	{
		if(max != this->max)
		{
			changed = TRUE;
			this->max = max;
		}
		bOK = TRUE;
	}

	if(changed)
		setNewIDRange(TRUE);

	return bOK;
}


///////////////////////////////////
IDRangeManager::IDRangeManager(ResManager* resManager, int type, const char* name, int limit_min, int limit_max)
    : type(type), 
    limit_min(limit_min), 
    limit_max(limit_max),
    resManager(resManager)
{
	if(name)
		this->name = name;
	next_id = -1;
	cur_idrange = NULL;
	header = NULL;
}

IDRangeManager::~IDRangeManager()
{
	IDRange* idr = header; 
	while(idr) 
	{
		IDRange* tmp = idr;
		idr = idr->next;
		delete tmp;
	}
}

void IDRangeManager::includeIDRange(IDRangeInfoList& list, int min, int max)
{
	if(min >= max)
		return;
	
	if(list.size() <= 0)
	{
		list.push_back(IDRangeInfo(min,max));
		return;
	}

	IDRangeInfoList::iterator it = list.begin();
	while(it != list.end())
	{
		if(it->min > max){
			list.insert(it, IDRangeInfo(min,max));
			return;
		}

		if(it->max <= max)
		{
			if(it->min > min)
				it->min = min;
			break;
		}
		else
		{
			if(it->max >= min)
			{
				it->max = max;
				break;
			}
		}

		++it;
	}

	if(it == list.end())
	{
		list.push_back(IDRangeInfo(min,max));
	}
	else
	{
		IDRangeInfoList::iterator nextit = it + 1;

		while(nextit != list.end())
		{
			if(it->max >= nextit->min)
			{
				it->max = nextit->max;
				IDRangeInfoList::iterator tmpit = nextit;
				++nextit;
				list.erase(tmpit);
			}
			else
				break;
		}
	}
}

void IDRangeManager::excludeIDRange(IDRangeInfoList& list, int min, int max)
{
	if(list.size() <= 0)
		return;

	if(min >= max){
		list.empty();
		return;
	}

	int i = 0;
	while(i < list.size())
	{
		IDRangeInfoList::iterator it = list.begin() + i;
		if(it->min >= max)
			return;

		if(it->min < min)
		{
			if(it->max <= max)
				it->max = min;
			else
			{
				int prev_max = it->max;
				//insert one
				it->max = min;

				list.insert(it, IDRangeInfo(max, prev_max));
				return;
			}
		}
		else if(it->min >= min)
		{
			if(it->max <= max)
			{
				list.erase(it);
				continue;
			}
			if(it->max > max)
			{
				it->min = max;
				return;
			}

		}
		i ++;
	}

}

BOOL IDRangeManager::getFreeIDRanges(IDRangeInfoList & list)
{
	includeIDRange(list, limit_min, limit_max);

	for(IDRange* idrange = header; idrange; idrange = idrange->next)
	{
		excludeIDRange(list, idrange->min, idrange->max);
	}
	return TRUE;
}


IDRange* IDRangeManager::getIDRangeById(int id)
{
	id &= 0xFFFF;
	for(IDRange* idrange = header; idrange; idrange = idrange->next)
	{
		if(idrange->isInRange(id))
			return idrange;
	}
	return NULL;
}

BOOL IDRangeManager::useId(int id)
{
	if(ID2TYPE(id) != type)
		return FALSE;
		
	IDRange* idr = getIDRangeById(id);

	if(!idr)
		return FALSE;

	++ (*idr);
	return TRUE;
}

BOOL IDRangeManager::unuseId(int id)
{
	if(ID2TYPE(id) != type)
		return FALSE;

	IDRange *idr = getIDRangeById(id);

	if(!idr)
		return FALSE;
	
	-- (*idr);
	return TRUE;
}

static IDRange * get_next_idrange(IDRange* begin, IDRangeOwner* owner)
{
	while(begin && begin->owner != owner)
		begin = begin->next;
	return begin;
}

int IDRangeManager::nextId(IDRangeOwner* owner)
{
	if(cur_idrange == NULL)
	{
		cur_idrange = get_next_idrange(header, owner);
	}
	else
	{
		cur_idrange = get_next_idrange(cur_idrange, owner);
		if(!cur_idrange)
			cur_idrange = get_next_idrange(cur_idrange, owner);
	}


	if(!cur_idrange)
		return -1;

	if(next_id <= cur_idrange->min)
		next_id = cur_idrange->min+1;
	else if(next_id > cur_idrange->max)
	{
		cur_idrange = cur_idrange->next;
		if(!cur_idrange)
		{
			//find next used is not null
			cur_idrange = header;
            while(cur_idrange && cur_idrange->unusedCount() <= 0)
                cur_idrange = cur_idrange->next;

			while(cur_idrange)
			{
				cur_idrange = get_next_idrange(cur_idrange, owner);
				if(cur_idrange && cur_idrange->getUsed() < (cur_idrange->max - cur_idrange->min))
					break;
				cur_idrange = cur_idrange->next;
			}	
			
			if(!cur_idrange)
			{
				next_id = -1;
				return -1;
			}
		}
		next_id = cur_idrange->min + 1;
	}
	
	int new_id = next_id | (type<<16);
	next_id ++;
	return new_id;
}

IDRange* IDRangeManager::addIDRange(int min, int max, IDRangeOwner* owner)
{
	if(min >= max)
		return NULL;

	if(!header)
	{
		header = new IDRange(min, max);
		header->manager = this;
		header->owner = owner;
		owner->addIDRange(header);
		return header;
	}


	//insert and combine 
	IDRange* idrange, *prev = header;
	idrange = header; 
	while(idrange)
	{
		if(max <= idrange->min)
		{
			if(prev == header)
			{
				if(owner == idrange->owner && max == idrange->min)
				{
					idrange->extend(min, idrange->max);
					return idrange;
				}
				//new a idrange , and insert at the head of idrange list
				idrange = new IDRange(min, max);
				idrange->owner = owner;
				idrange->next = header;
				header = idrange;
				idrange->manager = this;
				owner->addIDRange(idrange);
				return idrange;
			}
			else
			{
				if(prev->max <= min)
				{
					if(owner == prev->owner)
					{
						//combin the idrange and prev
						if(owner == idrange->owner && max == idrange->min)
						{
							prev->extend(prev->min, idrange->max);
							prev->next = idrange->next;	
							if(idrange->next->prev)
								idrange->next->prev = prev;
							delete idrange;
							return prev;
						}
					}
					else if(owner == idrange->owner && max == idrange->min) //combin min and max to idrange
					{
						idrange->extend(min, idrange->max);
						return idrange;
					}
					else if(prev->max >= min)
					{
						min = prev->max + 1;
						if(min >= max)
							return NULL;
					}

					IDRange * pt = new IDRange(min, max);
					pt->owner = owner;
					pt->next = idrange;
					pt->prev = prev;
					prev->next = pt;
					idrange->prev = pt;
					idrange->manager = this;
					owner->addIDRange(pt);
					return pt;
				}
			}
		}

		prev = idrange;
		idrange = idrange->next;
	}

	//insert at last
	if(prev->max >= min && prev->owner == owner)
	{
		prev->extend(prev->min, max>prev->max?max:prev->max);
		return prev;
	}

	if(prev->max >= min)
	{
		min = prev->max + 1;
		if(min >= max)
			return NULL;
	}

	idrange = new IDRange(min, max);
	idrange->manager = this;
	idrange->next = prev->next;
	idrange->prev = prev;
	prev->next = idrange;
	idrange->owner = owner;
	owner->addIDRange(idrange);
	
	return idrange;
}

