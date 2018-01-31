/*
 * imgeditor.cpp
 *
 *  Created on: 2009-3-21
 *      Author: dongjunjie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <list>
#include <map>
#include "mapex.h"
#include <vector>

#include "log.h"

#include "mgheads.h"
#include "mgfcheads.h"
#include "xmlheads.h"
#include "xmlhelp.h"
#include "msd_intl.h"
using namespace std;

#include "stream.h"
#include "resenv.h"
#include "panel.h"
#include "panellayout.h"
#include "reseditor.h"

#include "dirtreepanel.h"
#include "imgres-listpanel.h"
#include "imageview.h"
#include "dirimgview.h"
#include "resimgview.h"
#include "resdescpanel.h"
#include "imgeditor.h"

#include "img-event-id.h"

ImageEditor::ImageEditor()
    :dir_img_view(NULL)
    ,dir_tree_panel(NULL)
    ,img_res_list(NULL)
    ,res_img_view(NULL)
    ,res_desc_panel(NULL)
    ,idrm(this, NCSRT_IMAGE)
{
	ImageView::InitImageView();
}

ImageEditor::~ImageEditor()
{
    delete dir_tree_panel;
	dir_tree_panel = NULL;
    delete img_res_list;
	img_res_list = NULL;
    delete dir_img_view;
	dir_img_view = NULL;
    delete res_img_view;
	res_img_view = NULL;
    delete res_desc_panel;
	res_desc_panel = NULL;
	ImageView::UnitImageView();
}

Panel* ImageEditor::createPanel(const char* name, const char* caption, const mapex<string, string>* params)
{
	if(strcasecmp(name, "DirTreePanel") == 0){
		return dir_tree_panel = new DirTreePanel(this);
	}
	else if(strcasecmp(name, "ImageResListPanel") == 0){
		return img_res_list = new ImageResListPanel(this);
	}
	else if(strcasecmp(name, "DirImageView") == 0){
		return dir_img_view = new DirImageView(this);
	}
	else if(strcasecmp(name, "ResImageView") == 0){
		return res_img_view = new ResImageView(this);
	}
	else if(strcasecmp(name, "ResDescPanel") == 0){
		return res_desc_panel = new ResDescPanel(this);
	}
	return NULL;
}

DWORD ImageEditor::processEvent(Panel* sender, int event_id, DWORD param1, DWORD param2)
{
	switch(event_id)
	{
	case DIR_TREE_PANEL_DIR_CHANGED:
		if(dir_img_view){
			dir_img_view->SetPath((const char *)param1);
			dir_img_view->Refresh();
			activeImageView(FALSE);
		}
		break;
	case IMGRES_LISTPANEL_ADD_IMAGE:
		if(res_img_view){
			activeImageView(TRUE);
			//insert into
			char szFile[MAX_PATH];
			sprintf(szFile, "%s/%s",g_env->getProjectPath(),getImageResFile((int)param1));
			if(res_img_view->AddFile(szFile,(int)param1))
				enableMenuItem(GBC_SAVE);
		}
		break;
	case IMGRES_LISTPANEL_SEL_CHANGE:
		if(res_img_view){
			activeImageView(TRUE);
			BOOL ret = res_img_view->SelectFile(NULL,param1);
			enableMenuItem(IMG_MENUCMD_REMOVE, ret);
		}
		break;
	case IMAGE_VIEW_SEL_CHANGED:
		if(res_desc_panel)
			res_desc_panel->setText((const char *)param1);
		if(sender == (Panel*)res_img_view && img_res_list){
			img_res_list->SelectRes((int)param2);
			enableMenuItem(IMG_MENUCMD_REMOVE);
		}
		else if(sender == (Panel*)dir_img_view)
		{
			enableMenuItem(IMG_MENUCMD_IMPORT, param1 != 0);	
		}
		break;
	case IMAGE_VIEW_REMOVED:
		if(sender == (Panel*)res_img_view && img_res_list){
			if(ResEditor::removeRes((int)param2))
			{
				img_res_list->RemoveRes((int)param2);
				return TRUE;
			}
			else
			{
				InfoBox(_("Error"),
						_("The \'%s\' cannot be removed. It\'s used by other resource or not exists."),
						idToName((int)param2));
				return FALSE;
			}
		}
		break;
	}
	return 0;
}


#define IS_CURRENT(path)  (path[0] == '.' && (path[1] == '\0' || path[1] == '/' || path[1] == '\\'))
#define IS_PARENT(path)  (path[0] == '.' && path[1] == '.' && (path[2] == '\0' || path[2] == '/' || path[2] == '\\'))
#ifdef WIN32
#include <dirent.h>
#endif

const int path_expend(const char* path, char* exPath)
{
    int index = 0;
    if(!path || !exPath)
        return 0;

    //linux or windows
    //I absloute path?
    if(path[0] == '/' || ( ((path[0]  >= 'A' && path[0] <='Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':'))
    {
        exPath[index++] = '/';
        if(path[0] == '/')
            path ++;
        else
            path += 2;
        if(path[0] == '\0')
            goto END;

        while (path[0] && (path[0] == '/' || path[0] == '\\'))
            path++;
    }
    else // releated path
    {
        char szcwd[1024];
        int len;
        getcwd(szcwd, sizeof(szcwd));
        len = strlen(szcwd) ;
        if(IS_PARENT(path))
        {
            while(len > 0 && szcwd[len] != '/' && szcwd[len] != '\\')
                len --;
        }
        while(len > 0 && (szcwd[len] == '/' || szcwd[len] == '\\')) len --; //skip the unsued '/'
        strncpy(exPath, szcwd, len);
        index = len;
        
        if(IS_CURRENT(path) ||IS_PARENT(path))
        {
            if(path[1] == '.') 
                path += 2;
            else
                path += 1;
            if(!path[0])
                goto END;
            path ++;
            while(path[0] && (path[0] == '/' || path[0] == '\\'))
                path ++;
        }
        exPath[index++] = '/';
    }

    while(path[0])
    {
        if(IS_PARENT(path))
        {
            if(index > 1)
                index -= 2;
            while(index > 1 && exPath[index] != '/' && exPath[index] != '\\')
                index --;
            path += 2;
            if(!path[0])
                break;
        }
        else if(IS_CURRENT(path))
        {
            path += 1;
            if(!path[0])
                break;
        }
        
        if(path[0] == '/' || path[0] == '\\')
        {
            exPath[index++] = '/';
            do {
                path ++;
            }while(path[0] && (path[0] == '/' || path[0] == '\\'));
        }
        else
        {
            exPath[index++] = path[0];
            path ++;
        }
    }

END:
    exPath[index] = 0;
    return index;
}

static BOOL copyfile(const char* filesrc, const char* filedst)
{
	BOOL bret = FALSE;

    if(!filesrc || !filedst)
        return FALSE;
#ifdef WIN32
    if(strcasecmp(filesrc, filedst) == 0)
#else
    if(strcmp(filesrc, filedst) == 0)
#endif
        return TRUE;
    {
        char szSrc[1024*4];
        char szDest[1024*4];
        path_expend(filesrc, szSrc);
        path_expend(filedst, szDest);
#ifdef WIN32
        if(strcasecmp(filesrc, filedst) == 0)
#else
        if(strcmp(filesrc, filedst) == 0)
#endif
            return TRUE;
    }

	FILE* fpsrc = fopen(filesrc,"rb");
	FILE* fpdst = fopen(filedst,"wb");

	if(fpsrc == NULL || fpdst == NULL)
		goto FAILED;

	while(!feof(fpsrc))
	{
		char szText[1024];
		size_t len = fread(szText,1, sizeof(szText), fpsrc);
		fwrite(szText, 1, len, fpdst);
	}
	bret = TRUE;

FAILED:
	if(fpsrc)
		fclose(fpsrc);
	if(fpdst)
		fclose(fpdst);
	return bret;
}

#define IDC_INPUT 		201
#define IDC_IMPORT 		202

#include "dlgtmpls.h"

typedef struct _IMPORT_DLG_ADDDATA {
    ImageEditor *imgEditor;
    char        *idName;
}IMPORT_DLG_ADDDATA;

static LRESULT _importProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit;
	char * str_id_name;

	switch(message)
	{
		case MSG_INITDIALOG:
        {
            IMPORT_DLG_ADDDATA* addData = (IMPORT_DLG_ADDDATA*)lParam;
			str_id_name = addData->idName;
			SetWindowAdditionalData(hDlg, lParam);

			hEdit = GetDlgItem(hDlg,IDC_INPUT);
			SetWindowText(hEdit, str_id_name);
			SetFocus(hEdit);
			SendMessage(hEdit, EM_SELECTALL, 0, 0);
			return 0;
        }

		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_IMPORT:
            {
                IMPORT_DLG_ADDDATA* addData = (IMPORT_DLG_ADDDATA*)GetWindowAdditionalData(hDlg);
				hEdit = GetDlgItem(hDlg,IDC_INPUT);
				str_id_name = addData->idName;

				if(GetWindowText(hEdit, str_id_name, 256) > 0) {
                    if (!ValidIDName(str_id_name)) {
                        MessageBox(hDlg,
                                _("Invalid ID name, ID name must start with a Alpha, and only include Alpha, \'_\', digit, e.g. IDB_IMAGE1"),
                                _("Error") ,MB_OK);
                        break;
                    }

                    if (!addData->imgEditor->isAvailableIDName(str_id_name)) {
                        MessageBox(hDlg,
                                _("ID name has been used. please input again."),
                                _("Error") ,MB_OK);
                        break;
                    }
					EndDialog(hDlg, 1);
					break;
				}

				SendMessage(hEdit, EM_SELECTALL, 0, 0);
				SetFocus(hEdit);
				break;
            }
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
	}

	return AutoCenterDlgProc(hDlg, message, wParam, lParam);
}

void ImageEditor::executeCommand(int cmd_id, int status, DWORD param)
{
	if(!dir_tree_panel) //deleting self
		return ;

	switch(cmd_id)
	{
		case IMG_MENUCMD_SORT:
			//FIXME
			res_img_view->Sort(FALSE, ImageView::sort_by_name);
			break;

		case IMG_MENUCMD_CLEAN:
			//TODO
			break;

		case IMG_MENUCMD_REMOVE:    //used in res-image-view
		{
		/*	char buff[MAX_NAME];
			if (0 > res_img_view->GetSelectImage(buff, MAX_NAME-1)){
				::MessageBox(res_img_view->getHandler(), "no selected image", "You should select one remove.", MB_OK);
				return;
			}*/
			if(res_img_view->removeSelectedImage() != -1)
				enableMenuItem(GBC_SAVE, TRUE);
			if(reses.size() < 0) //no resource
				enableMenuItem(IMG_MENUCMD_REMOVE, FALSE);
			break;
		}
		case IMG_MENUCMD_IMPORT:    //used in dir-image-view
		{
			import();
			break;
		}
		case IMG_MENUCMD_IMPORTALL:     //used in dir-image-view
		{
			importAll();
			break;
		}
		default:
			break;
	}
}

void ImageEditor::updateRes()
{
	char xmlName[1024];

	enableMenuItem(IMG_MENUCMD_REMOVE, FALSE);
	sprintf(xmlName, "%s/image/id.xml", g_env->getResourcePath());
	open(xmlName);
}

BOOL ImageEditor::open(const char* xmlFile)
{
	Resource* resource;
	map<int, Resource*>::iterator it;
	int count = 0;

	char szFile[MAX_PATH];
	char * img_file;
	loadXMLIDs(xmlFile);

	sprintf(szFile,"%s/", g_env->getProjectPath());
	img_file = szFile + strlen(szFile);


	for(it=reses.begin(); it != reses.end(); ++it)
	{
		if((resource = it->second)){
			img_res_list->AddRes(resource->name.c_str(), it->first);
			strcpy(img_file, getImageResFile(it->first));
			if(res_img_view->AddFile(szFile, it->first, TRUE))
				count ++;
		}
	}

	if(count > 0)
		res_img_view->SyncLoadImages();

	return TRUE;
}

string ImageEditor::save(BinStream* bin)
{
	string binFile = "image.res";
	char xmlFullName[MAX_PATH];

	int count = reses.size();
	Resource* resource;
	map<int, Resource*>::iterator it;

	if (!count) {
		return "";
	}

	sprintf(xmlFullName,"%s/image/id.xml", g_env->getResourcePath());
	saveXMLIDs(xmlFullName);

	bin->save32(count * sizeof(NCSRM_IDITEM) + sizeof(NCSRM_SECTHEADER));
	bin->save32(count);

	for(it=reses.begin(); it != reses.end(); ++it)
	{
		if((resource = it->second)) {
			bin->save32(resource->id);		//id
			bin->save32(resource->source_id); //file name
			bin->save32(0);					//offset
		}
	}

	return binFile;
}

//added by ly
BOOL ImageEditor::setResId(int oldId, int newId, DWORD res/*=0*/)
{
    if (oldId == newId)
        return FALSE;

	if(ID2TYPE(oldId) != NCSRT_IMAGE)
        return FALSE;

	if(ID2TYPE(oldId) != ID2TYPE(newId))
        return FALSE;

    if (reses.at(newId) != NULL)
        return FALSE;

    img_res_list->updateIDValue(oldId, newId);

    return ResEditor::setResId(oldId, newId);
}

int ImageEditor::setResName(int id, const char* name, DWORD res/*=0*/)
{
	DPRINT("in ImageEditor setResName\n");
	if(name == NULL)
		return -1;

	//update resource
	int newid = ResEditor::setResName(id, name);

	if(newid != -1)
	{
        //ImgResListPanel item text
        img_res_list->updateIDName(newid, id);
        return newid;
    }
 return -1;
}

void ImageEditor::onResNameChanged(int id, const char* newName)
{
}

void ImageEditor::onResIdChanged(int oldId, int newId)
{
    //modify reference
}
//

static inline BOOL isInvalidChar(char ch)
{
	return !((ch >= '0' && ch <='9')
						|| (ch >='A' && ch<='Z')
						|| (ch >='a' && ch<='z')
						|| ch == '_');
}

static void createImageIdName(const char* fileName, char* idName)  
{
    if (!fileName || !idName)
        return;

    const char *endPos = strrchr(fileName, '.');
    int length = 0;
    char idPrefix[] = "IDB_";
    int totalLen = strlen(idPrefix);

    if (endPos)
        length = endPos - fileName;
    else
        length = strlen(fileName);

    totalLen += length;
    //length need include the trailing '\0' 
	snprintf(idName, totalLen + 1, "IDB_%s", fileName);
    idName[totalLen] = '\0';

    for(int i = 1; idName[i]; i++) {
        if(isInvalidChar(idName[i]))
            idName[i] = '_';
        else
            idName[i] = toupper(idName[i]);
    }
}

BOOL ImageEditor::isAvailableIDName(const char* idName)
{
    if (idName) {
        if (!namedRes.at(idName))
            return TRUE;
    }
    return FALSE;
}

int ImageEditor::insertImageRes(const char* source_file,const char* file_name, char* id_name)
{
	char szName[MAX_PATH];
	char szDest[MAX_PATH];
	ResEditor::Resource * res = NULL;

	if(source_file == NULL)
		return -1;

	if(file_name == NULL)
	{
		file_name = strrchr(source_file, '/');
		if(!file_name)
			file_name = strrchr(source_file, '\\');
		if(file_name)
			file_name ++;
		else
			file_name = source_file;
	}

	if(id_name == NULL || !id_name[0]) {
        createImageIdName(file_name, szName);

        if (namedRes.at(szName)) {
            char baseName[MAX_PATH];
            int randSeed = 100, loop = 0;

            strcpy(baseName, szName);
            srand((unsigned)time(0));

            do {
                sprintf(szName, "%s_%d", baseName, rand()%randSeed);
                loop++;
            } while (namedRes.at(szName) && loop < randSeed);

            if (loop >= randSeed) {
                InfoBox(_("Error"), 
                        _("There are too many ID names using \"%s\" as ID prefix. Insert image resource failure."),
                        baseName);
                return -1;
            }
        }

		if(!id_name)
			id_name = szName;
		else
			strcpy(id_name, szName);
	}

	sprintf(szDest, "%s/image/%s",g_env->getResourcePath(), file_name);
	const char* strSource = strstr(szDest, "res/image");

	int source_id = g_env->stringToId(strSource);
	if(source_id != -1) 
		res = getResourceBySourceId(source_id);

	if(!res)
		res = namedRes.at(id_name);

    //resource exist
	if(res) {
		if(YesNoBox(_("Error"),
                    _("Resource \"%s\" has been imported, do you want overwrite it?\nSource: \"%s\"\n Destination: \"%s\""),
                    id_name, source_file, g_env->getString(res->source_id)) 
                != IDYES)
		{
			return -1;
		}

		if(strcmp(res->name.c_str(), id_name) != 0){
			if(YesNoBox(_("Error"),
                        _("Image \"%s\" has a different name in the resource.\nDo you want to repleace the Old Name [%s] to New Name [%s]?\n"),
                        file_name, res->name.c_str(), id_name) 
                    == IDYES)
			{
				 //eraser
				map<string, ResEditor::Resource*>:: iterator it = namedRes.find(res->name);
				namedRes.erase(it);
				res->name = id_name;
				namedRes[res->name] = res;
				g_env->updateResName(res->id, id_name);
			}
			else {
				strcpy(id_name, res->name.c_str());
			}
		}

		//copy file
		if(!copyfile(source_file, szDest)){
			InfoBox(_("Error"),_("Cannot copy file from \"%s\" to \"%s\""),source_file, szDest);
			return -1;
		}
		//change source id
		if(source_id != res->source_id)
		{
			g_env->removeString(res->source_id);
			res->source_id = g_env->addString(strSource);
		}
		return res->id;
	}
	else {
		int id = createRes(NCSRT_IMAGE, id_name, -1, strSource, 0);
		if(id == -1)
		{
			InfoBox(_("Error"), _("Cannot create image resource \"%s\"(file=\"%s\")"),id_name, szDest);
			return -1;
		}

		//copy file
		if(!copyfile(source_file, szDest)){
			InfoBox(_("Error"),_("Cannot copy file from \"%s\" to \"%s\""),source_file, szDest);
			removeRes(id);
			return -1;
		}

		return id;
	}
}

void ImageEditor::import()
{
	if(dir_img_view == NULL)
		return;

	ImageView::ViewItem * vi = dir_img_view->getCurImage();
	if(vi == NULL){
			InfoBox(_("Error"), _("Please select an image!"));
			return ;
	}

	char szName[256];
    createImageIdName(vi->fileName, szName);
    IMPORT_DLG_ADDDATA addData = {this, szName};
	if(DialogBoxIndirectParam(GetDlgTemplate(ID_IMPORTIMAGE), 
                dir_img_view->getHandler(), _importProc, (DWORD)&addData))
	{
		//import file
		int id = insertImageRes(vi->strFile.c_str(), vi->fileName, szName);
		if(id != -1)
		{
			img_res_list->AddRes(szName, id);
			res_img_view->Add(vi, id);
			enableMenuItem(GBC_SAVE);
		}
	}
}


void ImageEditor::importAll()
{
	BOOL badd = FALSE;
	for(ImageView::ViewItemIterator it = dir_img_view->begin(); !it.isEnd(); ++it){
		ImageView::ViewItem * vi = *it;
		char szName[256]="\0";
		int id = insertImageRes(vi->strFile.c_str(), vi->fileName, szName);
		if(id != -1)
		{
			img_res_list->AddRes(szName, id);
			res_img_view->Add(vi, id, TRUE);
			badd = TRUE;
		}
	}
	if(badd)
	{
		enableMenuItem(GBC_SAVE);
		res_img_view->SyncLoadImages();
	}
}

BOOL ImageEditor::loadConfig(xmlNodePtr root_node)
{
	if(!root_node)
		return FALSE;

	BOOL bsetPath = FALSE;

	xmlNodePtr node = xhGetChild(root_node, "image-editor");
	if(node )
	{

		for(node=node->children; node; node = node->next)
		{
			if(xhIsNode(node, "dir-path"))
			{
				xmlChar* xpath = xhGetNodeText(node);
				if(xpath){
					if(dir_tree_panel){
						dir_tree_panel->setPath((const char*)xpath);
						xmlFree(xpath);
						bsetPath = TRUE;
					}
				}
			}
		}
	}

	if(!bsetPath){
		if(dir_tree_panel){
			dir_tree_panel->setPath(getenv("HOME"));
		}
	}

	return TRUE;
}

BOOL ImageEditor::saveConfig(TextStream* stream)
{
	stream->println("<image-editor>");
	stream->indent();

	char szPath[MAX_PATH];

	if(dir_tree_panel && dir_tree_panel->getCurPath(szPath, sizeof(szPath)-1)>0)
		stream->println("<dir-path>%s</dir-path>",szPath);

	stream->unindent();
	stream->println("</image-editor>");
	return TRUE;
}

BOOL ImageEditor::WndProc(UINT iMsg, WPARAM wParam, LPARAM lParam, int *pret)
{
	if(iMsg == MSG_COMMAND)
	{
		if(HIWORD(wParam) == PSN_ACTIVE_CHANGED)
		{
			HWND hProp = (HWND)lParam;
			LRESULT idx = ::SendMessage(hProp, PSM_GETACTIVEINDEX, 0, 0);
			HWND hActivePage = (HWND)::SendMessage(hProp, PSM_GETPAGE, idx, 0);
			BOOL bIsDir = hActivePage == ::GetParent(dir_img_view->getHandler());
			BOOL bIsImportEnable = FALSE;
			if(bIsDir)
			{
				bIsImportEnable = ::SendMessage(dir_img_view->getHandler(), IVM_GETCURSEL, 0, 0) >= 0;
			}

			enableMenuItem(IMG_MENUCMD_IMPORT, bIsImportEnable);
			enableMenuItem(IMG_MENUCMD_IMPORTALL, bIsDir);
			return TRUE;
		}
	}

	return ResEditor::WndProc(iMsg, wParam, lParam, pret);
}

///////////////////////
//Create ResIdEditor
DECLARE_RESEDITOR(ImageEditor)
