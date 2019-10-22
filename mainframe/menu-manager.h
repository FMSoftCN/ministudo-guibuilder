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
