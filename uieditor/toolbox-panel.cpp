/*
 * toolbox-panel.cpp
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>
#include "mapex.h"
#include <list>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

#include "msd_intl.h"
using namespace std;

#include "panel.h"
#include "resenv.h"

#include "ui-event-id.h"

#include "toolbox-panel.h"
#include "log.h"

#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"

struct ItemData{
	string classname;
	string desc;
};

ToolboxPanel::ToolboxPanel(PanelEventHandler* handler,const mapex<string,string>*params)
:Panel(handler)
{
	// TODO Auto-generated constructor stub
	sendBySystem = 0;
	if(params)
	{
		mapex<string,string>::const_iterator it = params->find("ctrllist");
		if( it != params->end())
			ctrlistName = it->second;
	}

}

ToolboxPanel::~ToolboxPanel() {
    for (set<PBITMAP>::iterator it = bmps.begin(); it != bmps.end(); it++) {
        UnloadBitmap(*it);
        delete *it;
    }
    bmps.clear();

    for (set<char*>::iterator it = items.begin(); it != items.end(); it++)
        delete (ItemData*)(*it);
    items.clear();
}

HWND ToolboxPanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent,&rt);
	listview.Create("", hParent, 0, 0, RECTW(rt), RECTH(rt), WS_CHILD| WS_VISIBLE | LVS_TREEVIEW | WS_VSCROLL | WS_HSCROLL,0,344);
	listview.AddColumn(0, 15, "Parent",  0, NULL, LVCF_LEFTALIGN);
	listview.AddColumn(1, 30, "Icon",  0, NULL, LVCF_LEFTALIGN);
	listview.AddColumn(2, 500, "Name",  0, NULL, LVCF_LEFTALIGN);
	listview.SendMessage(LVM_SETHEADHEIGHT, 0 , 0);

	listview.SetNotification(this);

	//load config
	loadConfig(g_env->getConfigFile(ctrlistName.c_str()).c_str());

	return listview.GetHandle();
}

BOOL ToolboxPanel::loadConfig(const char* cfgfile)
{
	xmlDocPtr doc;
	xmlNodePtr node;
	int nItem = 0;
	if(cfgfile == NULL){
		LOG_WARNING("ToolboxPanel::load: param \"cfgfile\" is NULL\n");
		return TRUE;
	}

	doc = xmlParseFile(cfgfile);
	if(doc == NULL){
		LOG_WARNING( "TooboxPanel::load: cfgfile=\"%s\" is not a corrent xml file\n",cfgfile);
		return FALSE;
	}

	node = xmlDocGetRootElement(doc);
	if(node == NULL || xmlStrcmp(node->name, (const xmlChar*)"controls")){
		LOG_WARNING( "ToolboxPanel::load: cfgfile=\"%s\" did not find root \"controls\"\n",cfgfile);
		return FALSE;
	}

	for(node=node->xmlChildrenNode; node; node = node->next)
	{
		if(xhIsNode(node, "control"))
			loadControl(node,0,nItem++);
		else if(xhIsNode(node, "controls"))
			loadControls(node, 0,nItem ++);
	}

	xmlFreeDoc(doc);

	return TRUE;
}

BOOL ToolboxPanel::loadControl(xmlNodePtr node, HLVITEM hlvItem, int nItem)
{
	xmlChar *xname = NULL;
	xmlChar *xicon = NULL;
	xmlChar *xclassname = NULL;
	xmlChar *xdesc = NULL;

	GHANDLE hItem = 0;
	LVITEM lvItem;
	LVSUBITEM lvSubItem;
	memset(&lvItem, 0, sizeof(lvItem));
	lvItem.nItemHeight = 26;
	memset(&lvSubItem, 0, sizeof(lvSubItem));
	lvItem.nItem = nItem;

	for(node = node->xmlChildrenNode;node; node=node->next){
		if(node->type != XML_ELEMENT_NODE)
			continue;

		if(xmlStrcmp(node->name, (const xmlChar*)"name") == 0){
			xname = xhGetNodeText(node);
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"icon") == 0){
			xicon = xhGetNodeText(node);
		}
		else if(xmlStrcmp(node->name,(const xmlChar*)"classname") == 0){
			xclassname = xhGetNodeText(node);
		}
		else if(xmlStrcmp(node->name, (const xmlChar*)"desc") == 0){
			xdesc = xhGetNodeText(node);
		}
	}

	if(xicon &&  xname){
		ItemData * itemdata = NULL;
		if(xclassname)
		{
			itemdata = new ItemData;
			itemdata->classname = (const char*)xclassname;
            Class* cls = Class::getClassByName("window", (const char*)xclassname); 
            cls->initAsControl(TRUE);

			if(xdesc)
				itemdata->desc = (const char*)xdesc;
            items.insert((char*)itemdata);
		}
		//add item
		lvItem.itemData = (DWORD)itemdata;
		hItem = listview.AddItem(hlvItem, &lvItem);
		//set subitem
		lvSubItem.nItem = lvItem.nItem;
		PBITMAP pbmp= new BITMAP;
		if(LoadBitmapFromFile(HDC_SCREEN, pbmp, g_env->getConfigFile((const char*)xicon).c_str())==0){
            bmps.insert(pbmp);

			lvSubItem.subItem = 1;
			lvSubItem.image = (DWORD)pbmp;
			lvSubItem.pszText = NULL;
			lvSubItem.nTextMax = 0;
			lvSubItem.nTextColor = 0;
			lvSubItem.flags = LVFLAG_BITMAP;
			listview.SetSubitem(hItem,&lvSubItem);
		}
		else{
			delete pbmp;
		}
		lvSubItem.subItem = 2;
		lvSubItem.pszText = (char *)_((char*) xname);
		lvSubItem.nTextMax = xmlStrlen(xname);
		lvSubItem.nTextColor = 0;
		lvSubItem.image  = 0;
		lvSubItem.flags = 0;
		listview.SetSubitem(hItem,&lvSubItem);
		lvItem.nItem ++;
	}
	if(xicon)
		xmlFree(xicon);
	if(xclassname)
		xmlFree(xclassname);
	if(xname)
		xmlFree(xname);
	if(xdesc)
		xmlFree(xdesc);

	return TRUE;
}

BOOL ToolboxPanel::loadControls(xmlNodePtr node, HLVITEM hlvParent, int nItem)
{
	HLVITEM hItem;
	LVITEM lvItem;
	LVSUBITEM lvSubItem;
	int nSubItem = 0;
	if(!node)
		return FALSE;

	memset(&lvItem, 0, sizeof(lvItem));
	lvItem.nItemHeight = 26;
	memset(&lvSubItem, 0, sizeof(lvSubItem));
	lvItem.nItem = nItem;
	//get the caption
	xmlChar * xcaption = xmlGetProp(node, (const xmlChar*)"caption");

	hItem = listview.AddItem(hlvParent,&lvItem);
	if(xcaption)
	{
		listview.SetSubitemText(hItem, 2, _((const char*)xcaption));
		xmlFree(xcaption);
	}

	for(node=node->xmlChildrenNode; node; node = node->next)
	{
		if(xhIsNode(node, "control"))
			loadControl(node,hItem,nSubItem++);
		else if(xhIsNode(node, "controls"))
			loadControls(node, hItem,nSubItem++);
	}
	return TRUE;
}

//Notification
void ToolboxPanel::OnCtrlNotified(MGWnd *sender, int id, int code, DWORD add_data)
{
	if(sendBySystem || sender != &listview)
		return;

	if(code == LVN_SELCHANGE || code == LVN_CLICKED || code == LVN_ITEMDBCLK)
	{
        HLVITEM hlvItem = listview.GetSelectedItem();
        ItemData *item_data = (ItemData*)listview.GetItemAddData(hlvItem, 0);

		sendEvent(TOOLBOX_SELCHANGE,
				code == LVN_ITEMDBCLK,
				item_data?(DWORD)item_data->classname.c_str():0);
	}
}

void ToolboxPanel::cancelSelect()
{
	sendBySystem = 1;
	listview.SelectItem(0,0);
	sendBySystem = 0;
}
