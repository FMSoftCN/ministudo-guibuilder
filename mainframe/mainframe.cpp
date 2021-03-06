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

using namespace std;

#include "log.h"

#include "stream.h"
#include "resenv.h"
#include "panel.h"
#include "panellayout.h"

#include "undo-redo.h"
#include "valuetype.h"
#include "class-instance.h"

#include "reseditor.h"
#include "menu-manager.h"
#include "mainframe.h"
#include "select-project-type-dlg.h"
#include "socket-client.h"
#include "idrange-editor.h"
#include "newidrange.h"
#include "msd_intl.h"

#if defined(WIN32) || defined(_MSTUDIO_OFFICIAL_RELEASE)
#include "func-win.h"
#endif

#include "qvfb.h"

#include "dlgtmpls.h"
#include "gbconfig.h"
#include "build_version.h"

extern HACCEL ghAccel;

int _mainwnd_destroy = 0;

static MainFrame* mainframe = NULL;

#define MAINFRAMECFG "mainframe.cfg"
#define TOOLBARIMG  "maintoolbar.png"

#define TOOLBAR_HEIGHT      30
#define REFPANEL_HEIGHT     40

#define RESSELECTOR_ID      101

#define MAX_SAVE_INTERVAL   150 /*100 seconds*/

MainFrame::Version MainFrame::gbVersion(GUIBUILD_MAJOR_VERSION, GUIBUILD_MINOR_VERSION, GUIBUILD_MICRO_VERSION);


BOOL MainFrame::auto_save_timer(HWND hwnd, LINT id, DWORD count)
{
    MainFrame * mainframe = (MainFrame*)WndFromHandle(hwnd);
    if(mainframe) {
        if(mainframe->isModified) {
            mainframe->save_counter ++;
            //			printf("--- save_counter=%d\n",mainframe->save_counter);
            if(mainframe->save_counter > MAX_SAVE_INTERVAL) {
                ::PostMessage(hwnd, MSG_COMMAND, ResEditor::GBC_SAVE,0);
                mainframe->save_counter = 0;
            }
        }
    }

    return TRUE;
}

MainFrame::MainFrame()
    :curEditor(NULL)
    ,hResSelector(HWND_INVALID)
    ,selectorSize(20)
    ,hRefPanel(HWND_INVALID)
    ,isModified(FALSE)
    ,save_counter(0)
    ,resNumber(0)
    ,defRdr("classic")
#ifdef _MSTUDIO_OFFICIAL_RELEASE
    ,authMode(FALSE)
    ,validDay(0)
    ,sn("")
    ,licenseMode("")
#endif
    ,str_id_begin(NCSRM_SYSSTR_MAXID)
{
}

MainFrame::~MainFrame()
{
    Class::unloadAllClasses();
    ClearValueTypes();
}

#ifdef _MSTUDIO_OFFICIAL_RELEASE
#define _PROMPT_CAPTION_WARNING _("Warning")
#define _PROMPT_NOAUTH_PROJ     _("The evaluation version of miniStudio cannot open multi-windows project.")
#define _PROMPT_NOAUTH          _("The evaluation version of miniStudio doesn't support such multi-window operations.")
#define _PROMPT_DUETO           _("Product license has expired. For the continued use of the software, please contact FMSoft.")
#define _PROMPT_INSERTDOG       _("Please make sure the softdog is connected.")
/////////////////////////////////////////////////////////

static int showMessageBox(const char* text, int status)
{
    if(mainframe) {
        HWND hHosting = mainframe->GetHandle();

        if (hHosting == HWND_INVALID)
            hHosting = HWND_DESKTOP;
        return ::MessageBox(hHosting, text, _PROMPT_CAPTION_WARNING, MB_OK);
    } else {
        fprintf(stderr, "%s:%s\n", _("Warning"), text);
    }
    return 0;
}

BOOL MainFrame::checkSoftDog(int* remainDay)
{
    int ret = fmsoftCheckDogValidity(remainDay);
    switch (ret) {
    case MSTUDIO_ERR_NOAUTH:
    case MSTUDIO_ERR_INVALIDAUTH: {
        //get the number of windows in the project
        char szPath[1024];
        sprintf(szPath,"%s/res/res.project", getProjectPath());
        xmlDocPtr cfgdoc;
        cfgdoc = xmlParseFile(szPath);
        int nrWnds = 0;

        if(cfgdoc) {
            xmlNodePtr node = xmlDocGetRootElement(cfgdoc);
            node = xhGetChild(node, "uieditor");
            if(node) {
                for(node=node->children; node; node = node->next) {
                    if(xhIsNode(node, "visible-window"))
                        nrWnds ++;
                    if (nrWnds > _MSTUDIO_MAXNUM_LIMITWNDS)
                        break;
                }
            }
            xmlFreeDoc(cfgdoc);
        }

        if (nrWnds > _MSTUDIO_MAXNUM_LIMITWNDS) {
            showMessageBox(_PROMPT_NOAUTH_PROJ, MB_OK);
            return FALSE;
        }
        //else if (nrWnds == 0)
        // don't show the message box
#if 0
        showMessageBox(_PROMPT_NOAUTH, MB_OK);
#endif
        authMode = FALSE;
        break;
    }

    case MSTUDIO_ERR_DUETOAUTH:
        validDay = *remainDay;
        showMessageBox(_PROMPT_DUETO, MB_OK);
        return FALSE;

    case MSTUDIO_ERR_VALIDAUTH:
        validDay = *remainDay;
        authMode = TRUE;
        break;
    }
    return TRUE;
}

BOOL MainFrame::checkLimit()
{
    if (authMode) {
        switch (fmsoftCheckDogValidity(&validDay)) {
        case MSTUDIO_ERR_NOAUTH:
        case MSTUDIO_ERR_INVALIDAUTH:
            //please insert softdog and retry.
            showMessageBox(_PROMPT_INSERTDOG, MB_OK);
            return FALSE;

        case MSTUDIO_ERR_DUETOAUTH:
            showMessageBox(_PROMPT_DUETO, MB_OK);
            return FALSE;

        case MSTUDIO_ERR_VALIDAUTH:
            return TRUE;

        }
    } else {
        int nrWnds = 0;
        ResManager* resMgr = g_env->getResManager(NCSRT_UI);
        if (resMgr) {
            resMgr->callSpecial("getWndsCount", &nrWnds);
        }

        if (nrWnds >= _MSTUDIO_MAXNUM_LIMITWNDS) {
            showMessageBox(_PROMPT_NOAUTH, MB_OK);
            return FALSE;
        }
    }

    return TRUE;
}

string MainFrame::checkSNInfo()
{
    if (sn.length() <= 0) {
        char sn_info[MSTUDIO_SN_MAXLEN + 1] = {0};
        char license[MSTUDIO_LICENSE_MAXLEN + 1] = {0};

        fmsoftCheckSNInfo(sn_info, license);

        sn = sn_info;
        licenseMode = license;
    }
    return sn;
}
#endif

BOOL MainFrame::Create()
{
    MGMainWnd::Create(WS_VISIBLE,
                      _("MG GUI Builder 1.0.8"),
                      0,0,g_rcScr.right,g_rcScr.bottom,
                      HWND_DESKTOP,(HCURSOR)0,(HMENU)NULL,
                      (HICON)0,
                      DWORD2Pixel(HDC_SCREEN, 0x674E4A));

    char szPath[1024];
    //try load from project, which name "<project-path>/res/res.project"
    sprintf(szPath,"%s/res/res.project", getProjectPath());

    xmlDocPtr cfgdoc;
    cfgdoc = xmlParseFile(szPath);

    if(cfgdoc == NULL) { //not exist or cannot load
        //create a new GUI Builder Config
        if(!newGUIBuilderConfig())
            return FALSE;
        cfgdoc = xmlParseFile(szPath);
    }

    if(cfgdoc == NULL)
        return FALSE;

    xmlNodePtr node = xmlDocGetRootElement(cfgdoc);

    //load Version
    xmlChar* xstr;
    if((xstr = xmlGetProp(node, (const xmlChar*)"version"))) {
        int major, micro=0, minor=0;
        const char* str = (const char*)xstr;
        major = atoi(str);
        str = strchr(str, '.');
        if(str) {
            minor = atoi(str+1);
            str = strchr(str+1,'.');
            if(str)
                micro = atoi(str+1);
        }

        prjVersion.setVersion(major, minor, micro);

        xmlFree(xstr);
    }

    xmlNodePtr cfgNode = xhGetChild(node, "configuration");

    if(!configuration.load(cfgNode)) {
        xmlFreeDoc(cfgdoc);
        return FALSE;
    }

#ifdef _MSTUDIO_LOCALE
    msd_locale_init(getConfigFile("lang.cfg").c_str());
#endif
    //load editors
    loadConfig(getConfigFile(MAINFRAMECFG).c_str());

    //load logo
    {
        string str = getConfigFile("icon/left_top.png");
        LoadBitmapFromFile(HDC_SCREEN, &bmp_logo, str.c_str());
    }

    //create hwndPanel
    RECT rt;
    GetClientRect(&rt);
    hRefPanel = CreateWindow("static", "", WS_BORDER | SS_NOTIFY, -1,
                             selectorSize, 0, RECTW(rt) - selectorSize, REFPANEL_HEIGHT, m_hWnd, 0);

    ::GetClientRect(hRefPanel, &rt);
    //create OK, Cancel Button
    CreateWindow("button", _("OK"), BS_PUSHBUTTON|WS_VISIBLE, IDOK,
                 rt.right - (80+20)*2, (REFPANEL_HEIGHT - 30)/2 ,80,30, hRefPanel,0);

    CreateWindow("button", _("Cancel"), BS_PUSHBUTTON|WS_VISIBLE, IDCANCEL,
                 rt.right - (80+20)*1, (REFPANEL_HEIGHT - 30)/2 ,80,30, hRefPanel,0);

    if(!initEditorManager())
        return FALSE;

    //load idrange
    loadIDRanageInfo(xhGetChild(node, "id-range-manager"));

    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        ResEditorInfo *resEditor = *it;
        if(resEditor) {
            resEditor->editor->updateRes();
        }
    }

    //loadConfig Must be called after all the editors have been ready!
    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        ResEditorInfo *resEditor = *it;
        if(resEditor) {
            resEditor->editor->loadConfig(node);
        }
    }

    xmlFreeDoc(cfgdoc);
    SetTimerEx(m_hWnd,1, 100, auto_save_timer);

    if(prjVersion < gbVersion) { //should save
        MessageBox(_("Message"),
                   _("Your project is an old miniStudio project, it will be converted to new project format."), MB_OK);
        for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
            ResEditorInfo *resEditor = *it;
            if(resEditor) {
                resEditor->editor->setChanged();
            }
        }
        onSave();//save all
    }

    return TRUE;

}

static bool load_ntb_item(xmlNodePtr node, PNTBITEMINFO pntbii)
{
    xmlChar* xstr;
    //get type
    if((xstr = xmlGetProp(node, (const xmlChar*) "type"))) {
        if(xmlStrcmp(xstr, (const xmlChar*)"radio") == 0) {
            pntbii->flags = NTBIF_CHECKBUTTON;
        } else if(xmlStrcmp(xstr, (const xmlChar*)"separation") == 0) {
            pntbii->flags = NTBIF_SEPARATOR;
        }

        xmlFree(xstr);
    } else {
        pntbii->flags = NTBIF_PUSHBUTTON;
    }

    if(pntbii->flags == NTBIF_SEPARATOR)
        return true;

    //get id
    pntbii->id = xhGetIntProp(node, "id");

    //get icon
    pntbii->bmp_cell = xhGetIntProp(node, "icon");

    //get caption
    xmlChar* xcaption = xmlGetProp(node, (const xmlChar*)"caption");
    if(xcaption) {
        strcpy(pntbii->text, (const char*)xcaption);
        if (pntbii->tip)
            strcpy(pntbii->tip, _((const char*)xcaption));
        xmlFree(xcaption);
    }
    return true;
}


static void selector_res_proc(HWND hwnd, LINT id, int nc, DWORD add_data)
{
    HWND hParent = GetParent(hwnd);
    SendMessage(hParent, MSG_COMMAND, nc, 0);
}

void MainFrame::loadSelector(xmlNodePtr node)
{
    if(node == NULL) {
        return;
    }

    NTBINFO ntb_info;
    NTBITEMINFO ntbii;
    xmlChar* pic_file;
    xmlChar* cells_number;

    pic_file = xmlGetProp(node, (const xmlChar*) "icons");

    if (LoadBitmap(HDC_SCREEN, &bmpSelector, g_env->getConfigFile((const char*)pic_file).c_str())) {
        xmlFree(pic_file);
        return;
    }

    xmlFree(pic_file);
    cells_number = xmlGetProp(node, (const xmlChar*) "cells");
    ntb_info.nr_cells = atoi((const char*)cells_number);
    xmlFree(cells_number);

    ntb_info.w_cell = 0;
    ntb_info.h_cell = 0;
    ntb_info.nr_cols = 0;
    ntb_info.image = &bmpSelector;

    gal_pixel pixel;
    RECT rc;
    GetClientRect(&rc);

    hResSelector = CreateWindow (CTRL_NEWTOOLBAR,
                                 "",
                                 WS_CHILD | WS_VISIBLE | NTBS_VERTICAL,
                                 RESSELECTOR_ID,
                                 0, TOOLBAR_HEIGHT - 2,
                                 0, RECTH(rc)-TOOLBAR_HEIGHT + 2,
                                 GetHandle(),
                                 (DWORD) &ntb_info);
    ::SetNotificationCallback (hResSelector, selector_res_proc);

    pixel = GetPixelInBitmap (&bmpSelector, 1, 1);
    ::SetWindowBkColor (hResSelector, pixel);
    ::InvalidateRect (hResSelector, NULL, TRUE);

    //create new tool bar
    for (node = node->xmlChildrenNode; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        memset(&ntbii, 0, sizeof(ntbii));
        char szText[100]="\0";
        ntbii.text = szText;
        ntbii.tip = NULL;
        if (load_ntb_item(node, &ntbii)) {
            ntbii.flags = NTBIF_CHECKBUTTON;
            editorIDs[ntbii.text]= ntbii.id;
            ::SendMessage (hResSelector, NTBM_ADDITEM, 0, (LPARAM)&ntbii);
        }
    }

    ::GetClientRect(hResSelector, &rc);
    selectorSize = RECTW(rc);
}

void MainFrame::loadConfig(const char* cfgFile)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;

    RECT rt;
    doc = xmlParseFile(cfgFile);
    if(doc == NULL) {
        LOG_WARNING( "MainFrame::loadConfig parser file \"%s\" failed\n", cfgFile);
        return ;
    }

    //get root
    node = xmlDocGetRootElement(doc);
    if(node == NULL || xmlStrcmp(node->name, (const xmlChar*)"mainframe") ) {
        LOG_WARNING( "MainFrame::loadConfg file \"%s\" has not the root \"mainframe\"\n",cfgFile);
        return ;
    }

    GetClientRect(&rt);
    rt.top = TOOLBAR_HEIGHT;
    rt.left += selectorSize;

    for(node = node->xmlChildrenNode; node; node = node->next) {
        if(node->type != XML_ELEMENT_NODE)
            continue;

        if(xhIsNode(node,"menu-set")) {
            commMenu.loadFromXml(node);
        } else if(xhIsNode(node,"res-editor")) {
            ResEditorInfo * resEditorInfo =new ResEditorInfo(node, this);
            if(resEditorInfo) {
                resEditorInfo->editorId = editorIDs[resEditorInfo->strCaption.c_str()];
                resEditors.push_back(resEditorInfo);
                //move window
                resEditorInfo->editor->MoveWindow(&rt,FALSE);
            }
        } else if(xhIsNode(node, "selector")) {
            loadSelector(node);
        }
    }

    if(doc)
        xmlFreeDoc(doc);

    if(resEditors.size()>0) {
        int i = 0;
        resNumber = resEditors.size();
        resManagers = new ResManager*[resEditors.size()];
        for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                it != resEditors.end(); ++it) {
            resManagers[i++] = (*it)->editor;
        }
    }

    return ;
}

#define IDC_MENU_WINDOW   0x101111
void MainFrame::initWindowMenu(HMENU hMenu, ResEditorInfo * who)
{
    MENUITEMINFO mii, popmii;
    char szText[100];
    memset(&mii, 0, sizeof(mii));
    memset(&popmii, 0, sizeof(popmii));
    popmii.mask = MIIM_ID|MIIM_TYPE;
    popmii.typedata = (DWORD)szText;
    popmii.type = MFT_STRING;
    popmii.id = IDC_MENU_WINDOW;
    strcpy(szText, _("Window"));
    HMENU hSubMenu = CreatePopupMenu(&popmii);

    mii.mask = MIIM_ID|MIIM_TYPE|MIIM_CHECKMARKS|MIIM_STATE;
    mii.type = MFT_RADIOCHECK;

    int i = 0;
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end(); ++it, i++) {
        ResEditorInfo* res = *it;
        mii.id = res->editorId;
        mii.state = who==res?MFS_CHECKED:MFS_UNCHECKED;
        //		mii.typedata = (DWORD)res->strCaption.c_str();
        mii.typedata = (DWORD)_(res->strCaption.c_str());
        InsertMenuItem(hSubMenu,i,TRUE, &mii);
    }

    hSubMenu = StripPopupHead(hSubMenu);

    popmii.mask |= MIIM_SUBMENU;
    popmii.hsubmenu = hSubMenu;
    InsertMenuItem(hMenu,GetMenuItemCount(hMenu)-1, TRUE, &popmii);
}

static void checkSelectorRes(HWND hwnd, LINT id, BOOL check)
{
    NTBITEMINFO ntbii;
    ntbii.which = MTB_WHICH_FLAGS;
    ::SendMessage(hwnd, NTBM_GETITEM, id, (LPARAM)&ntbii);

    if ((check && (ntbii.flags & NTBIF_CHECKED)) ||
            (!check && !(ntbii.flags & NTBIF_CHECKED)))
        return;

    if (check)
        ntbii.flags |= NTBIF_CHECKED;
    else
        ntbii.flags &= ~NTBIF_CHECKED;
    ::SendMessage(hwnd, NTBM_SETITEM, id, (LPARAM)&ntbii);
}

BOOL MainFrame::initEditorManager()
{
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it!= resEditors.end(); ++it) {
        ResEditorInfo* resEditorInfo = *it;
        NTBITEMINFO ntbii;
        ntbii.which = MTB_WHICH_ADDDATA;
        ntbii.add_data = (DWORD)resEditorInfo;
        ::SendMessage(hResSelector, NTBM_SETITEM, resEditorInfo->editorId, (LPARAM)&ntbii);
        initWindowMenu(resEditorInfo->hMenu,resEditorInfo);
    }

    checkSelectorRes(hResSelector, 1001, TRUE);
    if (!resEditors.empty())
        setCurResEditor(*resEditors.begin());
    return TRUE;
}

void MainFrame::setCurResEditor(ResEditorInfo *newcur)
{
    if(newcur == NULL)
        return;

    if(newcur != curEditor) {
        int oldid = -1;
        //hide old one
        if(curEditor) {
            oldid = curEditor->editorId;
            curEditor->editor->ShowWindow(SW_HIDE);
            ::ShowWindow(curEditor->hToolbar,SW_HIDE);
            curEditor->editor->active(false,0);
        }

        curEditor = newcur;

        //disable other button
        if (isRefMode()) {
            for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                    it!= resEditors.end(); ++it) {
                ResEditorInfo* editorInfo = *it;
                if (editorInfo->editorId != newcur->editorId)
                    ::SendMessage(hResSelector, NTBM_ENABLEITEM, editorInfo->editorId, FALSE);
            }
            //disable Window menu
            EnableMenuItem(newcur->hMenu, IDC_MENU_WINDOW, MFS_DISABLED);
        } else {
            for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                    it!= resEditors.end(); ++it) {
                ResEditorInfo* editorInfo = *it;
                ::SendMessage(hResSelector, NTBM_ENABLEITEM, editorInfo->editorId, TRUE);
            }
            EnableMenuItem(newcur->hMenu, IDC_MENU_WINDOW, MFS_ENABLED);
        }

        if (oldid != -1)
            checkSelectorRes(hResSelector, oldid, FALSE);

        ::SendMessage(hResSelector, NTBM_ENABLEITEM, newcur->editorId, TRUE);
        checkSelectorRes(hResSelector, newcur->editorId, TRUE);

        //set rect
        RECT rt;
        GetClientRect(&rt);

        rt.top += TOOLBAR_HEIGHT;
        rt.left += selectorSize;

        if (isRefMode()) {
            rt.bottom -= REFPANEL_HEIGHT;
            //show hwndPanel
            ::MoveWindow(hRefPanel, rt.left, rt.bottom, RECTW(rt), REFPANEL_HEIGHT, FALSE);
            ::ShowWindow(hRefPanel, SW_SHOW);
        } else {
            //hide hwndPanel
            ::ShowWindow(hRefPanel, SW_HIDE);
        }

        curEditor->editor->MoveWindow(&rt, TRUE);

        //show current editor
        curEditor->editor->ShowWindow(SW_SHOW);
        curEditor->editor->active(true, isRefMode()?ResEditor::REF_ACTIVE:ResEditor::USER_ACTIVE);
        SetMenu(curEditor->hMenu);
        //show toolbar
        ::ShowWindow(curEditor->hToolbar, SW_SHOW);

        // fix a bug for menubar hide ....
        UpdateWindow();
        //InvalidateRect();
        ghAccel = curEditor->hAccel;
    }
}

void MainFrame::setMainCaption(const char* caption)
{
    string str = caption;
    str += _(" - miniStudio 1.0 ");
#ifdef _MSTUDIO_OFFICIAL_RELEASE
    if (authMode) {
        char day[256];

        if (licenseMode.length() > 0)
            str.append(licenseMode.c_str());
        else {
            str.append("BIZ");
        }

        sprintf (day, _(" (%d days remaining)"), validDay);
        str.append(day);
    } else {
        char szText[256];
        sprintf(szText, _("(Evaluation version, only support %d window!)"),_MSTUDIO_MAXNUM_LIMITWNDS);
        str.append(szText);
    }
#endif

#ifdef WIN32
	char ascii_str[1024];
	utf8toascii(str.c_str(), ascii_str, sizeof(ascii_str));
	VFBSetCaption(ascii_str);
#else
	VFBSetCaption(str.c_str());
#endif
}

ResManager* MainFrame::getResManager(int type)
{
    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        ResEditorInfo* resedin = *it;
        if(resedin && resedin->editor)
            if(resedin->editor->getTypeMask() & type)
                return (ResManager*)resedin->editor;
    }

    return NULL;
}

int MainFrame::selectID(int type)
{
    ResEditorInfo * refedResEditor = NULL, *oldResEditor = NULL;
    int resid = -1;
    MSG Msg;
    //TODO : select resource
    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        refedResEditor = *it;
        if(refedResEditor && refedResEditor->editor)
            if(refedResEditor->editor->getTypeMask() & type)
                goto CONTINUE;
    }
    return -1;

CONTINUE:

    //1 push curEditor into stackManager
    stackManager.push_back(curEditor);
    //2 set refered editor as current editor
    setCurResEditor(refedResEditor);

    // run message loop
    while (GetMessage (&Msg, m_hWnd)) {
        TranslateMessage(&Msg);
        if(Msg.hwnd == hRefPanel && Msg.message == MSG_COMMAND) {
            LINT id = LOWORD (Msg.wParam);
            if(id == IDOK || id == IDCANCEL) {
                if(id == IDOK) {
                    resid = refedResEditor->editor->getSelectedResID();
                    if(resid == -1) { //select a invalide
                        if(YesNoBox(_("Error"),_("Please select a valid resource. Do you want to reselect?"))==IDYES)
                            continue;
                    }
                }
                goto SETRESID;
            }
        }
        DispatchMessage(&Msg);
    }
    PostQuitMessage();
    return -1;

SETRESID:
    oldResEditor = stackManager.back();
    stackManager.pop_back();
    setCurResEditor(oldResEditor);

    return resid;
}

#ifdef WIN32
#define COMMAND_RANGEEX(idbegin, idend, onCmdRange) \
	if(ctrlId >= (idbegin) && ctrlId <= (idend)){  \
		onCmdRange(ctrlId,HIWORD(wParam), (HWND)lParam); \
		break; }
#endif
BEGIN_MSG_MAP(MainFrame)
#ifdef WIN32
case MSG_COMMAND:
{
    int ctrlId = LOWORD(wParam);
    COMMAND_RANGEEX(ResEditor::MENUCMD_SELECTORID_BASE + 1, ResEditor::MENUCMD_SELECTORID_END, onResSwitch)
    COMMAND_RANGEEX(ResEditor::MENUCMD_SYSID_BASE, ResEditor::MENUCMD_SYSID_END, onSysMenuCmd)
    COMMAND_RANGEEX(ResEditor::EDITOR_MENUID_BASE, ResEditor::EDITOR_MENUID_MAX, onEditorMenuCmd)
}
break;
#else
BEGIN_COMMAND_MAP
MAP_COMMAND_RANGEEX(ResEditor::MENUCMD_SELECTORID_BASE + 1, ResEditor::MENUCMD_SELECTORID_END, onResSwitch)
MAP_COMMAND_RANGEEX(ResEditor::MENUCMD_SYSID_BASE, ResEditor::MENUCMD_SYSID_END, onSysMenuCmd)
MAP_COMMAND_RANGEEX(ResEditor::EDITOR_MENUID_BASE, ResEditor::EDITOR_MENUID_MAX, onEditorMenuCmd)
END_COMMAND_MAP
#endif
MAP_CSIZECHANGED(onCSizeChanged)
MAP_ERASEBKGNDNN(onEraseBkgnd)
MAP_DESTROY(onDestroy)
case MSG_CLOSE:
::DestroyMainWindow(m_hWnd);
_mainwnd_destroy = 1;
break;
END_MSG_MAP

void MainFrame::onEditorMenuCmd(int ctrlid, int code, HWND hwnd)
{
    if(curEditor)
        curEditor->editor->executeCommand(ctrlid, code, (DWORD)hwnd);
}

void MainFrame::onResSwitch(int editorId, int code, HWND hwnd)
{
    if (editorId == curEditor->editorId) {
        checkSelectorRes(hResSelector, editorId, TRUE);
        return;
    }

    NTBITEMINFO ntbii;
    ntbii.which = MTB_WHICH_ADDDATA;
    ::SendMessage(hResSelector, NTBM_GETITEM, editorId, (LPARAM)&ntbii);
    ResEditorInfo * resEditorInfo = (ResEditorInfo*) ntbii.add_data;
    setCurResEditor(resEditorInfo);
}

void MainFrame::onCSizeChanged(int cx, int cy)
{
    //move toolbar
    if(curEditor) {
        ::MoveWindow(curEditor->hToolbar, selectorSize - 2, 0,
                     cx - selectorSize + 2, TOOLBAR_HEIGHT, TRUE);
    }

    //move editor window
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end(); ++it) {
        if (isRefMode())
            (*it)->editor->MoveWindow(selectorSize, TOOLBAR_HEIGHT,
                                      cx - selectorSize, cy - TOOLBAR_HEIGHT - REFPANEL_HEIGHT);
        else
            (*it)->editor->MoveWindow(selectorSize, TOOLBAR_HEIGHT,
                                      cx - selectorSize, cy - TOOLBAR_HEIGHT);

    }
}

BOOL MainFrame::onEraseBkgnd(HDC hdc, const PRECT pclip)
{
    FillBoxWithBitmap(hdc, 0, 0, 0, 0, &bmp_logo);
    return TRUE;
}

void MainFrame::onDestroy()
{
    mainframe = NULL;
    KillTimer(1);
    onSave();

    SocketClient::getInstance()->closeSocket();

    for(list<ResEditorInfo*>::iterator it=resEditors.begin();
            it!=resEditors.end(); ++it) {
        delete *it;
    }
    resEditors.clear();

    for(map<int, StringEntry*>::iterator it = stringPool.begin(); it!= stringPool.end(); ++it) {
        delete it->second;
    }

    delete[] resManagers;

    UnloadBitmap(&bmpSelector);
    UnloadBitmap(&bmp_logo);

    PostQuitMessage();
}

/////////////////////////////////////////
#define   ID_VERSION        100
#define   ID_BUILDNO        101
#define   IDB_LOGO          104
#define   ID_LABEL_SN       105
#define   ID_SN             106
#define   LOGO_FILE         "icon/gblogo.png"
#define   PKG_STR_MAXLEN    256

static LRESULT AboutProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BITMAP logo_bmp;
    switch(message) {
        case MSG_INITDIALOG: 
            {
                //load logo
                LoadBitmapFromFile(HDC_SCREEN, &logo_bmp, g_env->getConfigFile(LOGO_FILE).c_str());
                SendDlgItemMessage(hwnd, IDB_LOGO, STM_SETIMAGE, (WPARAM)(&logo_bmp), 0);

                //set version
#ifdef _MSTUDIO_PACKAGE_STRING
                {
                    char str[PKG_STR_MAXLEN];
                    string package_version = _MSTUDIO_PACKAGE_STRING;

                    //replace separate character '_' to ' '
                    for (string::size_type i = 0; i != package_version.size(); i++)
                        if (package_version[i] == '_')
                            package_version[i] = ' ';
                    sprintf (str, "Special version for %s", package_version.c_str());

                    str[PKG_STR_MAXLEN - 1] = 0;
                    SetDlgItemText(hwnd, ID_VERSION, str);
                }
#else
                SetDlgItemText(hwnd, ID_VERSION, PACKAGE_STRING);
#endif

                SetDlgItemText(hwnd, ID_BUILDNO, BUILD_VERSION);

#ifdef _MSTUDIO_OFFICIAL_RELEASE
                {
                    //process sn information 
                    string sn = g_env->checkSNInfo();
                    if (sn.length() > 0)
                        SetDlgItemText(hwnd, ID_SN, sn.c_str());
                    else {
                        //no sn
                        ShowWindow(GetDlgItem(hwnd, ID_LABEL_SN), SW_HIDE);
                        ShowWindow(GetDlgItem(hwnd, ID_SN), SW_HIDE);
                    }
                }
#endif
            }
            return TRUE;

        case MSG_CLOSE:
            EndDialog(hwnd, 0);
            break;
        case MSG_COMMAND:
            if(LOWORD(wParam) == IDOK)
                EndDialog(hwnd,0);
            break;
        case MSG_DESTROY:
            UnloadBitmap(&logo_bmp);
            break;
    }

    return AutoCenterDlgProc(hwnd, message, wParam, lParam);
}

/////////////////////////////////////////
void MainFrame::onSysMenuCmd(int ctrlid, int code, HWND hwnd)
{
    switch(ctrlid) {
    case ResEditor::GBC_SAVE:
        onSave();
        break;

    case ResEditor::GBC_EXIT:
        onExit();
        break;

    case ResEditor::GBC_ABOUT:
        DialogBoxIndirectParam(GetDlgTemplate(ID_ABOUT), m_hWnd, AboutProc,0);
        break;

    case ResEditor::GBC_HELP:
        break;

    case ResEditor::GBC_IDRANGEMANAGER: {
        IDRangeEditor idRangeEditor(m_hWnd);
        if(idRangeEditor.DoMode())
			onMenuStatusChanged(NULL, ResEditor::GBC_SAVE, MFS_DISABLED, MSF_EXCLUDE);	
    }
    break;
    case ResEditor::INCORE_RES_MGNCSCFG:
    case ResEditor::INCORE_RES_PACKAGE:
        generateIncoreRes(ctrlid - ResEditor::INCORE_RES_MGNCSCFG);
        break;
    default:
        break;
    }
}

//input: id, str
//ouput: idmaps, strbuff, offset
static void saveString(int id, string str,
                       NCSRM_IDITEM *idmaps, string *strbuff, Uint32 *offset)
{
    Uint32 str_len = 0, fill_len = 0;
    const char end_chars[]="\0\0\0\0\0\0\0\0";

    //3:\0\0\0
    str_len = str.length();
    fill_len = str_len + 3;

    //for 4 bytes-align
    if (fill_len%4) {
        fill_len = ((fill_len>>2) + 1)<<2;
    }

    //id-item
    idmaps->id = id;
    idmaps->filename_id = 0;
    idmaps->offset = *offset;

#if 0
    fprintf (stderr, "saveStringMap=> <id, str>:<0x%0x, %s> need %d bytes, and fill %d bytes. \n",
             id, str.c_str(), str_len, fill_len);
#endif
    strbuff->append(str.c_str());
    strbuff->append(end_chars, fill_len - str_len);
    *offset += fill_len;
}

void MainFrame::writeStringResource(BinStream* bin)
{
    Uint32 i = 0, nr_id ;
    Uint32 base_sect, offset_data;
    NCSRM_IDITEM *idmaps;
    string strbuff;

    if (stringPool.size() <= 0)
        return;

    nr_id = stringPool.size() + 1; // for default renderer name string ...

    base_sect = sizeof(NCSRM_SECTHEADER) + nr_id * sizeof(NCSRM_IDITEM);
    offset_data = base_sect;

    idmaps = (NCSRM_IDITEM *)calloc (nr_id, sizeof(NCSRM_IDITEM));

    //first: save system string
    //defRdr
    saveString(NCSRM_SYSSTR_DEFRDR, defRdr, &(idmaps[i++]), &strbuff, &offset_data);

    //second: save stringPool
    for(map<int, StringEntry*>::iterator it = stringPool.begin(); it!= stringPool.end(); i++, ++it) {
        saveString(it->first, it->second->str, &(idmaps[i]), &strbuff, &offset_data);
    }

    //sect-header
    bin->save32(offset_data);	// ------ NCSRM_SECTHEADER::sect_size
    bin->save32(nr_id);			// ------ NCSRM_SECTHEADER::size_maps

    //id maps
    for(i = 0; i < nr_id; i++) {
        bin->save32(idmaps[i].id);			// ------NCSRM_IDITEM.id
        bin->save32(idmaps[i].filename_id);	// ------NCSRM_IDITEM.filename_id
        bin->save32(idmaps[i].offset);		// ------NCSRM_IDITEM.offset
    }

    //string data
    bin->save8Arr((const uint8_t*)strbuff.c_str(), offset_data - base_sect);

    free(idmaps);
}

void MainFrame::onExit()
{
    onSave();
#ifdef _MSTUDIO_LOCALE
    free_mo_info();
#endif
    PostQuitMessage();
}

void saveTypeItem(BinStream *bin, int idx, int type)
{
    int size, typePos;
    //save NCSRM_TYPEITEM information
    bin->seek(0, StreamStorage::seek_end);
    size = bin->tell();
    typePos = sizeof(NCSRM_HEADER) + idx * sizeof(NCSRM_TYPEITEM);
    bin->seek(typePos, StreamStorage::seek_begin);
    bin->save32(type);
    bin->save32(0);
    bin->save32(size);
    bin->seek(0, StreamStorage::seek_end);
}

#include "dirent.h"
void MainFrame::onSave()
{
    //The number of resource type.
    if (resEditors.size() <= 0) {
        return;
    }

    isModified = FALSE;
    save_counter = 0;

    //save configuration
    saveConfig();
    int nr_sect = 1, idx = 0;

    //save xxx_id.h
    FileStreamStorage headfss(getHeaderFile());
    TextStream headStream(&headfss);
    LOG_WARNING( "header(%s), package(%s) \n",
                 getHeaderFile(), getResPackageName().c_str());

    headStream.println("/****************************************/");
    headStream.println("/* This file is generate by miniStudio,    */");
    headStream.println("/* Please Don't edit it.                */");
    headStream.println("/*                                      */");
    headStream.println("/* This file define the IDs used by     */");
    headStream.println("/* resource.                            */");
    headStream.println("/****************************************/");
    headStream.println("\n\n");
    headStream.println("#ifndef _RESOURCE_ID_H_");
    headStream.println("#define _RESOURCE_ID_H_");
    headStream.println("");

    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it!=resEditors.end(); ++it) {
        ResEditorInfo * resEditorInfo = *it;
        Uint32 type = resEditorInfo->editor->getTypeMask() & NCSRT_MASK;
        if (type == 0) //skip id editor
            continue;
        nr_sect ++;
        //transIDs, generate id header file
        resEditorInfo->editor->transIDs(&headStream);
    }
    headStream.println("#endif /*_RESOURCE_ID_H_*/");
    headfss.close();

    //save resource package file
    NCSRM_TYPEITEM *type_item = (NCSRM_TYPEITEM *)calloc(nr_sect, sizeof(NCSRM_TYPEITEM));
    FileStreamStorage res_package(getResPackageName().c_str(), true);
    BinStream bin(&res_package, getEndian());

    //save the header of id table : NCSRM_HEADER
    //save magic mgrp(0x4d475250)
    bin.save32(0x4d475250);
    //save version , package_version[NCSRM_VERSION_LEN];
    bin.save8Arr((const uint8_t*)getVersion(),NCSRM_VERSION_LEN);
    //save format version,format_version[NCSRM_VERSION_LEN];
    bin.save8Arr((const uint8_t*)getFormatVersion(),NCSRM_VERSION_LEN);
    //save encoding: char_encoding[NCSRM_ENCODING_LEN];
    bin.save8Arr((const uint8_t*)getEncoding(), NCSRM_ENCODING_LEN);
    //save vendor: vendor[NCSRM_VENDOR_LEN];
    bin.save8Arr((const uint8_t*)getVendor(), NCSRM_VENDOR_LEN);
    //save the number of section, nr_sect;
    bin.save32(nr_sect);
    //write type item, ...
    bin.save8Arr((const uint8_t*)type_item, nr_sect * sizeof(NCSRM_TYPEITEM));

    idx = 0;
    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it!=resEditors.end(); ++it) {
        ResEditorInfo * resEditorInfo = *it;
        int type = resEditorInfo->editor->getTypeMask() & NCSRT_MASK;
        if (type == 0) //skip id editor
            continue;

        saveTypeItem(&bin, idx, type);
        resEditorInfo->editor->save(&bin);
        idx ++;
    }

    //write string section
    saveTypeItem(&bin, idx, NCSRT_STRING);
    writeStringResource(&bin);

    res_package.close();

    HPACKAGE hPackage = ncsLoadResPackageFromFile(getResPackageName().c_str());
    if (hPackage != HPACKAGE_NULL)
        ncsUnloadResPackage(hPackage);
    //enable save menu
    onMenuStatusChanged(curEditor->editor, ResEditor::GBC_SAVE, MFS_DISABLED, MSF_INCLUDE);

    //synchronous project
    SocketClient::getInstance()->syncProject(getPrjName());

    free(type_item);
}

static HMENU getMenuItemInfoById(HMENU hMenu, LINT id, MENUITEMINFO* info)
{
    MENUITEMINFO mii;
    if(GetMenuItemInfo(hMenu, id, MF_BYCOMMAND, info) == 0)
        return hMenu;


    memset(&mii, 0, sizeof(mii));
    int i;
    mii.mask = MIIM_SUBMENU;
    for(i=0; GetMenuItemInfo(hMenu,i,MF_BYPOSITION,&mii) == 0; i++) {
        if(mii.hsubmenu == 0)
            continue;
        HMENU howner = getMenuItemInfoById(mii.hsubmenu, id, info);
        if(howner)
            return howner;
    }

    return (HMENU)0;
}

void MainFrame::onMenuStatusChanged(ResEditor* editor, LINT id, DWORD newStatus, int type/*=MSF_INCLUDE*/)
{
    ResEditorInfo *rei = NULL;
    if(id == 0)
        return ;

    if(editor == NULL && !(id >= ResEditor::MENUCMD_SYSID_BASE && id <= ResEditor::MENUCMD_SYSID_END))
        return ;

    if(id >= ResEditor::MENUCMD_SYSID_BASE && id <= ResEditor::MENUCMD_SYSID_END)
        editor = NULL;

    if(id == ResEditor::GBC_SAVE && newStatus == MFS_DISABLED && type == MSF_EXCLUDE) {
        isModified = TRUE;
    }


    if(editor) {
        if(curEditor && curEditor->editor == editor)
            rei = curEditor;
        else {
            for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                    it != resEditors.end(); ++it) {
                rei = *it;
                if(rei->editor == editor)
                    break;
            }
        }
        setEditorMenuStatus(rei, id, newStatus, type);
    } else {
        for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                it != resEditors.end(); ++it) {
            setEditorMenuStatus(*it, id, newStatus, type);
        }
    }
}

int MainFrame::getEditorMenuStatus(ResEditor* editor, LINT id, UINT* status)
{
    if (!status)
        return -1;

    ResEditorInfo *rei = NULL;

    if (editor) {
        if (!curEditor || curEditor->editor == editor)
            rei = curEditor;
        else {
            for(list<ResEditorInfo*>::iterator it = resEditors.begin();
                    it != resEditors.end(); ++it) {
                if((*it)->editor == editor) {
                    rei = *it;
                    break;
                }
            }
        }
    } else
        rei = curEditor;

    if (!rei)
        return -1;

    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.mask = MIIM_STATE;

    if (getMenuItemInfoById(rei->hMenu, id, &mii)) {
        *status = mii.state;
        return 0;
    }

    return -1;
}

void MainFrame::setEditorMenuStatus(ResEditorInfo *rei, LINT id, DWORD newStatus, int type)
{

    if(!rei)
        return ;

    //find the item
    MENUITEMINFO mii;
    HMENU hOwner = 0;
    memset(&mii, 0, sizeof(mii));
    mii.mask = MIIM_STATE;
    if((hOwner=getMenuItemInfoById(rei->hMenu, id, &mii))==0)
        goto SET_TOOLBAR ;

    switch(type) {
    case MSF_INCLUDE:
        mii.state |= newStatus;
        break;
    case MSF_EXCLUDE:
        mii.state &= (~newStatus);
        break;
    case MSF_REPLACE:
        mii.state = newStatus;
        break;
    default:
        goto SET_TOOLBAR ;
    }

    SetMenuItemInfo(hOwner, id, MF_BYCOMMAND, &mii);

SET_TOOLBAR:

    if(newStatus&MFS_DISABLED) {
        ::SendMessage(rei->hToolbar, NTBM_ENABLEITEM, id, (LPARAM)(type == MSF_EXCLUDE));
    }

}

void MainFrame::processArgs(int argc, const char** argv)
{
    int i = 0;
    while (i < argc) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-project") == 0) {
                setProjectPath(argv[++i]);
            } else if(strcmp(argv[i], "-project-name") == 0) {
                strPrjName = argv[++i];
            } else if(strcmp(argv[i], "-project-file") == 0
                      || strcmp(argv[i], "file") == 0) {
                openFile(argv[++i]);
            } else if(strcmp(argv[i], "-update-file") == 0) {
                updateFile(argv[++i]);
            } else if(strcmp(argv[i],"-config-file") == 0) {
                configFile = argv[++i];
            } else if (strcmp(argv[i], "-def-renderer") == 0) {
                setDefRdrName(argv[++i]);
            } else if (strcmp(argv[i], "-addr") == 0) {
                SocketClient::getInstance()->setSocketIP((char*)argv[++i]);
            } else if (strcmp(argv[i], "-port") == 0) {
                SocketClient::getInstance()->setSocketPort(atoi(argv[++i]));
            }
        }

        i++;
    }

    SetResPath(getProjectPath());
}

void MainFrame::processArgs(CharStream* cs)
{
    strPrjName = "test";
    int len;
    if(cs == NULL)
        return ;

    int ch;
    while((ch = cs->getc()) && ch != '\r' && ch != '\n') {
        if(ch == '-') {
            char szName[1024];
            if(cs->getWord(szName, sizeof(szName)) <= 0)
                continue;
            if(strcmp(szName, "project") == 0) {
                //TODO get project path
                if((len = cs->getPath(szName,sizeof(szName))) > 0) {
                    if(szName[len-1] == '/' || szName[len-1] == '\\')
                        szName[len-1] = '\0';
                    setProjectPath(szName);
                }
            } else if(strcmp(szName, "project-name") == 0) {
                //TODO get prohject name
                if(cs->getPath(szName,sizeof(szName)) > 0) {
                    strPrjName = szName;
                }
            } else if(strcmp(szName, "project-file") == 0
                      || strcmp(szName, "file") == 0) {
                //TODO open file
                if(cs->getPath(szName,sizeof(szName)) > 0) {
                    openFile(szName);
                }
            } else if(strcmp(szName, "update-file") == 0) {
                //TODO update file
                if(cs->getPath(szName,sizeof(szName)) > 0) {
                    updateFile(szName);
                }
            } else if(strcmp(szName,"config-file") == 0) {
                if(cs->getPath(szName,sizeof(szName)) > 0)
                    configFile = szName;
            } else if (strcmp(szName, "def-renderer") == 0) {
                if((len = cs->getPath(szName, sizeof(szName))) > 0) {
                    setDefRdrName(szName);
                }
            } else if (strcmp(szName, "addr") == 0) {
                if((len = cs->getPath(szName, sizeof(szName))) > 0) {
                    SocketClient::getInstance()->setSocketIP(szName);
                }
            } else if (strcmp(szName, "port") == 0) {
                if((len = cs->getPath(szName, sizeof(szName))) > 0) {
                    SocketClient::getInstance()->setSocketPort(atoi(szName));
                }
            }
        }
    }
    SetResPath(getProjectPath());
}


void MainFrame::updateResName(int id, const char* newName)
{
    int type = ID2TYPE(id);
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end();  ++ it) {
        ResEditorInfo* resinfo = *it;
        //send to all the editors except itself
        if(!(type & resinfo->editor->getTypeMask())) {
            resinfo->editor->onResNameChanged(id, newName);
        }
    }
}

void MainFrame::updateResId(int oldId, int newId)
{
    if(ID2TYPE(oldId) != ID2TYPE(newId))
        return;

    int type = ID2TYPE(oldId);
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end();  ++ it) {
        ResEditorInfo* resinfo = *it;
        if(!(type & resinfo->editor->getTypeMask())) {
            resinfo->editor->onResIdChanged(oldId, newId);
        }
    }
}

void MainFrame::updateResValue(int id)
{
    int type = ID2TYPE(id);
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end();  ++ it) {
        ResEditorInfo* resinfo = *it;
        if(!(type & resinfo->editor->getTypeMask())) {
            resinfo->editor->onResValueChanged(id);
        }
    }
}

void MainFrame::updateAllRes(int type)
{
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end();  ++ it) {
        ResEditorInfo* resinfo = *it;
        if(!(type & resinfo->editor->getTypeMask())) {
            resinfo->editor->onAllResUpdated(type);
        }
    }
}

void MainFrame::deleteRefRes(int id)
{
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end();  ++ it) {
        ResEditorInfo* resinfo = *it;
        if(!(ID2TYPE(id) & resinfo->editor->getTypeMask())) {
            resinfo->editor->onRefResDeleted(id);
        }
    }
}

void MainFrame::setProjectPath(const char* path)
{
    char sep;
    if(path == NULL)
        return;

    sep = getPathSeparator();
    int len = strlen(path);
    if(path[len-1] == sep)
        len --;

    if(!strPrjPath.empty() && strncmp(strPrjPath.c_str(), path, len) == 0)
        return;

    strPrjPath.clear();
    strPrjPath.append(path, len);

	strResPath = strPrjPath + sep + "res";
	strHeaderFile = strPrjPath + sep + "include" + sep + "resource.h";
	strSrcPath = strPrjPath + sep + "src";

    //update res
    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        ResEditorInfo *resEditor = *it;
        if(resEditor)
            resEditor->editor->updateRes();
    }
}

const char* MainFrame::getResFileName(const char* file)
{
    const char* filename;

    if(file == NULL)
        return NULL;

    filename = strrchr(file, '/');
    if(filename == NULL)
        filename = strrchr(file, '\\');

    if(filename) {
        //compare it in the res
        if(strncmp(strResPath.c_str(), file, strResPath.length())!= 0)
            return NULL;

        if(filename != file) {
            filename --;
            while(filename > file && (*filename != '/' || *filename != '\\'))
                filename --;

            if(filename > file)
                filename ++;
        }
    } else {
        filename = file;
    }

    return filename;
}

void MainFrame::openFile(const char* file)
{
    const char* filename = getResFileName(file);
    //same as path
    if(filename == NULL)
        return ;

    //get the extend
    const char* strext = strrchr(filename,'.');
    if(strext == NULL)
        return ;

    strext ++;

    for(list<ResEditorInfo*>::iterator it = resEditors.begin(); it != resEditors.end(); ++it) {
        ResEditorInfo *resEditor = *it;
        if(resEditor && resEditor->isSupportFile(strext)) {
            resEditor->editor->open(filename);
            break;
        }
    }

}

void MainFrame::updateFile(const char* updateFile)
{
    //TODO update file
}

HMENU MainFrame::createPopMenuFromConfig(int popmenu_id, mapex<int, int> id_state)
{
    if (curEditor)
        return curEditor->menuManager.createPopMenu(popmenu_id, id_state);
    return 0;
}

BOOL MainFrame::newIDRange(IDRangeManager* idrm)
{
    if(idrm == NULL)
        return FALSE;

    NewIDRange newIdRange(m_hWnd, idrm);
    if(newIdRange.DoMode())
	{
		onMenuStatusChanged(NULL, ResEditor::GBC_SAVE, MFS_DISABLED, MSF_EXCLUDE);	
		return TRUE;
	}
	return FALSE;
}

static IDRangeManager* find_idrange_by(vector<IDRangeManager*>&list, int type, const char* name)
{
    for(vector<IDRangeManager*>::iterator it = list.begin();
            it != list.end(); ++it) {
        if((*it)->getType() == type) {
            if(!name || (name && strcmp(name, (*it)->getName()) == 0))
                return *it;
        }
    }
    return NULL;
}

BOOL MainFrame::loadIDRanageInfo(xmlNodePtr node)
{
    if(!node || !xhIsNode(node, "id-range-manager"))
        return FALSE;

    vector<IDRangeManager*> list;
    GetAllIDRangeManagers(list);

    for(node = node->xmlChildrenNode; node; node = node->next) {
        if(xhIsNode(node, "resource")) {
            int type = xhGetIntProp(node, "type");
            xmlChar* name = xmlGetProp(node,(xmlChar*)"name");
            IDRangeManager* idrm = find_idrange_by(list, type,(const char*)name);
            if(idrm) {
                xmlChar* xowner;
                int min, max;
                for(xmlNodePtr node_idrange = node->xmlChildrenNode; node_idrange; node_idrange = node_idrange->next) {
                    if(!xhIsNode(node_idrange, "id-range"))
                        continue;

                    min = xhGetIntProp(node_idrange, "min");
                    max = xhGetIntProp(node_idrange, "max");
                    xowner = xmlGetProp(node_idrange,(const xmlChar*)"owner");
                    if(xowner) {
                        IDRangeOwner *owner = idrange_owner_manager.getOwner((const char*)xowner);
                        if(!owner)
                            owner = idrange_owner_manager.newOwner((const char*)xowner);
                        if(owner)
                            idrm->addIDRange(min, max, owner);
                    }
                    if(xowner)
                        xmlFree(xowner);
                }
            }
            xmlFree(name);
        }
    }

    for(vector<IDRangeManager*>::iterator it = list.begin();
            it != list.end(); ++it) {
        (*it)->updateNewIDRanges();
    }

    return TRUE;
}

BOOL MainFrame::saveIDRangeInfo(TextStream &txtStream)
{
    vector<IDRangeManager*> list;
    GetAllIDRangeManagers(list);

    txtStream.println("<id-range-manager>");
    txtStream.indent();

    for(vector<IDRangeManager*>::iterator it = list.begin();
            it != list.end(); ++it) {
        IDRangeManager *idrm = *it;
        IDRange* idrange = idrm->getHeader();
        txtStream.printf("<resource type=\"%d\"", idrm->getType());
        const char* name = idrm->getName();
        if(name && name[0] != 0) {
            txtStream.printf(" name=\"%s\"", name);
        }
        txtStream.println(">");
        txtStream.indent();
        while(idrange) {
            txtStream.println("<id-range min=\"%d\" max=\"%d\" owner=\"%s\"/>", idrange->min, idrange->max, EntityReferenceTranslate(idrange->owner?idrange->owner->name.c_str():"").c_str());
            idrange = idrange->next;
        }
        txtStream.unindent();
        txtStream.println("</resource>");
    }
    txtStream.unindent();
    txtStream.println("</id-range-manager>");
    return TRUE;
}

///////////////////////////////////////////////////////////
//ResEditorInfo
MainFrame::ResEditorInfo::ResEditorInfo()
{
    hMenu = (HMENU) 0;
    hToolbar = HWND_INVALID;
    hIcon = (HICON) 0;
    editor = NULL;
    type = -1;
}
/*
static bool load_menu_item(xmlNodePtr node,PMENUITEMINFO pmii)
{
	//get caption of Menu
	xmlChar* xstr;
	char *menuString = (char*)pmii->typedata;
	xstr = xmlGetProp(node, (const xmlChar*)"caption");

	pmii->mask = MIIM_STATE|MIIM_TYPE;

	menuString[0] = '\0';
	if(xstr){
		strcpy(menuString,(const char*)xstr);
		xmlFree(xstr);
	}

	pmii->state = MFS_ENABLED;
	pmii->type = MFT_STRING;

	//get type
	xstr = xmlGetProp(node, (const xmlChar*)"type");

	//is prop menu
	if(xstr && xmlStrcmp(xstr, (const xmlChar*)"pop") == 0){
		MENUITEMINFO submii;
		int item = 0;
		char szSubText[100];
		HMENU hSub = CreatePopupMenu(pmii);
		for(xmlNodePtr subnode=node->xmlChildrenNode; subnode; subnode = subnode->next)
		{
			if(subnode->type != XML_ELEMENT_NODE)
				continue;

			memset(&submii, 0, sizeof(submii));
			submii.typedata = (DWORD) szSubText;
			if(load_menu_item(subnode, &submii))
			{
				InsertMenuItem(hSub, item, TRUE, &submii);
				item ++;
			}
		}
		//strip the head
		pmii->hsubmenu = StripPopupHead(hSub);
		pmii->mask |= MIIM_SUBMENU;
		xmlFree(xstr);
		return true;
	}
	else if(xstr)
	{
		if(xmlStrcmp(xstr, (const xmlChar*)"radio") == 0){
			pmii->type = MFT_RADIOCHECK;
		}
		else if(xmlStrcmp(xstr, (const xmlChar*)"check") == 0){
			pmii->type = MFT_MARKCHECK;
		}
		else if(xmlStrcmp(xstr, (const xmlChar*)"separation") == 0){
			pmii->type = MFT_SEPARATOR;
		}

		xmlFree(xstr);
	}

	if(pmii->type == MFT_SEPARATOR)
		return true;

	//get id
	xstr = xmlGetProp(node, (const xmlChar*)"id");
	if(xstr){
		pmii->id = atoi((const char*)xstr);
		pmii->mask |= MIIM_ID;
		xmlFree(xstr);
	}

	//get default
	if((pmii->type & (MFT_MARKCHECK|MFT_RADIOCHECK)))
	{
		pmii->state = MFS_UNCHECKED;
//		pmii->typedata = (DWORD)strdup(menuString); //bug: MiniGUI not copy the text when type is MARKCHECK or RADIOCHECK
		pmii->typedata = (DWORD)_(menuString);
		if((xstr = xmlGetProp(node, (const xmlChar*)"default")))
		{
			if(xmlStrcmp(xstr,(const xmlChar*)"checked") == 0)
				pmii->state |= (MFS_CHECKED&(~MFS_UNCHECKED));
			xmlFree(xstr);
		}
	}

	return true;
}
*/
////////////////
//Accelerator
static DWORD get_key_mask(const char* strkey)
{
    if(strkey == NULL)
        return 0;

    if(strcasecmp(strkey, "Ctrl") == 0)
        return ACCEL_CTRL;
    else if(strcasecmp(strkey, "Alt") == 0)
        return ACCEL_ALT;
    else if(strcasecmp(strkey, "Shift") == 0)
        return ACCEL_SHIFT;
    return 0;
}

static int scancode_name_to_code(const char* codename)
{
    if(codename == NULL)
        return 0;

#include "scancode-table"
    for(int i=0; i<(int)(sizeof(scancode_names)/sizeof(const char*)); i++) {
        if(strcasecmp(scancode_names[i], codename) == 0)
            return i;
    }
    return 0;
}

static int get_key(const char* strkey)
{
    if(strkey == NULL)
        return 0;

    if(strkey[0] == '$') { //acci code
        return strkey[1]&255;
    } else if(strkey[0] == '#') { //number, scancode
        int scancode = atoi(strkey+1) & 255;
        return scancode | 256;
    } else {
        return scancode_name_to_code(strkey)|256;
    }

}

static BOOL get_accel_key_info(const char* hotkey, int &key, DWORD& key_mask)
{
    if(hotkey == NULL)
        return FALSE;

    const char* strhotkey = hotkey;

    key = 0;
    key_mask = 0;
    char szKey[100];

    while(hotkey) {
        const char* strkey = strchr(hotkey, '+');
        if(strkey) {
            strncpy(szKey,hotkey, strkey-hotkey);
            szKey[strkey-hotkey] = 0;
            hotkey = strkey + 1;
        } else {
            strcpy(szKey,hotkey);
            hotkey = NULL;
        }

        DWORD tmp_key_mask = get_key_mask(szKey);
        if(tmp_key_mask != 0)
            key_mask |= tmp_key_mask;
        else {
            key = get_key(szKey);
            if(key == 256) { //load field
                LOG_WARNING("Unknown hotkey:%s",szKey);
            }
        }
    }

    if(key_mask == 0 || key == 0) {
        LOG_WARNING("load Accelerator hotkey field:%s",strhotkey);
        return FALSE;
    }

    return TRUE;
}

static void load_accelerator(xmlNodePtr node, HACCEL hAccel)
{
    if(node == NULL || hAccel == 0)
        return ;

    //get hotkey
    xmlChar* xhotkey = xmlGetProp(node, (const xmlChar*)"hotkey");
    //get code
    int code = xhGetIntProp(node, "id");

    if(xhotkey == NULL || code == 0) {
        LOG_WARNING("cannot read accelerator(hotkey=%s, id=%d",xhotkey?(const char*)xhotkey:"NULL",code);
        if(xhotkey)
            xmlFree(xhotkey);
        return ;
    }

    int key;
    DWORD key_mask;
    if(get_accel_key_info((const char*)xhotkey, key, key_mask)) {
        AddAccelerators(hAccel, key, key_mask, (WPARAM)code, 0);
    }

    xmlFree(xhotkey);

}
/////

static void newtoolbar_notif_proc(HWND htoolbar, LINT id, int nc, DWORD add_data)
{
    HWND hParent = GetParent(htoolbar);
    SendMessage(hParent, MSG_COMMAND, nc, 0);
}

MainFrame::ResEditorInfo::ResEditorInfo(xmlNodePtr node, MainFrame *mainframe)
    :menuManager(mainframe->getCommMenuManager())
{
    xmlChar* xstr;
    xmlChar* xstr2;

    xstr = xmlGetProp(node,(const xmlChar*)"class");

    xstr2 = xhGetChildText(node, (const char*)"panelLayout");

    if(xstr == NULL
            || (editor = ResEditor::createResEditor(
                             mainframe->GetHandle(),
                             (const char*)xstr,
                             g_env->getConfigFile((const char*)xstr2).c_str(),
                             mainframe)) == NULL) {
        throw("Create ResEditor Filed");
    }

    xmlFree(xstr);
    xmlFree(xstr2);

    //load types
    xmlNodePtr support_types = xhGetChild(node,"support-types");
    if(support_types)
        editor->loadTypeInfo(support_types);

    //load caption
    xstr = xmlGetProp(node, (const xmlChar*)"caption");

    if(xstr) {
        strCaption = (const char*)xstr;
        xmlFree(xstr);
    }

    //get ext
    xstr =  xhGetChildText(node, "resfile");
    if(xstr) {
        editorFileExt = (const char*)xstr;
        xmlFree(xstr);
    }

    //load menu-set
    menuManager.loadFromXml(xhGetChild(node,"menu-set"));
    //load menubar
    hMenu = menuManager.loadMenuBar(xhGetChild(node, "menubar"));

    //load toolbar

    NTBINFO ntb_info;
    NTBITEMINFO ntbii;
    gal_pixel pixel;

    xmlNodePtr ntbNode = xhGetChild(node, "toolbar");

    if(ntbNode) {
        xmlChar* pic_file;
        xmlChar* cells_number;

        pic_file = xmlGetProp(ntbNode, (const xmlChar*) "icons");

        if (LoadBitmap(HDC_SCREEN, &ntb_bmp, g_env->getConfigFile((const char*)pic_file).c_str())) {
            xmlFree(pic_file);
            return;
        }

        xmlFree(pic_file);
        cells_number = xmlGetProp(ntbNode, (const xmlChar*) "cells");
        ntb_info.nr_cells = atoi((const char*)cells_number);
        xmlFree(cells_number);

        ntb_info.w_cell = 0;
        ntb_info.h_cell = 0;
        ntb_info.nr_cols = 0;
        ntb_info.image = &ntb_bmp;

        RECT rc;
        ::GetClientRect(mainframe->GetHandle(), &rc);

        hToolbar = CreateWindow (CTRL_NEWTOOLBAR,
                                 "",
                                 WS_CHILD | NTBS_DRAWSEPARATOR,
                                 100,
                                 0, 0,
                                 RECTW(rc), 0,
                                 mainframe->GetHandle(),
                                 (DWORD) &ntb_info);

        pixel = GetPixelInBitmap (&ntb_bmp, 0, 0);
        ::SetWindowBkColor (hToolbar, pixel);

        ::GetClientRect(hToolbar, &rc);
        for (ntbNode = ntbNode->xmlChildrenNode; ntbNode; ntbNode = ntbNode->next) {
            if (ntbNode->type != XML_ELEMENT_NODE)
                continue;

            memset(&ntbii, 0, sizeof(ntbii));
            char szText[100]="\0";
            char szTip[100]="\0";
            ntbii.text = szText;
            ntbii.tip = szTip;
            if (load_ntb_item(ntbNode, &ntbii)) {
                ::SendMessage (hToolbar, NTBM_ADDITEM, 0, (LPARAM)&ntbii);
            }
        }
        ::SetNotificationCallback (hToolbar, newtoolbar_notif_proc);
    }
    //load accelerators
    xmlNodePtr accelerators = xhGetChild(node, "accelerators");
    hAccel = 0;
    if(accelerators) {
        hAccel = CreateAcceleratorTable(mainframe->GetHandle());
        for(xmlNodePtr child = accelerators->xmlChildrenNode; child; child = child->next) {
            if(xhIsNode(child,"accelerator"))
                load_accelerator(child,hAccel);
        }
    }
}

MainFrame::ResEditorInfo::~ResEditorInfo()
{
    UnloadBitmap(&ntb_bmp);
    delete editor;
}

BOOL MainFrame::ResEditorInfo::isSupportFile(const char* ext)
{
    if(ext == NULL || editorFileExt.empty())
        return FALSE;

    size_t pos = editorFileExt.find(ext,0);

    if(pos < 0 || pos > editorFileExt.length())
        return FALSE;

    char ch = editorFileExt.at(pos+strlen(ext));
    if(ch != '\0' && ch != ';')
        return TRUE;

    return FALSE;
}

////////////////////////////

ResEditorEnv * g_env = NULL;

int CharStream::getPath(char* buf, int max)
{
    char ch;
    if(buf == NULL || max <= 0)
        return 0;

    while(isSpace((ch=getc())));

    char chend = '\0';
    buf[0] = ch;

    if(buf[0] == '\"' || buf[0] == '\'') {
        chend = buf[0];
        buf[0] = getc();
    }

    int i;
    for(i=1; i<max-1; i++) {
        ch = getc();
#ifndef WIN32
        if(ch == '\\') { //translate chars
            ch = getc();
        } else
#endif
        {
            if(chend) {
                if(chend == ch)//
                    break;
            } else {
                if(isSpace(ch))
                    break;
            }
        }
        buf[i] = ch;
    }

    buf[i] = 0;
    return i;
}

struct StringStream : public CharStream {
    const char* str;
    int idx;
    int len;

    StringStream(const char* str, int len = -1) {
        this->str = str;
        if(len == -1)
            this->len = strlen(str);
        else
            this->len = len;
        idx = 0;
    }

    int getc() {
        if(idx >= len)
            return 0;

        return str[idx++];
    }

    int getWord(char* buf, int max) {
        if(buf == NULL || max <= 0)
            return 0;

        while(isSpace(str[idx++]));

        if(idx >= len)
            return 0;

        int i ;
        for(i = 0; i < max -1 && !isSpace(str[idx]); i++)
            buf[i] = str[idx++];

        buf[i] = '\0';
        return i;
    }
};

struct ArgsStream : public CharStream {
    int argc;
    int arg_idx;
    const char** argv;
    int argv_idx;

    ArgsStream(int argc, const char** argv) {
        this->argc = argc;
        this->argv = argv;
        arg_idx = argv_idx = 0;
    }

    int getc() {
        if(arg_idx >= argc)
            return 0;

        int ch = argv[arg_idx][argv_idx++];
        if(ch == '\0') {
            arg_idx ++;
            argv_idx = 0;
        }
        return ch;
    }

    int getWord(char* buf, int max) {
        if(buf == NULL || max <= 0)
            return 0;

        if(arg_idx >= argc)
            return 0;

        int i;

        for(i=0; i<max-1 && !isSpace(argv[arg_idx][argv_idx]); i++) {
            //printf("----c=%c(%d)\n", argv[arg_idx][argv_idx],argv[arg_idx][argv_idx]);
            buf[i] = argv[arg_idx][argv_idx++];
        }

        buf[i] = '\0';
        return i;
    }
}; //static MainFrame* mainframe = NULL;

HWND ShowMainWindow(int argc, const char* argv[])
{
    mainframe = new MainFrame();
    g_env = (ResEditorEnv*)mainframe;

    mainframe->processArgs(argc-1, argv+1);

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    int day;
    if (!mainframe->checkSoftDog(&day)) {
        return HWND_INVALID;
    }
    mainframe->checkSNInfo();
#endif

    mainframe->Create();
    return mainframe->GetHandle();
}

void DeleteMainFrame(void)
{
    delete g_env;
}

void SendCmdLine(const char* cmdLine)
{
    if(cmdLine == NULL)
        return;

    if(mainframe == NULL)
        return;

    StringStream strs(cmdLine);
    mainframe->processArgs(&strs);
}

void exitSystem( void )
{
    MainFrame *mainframe = (MainFrame *)g_env;
    mainframe->onExit();
}

///////////////////////////////////////////////////////
//configuration
BOOL MainFrame::loadConfig()
{
    char szPath[1024];
    BOOL bret = FALSE;
    //try load from project, which name "<project-path>/res/res.project"
    sprintf(szPath,"%s/res/res.project", getProjectPath());
    xmlDocPtr doc;
    doc = xmlParseFile(szPath);

    if(doc == NULL) { //not exist or cannot load
        //create a new GUI Builder Config
        if(!newGUIBuilderConfig())
            return FALSE;
        doc = xmlParseFile(szPath);
    }

    if(doc == NULL)
        return FALSE;

    xmlNodePtr node = xmlDocGetRootElement(doc);

    xmlNodePtr cfgNode = xhGetChild(node, "configuration");

    if(!configuration.load(cfgNode))
        goto FAILED;

    //load editors
    loadConfig(getConfigFile(MAINFRAMECFG).c_str());

    //load idrange
    loadIDRanageInfo(xhGetChild(node, "id-range-manager"));

    //load config by every editor
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end(); ++it) {
        ResEditorInfo * rei = *it;
        if(rei && rei->editor)
            rei->editor->loadConfig(node);
    }

    bret = TRUE;

FAILED:
    xmlFreeDoc(doc);
    return bret;
}

static BOOL test_guibuilder_etc(const char* path)
{
    string str_guibuilder_etc = path;
#ifdef WIN32
    str_guibuilder_etc += "\\guibuilder.cfg";
#else
    str_guibuilder_etc += "/guibuilder.cfg";
#endif
    return isFileExist(str_guibuilder_etc.c_str());
}

#ifndef __NOUNIX__
#include <libgen.h>
#endif

void get_exec_path(char *path, int len)
{
    if (!path)
        return;

#ifndef __NOUNIX__
    //get absolute execute path
    char buf[MAX_PATH + 1];
    int  count;

    count = readlink("/proc/self/exe", buf, 1024);
    buf[count] = '\0';
    strcpy(path, dirname(dirname((char*)buf)));
#else
    strcpy(path, "/usr/local/");
#endif
}

static string get_etc_path()
{
#ifndef __NOUNIX__
    //get absolute execute path
    char buf[MAX_PATH + 1];
    int  count;
    char *envPath = NULL;
    string etcdir = "guibuilder.cfg";

    //1. get from current path
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    //2. get from ~/.guibuilder.cfg
    etcdir = getenv("HOME");
    etcdir.append("/.guibuilder.cfg");
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    //3. get from current etc path
    count = readlink("/proc/self/exe", buf, 1024);
    buf[count] = '\0';
    etcdir = dirname(dirname((char*)buf));

    etcdir.append("/etc/");
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    //4. get from current etc/guibuilder path
    etcdir.append("/guibuilder/");
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    etcdir = dirname((char*)buf);

    //5. get from $GUIBUILDER_PATH/etc/guibuilder
    envPath = getenv("GUIBUILDER_PATH");
    if (envPath) {
        etcdir = envPath;
        etcdir.append("/etc/guibuilder/");
        if(test_guibuilder_etc(etcdir.c_str()))
            return etcdir;
    }

    //6. get from /usr/local/etc/guibuilder/
    etcdir = "/usr/local/etc/guibuilder/";
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    //7. /usr/etc/guibuilder/
    etcdir = "/usr/etc/guibuilder/";
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    return string("/usr/etc/");

#else
#ifdef WIN32
    string etcdir = "";
    char gbPath[MAX_PATH + 1];

    //1. get from current config\guibuilder-etc
    win_get_exe_path(gbPath, MAX_PATH);
    etcdir = gbPath;
    etcdir.append("\\config\\guibuilder-etc\\");
    if(test_guibuilder_etc(etcdir.c_str()))
        return etcdir;

    //2. get from $GUIBUILDER_PATH\etc\guibuilder-etc
    etcdir = win_getenv("GUIBUILDER_PATH");
    etcdir.append("\\etc\\guibuilder-etc\\");
    return etcdir;
#else
    return ".";
#endif
#endif
}


BOOL MainFrame::newGUIBuilderConfig()
{
    configFile = get_etc_path();

#ifdef _MSTUDIO_LOCALE
	string str_lang = configFile + "/lang.cfg";
	msd_locale_init(str_lang.c_str());
#endif

#ifdef WIN32
    configFile += "\\guibuilder.cfg";
#else
    configFile += "/guibuilder.cfg";
#endif
    //open dialog to make a new config
    SelectProjectTypeDlg Dlg(m_hWnd,configFile.c_str());
    return Dlg.DoMode();
}

BOOL MainFrame::saveConfig()
{
    char szPath[1024];
    sprintf(szPath,"%s/res/res.project", getProjectPath());

    FileStreamStorage fss(szPath);
    TextStream stream(&fss);
    stream.println("<res-project version=\"%s\">", getVersion());
    stream.indent();
    configuration.save(&stream);

    saveIDRangeInfo(stream);

    //save every config
    for(list<ResEditorInfo*>::iterator it = resEditors.begin();
            it != resEditors.end(); ++it) {
        ResEditorInfo * rei = *it;
        if(rei && rei->editor)
            rei->editor->saveConfig(&stream);
    }

    stream.unindent();
    stream.println("</res-project>");
    return TRUE;
}

MainFrame::ConfigurationInfo::ConfigurationInfo()
{
    name = NULL;
    caption = NULL;
    mgVersion = NULL;
    controlSet = NULL;
    resPackName = NULL;
}

MainFrame::ConfigurationInfo::~ConfigurationInfo()
{
    if(name)
        xmlFree((xmlChar*)name);
    if(caption)
        xmlFree((xmlChar*)caption);
    if(mgVersion)
        xmlFree((xmlChar*)mgVersion);
    if(controlSet)
        xmlFree((xmlChar*)controlSet);
    if (resPackName)
        xmlFree((xmlChar*)resPackName);

    for(list<LibraryInfo*>::iterator it = libraries.begin(); it != libraries.end(); ++it) {
        delete *it;
    }
}


BOOL MainFrame::ConfigurationInfo::load(xmlNodePtr cfgNode)
{
    if(cfgNode == NULL)
        return FALSE;

    if(!xhIsNode(cfgNode, "configuration"))
        return FALSE;

    name = (char*)xmlGetProp(cfgNode, (const xmlChar*)"name");
    caption = (char*)xmlGetProp(cfgNode, (const xmlChar*)"caption");

    cfgPath = get_etc_path();
    //get MiniVersion
    for(xmlNodePtr child = cfgNode->children; child; child = child->next) {
        if(xhIsNode(child,"minigui-version")) {
            mgVersion = (char*)xhGetNodeText(child);
        } else if(xhIsNode(child, "control-set"))
            controlSet = (char*)xhGetNodeText(child);
        else if(xhIsNode(child, "path")) {
            char *node = (char*)xhGetNodeText(child);
            if(node == NULL)
                continue;

            if (*node != '$') {
                if(test_guibuilder_etc(node))
                    cfgPath = node;
            } else {
                char szName[256];
                if(node[1] == '{') {
                    const char* strend = strrchr(node, '}');
                    int len = strend ? strend - node - 2 : strlen(node+2);
                    strncpy(szName, node+2, len);
                    szName[len] = 0;
                } else
                    strcpy(szName, node+1);
                const char* strPath = getenv(szName);
                if(strPath && test_guibuilder_etc(strPath))
                    cfgPath = strPath;
            }
            xmlFree((xmlChar*)node);
        }

        else if(xhIsNode(child, "library")) {
            xmlChar* libpath, *symInit, *symUninit;
            libpath = xhGetChildText(child, "libpath");
            symInit = xhGetChildText(child, "init");
            symUninit = xhGetChildText(child, "uninit");
            LibraryInfo * libi = new LibraryInfo((char*)libpath, (char*)symInit, (char*)symUninit);
            libraries.push_back(libi);
        } else if (xhIsNode(child, "respack-name")) {
            resPackName = (char*) xhGetNodeText(child);
        }

    }

    return TRUE;
}

void MainFrame::ConfigurationInfo::save(TextStream *stream)
{
    if(stream == NULL)
        return;

    stream->printf("<configuration");
    if(name)
        stream->printf(" name=\"%s\"",_ERT(name));
    if(caption)
        stream->printf(" caption=\"%s\"",_ERT(caption));
    stream->println(">");
    stream->indent();
    if(mgVersion)
        stream->println("<minigui-version>%s</minigui-version>",mgVersion);
    if(controlSet)
        stream->println("<control-set>%s</control-set>",_ERT(controlSet));
    if (resPackName)
        stream->println("<respack-name>%s</respack-name>", _ERT(resPackName));

    stream->println("<path>%s</path>", cfgPath.c_str());

    for(list<LibraryInfo*>::iterator it = libraries.begin(); it != libraries.end(); ++it) {
        LibraryInfo* libi = *it;
        if(libi) {
            libi->save(stream);
        }
    }
    stream->unindent();
    stream->println("</configuration>");
}

//////////////////////////////
//LibraryInfo
#ifndef WIN32
#include <dlfcn.h>
#endif
MainFrame::ConfigurationInfo::LibraryInfo::LibraryInfo(char* path, char* symInit, char* symUninit)
{
    libPath = path;
    this->symInit = symInit;
    this->symUninit = symUninit;

    handler = dlopen(path, RTLD_LAZY);
    init = (int(*)(void))dlsym(handler, symInit);
    uninit = (void(*)(void))dlsym(handler, symUninit);

    if(init)
        (*init)();
}

MainFrame::ConfigurationInfo::LibraryInfo::~LibraryInfo()
{
    if(libPath)
        xmlFree((xmlChar*)libPath);
    if(symInit)
        xmlFree((xmlChar*)symInit);
    if(symUninit)
        xmlFree((xmlChar*)symUninit);

    if(uninit)
        (*uninit)();
    dlclose(handler);
}

void MainFrame::ConfigurationInfo::LibraryInfo::save(TextStream *stream)
{
    if(stream)
        return ;
    stream->println("<library>");
    stream->indent();
    if(libPath)
        stream->println("<libpath>%s</libpath>",libPath);
    if(symInit)
        stream->println("<init>%s</init>",symInit);
    if(symUninit)
        stream->println("<uninit>%s</uninit>",symUninit);
    stream->unindent();
    stream->println("</library>");
}

///////////////////////////////
//Incore Resource

//check and create resource directroies
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif

BOOL MainFrame::generateIncoreRes(int type)
{
    //save file int incore res
    string str_incore_res = strSrcPath + "/incore-res";
    mkdir(str_incore_res.c_str(), 0777);
    string strcmd;

    //call the command
    if(type == 0) { //mgncs.cfg
        strcmd = "mgcfg-trans --build-out --export-main ncsGetIncoreEtc --switch-marco _MGNCS_INCORE_RES -o ";
        strcmd += str_incore_res;
        strcmd += "/mgncs_cfg.c ";
        strcmd += strPrjPath;
        strcmd += "/mgncs.cfg";
    } else {
        strcmd = "res2c -p " + strPrjPath;
        //we should append correct resource package name
        {
            string packageName = getResPackageName();
            size_t found = packageName.find_last_of(getPathSeparator());

            if (found != string::npos) {
                packageName = packageName.substr(found + 1);
                if (packageName.compare(getResPackageSuffix()) != 0) {
                    strcmd += " -r ";
                    strcmd += packageName;
                }
            }
        }
        strcmd += " -d src/incore-res";
    }

#ifdef WIN32
	win_system(strcmd.c_str());
#else
    system(strcmd.c_str());
#endif
    return TRUE;
}


///////////////////////////////////////////

#ifndef WIN32
#include <pwd.h>
const char* get_cur_user_name(char* puser_name, int max)
{
    struct passwd *pw = getpwuid(getuid());

    puser_name[0] = 0;

    if(!pw)
        return NULL;

    strncpy(puser_name, pw->pw_name, max-1);
    puser_name[max-1] = 0;
    return puser_name;
}
#endif

MainFrame::IDRangeOwnerManagerImpl::IDRangeOwnerManagerImpl()
{
    //new a current owner
    char szUserName[256];
    get_cur_user_name(szUserName, sizeof(szUserName));

    cur_owner = newOwner(szUserName);

}

MainFrame::IDRangeOwnerManagerImpl::~IDRangeOwnerManagerImpl()
{
}

IDRangeOwner* MainFrame::IDRangeOwnerManagerImpl::getOwner(const char* name)
{
    map<string, IDRangeOwner>::iterator it = idrangOwners.find(name);
    if(it == idrangOwners.end())
        return NULL;
    return &(it->second);
}

IDRangeOwner* MainFrame::IDRangeOwnerManagerImpl::getCurOwner()
{
    return cur_owner;
}

int MainFrame::IDRangeOwnerManagerImpl::getAllOwners(IDRangeOwner*** owners)
{
    int i = 0;
    if(!owners || idrangOwners.size() <= 0)
        return 0;
    *owners = new IDRangeOwner*[idrangOwners.size()];
    for(map<string, IDRangeOwner>::iterator it = idrangOwners.begin();
            it != idrangOwners.end(); ++it) {
        (*owners)[i++] = &(it->second);
    }
    return i;
}

IDRangeOwner * MainFrame::IDRangeOwnerManagerImpl::newOwner(const char* name)
{
    if(name == NULL)
        return NULL;
    IDRangeOwner* idowner = &idrangOwners[name];
    if(idowner)
        idowner->name = name;
    return idowner;
}

