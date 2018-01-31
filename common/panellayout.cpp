/*
 * panellayout.cpp
 *
 *  Created on: 2008-12-16
 *      Author: dongjunjie
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef WIN32
#else
#include <dlfcn.h>
#endif

#include <string>
#include <map>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

#include "msd_intl.h"

using namespace std;

#include "panel.h"
#include "panellayout.h"
#include "log.h"

#ifdef WIN32
#include "func-win.h"
#endif

PanelLayout::PanelLayout(const char* name)
:PanelCell(name)
{

}

PanelLayout::PanelLayout(int row, int col,const char* name)
:GridLayout(row, col), PanelCell(name)
{

}

PanelLayout::~PanelLayout()
{

}

static BOOL PanelLayout_ShowPanel(GRID_CELL*cell,GRID_LAYOUT* _this,int row, int col,void * usr)
{
	if(cell->type == gct_user){
		GRID_USER_TYPE *usr = cell->data.user;
		PanelCell *pc = (PanelCell*) usr->user;
		if(pc)
			pc->show((int)usr);
	}
	else if(cell->type == gct_hwnd){
		ShowWindow(cell->data.hwnd, ((BOOL)usr)?SW_SHOW:SW_HIDE);
	}
	return TRUE;
}

void PanelLayout::show(int show)
{
	EnumAllCellObjs(PanelLayout_ShowPanel, (void*)show);
}

void PanelLayout::setSubPanelLayout(int row, int col, PanelLayout * pl)
{
	if(row<0 || row>=row_cnt){
		LOG_WARNING( "PanelLayout::setSubPanelLayout: param \"row\" out of range (0, %d)\n", row_cnt);
		return ;
	}

	if(col < 0 || col >= col_cnt){
		LOG_WARNING( "PanelLayout::setSubPanelLayout: param \"col\" out of range (0, %d)\n", col_cnt);
		return ;
	}

	if(!col_width || !row_height){
		LOG_WARNING( "PanelLayout::setSubPanelLayout: PanelLayout has not initialized\n");
		return;
	}

	if(col_width[col].type == lutSpliter  || row_height[row].type == lutSpliter){
		LOG_WARNING( "PanelLayout::setSubPanelLayout: (row=%d, col=%d)  at the  spliter bar\n", row, col);
		return ;
	}

	if(pl == NULL) {
		LOG_WARNING( "PanelLayout::setSubPanelLayout: param \"pl\" is NULL\n");
		return ;
	}

	GRID_CELL *obj = &cells[col][row];

	if(obj->type != gct_unknown) {
		LOG_WARNING( "PanelLayout::setSubPanelLayout: Cell (row=%d, col=%d) is not except type \n",row, col);
		return ;
	}

	obj->type = gct_user;
	obj->data.user = pl->GetUserType();

}


struct _FindPanelData{
	const char* name;
	GRID_CELL * cell;
};

static BOOL PanelLayout_findPanel(GRID_CELL* cell, GRID_LAYOUT* _this, int row, int col, void * usr)
{
	if(cell->type == gct_user)
	{
		GRID_USER_TYPE *usr_data = cell->data.user;
		PanelCell * pcell = (PanelCell*) usr_data->user;
		if(pcell){
			_FindPanelData * pdata = (_FindPanelData*)usr;
			//printf("%s : %s\n",pcell->getName(), pdata->name);
			if(strcasecmp(pcell->getName(), pdata->name) == 0)
			{
				pdata->cell = cell;
				return FALSE;
			}
			else
			{
				GridLayout* gl = (GridLayout*) pcell->userToGridlayout();
				if(gl){
					gl->EnumAllCellObjs(PanelLayout_findPanel, usr);
					if(pdata->cell)
						return FALSE;
				}
			}
		}
	}
	return TRUE;
}

GRID_CELL* PanelLayout::findCellByName(const char* name)
{
	_FindPanelData data = { name, NULL };
	if(name == NULL)
		return NULL;
	EnumAllCellObjs(PanelLayout_findPanel, (void*)&data);
	return data.cell;
}

static int _parser_col_row_size(xmlNodePtr node, const char* str, int *size)
{
	xmlChar* xstr = NULL;

	if(size)
		*size = 0;

	if(size == NULL || node == NULL || str==NULL)
		return lutPixel;


	xstr = xmlGetProp(node, (const xmlChar*)str);
	if(xstr == NULL)
		return lutPixel;

	if(strcmp((const char*)xstr, "spliter")==0)
	{
		xmlFree(xstr);
		return lutSpliter;
	}

	*size = atoi((const char*)xstr);

	if(xstr[strlen((const char*)xstr)-1] == '%')
	{
		xmlFree(xstr);
		return lutPercent;
	}

	xmlFree(xstr);
	return lutPixel;
}

bool PanelLayout::load(const char* panelInfo, PanelManager* pPanelManager)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr node = NULL;
	bool bret = true;

	if(panelInfo == NULL){
		LOG_WARNING("PanelLayout::load param \"panelInfo\" is NULL\n");
		return false;
	}

	doc = xmlParseFile(panelInfo);
	if(doc == NULL)
	{
		LOG_WARNING( "PanelLayout::load parser file \"%s\" failed\n", panelInfo);
		return false;
	}

	node = xmlDocGetRootElement(doc);
	if(node == NULL || xmlStrcmp(node->name, (const xmlChar*)"panelLayout") ){
		LOG_WARNING( "PanelLayout::load file \"%s\" has not the root \"PanelLayout\"\n",panelInfo);
	}
	else
	{
		bret = load(node, pPanelManager);
	}

	if(doc)
		xmlFreeDoc(doc);

	return bret;
}

bool PanelLayout::load(xmlNodePtr node, PanelManager* pPanelManager)
{

	//read cols and rows
	Init(xhGetIntProp(node, "rows"), xhGetIntProp(node,"cols"));

	//read col and row info
	node = node->xmlChildrenNode;

	while(node)
	{
		if(node->type != XML_ELEMENT_NODE){
			node = node->next;
			continue;
		}
		if(xhIsNode(node, "col"))
		{
			int idx = xhGetIntProp(node,"idx");
			int size;
			int type = _parser_col_row_size(node, "width",&size);
			SetColWidth(idx, type, size);
		}
		else if(xhIsNode(node, "row"))
		{
			int idx = xhGetIntProp(node,"idx");
			int size;
			int type = _parser_col_row_size(node, "height",&size);
			SetRowHeight(idx, type, size);
		}
		else if(xhIsNode(node, "cell"))
		{
			int row = xhGetIntProp(node, "row");
			int col = xhGetIntProp(node, "col");

			xmlChar* xname = xmlGetProp(node, (const xmlChar*)"name");
			if(xname)
			{
                CellInfo *info = new CellInfo(row, col, this);
				pPanelManager->namedCells[(const char*)xname] = info;// = ((col<<16)|row);
				xmlFree(xname);
			}
			loadCell(row, col,node,pPanelManager);
		}
		node = node->next;
	}

	return true;
}

void PanelLayout::loadCell(int row, int col,xmlNodePtr node, PanelManager* pPanelManager)
{

	//other cell
	xmlNodePtr child = node->xmlChildrenNode;

	for(;child; child=child->next)
	{
		if(child->type != XML_ELEMENT_NODE){
			continue;
		}
		//
		if(xhIsNode(child, "panelLayout"))
		{
			//new a PanelLayout
			PanelLayout *pl = new PanelLayout;
			pl->SetAttachedWnd(hwndAttached);
			if(!pl->load(child,pPanelManager)){
				delete pl;
				break;
			}
			setSubPanelLayout(row, col, pl);
		}
		else if(xhIsNode(child, "panel"))
		{
			loadPanel(row, col, child, pPanelManager);
		}
		else if(xhIsNode(child,"page"))
		{
			//get owner
			HWND howner = getWindowOwner(col, row);
			//create Tab Page Window
			HWND htabsheet = CreateWindow(CTRL_PROPSHEET, "",
								WS_CHILD|WS_VISIBLE |PSS_SCROLLABLE |WS_BORDER,
								1,
								0,0, 100, 100,
								howner, 0);

			insertWindow(col,row, htabsheet);

			loadCell(row, col, child, pPanelManager);
			int active_index = xhGetIntProp(child, "active-index", 0);
			::SendMessage(htabsheet, PSM_SETACTIVEINDEX, active_index, 0);
		}
		else if(xhIsNode(child,"window"))
		{
			//get class
			xmlChar* xclass = xmlGetProp(child,(const xmlChar*)"class");
			xmlChar* xtext = xmlGetProp(child,(const xmlChar*)"text");
			int id = xhGetIntProp(child, "id");
			DWORD style = xhGetIntProp(child, "style", 0);
			DWORD exstyle = xhGetIntProp(child, "exstyle", 0);

			//get owner
			HWND howner = getWindowOwner(col, row);
			//create this window
			HWND hwnd = CreateWindowEx((const char*) xclass,
					(const char*)xtext,
					style|WS_CHILD|WS_VISIBLE, exstyle,
					id, 0, 0, 0, 0,
					howner, 0);
			insertWindow(col, row, hwnd);
		}
	}
}

static void * load_library(const char* lib_name)
{
	if(lib_name == NULL)
		return NULL;
#ifdef  WIN32
	//find window library
	return win_look_load_lib(lib_name);
#else
	if(*lib_name == '/') //the hole  name
	{
		return dlopen(lib_name, RTLD_LAZY);
	}

	char szPath[1024];
	const char * libpaths = getenv("LD_LIBRARY_PATH");
	bool bwholename = strstr(lib_name, ".so") != NULL;
	if(libpaths)
	{
		while(libpaths){
			char * str = (char*)strchr(libpaths, ':');
			int len;
			if(str)
			{
				strncpy(szPath, libpaths, str - libpaths);
				libpaths = str + 1;
			}
			else
			{
				strcpy(szPath, libpaths);
				libpaths = NULL;
			}

			len = strlen(szPath);

			if(bwholename){
				sprintf(szPath+len, "/%s", lib_name);
			}
			else{
				sprintf(szPath+len, "/lib%s.so",lib_name);
			}
			void *handle = dlopen(szPath, RTLD_LAZY);
			if(handle)
				return handle;
		}
	}
	{
		void *handle;
		if(bwholename)
			sprintf(szPath, "/lib/%s", lib_name);
		else
			sprintf(szPath, "/lib/lib%s.so", lib_name);
		handle = dlopen(szPath, RTLD_LAZY);
		if(handle)
			return handle;
		if(bwholename)
			sprintf(szPath, "/usr/lib/%s", lib_name);
		else
			sprintf(szPath, "/usr/lib/lib%s.so", lib_name);
		handle = dlopen(szPath, RTLD_LAZY);
		if(handle)
			return handle;
		if(bwholename)
			sprintf(szPath, "/usr/local/lib/%s", lib_name);
		else
			sprintf(szPath, "/usr/local/lib/lib%s.so", lib_name);
		handle = dlopen(szPath, RTLD_LAZY);
		if(handle)
			return handle;
	}
	return NULL;
#endif
}

static void close_library(void* handle)
{
#ifdef WIN32
	win_close_lib(handle);
#else
	if(!handle)
		return;
	dlclose(handle);
#endif
}

static void exec_library_func(void* handle, const char* name)
{
	void (*init)(void);
	if(!handle || !name)
		return ;

#ifdef WIN32
	init = (void(*)(void)) win_dlsym(handle, name);
#else
	init = (void(*)(void)) dlsym(handle, name);
#endif
	if(!init)
		return;
	(*init)();
}

Panel * PanelLayout::loadPanel(int row, int col, xmlNodePtr node, PanelManager *pPanelManager)
{
	xmlChar* name = xmlGetProp(node, (const xmlChar*)"class");
	if(name == NULL)
		return NULL;

	xmlChar* xcaption = xmlGetProp(node, (const xmlChar*)"caption");

	Panel* panel = NULL;

	mapex<string, string> params;
	//find params
	xmlNodePtr child =  node->xmlChildrenNode;
	for(; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;
		if(xhIsNode(child, "param"))
		{
			xmlChar* param_name = xmlGetProp(child, (const xmlChar*)"name");
			if(!param_name)
				continue;
			xmlChar* param_value = xhGetNodeText(child);
			params[(char*)param_name] = (param_value?(const char*)param_value:"");
			xmlFree(param_name);
			if(param_value)
				xmlFree(param_value);
		}
		else if(xhIsNode(child, "lib"))
		{
			xmlChar* lib_name = xmlGetProp(child, (const xmlChar*)"src");
			if(!lib_name)
				continue;
			void *handle = load_library((char*)lib_name);
			if(handle == NULL)
			{
				fprintf(stderr, "cannot find the library :%s\n", lib_name);
				xmlFree(lib_name);
				continue;
			}

			xmlChar* start_name = xmlGetProp(child, (const xmlChar*)"init");

			if(start_name == NULL)
				close_library(handle);

			exec_library_func(handle, (const char*)start_name);

			xmlFree(start_name);

			xmlFree(lib_name);
		}
	}

	panel = pPanelManager->createPanel((const char*)name, (const char*)xcaption, &params);

	if(panel == NULL)
		return NULL;

	//get the owner of this panel
	HWND howner = getWindowOwner(col, row, (const char*)xcaption);
	panel->createPanel(howner);
	insertWindow(col, row,panel->getHandler());

	if(xcaption)
		xmlFree(xcaption);
	xmlFree(name);

	return panel;
}

///////////////////////////
static LRESULT _panel_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == MSG_SIZECHANGED)
	{
		DefaultControlProc(hwnd, message, wParam, lParam);
		HWND hchild = GetNextChild(hwnd,0);
		if(IsControl(hchild))
		{
			RECT rt;
			GetClientRect(hwnd, &rt);
			//printf("--- child(%p), %d,%d,%d,%d\n",hchild, rt.left, rt.top, rt.right, rt.bottom);
			MoveWindow(hchild, 0, 0, RECTW(rt),RECTH(rt),TRUE);
			return 0;
		}
	}
    else if(message == MSG_SHOWPAGE) {
        HWND hchild = GetNextChild(hwnd, HWND_NULL);
        if(IsWindow(hchild))
            return SendMessage(hchild, message, wParam, lParam);
        return 1;
    }

	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND PanelLayout::getWindowOwner(int col, int row, const char* strCaption/* = NULL*/)
{

	HWND howner = (HWND) GetCell(row, col);

	if(howner != (HWND)0)
	{
		DLGTEMPLATE panel_template;

		memset(&panel_template, 0, sizeof(DLGTEMPLATE));
		panel_template.caption = _(strCaption);

		LRESULT idx = ::SendMessage(howner, PSM_ADDPAGE, (WPARAM)&panel_template, (LPARAM) _panel_proc);

		if(idx != PS_ERR)
			return (HWND)::SendMessage(howner, PSM_GETPAGE, (WPARAM)idx, 0);
	}

	return hwndAttached;
}

void PanelLayout::insertWindow(int col, int row, HWND hwnd)
{
	if(GetCell(row, col) == NULL){
		SetCell(row,col, gct_hwnd, (void*)hwnd);
	}
}

BOOL PanelManager::getCellByName(const char* name, int &col, int &row, PanelLayout** layout)
{
	string strname;
	if(name == NULL)
		return FALSE;

	strname = name;

	if(namedCells.count(strname) == 0)
		return FALSE;
    CellInfo *info = namedCells[strname];
    row = info->getRow();
    col = info->getColumn();
    *layout = info->getPanelLayout();

	return TRUE;
}
