/**
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
#include "msd_intl.h"
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
#include "uieditor/ui-event-id.h"
#include "uieditor/rdrpanel.h"

#include "rdreditor.h"
#include "rdr-event-id.h"
#include "rdr-dialog.h"

void RendererEditor::getName(Class *cls,
        char* rdrName, char* clsName, char* typeName)
{
	if (!cls || !rdrName || !clsName)
		return;

	const char* fullName = cls->getClassName();

    if (!fullName)
        return;

    //renderer::Classic.page-window::PropSheet
	char* classname = (char *)strchr(fullName, '.');

	if(classname){
		strncpy(rdrName, fullName, classname - fullName);
		rdrName[classname - fullName] = '\0';

		while(*classname=='.') classname ++;

        //typename
        char* name = strstr(classname, "::");

        if(name) {
            if (typeName) {
                strncpy(typeName, classname, name - classname);
                typeName[name - classname] = '\0';
            }
            classname = name;
            while(*classname==':') classname ++;
        }
        else {
            if (typeName) {
                strcpy(typeName, "window");
            }
            while(*classname=='.') classname ++;
        }

        strcpy(clsName, classname);
	}
}

void RendererEditor::load_classes(const char* rdrlist)
{
	char szLine[256];
	//int cnt;
	FILE* fp = fopen(rdrlist, "rt");

	if(fp == NULL){
		LOG_DEAD("RdrEditor Load Classes field: cannot open rdrlist file\"%s\"", rdrlist);
		throw("open filed");
	}

	while(fgets(szLine, sizeof(szLine), fp)) {
		if (strlen(szLine) <= 1)
			continue;

		//read line end character
		if (strlen(szLine) != sizeof(szLine)-1)
			szLine[strlen(szLine)-1]='\0';

		string strfile = g_env->getConfigFile(szLine);

		DP("Class File:%s", strfile.c_str());

		xmlDocPtr doc = xmlParseFile(strfile.c_str());
		if(doc == NULL){
			LOG_WARNING("Cannot load Class File \"%s\"", strfile.c_str());
			continue;
		}

		xmlNodePtr node = xmlDocGetRootElement(doc);

		Class * cls = Class::loadFromXML(node);
		if (cls) {
			char rdrname[100], clsname[100], type[100];
			getName(cls, rdrname, clsname, type);

			classTypeList[clsname] = type;
			//control class visible
			if (ctrls_config.find(clsname) != ctrls_config.end())
            {
				renderers[rdrname].insert(clsname);
				classes[clsname].insert(rdrname);
			}
		}

		xmlFreeDoc(doc);
	}

	fclose(fp);
}

RendererEditor::RendererEditor()
    :rdrPreviewPanel(NULL)
    ,rdrTreePanel(NULL)
    ,rdrPanel(NULL)
    ,xmlRdrFile("res/renderer/rdr.xml")
    ,status(0)
    ,idrmRdr(this, NCSRT_RDR)
    ,idrmRdrSet(this, NCSRT_RDRSET)
    ,copyPasteInfo()
{
    mapex<string, Class*> maper = Class::getClassMaper();
    for (map<string, Class*>::iterator it = maper.begin();
            it != maper.end(); ++it)
    {
        Class* cls = Class::getClassByName("window", it->first.c_str());

        if (cls && cls->isControl()) {
            const char* clsName = strrchr(it->first.c_str(), ':') + 1;
            ctrls_config.insert(clsName);
        }

        //insert speical control
        ctrls_config.insert("MainWnd");
        ctrls_config.insert("DialogBox");
        ctrls_config.insert("Page");
    }

    load_classes(g_env->getConfigFile("renderer/rdrlist").c_str());
}


RendererEditor::~RendererEditor()
{
    delete rdrPanel;
    delete rdrPreviewPanel;
    delete rdrTreePanel;
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
BOOL RendererEditor::open(const char* xmlFile)
{
	return FALSE;
}

BOOL RendererEditor::initEditor()
{
	//init RendererSet::instances

	return TRUE;
}

BOOL RendererEditor::loadXmlRdrFile(const char* xmlFile)
{
	if(xmlFile == NULL || !isInvalidXmlFile(xmlFile)){
		return FALSE;
	}
	//load instance
	xmlDocPtr doc;
	xmlNodePtr node;
	RdrResource *resource;

	doc = xmlParseFile(xmlFile);
	if(doc == NULL){
		LOG_WARNING("Cannot open \"%s\"", xmlFile);
		return FALSE;
	}

	node = xmlDocGetRootElement(doc);

	if(!xhIsNode(node, "rdr")) {
		return FALSE;
	}

	for(xmlNodePtr child = node->xmlChildrenNode; child; child = child->next)
	{
		if(child->type != XML_ELEMENT_NODE)
			continue;
		if(xmlStrcmp(child->name,(const xmlChar*)"renderer") == 0) {
			RendererInstance * instance =
				RendererInstance::createFromXmlNode(child);
			if(instance){
				resource = (RdrResource *) reses.at(instance->getID());
				if (resource) {
					resource->setRes((DWORD)instance);
				}
			}
		}
		else if (xmlStrcmp(child->name,(const xmlChar*)"rdrset") == 0 ){
			RendererSet * instance =
				RendererSet::createFromXmlNode(child);
			if (instance) {
				resource = (RdrResource *) reses.at(instance->getID());
				if (resource) {
					resource->setRes((DWORD)instance);
				}
			}
		}
	}

	xmlFreeDoc(doc);

    //init NCSRT_RDR resource reference count
	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		RdrResource *resource = (RdrResource *)(it->second);

        if (ID2TYPE(it->first) == NCSRT_RDRSET) {
        	RendererSet *rdr_inst = (RendererSet *)(resource->getRes());
            if (rdr_inst)
                rdr_inst->initRdrSets();
        }
	}

	// init rdrTreePanel
	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		rdrTreePanel->insertItem(it->first, 0);
	}


	return initEditor();
}

void RendererEditor::saveXML()
{
	char szSavePath[MAX_PATH];

	//save id.xml
	sprintf(szSavePath,"%s/renderer/id.xml", g_env->getResourcePath());
    if (isIdXmlChanged() || !::isFileExist(szSavePath)) {
    	LOG_WARNING( "id.xml path:[%s] \n", szSavePath);
    	saveXMLIDs(szSavePath);
    	clearIdXmlChanged();
    }

    //save rdr.xml
	sprintf(szSavePath,"%s/%s", g_env->getProjectPath(),xmlRdrFile.c_str());
    if (isRdrXmlChanged() || !::isFileExist(szSavePath)) {
    	FileStreamStorage xmlfss(szSavePath);
    	TextStream text(&xmlfss);
    	text.println("<rdr>");
    	text.indent();

    	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
    		it != reses.end(); ++it)
    	{
    		RdrResource *resource = (RdrResource *)(it->second);
    		if(ID2TYPE(it->first) == NCSRT_RDR)
    		{
                RendererInstance *inst = (RendererInstance *)(resource->getRes());
                if(inst)
                	inst->saveXMLToStream(&text);
    		}
    		else if (ID2TYPE(it->first) == NCSRT_RDRSET)
    		{
                RendererSet *inst = (RendererSet *)(resource->getRes());
                if(inst)
                	inst->saveXMLToStream(&text);
    		}
    	}
    	text.unindent();
    	text.println("</rdr>");
    }
}

string RendererEditor::saveBin(BinStream* bin)
{
	string strBinName = "rdr.res";
    unsigned int i = 0;
    int offset, pos, begin;

	//save sect_size
    begin = bin->tell();
	bin->save32(0);
	bin->save32(reses.size());

	NCSRM_IDITEM item;
	memset(&item, 0, sizeof(NCSRM_IDITEM));
    for (i = 0; i < reses.size(); i++) {
		bin->save8Arr((const uint8_t*)&item, sizeof(NCSRM_IDITEM));
    }

	//save rdrinfo
    i = 0;
	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		offset = bin->tell() - begin;
		RdrResource * resource = (RdrResource *)(it->second);

		if(ID2TYPE(it->first) == NCSRT_RDR)
		{
            RendererInstance *inst = (RendererInstance *)(resource->getRes());
            if(inst)
				inst->saveBinToStream(bin);
		}
		else if (ID2TYPE(it->first) == NCSRT_RDRSET)
		{
            RendererSet *inst = (RendererSet *)(resource->getRes());
            if(inst)
            	inst->saveBinToStream(bin);
		}

        pos = begin + sizeof(NCSRM_SECTHEADER) + i*sizeof(NCSRM_IDITEM);
		bin->seek(pos, StreamStorage::seek_begin);
		bin->save32(it->first);
		bin->save32(0);
		bin->save32(offset);
		bin->seek(0, StreamStorage::seek_end);
		i++;
	}

	offset = bin->tell() - begin;
	bin->seek(begin, StreamStorage::seek_begin);
	bin->save32(offset);
    bin->seek(0, StreamStorage::seek_end);

	clearRdrXmlChanged();
	return strBinName;
}

string RendererEditor::save(BinStream* bin)
{
    saveXML();
    return saveBin(bin);
}

void RendererEditor::updateRes()
{
	char szPath[1024];

	//disable menus
	enableMenuItem(GBC_DELETE, FALSE);
	enableMenuItem(RDR_MENUCMD_DELRDR, FALSE);
	enableMenuItem(RDR_MENUCMD_DELRDRSET, FALSE);
	enableMenuItem(GBC_COPY, FALSE);
	enableMenuItem(GBC_PASTE, FALSE);
	enableMenuItem(GBC_CUT, FALSE);
	enableMenuItem(RDR_MENUCMD_ADDRDR, FALSE);

	sprintf(szPath,"%s/renderer/id.xml", g_env->getResourcePath());
	//load res
	if(loadXMLIDs(szPath))
	{
		char szPath[MAX_PATH];

		if (xmlRdrFile.length() > 0) {
			sprintf (szPath, "%s/%s", g_env->getProjectPath(), xmlRdrFile.c_str());
			loadXmlRdrFile (szPath);
		}
	}
}

int RendererEditor::newClsRenderer(HWND hParent, const char* clsName, char* strIdName, BOOL visibleCls, GHANDLE parent)
{
	NewRdrDialog rdrDialog (GetHandle(), NULL, clsName, visibleCls);
	BOOL insert_ok = FALSE;

	if (IDOK == rdrDialog.DoMode()) {
		int id = newRendererInstance(rdrDialog.getRdrName(),
						rdrDialog.getClsName(), rdrDialog.getIdName());
	    if (-1 != id) {
            if(parent) {
                int rdrset_id = rdrTreePanel->getItemAddData(parent);

                if (rdrset_id != -1 && ID2TYPE(rdrset_id) == NCSRT_RDRSET) {
                    RendererSet *rdrset_inst = (RendererSet*)getRes(rdrset_id);

                    if (rdrset_inst &&
                            rdrset_inst->insertInstance((RendererInstance *)getRes(id))) {
                        rdrTreePanel->insertItem(id, parent);
                        insert_ok = TRUE;
                    }
                }
            }

	    	if (!insert_ok)
	    		rdrTreePanel->insertItem(id, 0);

	    	if(strIdName)
				strcpy(strIdName,rdrDialog.getIdName());
	    }
	    return id;
	}
	if(strIdName)
		strIdName[0] = '\0';
	return -1;
}

BOOL RendererEditor::callSpecial(const char* strName, ...)
{
	va_list va;
	if(strName == NULL)
		return FALSE;

	typedef map<int, string>* PRDRINFO;
	va_start(va, strName);
	CALL_FUNCTION(strName, "getRdrByClassName",
			VCALL2(getIDListByCtrlClsName, const char*,PRDRINFO , va))

	CALL_FUNCTION(strName, "newRdr", RCALL5(newClsRenderer, int, HWND, const char*, char*, BOOL, GHANDLE, va))

	typedef set<string>* PRDRNAMESET;
	CALL_FUNCTION(strName, "getRdrList",
			VCALL2(getRendererList, const char*, PRDRNAMESET, va))
	return FALSE;
}

//not support RendererSet copy/paste temporary
void RendererEditor::copy(BOOL bremove)
{
	if (rdrTreePanel) {
		GHANDLE sel = rdrTreePanel->getSelItem();
		if (rdrTreePanel->isValidIdHandle(sel)) {
			int id = rdrTreePanel->GetItemAddData(sel);

			if (ID2TYPE(id) == NCSRT_RDR) {
	            RendererInstance *inst = (RendererInstance*)getRes(id);
	            if (!inst)
	                return;

                if (bremove) {
                    RdrResource *resource = (RdrResource *)reses.at(id);
                    if (!resource || resource->use_ref > 0) {
                        InfoBox(_("Error"),
                            _("The \'%s\' is used by other resource, cann't cut it.\n"), 
                            resource->name.c_str());
                        return;
                    }
                }

	    		Instance** instances = new Instance*[1];
	    		instances[0] = (Instance*)inst;
	    		Instance::copy(instances, 1);

	    		if(bremove) {
                    //delete preview window
                    RendererInstance *preview_inst = rdrPreviewPanel->getPreviewInstance();
                    if (preview_inst && (preview_inst == inst)) {
                        rdrPreviewPanel->destroyPreviewWindow();
                    }

					GHANDLE parent = rdrTreePanel->getParentHandle(sel);
                    if(parent) {
                        int rdrset_id = rdrTreePanel->getItemAddData((GHANDLE)parent);
                        if (rdrset_id != -1) {
                            RendererSet *rdrset_inst = (RendererSet *)getRes(rdrset_id);
                            rdrset_inst->removeInstance(inst);
                        }
                    }
                    copyPasteInfo.isCut = TRUE;
                    copyPasteInfo.name = idToName(id);
                    removeRes(id);
                    rdrTreePanel->removeItem(id, parent);
	    		}
                else {
                    copyPasteInfo.isCut = FALSE;
                    copyPasteInfo.name = idToName(id);
                }

	    		delete[] instances;
			}
		}
	}
}

void RendererEditor::paste()
{
	if (rdrTreePanel) {
		GHANDLE sel = rdrTreePanel->getSelItem();
		InstanceArray instances;

		if (sel && (instances = Instance::paste())) {
			int container_id = rdrTreePanel->getItemAddData(sel);
			if (ID2TYPE(container_id) != NCSRT_RDR) { //root or rdrset
                RendererSet *rdrset_inst = (RendererSet *)getRes(container_id);

				for(int i=0; instances[i]; i++) {
                    RendererInstance* cinst = dynamic_cast<RendererInstance*>(instances[i]);
                    if(cinst == NULL)
                        continue;

                    cinst = (RendererInstance*)cinst->clone();
                    if(!cinst)
                        continue;

                    //whether can be insert.
                    if (!rdrset_inst || rdrset_inst->accept(cinst)) {
						//FIXED ME don't use copyPasteInfo to releate a name
						string name = "";
						if(copyPasteInfo.isCut) {
							name = copyPasteInfo.name;
							int i = 0;
							while(!isValidName(name.c_str())){
								char szName[30];
								sprintf(szName, "%d", i++);
								name = copyPasteInfo.name + szName;
							}
						}

                        if (-1 == addResource<RendererInstance>(cinst, 
                                    copyPasteInfo.isCut ? name.c_str() : NULL)) {
                            continue;
                        }
                    }

                    if (rdrset_inst) {
                        rdrset_inst->insertInstance(cinst);
                    }
                    rdrTreePanel->insertItem(cinst->getID(), sel);
                    enableMenuItem(GBC_SAVE);
				}

				if(Instance::paste())
					enableMenuItem(GBC_PASTE, FALSE);
			}
		}
	}
}

void RendererEditor::executeCommand(int cmd_id, int status, DWORD param)
{
	switch (cmd_id)
	{
	case RDR_MENUCMD_NEWRDRSET:
	{
		//disable class
		NewRdrSetDialog rdrSetDialog (GetHandle());
		if (IDOK == rdrSetDialog.DoMode()) {
			int id = newRendererSet(rdrSetDialog.getRdrName(), rdrSetDialog.getIdName());
		    if (-1 != id) {
		    	if(rdrTreePanel->insertItem(id, 0))
					enableMenuItem(GBC_SAVE);
		    }
		}

		break;
	}
	case RDR_MENUCMD_NEWRDR:// new Renderer
	{
        GHANDLE parent = 0;
        int inst_id;

        if (param == (DWORD)rdrTreePanel->getHandler()) {
            int sel_id = rdrTreePanel->getSelItemAddData();

            if (sel_id != -1 && ID2TYPE(sel_id) == NCSRT_RDRSET) {
                parent = rdrTreePanel->getSelItem();
            }
        }
		inst_id = newClsRenderer(GetHandle(), NULL, NULL, TRUE, parent);
		if(inst_id != -1)
			enableMenuItem(GBC_SAVE);
	    break;
	}
	case GBC_CUT:
		copy(TRUE);
        break;

	case GBC_COPY:
		copy();
        break;

	case GBC_PASTE:
		paste();
        break;

    case RDR_MENUCMD_DELRDRSET:
    case RDR_MENUCMD_DELRDR:
	case GBC_DELETE:
    {
    	int id = -1;
    	//delete selected item
    	GHANDLE sel = rdrTreePanel->getSelItem();
		if (rdrTreePanel->isValidIdHandle(sel))
			id = rdrTreePanel->GetItemAddData(sel);

        if (id == -1)
        	break;

        GHANDLE parent = 0;

        RdrResource *resource = (RdrResource *)reses.at(id);

        if (!resource || resource->use_ref > 0) {
            InfoBox(_("Error"),
                    _("The \'%s\' is used by other resource, cannot delete it."),
                    resource->name.c_str());
            break;
        }
        DWORD instance = resource->getRes();
        if (ID2TYPE(id) == NCSRT_RDR && cmd_id != RDR_MENUCMD_DELRDRSET) {
            //delete preview window
            RendererInstance* inst = (RendererInstance*)instance;//resource->getRes();
            RendererInstance *preview_inst = rdrPreviewPanel->getPreviewInstance();
            if (preview_inst && (preview_inst == inst)) {
                rdrPreviewPanel->destroyPreviewWindow();
            }

            //remove reference in parent node
            parent = rdrTreePanel->getParentHandle(sel);
            if(parent) {
                int rdrset_id = rdrTreePanel->getItemAddData((GHANDLE)parent);
                if (rdrset_id != -1) {
                    RendererSet *rdrset_inst = (RendererSet *)getRes(rdrset_id);
                    rdrset_inst->removeInstance(inst);
                }
            }
            rdrTreePanel->removeItem(id, parent);
            removeRes(id);
            enableMenuItem(GBC_SAVE);
        }
        else if (ID2TYPE(id) == NCSRT_RDRSET && cmd_id != RDR_MENUCMD_DELRDR) {
        	RendererSet *inst = (RendererSet *)instance;
        	if (inst->hasRefInstance()) {
                InfoBox(_("Error"),
                        _("The RendererSet only has the reference resource, cann't delete it."));
				break;
        	}
            removeRes(id);
            rdrTreePanel->removeItem(id, parent);
            enableMenuItem(GBC_SAVE);
        }

        break;
    }

    case RDR_MENUCMD_ADDRDR:
	{
		int sel_id = rdrTreePanel->getSelItemAddData();
		if (sel_id == -1 || ID2TYPE(sel_id) != NCSRT_RDRSET)
			break;

		RendererSet* inst = (RendererSet *)getRes(sel_id);
		if (!inst)
			break;

		AddRdrDialog rdrDialog(GetHandle(), inst->getRdrName(), sel_id);

		if (IDOK == rdrDialog.DoMode()) {
			set <int> idList = rdrDialog.getIDList();
		    set <int>::iterator it;
		    GHANDLE parent = rdrTreePanel->getSelItem();
		    for (it = idList.begin(); it != idList.end(); ++it) {
		    	RendererInstance *rdrInst = (RendererInstance *)getRes(*it);
		    	if (rdrInst && inst->insertInstance(rdrInst)) {
                    //remove single existing item
                    rdrTreePanel->removeItem (*it, 0);
                    rdrTreePanel->insertItem (*it, parent);
                    setRdrXmlChanged();
                }
		    }
			enableMenuItem(GBC_SAVE);
		}

		break;
    }
	default:
		break;
	}
}


DWORD RendererEditor::processEvent(Panel* sender, int event_id, DWORD param1, DWORD param2)
{
	switch(event_id) {
	case RDRTREE_SELCHANGE:
		{
			Instance* inst = NULL;
			if(ID2TYPE(param1) == NCSRT_RDR)
			{
				inst = (Instance*)(RendererInstance*)getRes((int)param1);
				enableMenuItem(GBC_COPY, inst != NULL);
				enableMenuItem(GBC_CUT, inst != NULL);
				enableMenuItem(GBC_DELETE, inst != NULL);
				enableMenuItem(RDR_MENUCMD_NEWRDR, FALSE);
				enableMenuItem(RDR_MENUCMD_DELRDR, inst != NULL);

				enableMenuItem(RDR_MENUCMD_NEWRDRSET, FALSE);
				enableMenuItem(RDR_MENUCMD_DELRDRSET, FALSE);
				enableMenuItem(RDR_MENUCMD_ADDRDR, FALSE);
                enableMenuItem(GBC_PASTE, FALSE);
			}
			else if (ID2TYPE(param1) == NCSRT_RDRSET)
			{
				enableMenuItem(GBC_COPY, FALSE);
				enableMenuItem(GBC_CUT, FALSE);
				enableMenuItem(GBC_DELETE);
				enableMenuItem(RDR_MENUCMD_DELRDR, FALSE);
				enableMenuItem(RDR_MENUCMD_NEWRDR);

				enableMenuItem(RDR_MENUCMD_NEWRDRSET, FALSE);
				enableMenuItem(RDR_MENUCMD_DELRDRSET);
				enableMenuItem(RDR_MENUCMD_ADDRDR, TRUE);

                //update paste menu status
                InstanceArray instances = Instance::paste();
                BOOL accept = FALSE;
                if (instances) {
                    RendererSet *rdrset_inst = (RendererSet *)getRes(param1);
                    if (!rdrset_inst)
                        break;

                    for(int i=0; instances[i]; i++) {
                        RendererInstance* cinst = dynamic_cast<RendererInstance*>(instances[i]);
                        if(cinst == NULL)
                            continue;

                        if (rdrset_inst->accept(cinst)) {
                            accept = TRUE;
                            break;
                        }
                    }
                }
                //switch status
                if (accept)
                    enableMenuItem(GBC_PASTE);
                else
                    enableMenuItem(GBC_PASTE, FALSE);
			}
			else
			{
                //root
				enableMenuItem(GBC_COPY, FALSE);
				enableMenuItem(GBC_CUT, FALSE);
				enableMenuItem(GBC_DELETE, FALSE);
				enableMenuItem(RDR_MENUCMD_NEWRDR);
				enableMenuItem(RDR_MENUCMD_DELRDR, FALSE);

				enableMenuItem(RDR_MENUCMD_NEWRDRSET);
				enableMenuItem(RDR_MENUCMD_DELRDRSET, FALSE);
				enableMenuItem(RDR_MENUCMD_ADDRDR, FALSE);
                if (Instance::paste())
                    enableMenuItem(GBC_PASTE);
			}
			//rdrTreePanel send
			rdrPanel->setInstance(inst);
			//rdrPreviewPanel
			rdrPreviewPanel->createPreviewWindow((RendererInstance*)inst);
			break;
		}

	case FIELDPANEL_INSTANCE_FIELD_RESET:
    {
		//rdrPanel send
		if ((int*)param2 != 0) {
			rdrPreviewPanel->updateInstanceField((RendererInstance*)param1, (int*)param2);

			//call special
			ResManager *resMgr = g_env->getResManager(NCSRT_UI);
            if (resMgr)
                resMgr->callSpecial("updateRdrElements", (Instance*)(RendererInstance*)param1, (int*)param2);

			setRdrXmlChanged();
			//enable save menu
			enableMenuItem(GBC_SAVE);
        }

        break;
    }
	case FIELDPANEL_INSTANCE_FIELD_CHANGED:
	{
		//rdrPanel send
		if ((int)param2 != 0) {
			//update Preview Window
            int ids[2] = {(int)param2, 0};
			rdrPreviewPanel->updateInstanceField((RendererInstance*)param1, ids);
			//call special
			ResManager *resMgr = g_env->getResManager(NCSRT_UI);
			resMgr->callSpecial("updateRdrElements", (Instance*)(RendererInstance*)param1, ids);
			setRdrXmlChanged();
			//enable save menu
			enableMenuItem(GBC_SAVE);
		}
		else {
			//update rdrTreePanel tree item name
			rdrTreePanel->updateIDName(rdrTreePanel->getSelItemAddData(), NULL);
			setIdXmlChanged();
			enableMenuItem(GBC_SAVE);
		}
		break;
	}

	default:
		break;
	}

	return 0;
}

int RendererEditor::getSelectedResID()
{
	return rdrTreePanel->getSelItemAddData();
}

BOOL RendererEditor::setResId(int oldId, int newId, DWORD res/*=0*/)
{
    if (oldId == newId)
        return FALSE;

	if(ID2TYPE(oldId) != NCSRT_RDR && ID2TYPE(oldId) != NCSRT_RDRSET)
        return FALSE;

	if(ID2TYPE(oldId) != ID2TYPE(newId))
        return FALSE;

    if (reses.at(newId) != NULL)
        return FALSE;

    RdrResource* oldRes = (RdrResource *) reses.at(oldId);

    if (oldRes) {
        //change instance id
        DWORD inst = oldRes->getRes();
        if (inst) {
            if (ID2TYPE(oldId) == NCSRT_RDR) {
                ((RendererInstance *)inst)->setID(newId);
            }
            if (ID2TYPE(oldId) == NCSRT_RDRSET)
                ((RendererSet *)inst)->setID(newId);
        }

        //update rdrTreePanel
        rdrTreePanel->updateIDValue(oldId, newId);
    }

    //update resource id
    if (ResEditor::setResId(oldId, newId)) {
    	setAllChanged();
		enableMenuItem(GBC_SAVE);
    	return TRUE;
    }
    return FALSE;
}

int RendererEditor::setResName(int id, const char* name, DWORD res/*=0*/)
{
	if(name == NULL)
		return -1;

	//rdrTreePanel item text
	int newid =ResEditor::setResName(id, name);
	if(newid != -1)
	{
		if(newid != id)
			rdrTreePanel->updateIDValue(id, newid);

   rdrTreePanel->updateIDName(id, NULL);
		rdrPanel->refreshInstanceIdName(id, name);
		setIdXmlChanged();
		enableMenuItem(GBC_SAVE);
		//update resource
		return newid;
	}
	return -1;
}

Panel* RendererEditor::createPanel(const char* name, const char* caption, const mapex<string, string>*params)
{
	if(strcmp(name, "RendererTreePanel") == 0) {
		return (rdrTreePanel = new RendererTreePanel(this));
	}
	else if(strcmp(name, "RendererPreviewPanel") == 0) {
		return (rdrPreviewPanel = new RendererPreviewPanel(this));
	}
	else if(strcmp(name, "RendererPanel") == 0) {
		return (rdrPanel = new RendererPanel(this));
	}

	return NULL;
}

int RendererEditor::createRes(int type,
		const char* name, int id, const char* source, DWORD init_res)
{
	if(name == NULL && id == -1)
		return -1;

	Resource *resource = NULL;
	RdrResource *rdrRes = NULL;
	BOOL isChanged = FALSE;

	if(id != -1)
		resource = reses.at(id);

	if(resource){
		if(name){
			//reset name
			if(strcmp(name, resource->name.c_str()) != 0) //change name
			{
				namedRes.erase(resource->name);
				resource->name = name;
				namedRes[resource->name] = resource;
                setIdXmlChanged();
			}
		}
		return resource->id;
	}

	if(id == -1) {
        if (isValidName(name) == FALSE)
            return -1;

		id = newResId(type, name);
		if(id == -1)
			return -1;
		isChanged = TRUE;
    }

	rdrRes = new RdrResource(name, id, source);

	if(NULL == rdrRes)
		return -1;

	reses[id] = (Resource *)rdrRes;
	if(name)
		namedRes[((Resource *)rdrRes)->name] = (Resource *)rdrRes;
	if (isChanged)
		setAllChanged();

	useId(id, rdrRes);
	return id;
}

template<class TInstance>
int RendererEditor::addResource(TInstance *instance, const char* idName)
{
	if (instance == NULL)
		return -1;

	const char* clsType = instance->getClassType();
	int type;

	if (strcmp(clsType, "renderer") == 0) {
		type = NCSRT_RDR;
	}
	else if (strcmp(clsType, "rdrset") == 0) {
		type = NCSRT_RDRSET;
	}
	else
		return -1;

	int id = instance->getID();
    BOOL bRemove = FALSE;

	if (id == -1) {
        bRemove = TRUE;
		id = createRes(type, idName?idName:instance->newName().c_str(), id, NULL,(DWORD)0);
		if (id != -1)
			instance->setID(id);
		else {
			delete instance;
			return -1;
		}
	}

	bIdChanged = TRUE;
	RdrResource *resource = (RdrResource *) reses.at(id);
	if (resource) {
		if (resource->setRes((DWORD)instance))
			return id;
        else {
            if (bRemove)
                removeRes(id);
        }
	}

	return -1;
}

int RendererEditor::newRendererInstance(const char* rdrName,
		const char* clsName, const char* idName)
{
	if (!rdrName || !clsName || !idName)
		return -1;

	char classname[100];
	const char *type;

	type = getClsType(clsName);

	if (type && strcmp(type, "window") == 0)
		sprintf (classname, "renderer::%s.%s", rdrName, clsName);
	else
		sprintf (classname, "renderer::%s.%s::%s", rdrName, type, clsName);

	RendererInstance * instance = RendererInstance::createFromClassName("renderer", classname);

	return addResource<RendererInstance>(instance, idName);
}

int RendererEditor::newRendererSet(const char* rdrName, const char* idName)
{
	if (!rdrName || !idName)
		return -1;

	RendererSet * instance = RendererSet::createFromClassName("rdrset", rdrName);
	return addResource<RendererSet>(instance, idName);
}

void RendererEditor::getIDListByRdrName (const char* rdrName, set<int> &idList, int idListFlag)
{
	if (!rdrName)
		return;

	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		RdrResource * resource = (RdrResource *)(it->second);

		if(ID2TYPE(it->first) == NCSRT_RDRSET) {
			if (idListFlag == IDLIST_RDRSET || idListFlag == IDLIST_ALL) {
				RendererSet * inst = (RendererSet *)(resource->getRes());
				if (strcmp(rdrName, inst->getRdrName()) == 0) {
					idList.insert(it->first);
				}
			}
		}
		else if(ID2TYPE(it->first) == NCSRT_RDR)
		{
			if (idListFlag == IDLSIT_RDR || idListFlag == IDLIST_ALL) {
				RendererInstance * inst = (RendererInstance *)(resource->getRes());
				if (strcmp(rdrName, inst->getRdrName()) == 0) {
					idList.insert(it->first);
				}
			}
		}
	}
}

const char* RendererEditor::getClsType(const char* clsname)
{
	if (clsname == NULL)
		return NULL;

    mapex <string, string>::iterator iter;
	iter = classTypeList.find(clsname);

	if (iter != classTypeList.end()){
		return iter->second.c_str();
	}

    return NULL;
}

void RendererEditor::getIDListByCtrlClsName (const char* clsName, map<int, string> *pidList)
{
	if (!clsName)
		return;

	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		RdrResource * resource = (RdrResource *)(it->second);

		if(ID2TYPE(it->first) == NCSRT_RDR)
		{
			RendererInstance * inst = (RendererInstance *)(resource->getRes());
			if(inst == NULL)
				continue;
			if (strcmp(clsName, inst->getClsName()) == 0) {
				(*pidList)[it->first] = resource->name;
			}
		}
	}
}

void RendererEditor::getRendererList (const char* clsName, set<string>* pRdrList)
{
    map <string,set <string> >::iterator iter;
    set <string>::iterator it;

    //get all renderer name
    if (!clsName) {
        for (iter = renderers.begin(); iter != renderers.end(); ++iter) {
        	(*pRdrList).insert(iter->first.c_str());
        }
    }
    else {
        for (iter = classes.begin(); iter != classes.end(); ++iter) {
        	if (strcmp(iter->first.c_str(), clsName) == 0) {
            	for (it = iter->second.begin(); it != iter->second.end(); ++it) {
            		(*pRdrList).insert(it->c_str());
            	}
        	}
        }
    }
}

void RendererEditor::getClassList (const char* rdrName, set<string> &clsList)
{
    map <string,set <string> >::iterator iter;
    set <string>::iterator it;

    //get all control class name
    if (!rdrName) {
        for (iter = classes.begin(); iter != classes.end(); ++iter) {
        	clsList.insert(iter->first.c_str());
        }
    }
    else {
        for (iter = renderers.begin(); iter != renderers.end(); ++iter) {
        	if (strcmp(iter->first.c_str(), rdrName) == 0) {
            	for (it = iter->second.begin(); it != iter->second.end(); ++it) {
            		clsList.insert(it->c_str());
            	}
        	}
        }
    }
}

BOOL RendererEditor::removeRes(int id, DWORD res/*=0*/)
{
    RdrResource *resource = (RdrResource *)reses.at(id);

    if (!resource || resource->use_ref > 0) {
        return FALSE;
    }

	DWORD dwres = resource->getRes();
    int ref = 0;

	if (dwres) {
        //delete instance
        if (ID2TYPE(id) == NCSRT_RDR) {
            ref = ((RendererInstance*)dwres)->getRef();
        }
        else if (ID2TYPE(id) == NCSRT_RDRSET) {
            //release renderer instance
            ((RendererSet*)dwres)->release();
            ref = ((RendererSet*)dwres)->getRef();
        }
    }

	//delete res id
    if (ref == 1) {
        if (ResEditor::removeRes(id)) {
        	setAllChanged();
            //rdrTreePanel->removeItem(id, 0, TRUE);
        	return TRUE;
        }
    }

    return FALSE;
}

void RendererEditor::onResValueChanged(int res_id)
{
	Instance * inst;

	inst = rdrPanel->getInstance();
	if(inst)
	{
		int updatedFields[100];
		int count = inst->getReferencedFieldIds(res_id, updatedFields, sizeof(updatedFields)/sizeof(int));
		if (count > 0) {
            updatedFields[count] = 0;
            rdrPreviewPanel->updateInstanceField((RendererInstance*)inst, updatedFields);
			setRdrXmlChanged();
		}
	}
}

void RendererEditor::onResIdChanged(int oldId, int newId)
{
	for(mapex<int, ResEditor::Resource*>::iterator it = reses.begin();
		it != reses.end(); ++it)
	{
		RdrResource* rdrres = (RdrResource*)it->second;
		if(rdrres)
		{
			if(ID2TYPE(rdrres->id) == NCSRT_RDR)
			{
				rdrres->instance.rdr->onResIdChanged(oldId,newId);
				setRdrXmlChanged();
			}
		}
	}

}

void RendererEditor::onResNameChanged(int res_id, const char* newName)
{
	Instance * inst;

	inst = rdrPanel->getInstance();
	if(inst)
	{
		int updatedFields[100];
		int count = inst->getReferencedFieldIds(res_id,updatedFields, sizeof(updatedFields)/sizeof(int));
		for(int i=0; i<count; i++)
		{
			rdrPanel->refreshField(inst, updatedFields[i]);
			setRdrXmlChanged();
		}
	}
}

void RendererEditor::refreshRdrPanel(RendererInstance *inst)
{
    rdrPanel->refreshFields(inst);
}

////////////////////////
DECLARE_RESEDITOR(RendererEditor)
//////////////////////////////////////////////////////
