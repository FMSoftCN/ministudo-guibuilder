/*
 * uieditor.cpp
 *
 *  Created on: 2009-3-22
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <vector>
#include <map>
#include "mapex.h"
#include <set>

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"

#include <mgutils/mgutils.h>
#include <mgncs/mgncs.h>
#include "msd_intl.h"
using namespace std;
#include "log.h"
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"
#include "valuetype.h"
#include "class-instance.h"
#include "component-instance.h"

#include "uiconfig.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "defundo-redo-observer.h"
#include "ui-event-id.h"

#include "editable-listview.h"

#include "structpanel.h"
#include "toolbox-panel.h"
#include "fieldpanel.h"
#include "proppanel.h"
#include "eventpanel.h"
#include "rdrpanel.h"
#include "edituipanel.h"
#include "navigator-panel.h"
#include "font-dialog.h"
#include "uieditor.h"
#include "select-template.h"

#include "dlgtmpls.h"

#include "luahlp.h"
extern "C" void luaL_openlibs (lua_State *L);

UIEditor * UIEditor::_instance = NULL;
#define MAX_SCREEN_WIDTH RECTW(g_rcScr)
#define MAX_SCREEN_HEIGTH RECTH(g_rcScr)

extern void loadCompoundType(const char* xmlFile);

int mygetline(char* line, int max_size, FILE* f)
{
    char ch;
    int idx = 0;
    if(line == NULL || max_size <=0 || f == NULL)
        return -1;

    while(!feof(f) && idx < max_size) {
        ch = fgetc(f);
        if(ch == '\r' || ch == '\n' || ch == -1 || ch == ' ') {
            break;
        } else {
            line[idx++] = ch;
        }
    }

    line[idx] = 0;
    return idx;

}

static BOOL load_class(string clss_file, bool from_config=true)
{
    string strfile;
    if(from_config)
        strfile = g_env->getConfigFile(clss_file.c_str());
    else
        strfile = clss_file;
    //printf("szLine=%s, strfile=%s\n", szLine, strfile.c_str());
    DP("Class File:%s", strfile.c_str());

    xmlDocPtr doc = xmlParseFile(strfile.c_str());
    if(doc == NULL) {
        LOG_WARNING("Cannot load Class File \"%s\"", strfile.c_str());
        return FALSE;
    }

    xmlNodePtr node = xmlDocGetRootElement(doc);

    Class* clss = Class::loadFromXML(node);

    xmlFreeDoc(doc);

    return clss != NULL;

}

static void load_classes(const char* ctrllist)
{
    char szLine[256];
    int cnt;
    FILE* fp = fopen(ctrllist, "rt");

    if(fp == NULL) {
        LOG_DEAD("UIEditor Load Classes field: cannot open ctrlist file\"%s\"", ctrllist);
        throw("open filed");
    }

    while(!feof(fp) && (cnt = mygetline(szLine, sizeof(szLine), fp)) >= 0) {
        if(cnt > 0) {
            load_class(szLine);
        }
    }
    fclose(fp);
}

UIEditor::UIEditor()
    :idrmUI(this, NCSRT_UI),
     idrmControl(this, NCSRT_CONTRL,NULL, 9)
{
    // TODO Auto-generated constructor stub
    _instance = this;
    configFlags = 0;
    curEdit = NULL;
    toolboxPanel = NULL;
    eventPanel = NULL;
    rdrPanel = NULL;
    propPanel = NULL;
    navigatorPanel = NULL;
    structPanel = NULL;
    //startWnd = NULL;
    startWndId = -1;

    //load types
    ValueType::loadCompoundTypes(g_env->getConfigFile("uieditor/compund-type.def").c_str());

    //update classes
    load_classes(g_env->getConfigFile("uieditor/ctrllist").c_str());

    screen_width  = 800;//MAX_SCREEN_WIDTH;
    screen_height = 600; //MAX_SCREEN_HEIGTH;
    screen_depth  = 0; //default is 16bpp.rgb565
}

UIEditor::~UIEditor()
{
    for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        delete it->second;
    }
    delete toolboxPanel;
    delete structPanel;
    delete propPanel;
    delete eventPanel;
    delete rdrPanel;
    delete navigatorPanel;
}

DWORD UIEditor::processEvent(Panel* sender, int event_id, DWORD param1, DWORD param2)
{
    switch(event_id) {
    case TOOLBOX_SELCHANGE: {
        if(curEdit)
            curEdit->setCreateInfo((const char*)param2, (BOOL)param1);
    }
    break;
    case EDITUIPANEL_FINISH_CREATE:
        if(toolboxPanel)
            toolboxPanel->cancelSelect();
        break;
    case EDITUIPANEL_SELCHANGE:
        if(propPanel)
            propPanel->setInstance((Instance*)param1, (UndoRedoObserver*)param2);
        if(eventPanel)
            eventPanel->setInstance((Instance*)param1, (UndoRedoObserver*)param2);
        if(structPanel)
            structPanel->selectInstance((ComponentInstance*)(Instance*)param1);

        if(param1) {
            if (curEdit)
                curEdit->setRendererEditor(rdrPanel);
        } else //clean rdrPanel
            rdrPanel->setInstance((Instance*)param1, (UndoRedoObserver*)param2);

        break;
    case FIELDPANEL_INSTANCE_FIELD_CHANGED:
        if(curEdit) {
            int ret = curEdit->updateInstanceField((Instance*)param1, (int)param2);
            if(ret == 0 && (int)param2 == ComponentInstance::PropRenderer)
                curEdit->setRendererEditor(rdrPanel);
            return ret;
        }
        break;
    case FIELDPANEL_INSTANCE_FIELD_RESET:
        rdrPanel->refreshFields((Instance*)param1);
        if(curEdit) {
            curEdit->recreatePreviewWnd((Instance*)param1);
        }
        break;
    case EDITUIPANEL_FIELD_UPDATE:
        if(propPanel)
            propPanel->refreshField((Instance*)param1, (int)param2);
        break;
    case EDITUIPANEL_BOUND_CHANGE:
        if(propPanel)
            propPanel->changeBounds((Instance*)param1, param2);
        break;
    case STRUCT_SELCHANGE: {
        EditUIPanel* ep = getEditUIByInstance((ComponentInstance*)param1);
        if(ep) {
            if(ep != curEdit)
                switchCurEditor(ep);
            ep->selectInstance((ComponentInstance*)param1);
        }
    }
    break;
    case STRUCT_SWITCHINSTANCE: {
        EditUIPanel* ep1 = getEditUIByInstance((ComponentInstance*)param1);
        EditUIPanel* ep2 = getEditUIByInstance((ComponentInstance*)param2);
        if (ep1 != ep2)
            break;

        if (ep1) {
            ComponentInstance* inst = ((ComponentInstance*)param1)->getParent();
            if(inst && inst == ((ComponentInstance*)param2)->getParent()) {
                inst->switchComponentInstances((ComponentInstance*)param1,
                                               (ComponentInstance*)param2);
                enableMenuItem(GBC_SAVE);
                ep1->update();
            }
        }

    }
    break;
    case STRUCT_INSTANCE_DBLCLKED: {
        EditUIPanel* ep = getEditUIByInstance((ComponentInstance*)param1);
        if(ep == NULL || !ep->isHidden())
            break;
        showPanel(ep, TRUE);
        break;
    }
    case EDITUIPANEL_INSTANCE_ADDED:
        if(structPanel)
            structPanel->insertInstance((ComponentInstance*)param1);
        break;
    case EDITUIPANEL_INSTANCE_DELETED:
        if(structPanel)
            structPanel->removeInstance((ComponentInstance*)param1);
        break;
    case EDITUIPANEL_CHANGE_PARENT:
        if(structPanel)
            structPanel->changeParent((vector<HWND>*)param1, (ComponentInstance*)param2);
        break;
    case EDITUIPANEL_INSTANCE_REFRESH:
        if(structPanel)
            structPanel->refreshInstance((ComponentInstance*)param1, (BOOL)param2);
        break;
    case EDITUIPANEL_UPDATE_SPECIAL_FIELD:
        updateAllSpecialFields((int)param1, param2);
        enableMenuItem(GBC_SAVE, TRUE);
        break;
    case EDITUIPANEL_UPDATE:
        if(navigatorPanel)
            navigatorPanel->updateEditUIPanel((EditUIPanel*)param1);
        break;
    case EDITUIPANEL_MODIFIED:
        onEditUIModified((EditUIPanel*)param1, (BOOL)param2);
        if(param2)
            enableMenuItem(GBC_SAVE,(BOOL)param2);
        break;
    case NAVIGATOR_SELCHANGE: {
        switchCurEditor((EditUIPanel*) param1);
        break;
    }
    case NAVIGATOR_SHOWPANEL: {
        //open((const char*)param1);
        showPanel((EditUIPanel*)param1);
        break;
    }
    case EVENTPANEL_GOTOCODE: {
        if(curEdit)
            curEdit->gotoCode((Instance*)param1, (int)param2);
        break;
    }
    case UIMENUITEM_ENABLE:
        enableMenuItem((int)param1, (BOOL)param2);
        break;
    case UIMENUITEM_CHECK:
        checkMenuItem((int)param1, (BOOL)param2);
        break;
    }

    return 0;
}

void UIEditor::switchCurEditor(EditUIPanel *eui)
{
    if(eui == NULL || curEdit == eui)
        return;
    //for each to find
    for(int i=0; i< (int)::SendMessage(hPropSheet, PSM_GETPAGECOUNT, 0, 0); i++) {
        HWND hPage = (HWND)::SendMessage(hPropSheet, PSM_GETPAGE, i, 0);

        if((EditUIPanel*)::GetWindowAdditionalData(hPage) == eui) {
            //set as active
            curEdit = eui;
            curEdit->active();
            ::SendMessage(hPropSheet, PSM_SETACTIVEINDEX, i, 0);
            break;
        }
    }
}

EditUIPanel * UIEditor::getEditUIByInstance(ComponentInstance * instance)
{
    if(instance == NULL)
        return NULL;
    while(instance->getParent())
        instance = instance->getParent();

    for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        if(it->second->getBaseInstance() == instance)
            return it->second;
    }

    return NULL;
}

Panel* UIEditor::createPanel(const char* name, const char* caption, const mapex<string,string>* params)
{
    if(strcasecmp(name, "NavigatorPanel") == 0)
        return (navigatorPanel = new NavigatorPanel(this));
    else if(strcasecmp(name, "StructPanel") == 0)
        return (structPanel = new StructPanel(this));
    else if(strcasecmp(name, "ToolboxPanel") == 0)
        return (toolboxPanel = new ToolboxPanel(this, params));
    else if(strcasecmp(name, "PropertyPanel") == 0)
        return (propPanel = new PropertyPanel(this));
    else if(strcasecmp(name, "EventPanel") == 0)
        return (eventPanel = new EventPanel(this));
    else if(strcasecmp(name, "RendererPanel") == 0)
        return (rdrPanel = new RendererPanel(this));

    return NULL;
}


BOOL UIEditor::initEditor()
{
    int row, col;
    PanelLayout *layout;
    if(getCellByName("DesignArea", col, row, &layout)) {
        hPropSheet = (HWND)layout->GetCell(row, col);
        if(::IsWindow(hPropSheet)) {
            ::SetNotificationCallback(hPropSheet, _propsheet_notifi);
            ::SetWindowAdditionalData(hPropSheet,(DWORD)this);
        }
    }
    return TRUE;
}

void UIEditor::updateRes()
{
    char szPath[1024];
    sprintf(szPath,"%s/ui/id.xml", g_env->getResourcePath());
    //load res
    if(loadXMLIDs(szPath)) {
        //load
        /*	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
        		it != reses.end(); ++it)
        	{
        		ResEditor::Resource * resource = it->second;
        		if(resource == NULL)
        			continue;
        		if(typeFromId(resource->id) == NCSRT_UI)
        		{
        			sprintf(szPath, "%s/%s",g_env->getProjectPath(), g_env->getString(resource->source_id));
        			open(szPath, TRUE);
        		}
        	}*/
    }
}

void UIEditor::setStartWindow(int newId, BOOL avoid_source_changed /*= FALSE*/)
{
    BOOL bClearSource = FALSE;
    if(newId == -1 || newId == startWndId)
        return;

    EditUIPanel * euiold = getEditUIPanel(startWndId);
    if(euiold) {
        if(avoid_source_changed)
            bClearSource = !euiold->isSourceChanged();
        euiold->setStartWnd(FALSE);
        if(bClearSource)
            euiold->clearSourceChanged();
        setTabPageTitle(euiold);
    }

    EditUIPanel * eui = getEditUIPanel(newId);
    if(eui) {
        if(avoid_source_changed)
            bClearSource = !eui->isSourceChanged();
        eui->setStartWnd(TRUE);
        if(structPanel)
            structPanel->setStartWnd(eui->getBaseInstance());
        startWndId = newId;
        if(bClearSource)
            eui->clearSourceChanged();
        setTabPageTitle(eui);
    }
}

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
static inline char lower(char ch)
{
	return (ch >= 'A' && ch <= 'Z') ? ch - 'A' + 'a' : ch;
}
static BOOL is_same_file(const char* f1, const char* f2)
{
    //is same file
    struct stat stat1;
    struct stat stat2;
    if(!f1 || !f2)
        return FALSE;

	int i1 = 0, i2 = 0;
	while(f1[i1] && f2[i2])
	{
		bool is_sperator_1 = false;
		bool is_sperator_2 = false;
		if(f1[i1] == '/' 
			|| f1[i1] == '\\')
		{
			is_sperator_1 = true;
			i1 ++;
			while(f1[i1] == '/' || f1[i1] == '\\') i1++;
		}
		if(f2[i2] == '/' || f2[i2] == '\\')
		{
			is_sperator_2 = true;
			i2 ++;
			while(f2[i2] == '/' || f2[i2] == '\\') i2 ++;
		}

		if(is_sperator_1 != is_sperator_2)
			break;
		if(!is_sperator_1 && 
#ifdef WIN32
			lower(f1[i1]) != lower(f2[i2])
#else
			f1[i1] != f2[i2]
#endif
		)
			break;
		if(!is_sperator_1)
		{
			i1 ++;
			i2 ++;
		}
	}

	if(f1[i1] == 0 && f2[i2] == 0)
		return TRUE;

    if(stat(f1, &stat1) != 0)
        return FALSE;

    if(stat(f2, &stat2) != 0)
        return FALSE;

    return memcmp(&stat1, &stat2, sizeof(struct stat)) == 0;
}

void UIEditor::executeCommand(int cmd_id, int status, DWORD param)
{
    if(cmd_id == GBC_OPEN) {
#ifdef _MSTUDIO_OFFICIAL_RELEASE
        if (!g_env->checkLimit())
            return;
#endif
        //open the restored files
        FILEDLGDATA pfdd ;
        memset(&pfdd, 0, sizeof(pfdd));
        strcpy(pfdd.filter, "UI Files(*.xml)");
#ifdef WIN32
		sprintf(pfdd.filepath, "%s\\ui",g_env->getResourcePath());
#else
        sprintf(pfdd.filepath,"%s/ui",g_env->getResourcePath());
#endif
        //printf("pfdd.filepath=%s\n",pfdd.filepath);
        if(FileOpenSaveDialog  (GetDlgTemplate(ID_FONTSELECT), m_hWnd, NULL, &pfdd)) {
            if(strcmp(pfdd.filename,"id.xml") == 0) {
                InfoBox("Error","Cannot open id file\n");
                return ;
            }

            EditUIPanel * euip = editors.at(pfdd.filename);
            if(euip) {
                switchCurEditor(euip);
                return ;
            }

            char szDestFile[1024*4];
            sprintf(szDestFile,"%s/ui/%s", g_env->getResourcePath(),pfdd.filename);
			BOOL bCopyFile = FALSE;
            if(!is_same_file(szDestFile,pfdd.filefullname)) {
                //copy files
                copyfile(pfdd.filefullname,szDestFile);
				bCopyFile = TRUE;
            }
            if(!open(szDestFile)) {
				if(bCopyFile) {
					remove(szDestFile);
				}
				InfoBox(_("error"), _("The file \"%s\" is\'nt a valid ui file."), pfdd.filefullname);
				return ;
			}
            euip = editors.at(parseFileName(szDestFile));
            if (euip) {
                euip->setSourceChanged();
            }
        }
    } else if(cmd_id == GBC_NEW) {
        open(NULL);
    } else if(cmd_id == GBC_CLOSE) {
        //close();
        showPanel(curEdit,FALSE);
    } else if(cmd_id == UI_MENUCMD_SETSTARTWIN) { //set statWnd
        //TODO Select a id
        setStartWindow(selectStartWnd());
        enableMenuItem(GBC_SAVE,TRUE);
    } else if(cmd_id == UI_MENUCMD_SETDEFRDR) { //preview
        string strName;
        if (selectDefRdrWnd(strName)) {
            //no change
            if (strcasecmp(g_env->getDefRdrName(), strName.c_str()) == 0)
                return;

            if (!g_env->setDefRdrName(strName.c_str()))
                return;
            for(mapex<string,EditUIPanel*>::iterator it = editors.begin();
                    it != editors.end(); ++it) {
                EditUIPanel *eui = it->second;
                if(eui) {
                    eui->setDefRenderer(g_env->getDefRdrName());
                    if (!def_cap_font.empty()) {
                        //LOGFONT *font = (def_cap_font.c_str());
                        LOGFONT *font = (LOGFONT *)LoadResource(def_cap_font.c_str(), RES_TYPE_FONT, 0L);
#ifdef WIN32
                        eui->editor_win_rdr->we_fonts[WE_CAPTION] = font;
#else
                        eui->editor_win_rdr.we_fonts[WE_CAPTION] = font;
#endif
                    }
                }
            }
            enableMenuItem(GBC_SAVE,TRUE);
        }
    } else if(cmd_id == UI_MENUCMD_PREVIEW) { //preview
        preview();
    } else if(cmd_id == UI_MENUCMD_SAVETMPLSOURCE) { //save dialog template
        if(!curEdit)
            return;
        //open file dialog
        FILEDLGDATA pfdd = {0};
        const char* str_home = getenv("HOME");
        if(str_home == NULL) {
            str_home = g_env->getResourcePath();
            if(str_home == NULL) {
#ifdef WIN32
                static const char _home[] = "c:\\";
                str_home = _home;
#else
                static const char _home[] = "/var/tmp/";
                str_home = _home;
#endif
            }
        }
        strcpy(pfdd.filepath, str_home);
        strcpy(pfdd.filter, "*.*");
        pfdd.is_save = TRUE;
        if(FileOpenSaveDialog(GetDlgTemplate(ID_FONTSELECT), m_hWnd ,NULL, &pfdd)) {
            curEdit->saveTemplates(pfdd.filefullname);
        }
    } else if(cmd_id == UI_MENUCMD_SETSCREENSIZE) {
        showScreenSetting();
        enableMenuItem(GBC_SAVE,TRUE);
    } else if(cmd_id == UI_MENUCMD_CONNECTEVENTS) {
        if(curEdit)
            curEdit->connectEvents();
    } else if (cmd_id == UI_MENUCMD_FONTMANAGE) {
        map<string, EditUIPanel*>::iterator it;
        //unsigned int old_size = exFontList.size();

        MngData data = {&def_cap_font, &def_clt_font, &exFontList};

        FontMngDlg mng(GetHandle(), &data);
        if (IDOK == mng.DoMode()) {
            if (!def_clt_font.empty()) {
                LOGFONT *clt_font = (LOGFONT *)LoadResource(def_clt_font.c_str(), RES_TYPE_FONT, 0L);
                for(it = editors.begin(); it != editors.end(); ++it) {
                    EditUIPanel* eui = it->second;
					eui->setDefClientFont(clt_font);
                }
            }

            if (!def_cap_font.empty()) {
                //LOGFONT *cap_font = (def_cap_font.c_str());
                LOGFONT *cap_font = (LOGFONT *)LoadResource(def_cap_font.c_str(), RES_TYPE_FONT, 0L);
                for(it = editors.begin(); it != editors.end(); ++it) {
                    EditUIPanel* eui = it->second;
#ifdef WIN32
                    eui->editor_win_rdr->we_fonts[WE_CAPTION] = cap_font;
#else
                    eui->editor_win_rdr.we_fonts[WE_CAPTION] = cap_font;
#endif
                    eui->update();
                }
            }

            enableMenuItem(GBC_SAVE,TRUE);
        }
    } else if(cmd_id == UI_MENUCMD_REMOVE_CUR ) {
        if(!curEdit)
            return;
        //close
        int id = YesNoCancelBox(_("Delete"),_("Do you want remove \"%s\" from disk?"), curEdit->getXMLFile());

        if(id == IDCANCEL)
            return;

        close(id == IDYES);
        enableMenuItem(GBC_SAVE,TRUE);
    } else if(cmd_id == GBC_UNDO) {
        if(curEdit)
            curEdit->undo();
    } else if(cmd_id == GBC_REDO) {
        if(curEdit)
            curEdit->redo();
    } else if(cmd_id == UI_MENUCMD_SNAPGRID) {

        for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
                it != editors.end(); ++it) {
            EditUIPanel* e = it->second;
            e->autoSetSnapeGrid();
        }
        if(configFlags & UICF_GRID)
            configFlags &= ~UICF_GRID;
        else
            configFlags |= UICF_GRID;
        checkMenuItem(UI_MENUCMD_SNAPGRID,configFlags & UICF_GRID);
        enableMenuItem(ResEditor::GBC_SAVE, TRUE);
    } else if (cmd_id == UI_MEMUCMD_EXPORT_TEMPLATE) {
        if(curEdit)
            curEdit->exportTemplate();
    } else {
        if(curEdit)
            curEdit->onPopMenuCmd(cmd_id);
    }
}

static BOOL isInvalidXmlFile(const char* xmlFile)
{
    FILE *fp = fopen(xmlFile, "rt");
    if(fp == NULL)
        return FALSE;

    fseek(fp, 0, SEEK_END);

    BOOL bvalid = ftell(fp) > 0;

    fclose(fp);

    return bvalid;
}

BOOL UIEditor::open(const char* xmlFile, BOOL bhide/*=FALSE*/)
{
    char szFile[512];
    BOOL bNewFile = FALSE;
    const char *file = xmlFile;

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    if (!g_env->checkLimit())
        return FALSE;
#endif
    if(xmlFile == NULL || !isInvalidXmlFile(xmlFile)) {
        SelectTemplate selectTempl(m_hWnd, NULL);
        if(!selectTempl.DoMode()) {
            return FALSE;
        }

        strcpy(szFile, selectTempl.getNewFileName());
        file = szFile;
        xmlFile = file;
        bNewFile = TRUE;
    }

    xmlFile = parseFileName(file);

    EditUIPanel * euip = editors.at(xmlFile);
    if(euip) {
        //euip->close();
        InfoBox("Error","The File has opened!");
        return TRUE;
    } else {
        euip = new EditUIPanel(this);
        if(euip == NULL)
			return FALSE;
    }

    //OPEN:

    if(!euip->open(file)) {
        delete euip;
        return FALSE;
    }

	if(!bhide) {
		//create panel, and insert into the tab
		int col, row;
		PanelLayout *layout;
		if(!getCellByName("DesignArea", col, row, &layout)) {
			delete euip;
			return FALSE;
		}
		char szCaption[1024];
		HWND htab = layout->getWindowOwner(col, row, getEditUITitle(euip,szCaption,xmlFile));
		HWND hwnd = euip->createPanel(htab);
		if(!::IsControl(hwnd)) {
			delete euip;
			return FALSE;
		}

		layout->insertWindow(col, row, hwnd);

		::SetWindowAdditionalData(htab,(DWORD)euip);
	} else {
		euip->hide(TRUE);
	}

    //insert into map
    editors[xmlFile] = euip;

    if (!def_clt_font.empty()) {
        //LOGFONT *font = CreateLogFontByName(def_clt_font.c_str());
        LOGFONT *font = (LOGFONT *)LoadResource(def_clt_font.c_str(), RES_TYPE_FONT, 0L);
        map<string, EditUIPanel*>::iterator it;
        for(it = editors.begin(); it != editors.end(); ++it) {
            EditUIPanel* eui = it->second;
            ComponentInstance* cinst = eui->getBaseInstance();
            for(cinst = cinst->getChildren(); cinst; cinst = cinst->getNext()) {
                if(NULL == (const char *)cinst->getField(ComponentInstance::PropFont)) {
                    ::SetWindowFont (cinst->getPreviewHandler(), font);
                }
            }
        }
    }

    if (!def_cap_font.empty()) {
        //LOGFONT *font = CreateLogFontByName(def_cap_font.c_str());
        LOGFONT *font = (LOGFONT *)LoadResource(def_cap_font.c_str(), RES_TYPE_FONT, 0L);
#ifdef WIN32
        euip->editor_win_rdr->we_fonts[WE_CAPTION] = font;
#else
        euip->editor_win_rdr.we_fonts[WE_CAPTION] = font;
#endif
    }

    if(!euip->isHidden())
        curEdit = euip;

    int id = euip->getBaseInstance()->getID();

    //update res
    updateInstanceRes(euip->getBaseInstance());

    if(navigatorPanel) {
        navigatorPanel->insertEditUIPanel(file,euip);
        if(!euip->isHidden())
            navigatorPanel->setCurPanel(euip);
    }

    if(structPanel) {
        structPanel->insertInstance(euip->getBaseInstance(),bhide,euip->isStartWnd());
    }

    if(startWndId == -1 || id == startWndId) {
        setStartWindow(id,TRUE);
    }

    //enable
    /*	if(bNewFile){
    		euip->setPropertyChanged(FALSE);
    		euip->saveXML();
    		euip->setSourceChanged(FALSE);
    		euip->saveSource();
    	}
    */
    if(bNewFile) {
        euip->setPropertyChanged();
        euip->setSourceChanged();
        enableMenuItem(ResEditor::GBC_SAVE, TRUE);
    }

	if(!bhide)
	{
		euip->active();
		enableMenuItem(ResEditor::GBC_CLOSE, TRUE);
		enableMenuItem(UI_MENUCMD_REMOVE_CUR, TRUE);
	}

    return TRUE;
}

void UIEditor::close(BOOL bremove_from_dist /*= FALSE*/)
{
    if(curEdit) {
        //TODO : save first
        //remove from curEdit
        editors.erase(curEdit->getFileName());
        //test the main wnd
        /*	if(curEdit == startWnd)
        	{
        		startWnd = NULL;
        		//select a new startWnd
        		startWnd = selectStartWnd();
        		onStartWndChanged();
        	}*/

        //delete from navigator and struct panel
        if(navigatorPanel)
            navigatorPanel->closePanel(curEdit);
        if(structPanel)
            structPanel->removeInstance(curEdit->getBaseInstance());

        //remove page
        HWND hwnd = curEdit->getHandler();
        hwnd = ::GetParent(hwnd);

        BOOL bNewStartWnd = curEdit->isStartWnd();
        //close
        curEdit->close(!bremove_from_dist);

        if(bremove_from_dist) {
            //delete from disk
            remove(curEdit->getXMLFile());
        }

        delete curEdit;
        curEdit = NULL;

        processEvent(NULL, EDITUIPANEL_SELCHANGE, 0, 0);

        //remove page
        int idx = ::SendMessage(hPropSheet,PSM_GETPAGEINDEX, (WPARAM)hwnd, 0);
        if(idx >= 0)
            ::SendMessage(hPropSheet, PSM_REMOVEPAGE, (WPARAM)idx, 0);

        changeCurEditor(TRUE);
        if(bNewStartWnd && curEdit)
            setStartWindow(curEdit->getBaseInstance()->getID());
        if(curEdit)
            curEdit->update();
        else
            startWndId = -1;

		enableMenuItem(UI_MENUCMD_REMOVE_CUR, curEdit != NULL);
		enableMenuItem(ResEditor::GBC_CLOSE, curEdit != NULL);
    }
}

string UIEditor::save(BinStream* bin)
{
    string strBinName = "ui.res";
    char szSavePath[MAX_PATH];
    int i = 0;
    int pos, begin, offset;
    NCSRM_IDITEM item;
    int count;
    BOOL bForceSave = bIdChanged;

#ifdef _MSTUDIO_OFFICIAL_RELEASE
    //only for authorization mode
    if (g_env->isAuthMode() && !g_env->checkLimit())
        return strBinName;
#endif
    sprintf(szSavePath,"%s/ui/id.xml", g_env->getResourcePath());
    //1. save ids
    saveXMLIDs(szSavePath);

    count = editors.size();

    /*
    if (count == 0)
    	return "";
        */

    begin = bin->tell();

    bin->save32(0);// ------------------------- section size (it will be save later)------------|
    //																		| section header space
    bin->save32(count); //--------------------- section maps (also be the count of windows) ----|
    memset(&item, 0, sizeof(NCSRM_IDITEM));
    for (i = 0; i < count; i ++) {
        bin->save8Arr((const uint8_t*)&item, sizeof(NCSRM_IDITEM));  //-------------------| section ID items table
    }

    if (count) {
        EditUIPanel ** panels = new EditUIPanel*[count];

        //sort by id
        i = 0;
        for(map<string, EditUIPanel*>::iterator it = editors.begin(); it != editors.end(); ++it) {
            EditUIPanel* eui = it->second;
            int id = eui->getBaseInstance()->getID();

            int j;
            //insert sort
            for(j=0; j<i; j++) {
                if(panels[j]->getBaseInstance()->getID() > id)
                    break;
            }
            //insert at j
            if(j < i) {
                for(int k=i; k>j; k--) {
                    panels[k] = panels[k-1];
                }
            }
            panels[j] = eui;
            i++;
        }

        for(i=0; i<count; i++) {
            EditUIPanel* eui = panels[i];
            //1. save every instances
            if(!eui->saveXML(bForceSave))
                continue;

            offset = bin->tell() - begin;
            pos = begin + sizeof(NCSRM_SECTHEADER) + i*sizeof(NCSRM_IDITEM);

            //1.reset MCSRM_SECTHREADER info
            bin->seek(pos, StreamStorage::seek_begin);
            bin->save32(eui->getBaseInstance()->getID());	//--id-----------------------------|
            bin->save32(0);									//--file name - for not incore-----| this is 1 sect id item
            bin->save32(offset);							//--offset ---- for incore---------|
            bin->seek(0, StreamStorage::seek_end);
            //2. save binaries
            eui->saveBin(bin);
            //3. save source
            eui->saveSource();
        }
        delete [] panels;
    }

    //change section size
    offset = bin->tell() - begin;
    bin->seek(begin, StreamStorage::seek_begin);
    bin->save32(offset); // ------------------------- section size ,saved.
    bin->seek(0, StreamStorage::seek_end);
    //save window headers
    saveWindowHeaders();

    //export MiniGUI.cfg
    saveMgConfig();

    bIdChanged = FALSE;

    return strBinName;
}

BOOL UIEditor::transIDs(TextStream* stream)
{
    Resource* resource;
    map<int, Resource*>::iterator it;

    if(stream == NULL || reses.size() <= 0)
        return FALSE;

    for(it=reses.begin(); it != reses.end(); ++it) {
        resource = it->second;
        if(resource && resource->name.length()>0) {
            int id;
            if(ID2TYPE(resource->id) == NCSRT_CONTRL) {
                if(getPredefinedId (resource->name.c_str())!=-1)
                    continue;
                id = resource->id&0xFFFF;
            } else
                id = resource->id;
            stream->println("#define %s\t\t\t0x%0x",resource->name.c_str(), id);
        }
    }

    return TRUE;
}

BOOL UIEditor::isPredefinedID(int id)
{
    id = id&0xFFFF;
    return id>=IDC_STATIC && id<=IDNO;
}

void UIEditor::closeAll()
{

}

void UIEditor::_propsheet_notifi(HWND hwnd, int id, int nc, DWORD add_data)
{
    if(nc == PSN_ACTIVE_CHANGED) {
        UIEditor * uie = (UIEditor*)::GetWindowAdditionalData(hwnd);
        uie->changeCurEditor(TRUE);
    }
}

void UIEditor::changeCurEditor(BOOL bSendEvent/* = FALSE*/)
{
    HWND hpage = (HWND)::SendMessage(hPropSheet, PSM_GETPAGE,(WPARAM)::SendMessage(hPropSheet, PSM_GETACTIVEINDEX, 0, 0), 0);
    EditUIPanel * newCurPanel = (EditUIPanel*)::GetWindowAdditionalData(hpage);
    if(newCurPanel == NULL) {
        if(!curEdit)
            return ;

        curEdit = NULL;

        processEvent(NULL, EDITUIPANEL_SELCHANGE, 0, 0);
        processEvent(NULL, UIMENUITEM_ENABLE, UI_MENUCMD_REMOVE_CUR, FALSE);
        processEvent(NULL, UIMENUITEM_ENABLE, ResEditor::GBC_CLOSE, FALSE);
        return ;
    }

    if(newCurPanel != curEdit) {
        curEdit = newCurPanel;

        curEdit->active();
        processEvent(NULL, UIMENUITEM_ENABLE, UI_MENUCMD_REMOVE_CUR, TRUE);
        processEvent(NULL, UIMENUITEM_ENABLE, ResEditor::GBC_CLOSE, TRUE);
    }

    if(bSendEvent && navigatorPanel)
        navigatorPanel->setCurPanel(curEdit);

    updateMainWindowCaption();
}

static const char* predefined_names[]= {
    "IDC_STATIC",
    "IDOK",
    "IDCANCEL",
    "IDABORT",
    "IDRETRY",
    "IDIGNORE",
    "IDYES",
    "IDNO"
};
const char* UIEditor::getPredefineName(int id)
{
    if(ID2TYPE(id)!=NCSRT_CONTRL)
        return NULL;
    id = id & 0xFFFF;
    if(id >= IDC_STATIC && id <= IDNO)
        return predefined_names[id];
    return NULL;
}

int UIEditor::getPredefinedId(const char* name)
{
    if(name == NULL)
        return -1 ;
    for(unsigned int i = 0; i < sizeof(predefined_names)/sizeof(char*); i++) {
        if(strcmp(predefined_names[i], name) == 0)
            return i|(NCSRT_CONTRL<<16);
    }
    return -1;
}

void UIEditor::updateMainWindowCaption()
{
    char szCaption[1024*4];
    strcpy(szCaption,"UI Editor - ");
    if(curEdit)
        getEditUITitle(curEdit,szCaption + strlen(szCaption));
    g_env->setMainCaption(szCaption);
}
int UIEditor::createRes(int type, const char* name, int id, const char* source, DWORD init_res)
{
    if(name == NULL && id == -1)
        return -1;

    if(type == NCSRT_UI) {
        UIResource *resource;
        string strName = name?name:"";

        resource = (UIResource*)reses.at(id);
        if(resource) { //id exist
            if(resource->instsCount()<=0
                    || resource->isset((Instance*)init_res)) {
                if(source && resource->source_id <= 0)
                    resource->source_id = g_env->addString(source);
                return id;
            }

            if(name == NULL || name[0] == '\0') {
                strName = resource->name;
                do {
                    strName += "_RP";
                } while(namedRes.at(strName));
            }

            id = newResId(NCSRT_UI, strName.c_str());
        }

        if(id <= 0)
            id = newResId(NCSRT_UI, name);

        if(id <= 0)
            return -1;

        if(name && ( getPredefinedId(name)!=-1 || namedRes.at(strName))) {
            //recreate a name
            do {
                strName +="_RP";
            } while(namedRes.at(strName));
        }
        //add into the resource
        UIResource * uires = new UIResource(strName.c_str(), id, source);
        reses[id] = uires;
        namedRes[uires->name] = uires;
        uires->insert((Instance*)init_res);
        bIdChanged = TRUE;
        useId(id, uires);
        return id;
    } else if(type == NCSRT_CONTRL) {
        UIResource *uires = NULL;
        int tid = getPredefinedId(name);
        if(tid != -1) {
            id = tid;
        }

        if(id != -1)
            uires = (UIResource*)reses.at(id);
        else if(name)
            uires = (UIResource*)namedRes.at(name);

        if(uires) {
            uires->insert((Instance*)init_res);
            return id;
        }

        if(id == -1)
            id = newResId(type, name);
        if(id == -1)
            return -1;
        //new a value and insert
        uires = new UIResource(name, id, source);
        uires->insert((Instance*)init_res);
        reses[id] = uires;
        namedRes[uires->name] = uires;
        useId(id, uires);
        bIdChanged = TRUE;
    }

    return id;
}

int UIEditor::setResName(int id, const char* name, DWORD res/*=0*/)
{
    UIResource *resource = NULL;

    if(ID2TYPE(id) == NCSRT_CONTRL) {
        if(!ValidIDName(name))
            return -1;

        UIResource *oldres = (UIResource*)reses.at(id);
        if(!oldres)
            return -1;

        if(oldres && strcmp(oldres->name.c_str(), name)==0) {
            return -1;
        }

        int tid = getPredefinedId(name);
        if(tid != -1) { //set As Predefined Name
            //unuse the old id
            unuseId(id);
            id = tid;
        }

        //old id is predefined, but name is not, change the id
        if(tid == -1 && isPredefinedID(id)) {
            id = newResId(NCSRT_CONTRL,name);
            if(id == -1)
                return -1;
        }

        //if the name has implement, insert the new name
        UIResource *newres = (UIResource*) namedRes.at(name);
        if(newres) {
            id = newres->id;
            if(res == 0 || oldres->instsCount()) { //changed all
                //remove from oldres, insert int newres
                for(set<Instance*>::iterator it = oldres->insts.begin();
                        it != oldres->insts.end(); ++it) {
                    Instance *inst = *it;
                    if(!inst)
                        continue;
                    inst->setID(id);
                    newres->insert(inst);
                }
                //remove and delete oldres
                reses.erase(oldres->id);
                namedRes.erase(oldres->name);
                delete oldres;
            } else { //changed only one
                oldres->remove((Instance*)res);
                newres->insert((Instance*)res);
                ((Instance*)res)->setID(newres->id);
            }
        } else {
            if(res == 0 || oldres->instsCount() == 1) { //changed all
                namedRes.erase(oldres->name);
                oldres->name = name;
                if(oldres->id != id) {
                    for(set<Instance*>::iterator it = oldres->insts.begin();
                            it != oldres->insts.end(); ++it) {
                        Instance *inst = *it;
                        if(!inst)
                            continue;
                        inst->setID(id);
                    }
                    reses.erase(oldres->id);
                    oldres->id = id;
                    reses[id] = oldres;
                }
                namedRes[oldres->name] = oldres;
                newres = oldres;
            } else {
                //create a new resource for it
                Instance * inst = (Instance*)res;
                id = newResId(NCSRT_CONTRL, name);
                if(id <= 0)
                    return 0;
                newres = new UIResource(name, id, NULL);
                newres->insert(inst);
                inst->setID(id);
                reses[id] = newres;
                namedRes[newres->name] = newres;
            }
        }
        resource = newres;
    } else {
        if(!isValidName(name))
            return -1;
        //main window
        if(getPredefinedId(name)!=-1)
            return -1;


        if(ResEditor::setResName(id, name) == -1)
            return -1;

        resource = (UIResource*)reses.at(id);
    }

    if(!resource)
        return -1;

    if(propPanel)
        propPanel->refreshInstanceIdName(id, name);

    {

        for(set<Instance*>::iterator it = resource->insts.begin();
                it != resource->insts.end(); ++it) {
            ComponentInstance *cinst = (ComponentInstance*)(*it);
            //if(cinst)
            //	cinst->setID(resource->id);
            if(structPanel)
                structPanel->refreshInstance(cinst, FALSE);
        }
    }

    bIdChanged = TRUE;
    return id;
}

BOOL UIEditor::setResId(int oldId, int newId, DWORD res/*=0*/)
{
    UIResource * oldres = NULL;
    UIResource * newres = NULL;
    if(oldId == newId)
        return FALSE;

    if(ID2TYPE(oldId) != ID2TYPE(newId))
        return FALSE;


    oldres = (UIResource*)reses.at(oldId);
    newres = (UIResource*)reses.at(newId);

    if(newres) {
        InfoBox(_("Error"), _("This ID already exists, please select another ID!"));
        return FALSE;
    }

    if(ID2TYPE(oldId) == NCSRT_UI) {
        oldres->id = newId;
        reses.erase(oldId);
        reses[newId] = oldres;
        //reset id
        for(set<Instance*>::iterator it = oldres->insts.begin();
                it != oldres->insts.end(); ++it) {
            (*it)->setID(newId);
        }
        newres = oldres;
    } else { //control
        const char* strNewName = getPredefineName(newId);
        if(strNewName != NULL) {
            InfoBox(_("Error"), _("This ID is reserved ID and cannot be used. Please change the ID name as \"%s\" to keep using it"),strNewName);
            return FALSE;
        }

        if(isPredefinedID(oldId) && !isPredefinedID(newId)) {
            useId(newId, oldres);
        }

        if(res == 0 || oldres->instsCount() == 1) { //changed all
            reses.erase(oldId);
            oldres->id = newId;
            reses[newId] = oldres;
            newres = oldres;
            //reset id
            for(set<Instance*>::iterator it = oldres->insts.begin();
                    it != oldres->insts.end(); ++it) {
                (*it)->setID(newId);
            }
        } else {
            //remove res from oldres, and create a new res
            string strName;
            if(strNewName)
                strName = strNewName;
            else {
                //create a new name
                strName = oldres->name;
                do {
                    strName +="_RP";
                } while(namedRes[strName]);
            }

            //remove inst
            oldres->remove((Instance*)res);
            //create a new res
            newres = new UIResource(strName.c_str(), newId, NULL);
            newres->insert((Instance*)res);
            ((Instance*)res)->setID(newId);
            //insert
            reses[newId] = newres;
            namedRes[newres->name] = newres;

            //id name changed, update panels
            if(propPanel)
                propPanel->refreshInstanceIdName(newId, newres->name.c_str());

            if(structPanel) {
                for(set<Instance*>::iterator it = newres->insts.begin();
                        it != newres->insts.end(); ++it) {
                    ComponentInstance *cinst = (ComponentInstance*)(*it);

                    structPanel->refreshInstance(cinst, FALSE);
                }
            }
        } //end of res = 0
    }//end of control

    bIdChanged = TRUE;
	useId(newId);
	unuseId(oldId);
    return TRUE;
}

BOOL UIEditor::WndProc(int iMsg,WPARAM wParam,LPARAM lParam,int *pret)
{
    if(iMsg == MSG_SLT_ISOPENED) {
        *pret = isFileOpend((const char*)lParam);
        return TRUE;
    } else if(iMsg == MSG_KEYDOWN || iMsg == MSG_KEYUP) {
        if(curEdit && (wParam == SCANCODE_ESCAPE)) {
            ::SendMessage(curEdit->GetHandle(), iMsg, wParam, lParam);
            return 0;
        }
    }

    return ResEditor::WndProc(iMsg, wParam, lParam, pret);
}

BOOL UIEditor::removeRes(int id, DWORD res/*=0*/)
{

    UIResource *uires = (UIResource*)reses.at(id);
    if(!uires)
        return FALSE;

    if(res == 0 || uires->instsCount() == 1) {
        reses.erase(id);
        unuseId(id);
        namedRes.erase(uires->name);
        delete uires;
    } else {
        uires->remove((Instance*)res);
    }

    bIdChanged = TRUE;
    return TRUE;
}

static const char* _predefineIds[] = {
    "IDC_STATIC", //0
    "IDOK",
    "IDCANCEL",
    "IDABORT",
    "IDRETRY",
    "IDIGNORE",
    "IDYES",
    "IDNO"
};

void UIEditor::onResNameChanged(int res_id, const char* newName)
{
    for(map<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel * euip = it->second;
        euip->updateRefResIdName(res_id);
    }
}

void UIEditor::onResIdChanged(int old_res_id, int new_res_id)
{
    for(map<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel * euip = it->second;
        if(euip->updateRefResId(old_res_id, new_res_id) && euip == curEdit) {
            propPanel->updateEditingField();
        }

    }
}
void UIEditor::onResValueChanged(int res_id)
{
    if(ID2TYPE(res_id) != NCSRT_TEXT)
        return;
    for(map<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel * euip = it->second;
        euip->updateRefResValue(res_id);
    }
}

void UIEditor::onAllResUpdated(int type)
{
    if(type != NCSRT_TEXT)
        return ;

    for(map<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel * euip = it->second;
        euip->updateTexts();
    }
}

void UIEditor::onRefResDeleted(int id)
{
    for(map<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel * euip = it->second;
        euip->deleteRefRes(id);
    }
}

const char* UIEditor::idToName(int id)
{
    if(id >= 0 && id <= 7) {
        return _predefineIds[id];
    }

    return ResEditor::idToName(id);
}

int UIEditor::nameToId(const char* name)
{
    if(name == NULL)
        return -1;

    for(unsigned int i=0; i< sizeof(_predefineIds)/sizeof(char*); i++) {
        if(strcmp(_predefineIds[i], name) == 0)
            return (NCSRT_CONTRL<<16)|i;
    }

    return ResEditor::nameToId(name);
}

BOOL UIEditor::loadConfig(xmlNodePtr root_node)
{
    //open UI
    BOOL hasVisable = FALSE;
    char szPath[1024];
    xmlNodePtr node = xhGetChild(root_node, "uieditor");
    if(node) {
        //support extend
        loadExtends(xhGetChild(node,"extends"));
        for(node=node->children; node; node = node->next) {
            if(xhIsNode(node, "def-renderer")) {
                xmlChar * rdrname = xhGetNodeText(node);
                g_env->setDefRdrName((const char*) rdrname);
                xmlFree(rdrname);
            } else if (xhIsNode(node,"def-font")) {
                for(xmlNodePtr child = node->xmlChildrenNode; child; child=child->next) {
                    if(xhIsNode(child,"caption"))
                        def_cap_font = (const char *)xhGetNodeText(child);
                    if(xhIsNode(child,"client"))
                        def_clt_font = (const char *)xhGetNodeText(child);
                }
            } else if (xhIsNode(node,"import-font")) {
                string abs_file_name = g_env->getProjectPath();
                ExFont *font = new ExFont;

                font->font_name = (const char *)xmlGetProp(node, (const xmlChar*)"name");
                font->file_name = (const char *)xmlGetProp(node, (const xmlChar*)"file");
                font->ref = 0;
                exFontList.push_back(font);

                //according to absolute path, loading device font.
                abs_file_name += "/";
                abs_file_name += font->file_name;
                ::LoadDevFontFromFile(font->font_name.c_str(), abs_file_name.c_str());
            }
        }
    }

    //printf("reses.count=%d\n",reses.size());
    for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
            it != reses.end(); ++it) {
        ResEditor::Resource * resource = it->second;
        //printf("res: %d, %p\n", it->first, resource);
        if(resource == NULL)
            continue;
        if(typeFromId(resource->id) == NCSRT_UI) {
            const char* fileName = g_env->getString(resource->source_id);
            if(fileName == NULL)
                continue;
            sprintf(szPath, "%s/%s",g_env->getProjectPath(), fileName);
            open(szPath, TRUE);
        }
    }


    node = xhGetChild(root_node, "uieditor");
    if(node == NULL)
        return FALSE;

    //read start window
    for(node=node->children; node; node = node->next) {
        if(xhIsNode(node, "start-window")) {
            setStartWindow(xhGetNodeInt(node),TRUE);
        } else if(xhIsNode(node,"visible-window")) {
            EditUIPanel *eup = getEditUIPanel(xhGetNodeInt(node));
            if(eup)
                showPanel(eup, TRUE);
            hasVisable = TRUE;
        } else if(xhIsNode(node,"active-window")) {
            EditUIPanel *eup = getEditUIPanel(xhGetNodeInt(node));
            if(eup) {
                switchCurEditor(eup);
            }
            hasVisable = TRUE;
        } else if(xhIsNode(node,"screen")) {
            for(xmlNodePtr child = node->xmlChildrenNode; child; child=child->next) {
                if(xhIsNode(child,"width"))
                    screen_width =	xhGetNodeInt(child);
                else if(xhIsNode(child,"height"))
                    screen_height = xhGetNodeInt(child);
                else if(xhIsNode(child,"depth"))
                    screen_depth = xhGetNodeInt(child);
            }
            if(screen_width > MAX_SCREEN_WIDTH)
                screen_width = MAX_SCREEN_WIDTH;
            if(screen_height > MAX_SCREEN_HEIGTH)
                screen_height = MAX_SCREEN_HEIGTH;
            if(screen_depth < 0 || screen_depth > 3)
                screen_depth = 0;
        } else if(xhIsNode(node, "snape-grid")) {
            xmlChar* xstr = xhGetNodeText(node);
            if(xstr && strcmp((const char*)xstr, "True") == 0) {
                configFlags |= UICF_GRID;
                for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
                        it != editors.end(); ++it) {
                    EditUIPanel* e = it->second;
                    e->setSnapeGrid();
                }
                checkMenuItem(UI_MENUCMD_SNAPGRID, TRUE);
            }
            if(xstr)
                xmlFree(xstr);
        }

    }
    if (!hasVisable) {
        processEvent(NULL, UIMENUITEM_ENABLE, UI_MENUCMD_REMOVE_CUR, FALSE);
        processEvent(NULL, UIMENUITEM_ENABLE, ResEditor::GBC_CLOSE, FALSE);
    }
    return TRUE;
}

BOOL UIEditor::saveConfig(TextStream* stream)
{
    stream->println("<uieditor>");
    stream->indent();

    //save extends
    saveExtends(stream);

    stream->println("<def-renderer>%s</def-renderer>", _ERT(g_env->getDefRdrName()));

    stream->println("<screen>");
    stream->indent();
    stream->println("<width>%d</width>",screen_width);
    stream->println("<height>%d</height>",screen_height);
    stream->println("<depth>%d</depth><!-- 0=16bpp.rgb565,1=16bpp.rgb555,2=24bpp,3=32bpp -->", screen_depth);
    stream->unindent();
    stream->println("</screen>");

    stream->println("<def-font>");
    stream->indent();
    if (!def_cap_font.empty())
        stream->println("<caption>%s</caption>", _ERT(def_cap_font.c_str()));
    if (!def_clt_font.empty())
        stream->println("<client>%s</client>", _ERT(def_clt_font.c_str()));
    stream->unindent();
    stream->println("</def-font>");

    // import fonts ......
    if (!exFontList.empty()) {
        list<ExFont*>::iterator it;
        for (it = exFontList.begin(); it != exFontList.end(); it++) {
            stream->println("<import-font name=\"%s\" file=\"%s\" />",
                            _ERT((*it)->font_name.c_str()), _ERT((*it)->file_name.c_str()));
        }
    }

    //set start window
    if(startWndId!=-1)
        stream->println("<start-window>0x%08X</start-window>",startWndId);

    //set the visible window
    for(mapex<string,EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel *eui = it->second;
        if(eui && eui->getBaseInstance() && !eui->isHidden()) {
            stream->println("<visible-window>0x%08X</visible-window>",eui->getBaseInstance()->getID());
        }
    }

    //set active window
    if(curEdit && curEdit->getBaseInstance()) {
        stream->println("<active-window>0x%08X</active-window>",curEdit->getBaseInstance()->getID());
    }

    if(configFlags&UICF_GRID)
        stream->println("<snape-grid>True</snape-grid>");

    stream->unindent();
    stream->println("</uieditor>");

    return TRUE;
}

void UIEditor::loadExtends(xmlNodePtr node)
{
    xmlChar* xstr;
    if(!node)
        return ;

    //load extend-template dir
    for(node=node->children; node; node = node->next) {
        if(xhIsNode(node,"extend-templates")) {
            xstr = xhGetNodeText(node);
            if(xstr) {
                dir_extend_templates = (const char*)xstr;
                xmlFree(xstr);
            }
        } else if(xhIsNode(node, "extend-mainwnd")) {
            xstr = xhGetNodeText(node);
            if(xstr) {
                extend_mainwnds.push_back((const char*)xstr);
                //load it into class
                string clss_file = g_env->getProjectPath();
                clss_file += "/";
                clss_file += (const char*)xstr;
                load_class(clss_file.c_str(),false);
                xmlFree(xstr);
            }
        }
    }
}

void UIEditor::saveExtends(TextStream *stream)
{
    if(!stream)
        return;

    stream->println("<extends>");
    stream->indent();
    if(dir_extend_templates.size() > 0)
        stream->println("<extend-templates>%s</extend-templates>",_ERT(dir_extend_templates.c_str()));
    for(vector<string>::iterator it = extend_mainwnds.begin();
            it != extend_mainwnds.end(); ++it) {
        string &str = *it;
        if(str.size() > 0)
            stream->println("<extend-mainwnd>%s</extend-mainwnd>", _ERT(str.c_str()));
    }
    stream->unindent();
    stream->println("</extends>");
}

///////////////////////////////////////////////////////

typedef struct SelectStartWndInfo {
    vector<ResEditor::Resource*> mainwindows;
    int startWndId;
} SelectStartWndInfo;

static void init_startwnd_list(HWND hwnd, SelectStartWndInfo* info)
{
    HWND hlist = GetDlgItem(hwnd, 100);
    int selidx = -1;

    for(int i=0; i<(int)info->mainwindows.size(); i++) {
        ResEditor::Resource * res = info->mainwindows[i];
        int idx;
        if(res->name.length()>0) {
            idx = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)res->name.c_str());
        } else {
            char szText[256];
            sprintf(szText,"%d(%s)",res->id,g_env->getString(res->source_id));
            idx = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)szText);
        }

        SendMessage(hlist, LB_SETITEMADDDATA, idx, (LPARAM)res->id);
        if(info->startWndId == res->id)
            selidx = idx;
    }

    SendMessage(hlist, LB_SETCURSEL, selidx,0);
}

static int _startwnd_proc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    SelectStartWndInfo * info = (SelectStartWndInfo*)GetWindowAdditionalData(hwnd);
    switch(message) {
    case MSG_INITDIALOG:
        info = (SelectStartWndInfo*)lParam;
        SetWindowAdditionalData(hwnd, lParam);
        init_startwnd_list(hwnd, info);
        //init start wnd
        return TRUE;
    case MSG_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK: {
            int idx = SendDlgItemMessage(hwnd, 100, LB_GETCURSEL, 0, 0);
            if(idx < 0) {
                MessageBox(hwnd, _("Please select a start window"), _("Error"), MB_OK);
                break;
            }
            info->startWndId = (int)SendDlgItemMessage(hwnd, 100, LB_GETITEMADDDATA, idx, 0);
            EndDialog(hwnd,1);
            break;
        }
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;
        }
        break;
    }

    return AutoCenterDlgProc(hwnd, message, wParam, lParam);
}

int UIEditor::selectStartWnd()
{
    SelectStartWndInfo info;
    info.startWndId = startWndId;
    for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
            it != reses.end(); ++it) {
        ResEditor::Resource * res = it->second;
        if(res && ID2TYPE(res->id) == NCSRT_UI)
            info.mainwindows.push_back(res);
    }
    if(!DialogBoxIndirectParam(GetDlgTemplate(ID_SETSTARTWINDOW), m_hWnd,_startwnd_proc,(DWORD)&info))
        return -1;
    return info.startWndId;
}

///////////////////////////////////////////////////////
#if 0
static CTRLDATA _rdr_templ_ctrls[] = {
    {
        CTRL_STATIC,
        WS_VISIBLE,
        10, 10, 280, 25,
        IDC_STATIC,
        "Select Default Renderer:",
        0
    },
    {
        CTRL_LISTBOX,
        WS_VISIBLE|LBS_NOTIFY|LBS_SORT|WS_BORDER,
        10, 40, 180, 150,
        100,
        "",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE|BS_PUSHBUTTON,
        200, 40, 80,30,
        IDOK,
        "OK",
        0
    },
    {
        CTRL_BUTTON,
        WS_VISIBLE|BS_PUSHBUTTON,
        200, 80, 80,30,
        IDCANCEL,
        "Cancel",
        0
    }
};

static DLGTEMPLATE _rdr_templ_tmpl = {
    WS_BORDER|WS_CAPTION|WS_DLGFRAME,
    WS_EX_NONE,
    300,200,300,250,
    "Set Default Renderer...",
    0,0,
    sizeof(_rdr_templ_ctrls)/sizeof(CTRLDATA),
    _rdr_templ_ctrls,
    0
};
#else
#include "dlgtmpls.h"
#endif

typedef struct SelectDefRdrInfo {
    set<string> rdrNameSet;
    string      rdrName;
} SelectDefRdrInfo;

static void init_rdrwnd_list(HWND hwnd, SelectDefRdrInfo* info)
{
    HWND hlist = GetDlgItem(hwnd, 100);
    int selidx = -1;
    int idx;
    for(set<string>::iterator it = info->rdrNameSet.begin();
            it != info->rdrNameSet.end(); it++) {
        idx = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)it->c_str());
        if(strcasecmp(info->rdrName.c_str(), it->c_str()) == 0)
            selidx = idx;
    }

    SendMessage(hlist, LB_SETCURSEL, selidx, 0);
}

static int _rdrwnd_proc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    SelectDefRdrInfo * info = (SelectDefRdrInfo*)GetWindowAdditionalData(hwnd);
    switch(message) {
    case MSG_INITDIALOG:
        info = (SelectDefRdrInfo*)lParam;
        SetWindowAdditionalData(hwnd, lParam);
        init_rdrwnd_list(hwnd, info);
        return 1;

    case MSG_COMMAND:
        switch(LOWORD(wParam)) {
        case IDOK: {
            int idx = SendDlgItemMessage(hwnd, 100, LB_GETCURSEL, 0, 0);
            char szName[100];
            if(idx < 0) {
                MessageBox(hwnd, _("Please select default look-and-feel renderer"), _("Error"), MB_OK);
                break;
            }
            if (SendDlgItemMessage(hwnd, 100, LB_GETTEXT, idx, (LPARAM)szName) == 0)
                info->rdrName = szName;
            EndDialog(hwnd, TRUE);
            break;
        }

        case IDCANCEL:
            EndDialog(hwnd, FALSE);
            break;

        default:
            break;
        }
        break;
    case MSG_CLOSE:
        EndDialog(hwnd, FALSE);
        break;

    default:
        break;
    }

    //	return DefaultDialogProc(hwnd, message, wParam, lParam);
    return AutoCenterDlgProc(hwnd, message, wParam, lParam);
}

BOOL UIEditor::selectDefRdrWnd(string& strName)
{
    SelectDefRdrInfo info;
    info.rdrName = g_env->getDefRdrName();

    ResManager* resMgr = g_env->getResManager(NCSRT_RDR);
    if(!resMgr)
        return FALSE;

    if (!resMgr->callSpecial("getRdrList", NULL, &(info.rdrNameSet)))
        return FALSE;

#if 0
    if(!DialogBoxIndirectParam(&_rdr_templ_tmpl, m_hWnd,_rdrwnd_proc,(DWORD)&info))
#else
    if(!DialogBoxIndirectParam(GetDlgTemplate(ID_SETDEFAULTRENDERER), m_hWnd,_rdrwnd_proc,(DWORD)&info))
#endif
        return FALSE;

    strName = info.rdrName;
    return TRUE;
}

void UIEditor::updateAllSpecialFields(int field_id, DWORD param)
{
    for(map<string,EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        EditUIPanel* eup = it->second;
        if(eup)
            eup->updateSpecialField(field_id, param);
    }
}

void UIEditor::preview()
{
    if(curEdit) {
        HWND hwnd = curEdit->setPreview(TRUE);
        if(!::IsWindow(hwnd))
            return ;

        RECT rt;
        ::GetWindowRect(hwnd, &rt);

        // get message
        MSG Msg;

        HWND hOwner = ::GetMainWindowHandle(m_hWnd);

        if (hOwner && hOwner != HWND_DESKTOP) {
            if (::IsWindowEnabled (hOwner)) {
                ::EnableWindow (hOwner, FALSE);
                ::IncludeWindowExStyle (hOwner, 0x10000000L); //WS_EX_MODALDISABLED
            }
            while (PeekPostMessage (&Msg, hOwner,
                                    MSG_KEYDOWN, MSG_KEYUP, PM_REMOVE));
        }


        //Move Window
        {
            int x, y;
            x = (g_rcScr.left + g_rcScr.right - RECTW(rt))/2;
            y = (g_rcScr.top + g_rcScr.bottom - RECTH(rt))/2;
            if(x < 0)
                x = 0;
            if(y < 0)
                y = 0;
            ::MoveWindow(hwnd, x, y, RECTW(rt), RECTH(rt), TRUE);
        }

        ::ShowWindow(hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(hwnd,TRUE);
        ::SetActiveWindow(hwnd);

        while(GetMessage(&Msg, hwnd)) {
            TranslateMessage(&Msg);
            //preview escape and close
            if(Msg.hwnd == hwnd && ((Msg.message == MSG_KEYDOWN && Msg.wParam == SCANCODE_ESCAPE)
                                    || Msg.message == MSG_CLOSE )) {
                break;
            }
            DispatchMessage(&Msg);
        }

        if (hOwner != HWND_DESKTOP && hOwner != HWND_NULL) {
            if (::GetWindowExStyle (hOwner) & 0x10000000L) {
                ::EnableWindow (hOwner, TRUE);
                ::ExcludeWindowExStyle (hOwner, 0x10000000L);
                ::SetActiveWindow (hOwner);
            }
        }

        ::MoveWindow(hwnd, -RECTW(rt), -RECTH(rt), RECTW(rt), RECTH(rt), FALSE);

        curEdit->setPreview(FALSE);
    }
}


void UIEditor::updateInstanceRes(ComponentInstance* cinst)
{
    if(!cinst)
        return;

    if(setRes(cinst->getID(),(DWORD)(Instance*)cinst)) {
        for(ComponentInstance* child = cinst->getChildren(); child; child = child->getNext()) {
            updateInstanceRes(child);
        }
    }

}

void UIEditor::getButtonGroupList(map<int, string> *pidList)
{
    ComponentInstance *cinstance =
        curEdit->getCurrentInstance();
    if(!cinstance)
        return ;
    cinstance = cinstance->getParent();
    if(!cinstance)
        return ;

    cinstance = cinstance->getChildren();

    while(cinstance) {
        if (strcasecmp("ButtonGroup", cinstance->getClass()->getClassName()) == 0) {
            int id = cinstance->getID();
            (*pidList)[id] = idToName(id);
        }
        cinstance = cinstance->getNext();
    }
}


BOOL UIEditor::callSpecial(const char* strName, ...)
{
    va_list va;
    if(strName == NULL)
        return FALSE;

    typedef map<int, string>* PRDRINFO;
    va_start(va, strName);
    CALL_FUNCTION(strName, "getWndsCount",
                  RCALL0(editorsCount, int, va))

    CALL_FUNCTION(strName, "getGroupList",
                  VCALL1(getButtonGroupList, PRDRINFO , va))

    CALL_FUNCTION(strName, "updateRdrElements",
                  VCALL2(updateRdrElements, Instance* ,int* , va))

    if(strcmp(strName, "getExtendTemplates") == 0) {
        string *pstr_templs = va_arg(va, string*);
        if(dir_extend_templates.size() <= 0)
            return FALSE;
        *pstr_templs = g_env->getProjectPath();
        *pstr_templs += "/";
        *pstr_templs += dir_extend_templates;
        return TRUE;
    }



    return FALSE;
}

void UIEditor::showPanel(EditUIPanel* panel , BOOL bshow /*= TRUE*/)
{
    if(panel == NULL)
        return ;

    if( (panel->isHidden() && !bshow)
            || (!panel->isHidden() && bshow))
        return ;

    if(bshow) {
        //open it
        int col, row;
        PanelLayout *layout;
        if(!getCellByName("DesignArea", col, row, &layout)) {
            return;
        }

        char szCaption[1024] = "\0";
        HWND htab = layout->getWindowOwner(col, row, getEditUITitle(panel,szCaption));
        HWND hwnd = panel->createPanel(htab);
        if(!::IsControl(hwnd)) {
            return;
        }

        layout->insertWindow(col, row, hwnd);

        ::SetWindowAdditionalData(htab,(DWORD)panel);

        panel->hide(FALSE);
        changeCurEditor(TRUE);
    } else {
        //hide it
        //remove page
        HWND hwnd = curEdit->getHandler();
        hwnd = ::GetParent(hwnd);
        //remove page
        int idx = ::SendMessage(hPropSheet,PSM_GETPAGEINDEX, (WPARAM)hwnd, 0);
        if(idx >= 0)
            ::SendMessage(hPropSheet, PSM_REMOVEPAGE, (WPARAM)idx, 0);
        panel->hide(TRUE);
    }

    if(navigatorPanel)
        navigatorPanel->updateEditUIPanel(panel);

    if(structPanel)
        structPanel->setInstanceHidden(panel->getBaseInstance(),!bshow, panel->isStartWnd());

}

BOOL UIEditor::removeRes(ComponentInstance *cinst)
{
    UIResource* res = (UIResource*)reses.at(cinst->getID());
    set<Instance*>::iterator it;
    if (res) {
        it = res->insts.find((Instance*)cinst);
        if (it != res->insts.end()) {
            res->insts.erase(it);
            if (res->insts.size() == 0) {
                ResEditor::removeRes(cinst->getID());
            }
        }
    }
    return TRUE;
}

void UIEditor::removeInstanceRes(ComponentInstance *cinst)
{
    if(cinst == NULL)
        return ;

    for(ComponentInstance * child = cinst->getChildren(); child; child = child->getNext())
        removeInstanceRes(child);

    removeRes(cinst);

}


/**
 *
 */
#include "screen_setting_templ"
void UIEditor::showScreenSetting()
{
    int width = screen_width;
    int height = screen_height;
    if(getScreenSetting(GetHandle(), &width, &height, &screen_depth)
            && (width != screen_width || height != screen_height)) {
        screen_width = width;
        screen_height = height;
        //update ALL
        for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
                it != editors.end(); ++it) {
            if(curEdit == it->second) {
                curEdit->updateScrollbar(TRUE);
                curEdit->InvalidateRect();
            } else {
                it->second->updateScrollbar(FALSE);
            }
        }
    }
}

void UIEditor::updateRdrElements(Instance *inst, int* ele_ids)
{
    //update ALL
    for(mapex<string, EditUIPanel*>::iterator it = editors.begin();
            it != editors.end(); ++it) {
        it->second->updateRdrElements(inst, ele_ids);
    }

    //update renderer panel and preview window
    rdrPanel->refreshFields(inst, ele_ids);
    if (curEdit)
        curEdit->recreatePreviewWnd(inst);
}

void UIEditor::onEditUIModified(EditUIPanel *euip, BOOL bModified)
{
    setTabPageTitle(euip);
}

void UIEditor::setTabPageTitle(EditUIPanel *euip)
{
    if(!euip)
        return;

    //for each to find
    for(int i=0; i< (int)::SendMessage(hPropSheet, PSM_GETPAGECOUNT, 0, 0); i++) {
        HWND hPage = (HWND)::SendMessage(hPropSheet, PSM_GETPAGE, i, 0);

        if((EditUIPanel*)::GetWindowAdditionalData(hPage) == euip) {
            char szText[1024];
            ::SendMessage(hPropSheet,PSM_SETTITLE, i, (LPARAM)getEditUITitle(euip,szText));
            if(curEdit == euip)
                updateMainWindowCaption();
            break;
        }
    }
}

void UIEditor::setChanged(BOOL bChanged)
{
    for(mapex<string,EditUIPanel*>::iterator it = editors.begin();
        it != editors.end(); ++it)
    {
        EditUIPanel *eui = it->second;
        eui->setSourceChanged(bChanged);
        eui->setPropertyChanged(bChanged);
    }
}


///save window headers
#define SAVE_HEAD_FUNC "ncs_wnd_head"
void UIEditor::saveWindowHeaders()
{

    //get the translate lua script
    const char* strFile = g_env->getSysConfig("uieditor/trans/wndhead","uieditor/trans/ncs-wndhead.lua");
    string strScriptFile = g_env->getConfigFile(strFile);

    //get dest file
    string strdst =	g_env->getProjectPath();
    strdst += "/include/ncs-windows.h";

    //open lua
    lua_State* L = luaL_newstate();

    if(L == NULL) {
        return ;
    }

    //open standare lib
    luaL_openlibs(L);

    //run file
    luaL_dofile(L, strScriptFile.c_str());

    //call function
    //push function args :
    // wnd_list, filename, start_wnd_names

    lua_getglobal(L, SAVE_HEAD_FUNC);

    lua_newtable(L);
    int wnd_list = lua_gettop(L);
    string start_wnd;
    int idx = 0;

    for(map<string, EditUIPanel*>::iterator it = editors.begin(); it != editors.end(); ++it) {
        EditUIPanel* eui = it->second;
        string name = idToName(eui->getBaseInstance()->getID());
        if(name.length() <= 0) {
            char szText[100];
            sprintf(szText,"ID_MAINWND%d", eui->getBaseInstance()->getID());
            name = szText;
        }
        if(eui->isStartWnd())
            start_wnd = name;
        //push string
        lua_pushinteger(L, idx++);
        lua_pushstring(L, name.c_str());
        lua_settable(L, wnd_list);
    }

    //push the file name
    lua_pushstring(L,strdst.c_str());

    //push the start window
    if (!start_wnd.empty())
        lua_pushstring(L, start_wnd.c_str());
    else
        lua_pushstring(L, NULL);

    //call function
    lua_pcall (L, 3, 0, 0);

    //create the ncs-window-types.h

    strdst = g_env->getProjectPath();
    strdst += "/include/ncs-window-types.h";

    if(!::isFileExist(strdst.c_str())) {
        FILE *fp = fopen(strdst.c_str(), "wt");
        const char* strFormat=
            "/*************************************************************\n"
            " *  This File define the structs \n"
            " *  Need by the main window\n"
            " *  Generated by mStudio\n"
            " *\n"
            " *  You can Modify this file\n"
            " *\n"
            " ***********************************************************/\n"
            "\n\n"
            "#ifndef NCS_WINDOW_TYPES\n"
            "#define NCS_WINDOW_TYPES\n\n"
            "#ifdef __cplusplus\n"
            "extern \"C\" { \n"
            "#endif\n\n"
            " //TODO -- define you struct here\n\n"
            "#ifdef __cplusplus\n"
            "}\n"
            "#endif\n\n"
            "#endif /** NCS_WINDOW_TYPES*/\n";
        fprintf(fp,"%s", strFormat);
        fclose(fp);
    }

    lua_close(L);
}

void UIEditor::saveMgConfig(void)
{
    const char* strFile = g_env->getSysConfig("uieditor/trans/MiniGUI.cfg","uieditor/trans/minigui-cfg.lua");
    string strScriptFile = g_env->getConfigFile(strFile);

    lua_State* L = luaL_newstate();

    if(L == NULL) {
        return ;
    }

    luaL_openlibs(L);

    //create a global table : cfginfo
    lua_newtable(L);
    int cfginfo = lua_gettop(L);
    lua_pushvalue(L, cfginfo);
    lua_setglobal(L, "cfginfo"); //cfginfo = {}

    //set the project path
    lua_pushstring(L, g_env->getProjectPath());
    lua_setfield(L, cfginfo, "projectDir");

    //set screensize
    // screen = { width=sz.cx, height=sz.cy}
    SIZE sz = getScreenSize();
    lua_newtable(L); //screen
    lua_pushinteger(L, sz.cx);
    lua_setfield(L, -2,"width");
    lua_pushinteger(L, sz.cy);
    lua_setfield(L, -2,"height");
    lua_pushinteger(L, screen_depth);
    lua_setfield(L, -2, "depth");
    lua_setfield(L, cfginfo, "screen"); //cfginfo.screen={width=sz.cx, height=sz.cy}

    //create fonts table
    //caption font
    if(!def_cap_font.empty()) {
        lua_pushstring(L, def_cap_font.c_str());
        lua_setfield(L, cfginfo, "caption_font");
    }
    //control font
    if(!def_clt_font.empty()) {
        lua_pushstring(L, def_clt_font.c_str());
        lua_setfield(L, cfginfo,"control_font");
    }


    if(exFontList.size()>0) {
        //fonts
        //cfg.fonts={ type={ name1=font_file1, name2=font_file2 }, ... }
        lua_newtable(L);
        int fonts = lua_gettop(L);

        list<ExFont*>::iterator it;
        for (it = exFontList.begin(); it != exFontList.end(); it++) {
            const char* font_type = strrchr((*it)->file_name.c_str(),'.');
            if(!font_type)
                continue;
            font_type ++;
            //get the table of font
            int type_table;
            lua_getfield(L, fonts, font_type);
            if(lua_isnil(L,-1)) {
                lua_newtable(L);
                lua_pushvalue(L,-1);
                lua_setfield(L, fonts, font_type);
            }
            type_table = lua_gettop(L);

            lua_pushstring(L, (*it)->file_name.c_str());
            lua_setfield(L, type_table, (*it)->font_name.c_str());
            lua_pop(L,1); //pop type_table
        }

        lua_pushvalue(L, fonts);
        lua_setfield(L, cfginfo, "fonts");
    }

    luaL_dofile(L, strScriptFile.c_str());

    lua_close(L);

}

///////////////////////////////////////////////////////

////////////////////////
DECLARE_RESEDITOR(UIEditor)

//////////////////////////
DWORD getUIConfig()
{
    UIEditor * uieditor = UIEditor::getInstance();
    if(uieditor)
        return uieditor->getConfig();
    return 0;
}

SIZE getScreenSize()
{
    SIZE sz= {0,0};
    UIEditor * uieditor = UIEditor::getInstance();
    if(uieditor)
        return uieditor->getScreenSize();
    return sz;
}

const char *getDefaultClientFont()
{
    UIEditor * uieditor = UIEditor::getInstance();
    if(uieditor)
        return uieditor->getDefaultClientFont();
    return NULL;
}

