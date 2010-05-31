/*
 * menu-manager.h
 *
 *  Created on: 2009-5-8
 *      Author: dongjunjie
 */

#ifndef MENUMANAGER_H_
#define MENUMANAGER_H_

#define SeparationMenuItem ((MenuManager::MenuItem*)0xFFFFFFFF)
class MenuManager
{
protected:
	struct MenuItem{
		MenuItem(int id, const char* strCaption, int defState=0);
		virtual ~MenuItem();
		virtual int getType(){ return MFT_STRING; }

		virtual BOOL fillMenuItemInfo(MENUITEMINFO *menuItem, DWORD param = 0);
		char *caption;
		int id;
		int state;
	};

	struct RadioMenuItem : public MenuItem
	{
		RadioMenuItem(int id, const char* strCaption, int defState)
		:MenuItem(id, strCaption, defState){}
		int getType(){ return MFT_RADIOCHECK; }
	};

	struct CheckMenuItem : public RadioMenuItem
	{
		CheckMenuItem(int id, const char* strCaption, int defState)
		:RadioMenuItem(id, strCaption, defState){}
		BOOL fillMenuItemInfo(MENUITEMINFO *menuItem, DWORD param = 0)
		{
			if(MenuItem::fillMenuItemInfo(menuItem, param))
			{
				menuItem->mask |= MIIM_CHECKMARKS;
				return TRUE;
			}
			return FALSE;
		}
	};

	struct PopMenuItem : public MenuItem
	{
		PopMenuItem(int id, const char* strCaption);
		BOOL fillMenuItemInfo(MENUITEMINFO *menuItem, DWORD param = 0);
		BOOL fillSelectedMenuItemInfo(MENUITEMINFO *menuItem, DWORD param,
                mapex<int, int> selectedIdState);

		vector<MenuItem*> submenuitems;
		void insertMenuItem(MenuItem *menuItem){
			submenuitems.push_back(menuItem);
		}
	};

	mapex<int, MenuItem*> menuitems;

	MenuManager *parent;

	typedef void (MenuManager::*PSetMenuItem)(xmlNodePtr child,MENUITEMINFO *menuitem, void *data);
	BOOL buildMenuItem(xmlNodePtr node, PSetMenuItem psetMenuItem, void *data, DWORD param=0);

	void setMenuItem(xmlNodePtr child,MENUITEMINFO *menuItem, void* data);
	void setSysMenuItem(xmlNodePtr child,MENUITEMINFO* menuItem, void *data);

public:
	MenuManager(MenuManager *parent = NULL);
	~MenuManager();

	void loadFromXml(xmlNodePtr node);

	MenuItem * getMenuItem(int id){
		MenuItem *mi;
		if(parent && (mi=parent->getMenuItem(id)))
			return mi;
		return menuitems.at(id);
	}

	HMENU createPopMenu(int popMenuId, mapex<int, int> selectedIds);

	HMENU loadMenuBar(xmlNodePtr node);
};

#endif /* MENUMANAGER_H_ */
