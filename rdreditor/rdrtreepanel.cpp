/**
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "mapex.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

using namespace std;

#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "class-instance.h"
#include "../uieditor/component-instance.h"
#include "../uieditor/window-instance.h"
#include "rdr-instance.h"

#include "rdrtreepanel.h"
#include "rdr-preview-panel.h"
#include "editable-listview.h"
#include "uieditor/fieldpanel.h"
#include "uieditor/rdrpanel.h"
#include "rdreditor.h"

#include "rdr-event-id.h"
#include "rdr-dialog.h"

RendererTreePanel::RendererTreePanel(PanelEventHandler* handler)
:Panel(handler)
{
}

RendererTreePanel::~RendererTreePanel()
{
}

HWND RendererTreePanel::createPanel(HWND hParent)
{
	RECT rt;
	::GetClientRect(hParent, &rt);

	TMGStaticSubclass<MGTreeView>::Create("", hParent, 0, 0, RECTW(rt), RECTH(rt),
			WS_CHILD| WS_VISIBLE| TVS_NOTIFY | WS_VSCROLL | WS_HSCROLL,
			WS_EX_NONE, 1110, (DWORD)0);

	Subclass();
	SetNotification(this);

	return getHandler();
}

void RendererTreePanel::OnCtrlNotified(MGWnd* ctrl, int id, int code, DWORD add_data)
{
	if(code == TVN_SELCHANGE)
	{
        GHANDLE sel = GetSelItem();
        if (sel) {
            int cmdId = -1;
            if (sel != GetRoot()) {
            	cmdId = GetItemAddData(sel);
			}
       		sendEvent(RDRTREE_SELCHANGE, cmdId, 0);
        }
	}
}

BOOL RendererTreePanel::isValidIdHandle(GHANDLE handle)
{
	if (handle && handle != GetRoot())
		return TRUE;
	return FALSE;
}

GHANDLE RendererTreePanel::getParentHandle(GHANDLE handle)
{
	GHANDLE parent = GetParentItem(handle);
	return (parent != GetRoot()) ? parent : 0;
}

GHANDLE RendererTreePanel::setSelItem(GHANDLE selHandle)
{
	return SetSelItem(selHandle?selHandle:GetRoot());
}

BOOL RendererTreePanel::getIDName (int id, char* idName)
{
	int type = ID2TYPE(id);

	if (idName == NULL || (type != NCSRT_RDR && type != NCSRT_RDRSET))
		return FALSE;

	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
	DWORD inst = resMgr->getRes(id);

	if(inst == 0)
		return FALSE;

	if (type == NCSRT_RDR) {
		sprintf (idName, "%s(%s.%s)",
				(char *)(resMgr->idToName(id)),
				((RendererInstance *)inst)->getRdrName(),
				((RendererInstance *)inst)->getClsName());
	}
	else if (type == NCSRT_RDRSET) {
		sprintf (idName, "%s(%s)",
				(char *)(resMgr->idToName(id)),
				((RendererSet *)inst)->getRdrName());
	}
	else {
		return FALSE;
	}

	return TRUE;
}

//name == NULL for field changed
BOOL RendererTreePanel::updateIDName (int id, const char* name)
{
	int type = ID2TYPE(id);
	char text[256];
	GHANDLE hitem;
	const char *oldName, *newName;

	if (type != NCSRT_RDR && type != NCSRT_RDRSET)
		return FALSE;

	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
	oldName = resMgr->idToName(id);
	if (name)
		newName = name;
	else
		newName = oldName;


	if (oldName && (!name || strcmp(oldName, newName) != 0)) {
		DWORD inst = resMgr->getRes(id);

		if (type == NCSRT_RDR) {
            sprintf (text, "%s(%s.%s)",
           //TODO: changed 		(char *)(resMgr->idToName(id)),
            		newName,
                    ((RendererInstance *)inst)->getRdrName(),
                    ((RendererInstance *)inst)->getClsName());
		}
		else if (type == NCSRT_RDRSET) {
            sprintf (text, "%s(%s)",
                    (char *)(resMgr->idToName(id)),
                    ((RendererSet *)inst)->getRdrName());
		}
		else {
			return FALSE;
		}

		if (type == NCSRT_RDR) { //maybe multiple item
		    for(GHANDLE child = GetFirstChild(GetRoot()); child; child = GetNextSibling(child))
		    {
		    	hitem = findItem(id, child);

		    	if (hitem) {
		    		if (-1 == SetItemText(hitem, text))
		    			return FALSE;
		    	}
		    }
		}
		else if (type == NCSRT_RDRSET) { //only once
			hitem = findItem(id, 0);

	    	if (hitem) {
	    		if (-1 == SetItemText(hitem, text))
	    			return FALSE;
	    	}
		}
	}
	return TRUE;
}

BOOL RendererTreePanel::updateIDValue (int oldId, int newId)
{
	if (ID2TYPE(oldId) == NCSRT_RDRSET) {
		GHANDLE hitem = findItem(oldId, 0);
    	if (!hitem)
    		return FALSE;

        if (SetItemAddData(hitem, newId) == 0)
        	return TRUE;
	}
	else if (ID2TYPE(oldId) == NCSRT_RDR) {
	    for(GHANDLE child = GetFirstChild(GetRoot()); child; child = GetNextSibling(child))
	    {
	    	GHANDLE hitem = findItem(oldId, child);
	    	if (!hitem)
	    		return FALSE;

	        if (SetItemAddData(hitem, newId) == 0)
	        	return TRUE;
	    }
	}
    return FALSE;
}

BOOL RendererTreePanel::insertItem(int id, GHANDLE parent)
{
	char text[256];
	GHANDLE handle;
	int type = ID2TYPE(id);

	if (type != NCSRT_RDR && type != NCSRT_RDRSET)
		return FALSE;

	RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
	DWORD inst = resMgr->getRes(id);

	if(inst == 0)
		return FALSE;

	if (type == NCSRT_RDR) {
		sprintf (text, "%s(%s.%s)",
				(char *)(resMgr->idToName(id)),
				((RendererInstance *)inst)->getRdrName(),
				((RendererInstance *)inst)->getClsName());
	}
	else if (type == NCSRT_RDRSET) {
		sprintf (text, "%s(%s)",
				(char *)(resMgr->idToName(id)),
				((RendererSet *)inst)->getRdrName());
	}
	else {
		return FALSE;
	}

	if (parent == 0)
		parent = GetRoot();

    //have item
	if (findItem(id, parent) == 0) {
		handle = AddItem(parent, text, 0, id);

		if (ID2TYPE(id) == NCSRT_RDRSET) {
            set<RendererInstance *> insts = ((RendererSet *)inst)->getInsts();

            for(set<RendererInstance *>::iterator it = insts.begin(); it != insts.end(); ++it)
			{
				insertItem((*it)->getID(), handle);
			}
		}

	}
	return TRUE;
}

BOOL RendererTreePanel::removeItem(int id, GHANDLE parent, BOOL delAllRef)
{
	if (ID2TYPE(id) != NCSRT_RDR && ID2TYPE(id) != NCSRT_RDRSET)
        return FALSE;

    if(parent == (GHANDLE)0)
        parent = GetRoot();

    for(GHANDLE child = GetFirstChild(parent); child; child = GetNextSibling(child))
    {
        if((int)GetItemAddData(child) == id) {
        	if (ID2TYPE(id) == NCSRT_RDRSET) {
        		//delete sub items
        	    for(GHANDLE subchild = GetFirstChild(child); subchild; subchild = GetNextSibling(subchild)) {
        	    	DeleteTree(subchild);
        	    }
        	}

        	return DeleteTree(child) ? FALSE : TRUE;
        }
        if (delAllRef)
            removeItem(id, child);
    }

    return FALSE;
}

BOOL RendererTreePanel::selectItem(int id, GHANDLE parent)
{
    GHANDLE item = findItem(id, parent);
    return (NULL != SetSelItem(item));
}

GHANDLE RendererTreePanel::findItem(int id, GHANDLE parent)
{
	if (ID2TYPE(id) != NCSRT_RDR && ID2TYPE(id) != NCSRT_RDRSET)
        return (GHANDLE)0;

    if(parent == (GHANDLE)0)
        parent = GetRoot();

    if (parent != GetRoot())
        if((int)GetItemAddData(parent) == id)
        	return parent;

    for(GHANDLE child = GetFirstChild(parent); child; child = GetNextSibling(child))
    {
        if((int)GetItemAddData(child) == id)
            return child;

        GHANDLE hitem = findItem(id, child);
        if(hitem != (GHANDLE)0)
            return hitem;
    }

    return (GHANDLE)0;
}

GHANDLE RendererTreePanel::getSelItem()
{
    return GetSelItem();
}

DWORD RendererTreePanel::getItemAddData(GHANDLE hitem)
{
	if (hitem && hitem != GetRoot())
		return GetItemAddData(hitem);
	else
		return -1;
}

DWORD RendererTreePanel::getSelItemAddData()
{
	GHANDLE sel = GetSelItem();
	if (sel && sel != GetRoot())
		return GetItemAddData(sel);
	else
		return -1;
}

void RendererTreePanel::updateInstance()
{
    GHANDLE sel = GetSelItem();
    if (sel && sel != GetRoot()) {
        int id = GetItemAddData (sel);

        if (ID2TYPE(id) == NCSRT_RDR) {
            GHANDLE hParent = GetParentItem (sel);
            DeleteTree(sel);
            insertItem (id, hParent);
        }
        sendEvent(RDRTREE_SELCHANGE, id, 0);
    }
}

#ifndef WIN32
BEGIN_MSG_MAP(RendererTreePanel)
MAP_RBUTTONUP(onRButtonUp)
	BEGIN_COMMAND_MAP
	MAP_COMMAND_RANGE(ResEditor::MENUCMD_COMMID_BASE, ResEditor::MENUCMD_COMMID_END, onPopupMenuCmd)
	END_COMMAND_MAP
END_MSG_MAP
#else
BOOL RendererTreePanel::WndProc(UINT iMsg,WPARAM wParam,LPARAM lParam,int *pret) 
{
	if(iMsg == MSG_RBUTTONUP)
	{
		onRButtonUp(LOSWORD(lParam),HISWORD(lParam),(DWORD)wParam); 
		RETURN(0); 
	} else if (iMsg == MSG_COMMAND){
		int ctrlId = LOWORD(wParam);
		if (ctrlId >= ResEditor::MENUCMD_COMMID_BASE && ctrlId <= ResEditor::MENUCMD_COMMID_END)
		{
			onPopupMenuCmd(ctrlId);
		}
	}

	return FALSE;
}
#endif

//execute popup menu command
void RendererTreePanel::onPopupMenuCmd(int id)
{
    RendererEditor* resMgr =
        (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));

    resMgr->executeCommand(id, 0, (DWORD)m_hWnd);
}

void RendererTreePanel::onRButtonUp(int x, int y, DWORD key_flag)
{
    GHANDLE sel = GetSelItem();
    mapex<int, int>idsets;
    UINT status = MFS_DISABLED;

    if (!sel || sel == GetRoot()) {
        idsets[ResEditor::RDR_MENUCMD_NEWRDRSET] = 0;
        idsets[ResEditor::RDR_MENUCMD_NEWRDR] = 0;

        RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
        resMgr->getMenuStatus(ResEditor::GBC_PASTE, &status);
        idsets[ResEditor::GBC_PASTE] = MAP_MASK_STATE(0, status);
    }
    else {
    	int id = GetItemAddData(sel);
        if (ID2TYPE(id) != NCSRT_RDRSET && ID2TYPE(id) != NCSRT_RDR)
            return;

    	if (ID2TYPE(id) == NCSRT_RDRSET) { //not support copy
            idsets[ResEditor::RDR_MENUCMD_NEWRDR] = MAP_MASK_STATE(0, 0);
            idsets[ResEditor::RDR_MENUCMD_ADDRDR] = MAP_MASK_STATE(0, 0);

            RendererEditor* resMgr = (RendererEditor*)(g_env->getResManager(NCSRT_RDR | NCSRT_RDRSET));
            resMgr->getMenuStatus(ResEditor::GBC_PASTE, &status);
            idsets[ResEditor::GBC_PASTE] = MAP_MASK_STATE(0, status);

            idsets[ResEditor::RDR_MENUCMD_DELRDRSET] = MAP_MASK_STATE(0, 0);
    	}
        else {
            idsets[ResEditor::GBC_CUT] = MAP_MASK_STATE(0, 0);
            idsets[ResEditor::GBC_COPY] = MAP_MASK_STATE(0, 0);
            idsets[ResEditor::RDR_MENUCMD_DELRDR] = MAP_MASK_STATE(0, 0);
        }
    }

    HMENU hMenu = g_env->createPopMenuFromConfig(ResEditor::RDR_POPMENU, idsets);
    //get tree item height
    HDC hdc = ::GetClientDC(getHandler());
    int itemHeight = ::GetFontHeight(hdc);
    ::ReleaseDC(hdc);

	ClientToScreen(&x, &y);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN |TPM_DESTROY, x, itemHeight + y, getHandler());
}
