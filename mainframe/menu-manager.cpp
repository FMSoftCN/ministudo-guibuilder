/*
 * menu-manager.cpp
 *
 *  Created on: 2009-5-8
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include "msd_intl.h"

using namespace std;

#include "log.h"
#include "stream.h"
#include "resenv.h"

#include "menu-manager.h"

MenuManager::MenuManager(MenuManager *parent) {
	// TODO Auto-generated constructor stub
	this->parent = parent;
}

MenuManager::~MenuManager() {
	// TODO Auto-generated destructor stub
	for(mapex<int, MenuItem*>::iterator it = menuitems.begin();
		it != menuitems.end(); ++it)
	{
		MenuItem * menuitem = it->second;
		if(menuitem && SeparationMenuItem != menuitem)
			delete menuitem;
	}
}

void MenuManager::loadFromXml(xmlNodePtr node)
{
	if(node == NULL){
		return;
	}

	if(!xhIsNode(node, "menu-set"))
	{
		LOG_WARNING("MenuManager need \"menu-set\" node");
		return;
	}

	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;

		buildMenuItem(node,&MenuManager::setMenuItem,NULL);

	}
}

BOOL MenuManager::buildMenuItem(xmlNodePtr node, PSetMenuItem psetMenuItem, void *data,DWORD param)
{
	MENUITEMINFO menuitem;
	BOOL bfreecaption = FALSE;

	if(xmlStrcmp(node->name,(const xmlChar*)"menuitem") != 0)
	{
		return FALSE;
	}

	memset(&menuitem, 0, sizeof(menuitem));

	menuitem.mask = MIIM_STATE|MIIM_TYPE|MIIM_ID;

	menuitem.state = MFS_ENABLED;
	menuitem.type = MFT_STRING;

	//get id
	int menu_id = xhGetIntProp(node, "id", 0);

	//is a menu reference?
	if(menu_id == 0)
	{
		menu_id = xhGetNodeInt(node);
		if(menu_id <= 0) //invalidate reference of id
		{
			xmlChar* xstr = xmlGetProp(node, (const xmlChar*)"type");
			if(xstr && xmlStrcmp(xstr, (const xmlChar*)"separation") == 0)
			{
				menuitem.mask = MIIM_TYPE;
				menuitem.type = MFT_SEPARATOR;
                xmlFree(xstr);
			}
			else {
                if (xstr) xmlFree(xstr);
				return FALSE;
            }
		}
		else
		{
			MenuItem* mi = getMenuItem(menu_id);
			if(!mi)
				return FALSE;

			if(!mi->fillMenuItemInfo(&menuitem, param))
				return FALSE;
		}
	}
	else
	{
		menuitem.id = menu_id;
		//load
		//get type
		xmlChar* xstr = xmlGetProp(node, (const xmlChar*)"type");
		if(xstr)
		{
			//is prop menu
			if(xmlStrcmp(xstr, (const xmlChar*)"pop") == 0){
				menuitem.mask |= MIIM_SUBMENU;
			}
			else if(xmlStrcmp(xstr,(const xmlChar*)"check") == 0){
				//menuitem.mask |= MIIM_CHECKMARKS;
				menuitem.type = MFT_MARKCHECK;
			}
			else if(xmlStrcmp(xstr, (const xmlChar*)"radio") == 0){
				menuitem.type = MFT_RADIOCHECK;
			}
			else if(xmlStrcmp(xstr, (const xmlChar*)"separation") == 0){
				menuitem.type = MFT_SEPARATOR;
			}

			xmlFree(xstr);
		}
		//get caption
		menuitem.typedata = (DWORD)xmlGetProp(node,(const xmlChar*)"caption");
		bfreecaption = TRUE;
		//get disable
		xstr = xmlGetProp(node, (const xmlChar*)"disable");
		if(xstr && xmlStrcmp(xstr,(const xmlChar*)"true") == 0)
		{
			menuitem.state |= MFS_DISABLED;
		}
        if (xstr) xmlFree(xstr);

		//get default
		if(menuitem.type == MFT_RADIOCHECK || menuitem.type == MFT_MARKCHECK)
		{
			xmlChar* xdef = xmlGetProp(node,(const xmlChar*)"default");
			if(xdef)
			{
				if(xmlStrcmp(xdef,(const xmlChar*)"checked") == 0)
					menuitem.state |= (MFS_CHECKED&(~MFS_UNCHECKED));
			}
			xmlFree(xdef);
		}
	}

	(this->*psetMenuItem)(node->xmlChildrenNode,&menuitem, data);

#ifndef _MSTUDIO_LOCALE
	if(bfreecaption && menuitem.typedata)
		xmlFree((xmlChar*)menuitem.typedata);
#endif
	return TRUE;
}


void MenuManager::setMenuItem(xmlNodePtr child,MENUITEMINFO *menuItem, void* data)
{
	if(!menuItem)
		return ;

	//create menuitem
	MenuItem * mi = NULL;
	if(menuItem->type & MFT_SEPARATOR)
		mi = SeparationMenuItem;
	else if(menuItem->type & MFT_RADIOCHECK)
	{
		if(menuItem->mask & MIIM_CHECKMARKS)
			mi = new CheckMenuItem(menuItem->id, (const char*)menuItem->typedata, menuItem->state);
		else
			mi = new RadioMenuItem(menuItem->id, (const char*)menuItem->typedata, menuItem->state);
	}
	else if(menuItem->mask & MIIM_SUBMENU){
		PopMenuItem * pmi  = new PopMenuItem(menuItem->id, (const char*)menuItem->typedata);
		for(; child; child = child->next)
		{
			if(child->type != XML_ELEMENT_NODE)
				continue;
			buildMenuItem(child, &MenuManager::setMenuItem, (void*)pmi);
		}
		mi = pmi;
	}
	else if(menuItem->type & MFT_SEPARATOR)
		mi = SeparationMenuItem;
	else
		mi = new MenuItem(menuItem->id, _((const char*)menuItem->typedata), menuItem->state);

	PopMenuItem* pmi = (PopMenuItem*)data;
	if(pmi)
		pmi->insertMenuItem(mi);

	//insert into
	menuitems[menuItem->id] = mi;
}

void MenuManager::setSysMenuItem(xmlNodePtr child,MENUITEMINFO* menuItem, void *data)
{
	HMENU hMenu = (HMENU)data;
	if(!hMenu)
		return ;

	if(menuItem->mask & MIIM_SUBMENU && menuItem->hsubmenu == 0)
	{
		menuItem->hsubmenu = CreatePopupMenu(menuItem);

		for(; child; child = child->next)
		{
			if(child->type != XML_ELEMENT_NODE)
				continue;
			buildMenuItem(child, &MenuManager::setSysMenuItem, (void*)menuItem->hsubmenu,1);
		}

		menuItem->hsubmenu = StripPopupHead(menuItem->hsubmenu);
	}

#if 0
	/******************/
	/** Delete this  code When MiniGUI's Menu Bug fixed:
	  when the type is MFT_MARKCHECK or MFT_RADIOCHECK,
	  menuItem->typedata would be not copyed, just use the pointer
	*/
	DWORD old_typedata;
	if(menuItem->typedata && (menuItem->type == MFT_MARKCHECK || menuItem->type == MFT_RADIOCHECK))
	{
//		old_typedata = menuItem->typedata;
	//	menuItem->typedata = (DWORD)(strdup((char*)(menuItem->typedata)));
	}
	/*****************/
#endif
#ifdef _MSTUDIO_LOCALE
	if (menuItem->typedata)
	{
		//fprintf(stderr, "menu item is %s\n", _((char *)menuItem->typedata)t);
		//FIXME: some typedatas have been translated in setMenuItem, no need to translate all
		menuItem->typedata = (DWORD) _((char*) (menuItem->typedata));
	}
#endif
	InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, menuItem);

#if 0
	/***************/
	/** delete Me As fixed MiniGUI's Menu Bug **/
	//	if(menuItem->typedata && (menuItem->type == MFT_MARKCHECK || menuItem->type == MFT_RADIOCHECK))
	//	menuItem->typedata = old_typedata;
	/**************/
#endif
}

HMENU MenuManager::createPopMenu(int popMenuId, mapex<int, int> selectedIds)
{
	MenuItem* mi = getMenuItem(popMenuId);
	if(!mi || mi == SeparationMenuItem)
		return 0;

	PopMenuItem * pmi = dynamic_cast<PopMenuItem*>(mi);
	if(!pmi)
		return 0;

	MENUITEMINFO mii;
	if(pmi->fillSelectedMenuItemInfo(&mii,1, selectedIds))
	{
		return mii.hsubmenu;
	}

	return 0;
}

HMENU MenuManager::loadMenuBar(xmlNodePtr node)
{
	if(!node)
		return 0;
	if(!xhIsNode(node, "menubar"))
		return 0;

	HMENU hMenuBar = CreateMenu();

	for(node = node->xmlChildrenNode; node; node = node->next)
	{
		if(node->type != XML_ELEMENT_NODE)
			continue;
		buildMenuItem(node, &MenuManager::setSysMenuItem, (void*)hMenuBar, 1);
	}


	return hMenuBar;
}

/////////////////////////
MenuManager::MenuItem::MenuItem(int id, const char* strCaption, int defState)
{
	this->id = id;
#ifdef _MSTUDIO_LOCALE
	this->caption = (char *)strCaption;
#else
	this->caption = strCaption?strdup(strCaption):NULL;
#endif
	this->state = defState;
}

MenuManager::MenuItem::~MenuItem()
{
#ifndef _MSTUDIO_LOCALE
	if(caption)
	    free(caption);
#endif
}

BOOL MenuManager::MenuItem::fillMenuItemInfo(MENUITEMINFO *menuItem, DWORD param)
{
	if(!menuItem)
		return FALSE;

	memset(menuItem, 0, sizeof(MENUITEMINFO));

	menuItem->mask = MIIM_STATE|MIIM_ID|MIIM_TYPE;
	menuItem->type = getType();
	menuItem->typedata = (DWORD)caption;
	menuItem->id = id;
	menuItem->state = state;

	return TRUE;
}

BOOL MenuManager::PopMenuItem::fillMenuItemInfo(MENUITEMINFO *menuItem, DWORD param)
{
	mapex<int,int> empty; //empty as all
    return fillSelectedMenuItemInfo(menuItem, param, empty);
}

BOOL MenuManager::PopMenuItem::fillSelectedMenuItemInfo(MENUITEMINFO *menuItem,
        DWORD param, mapex<int, int> selectedIdState)
{
	if(MenuItem::fillMenuItemInfo(menuItem, param))
	{
		menuItem->mask |= MIIM_SUBMENU;
		if(param) //load sub menu
		{
			int idx = 0, separator = 0;
			HMENU hMenu = CreatePopupMenu(menuItem);
			BOOL fillAll = selectedIdState.empty();
            int size = selectedIdState.size();
			for(int i=0; i<(int)submenuitems.size(); i++)
			{
                if (!fillAll && idx - separator == size)
                    break;

				MenuItem *mi = submenuitems[i];
				if(!mi)
					continue;

				MENUITEMINFO mii;
				if(mi == SeparationMenuItem)
				{
                    if (idx == 0)
                        continue;
					memset(&mii, 0, sizeof(mii));
					mii.mask = MIIM_TYPE;
					mii.type = MFT_SEPARATOR;
					InsertMenuItem(hMenu, idx++,MF_BYPOSITION, &mii);
                    separator++;
				}
				else {
                    //needn't show, empty as all
					if (!fillAll && (selectedIdState.find(mi->id) == selectedIdState.end())) {
	                    continue;
	                }

                    if(mi->fillMenuItemInfo(&mii,param))
                    {
                    	if (!fillAll) {
                            int mask_state = selectedIdState.at(mii.id);

                    		mii.state = GET_STATE(mask_state);
                    		mii.mask = GET_MASK(mask_state);
                        }
                        InsertMenuItem(hMenu, idx++,MF_BYPOSITION, &mii);
                    }
                }
			}

			menuItem->hsubmenu = StripPopupHead(hMenu);
		}

		return TRUE;
	}
	return FALSE;
}

MenuManager::PopMenuItem::PopMenuItem(int id, const char* strCaption)
:MenuManager::MenuItem(id, strCaption)
{

}

