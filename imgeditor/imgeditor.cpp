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
:idrm(this, NCSRT_IMAGE)
{
}

ImageEditor::~ImageEditor()
{
    delete dir_tree_panel;
    delete img_res_list;
    delete dir_img_view;
    delete res_img_view;
    delete res_desc_panel;
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
						_("The \'%s\' cannot be removed. It\'s used by other resource or not exist."),
						idToName((int)param2));
				return FALSE;
			}
		}
		break;
	}
	return 0;
}

static BOOL copyfile(const char* filesrc, const char* filedst)
{
	BOOL bret = FALSE;
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

#if 1
#include "dlgtmpls.h"
#else
static CTRLDATA _input_ctrls [] =
{
	{
		"static", WS_VISIBLE,
		10, 10, 200, 40,
		0,
		_("Please give a name for the adding image: \n (---- likes 'IDB_IMG_254' ----)"),
		0
	},
	{
		"sledit", WS_VISIBLE|WS_BORDER,
		10, 50, 210, 30,
		IDC_INPUT,
		"",
		0
	},
	{
		"button", WS_VISIBLE|BS_PUSHBUTTON,
		110, 95, 70, 25,
		IDC_IMPORT,
		_("Import"),
		0
	},
	{
		"button", WS_VISIBLE|BS_PUSHBUTTON,
		190, 95, 70, 25,
		IDCANCEL,
		_("Cancel"),
		0
	}
};

static DLGTEMPLATE _input_dlg =
{
	WS_BORDER|WS_CAPTION|WS_DLGFRAME,
	WS_EX_NONE,
	200, 150,  280, 160,
	_("Import Image Resource"),
	0, 0,
	sizeof(_input_ctrls)/sizeof(CTRLDATA),
	_input_ctrls
};
#endif

static int _importProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	HWND hEdit;
	char * str_id_name;
	switch(message)
	{
		case MSG_INITDIALOG:
			hEdit = GetDlgItem(hDlg,IDC_INPUT);
			str_id_name = (char*)lParam;/*GetWindowAdditionalData(hDlg)*/;
			SetWindowAdditionalData(hDlg, lParam);
			SetWindowText(hEdit, str_id_name);
			SetFocus(hEdit);
			SendMessage(hEdit, EM_SELECTALL, 0, 0);
			return 0;
		case MSG_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_IMPORT:
				hEdit = GetDlgItem(hDlg,IDC_INPUT);
				str_id_name = (char*)GetWindowAdditionalData(hDlg);
				if(GetWindowText(hEdit, str_id_name, 256)>0 && ValidIDName(str_id_name))
				{
					EndDialog(hDlg, 1);
					break;
				}
				MessageBox(hDlg,_("Invalidate Name, ID name must start with a Alpha, and only include Alpha, \'_\', digit, e.g. IDB_IMAGE1"),_("Error") ,MB_OK);
				SendMessage(hEdit, EM_SELECTALL, 0, 0);
				SetFocus(hEdit);
				break;
			case IDCANCEL:
				EndDialog(hDlg, 0);
				break;
			}
	}
	//return DefaultDialogProc(hDlg, message, wParam, lParam);
	return AutoCenterDlgProc(hDlg, message, wParam, lParam);
}

void ImageEditor::executeCommand(int cmd_id, int status, DWORD param)
{
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

	loadXMLIDs(xmlFile);

	for(it=reses.begin(); it != reses.end(); ++it)
	{
		if((resource = it->second)){
			img_res_list->AddRes(resource->name.c_str(), it->first);
		}
	}

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

static inline BOOL isInvalidateChar(char ch)
{
	return !((ch >= '0' && ch <='9')
						|| (ch >='A' && ch<='Z')
						|| (ch >='a' && ch<='z')
						|| ch == '_');
}
int ImageEditor::insertImageRes(const char* source_file,const char* file_name, char* id_name)
{
	char szName[256];
	char szdest[MAX_PATH];
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

	if(id_name == NULL || !id_name[0]){
		const char* fileName = file_name;

		sprintf(szName,"IDB_%s", fileName);
		fileName = strrchr(szName, '.');
		if(fileName)
			*(char*)fileName = '\0';
		for(int i=1; szName[i]; i++){
			if(isInvalidateChar(szName[i]))
				szName[i] = '_';
			else
				szName[i] = toupper(szName[i]);
		}
		if(!id_name)
			id_name = szName;
		else
			strcpy(id_name,szName);
	}

	sprintf(szdest, "%s/image/%s",g_env->getResourcePath(),file_name);
	const char* strSource = strstr(szdest,"res/image");

	int source_id = g_env->stringToId(strSource);

	if(source_id != -1)
		res = getResourceBySourceId(source_id);

	if(!res)
		res = namedRes.at(id_name);

	if(res)//exist
	{
		if(YesNoBox(_("Error"),_("Resource \"%s\" has been imported, Do you want overwrite it?\nFrom File: \"%s\"\n Over Write: \"%s\""),id_name,source_file, g_env->getString(res->source_id)) != IDYES)
		{
			return -1;
		}

		if(strcmp(res->name.c_str(), id_name) != 0){
			if(YesNoBox(_("Error"),
					_("Image \"%s\" Have an diffrent name in the resource.\nDo you want repleace the Old Name [%s] to New Name [%s]\n"),
					file_name, res->name.c_str(), id_name) == IDYES)
			{
				 //eraser
				map<string, ResEditor::Resource*>:: iterator it = namedRes.find(res->name);
				namedRes.erase(it);
				res->name = id_name;
				namedRes[res->name] = res;
			}
		}

		//copy file
		if(!copyfile(source_file, szdest)){
			InfoBox(_("Error"),_("Cannot copy file from \"%s\" to \"%s\""),source_file, szdest);
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
	else
	{
		int id = createRes(NCSRT_IMAGE, id_name, -1, strSource, 0);
		if(id == -1)
		{
			InfoBox(_("Error"), _("Cannot Create Image Res \"%s\"(file=\"%s\")"),id_name, szdest);
			return -1;
		}
		//copy file
		if(!copyfile(source_file, szdest)){
			InfoBox(_("Error"),_("Cannot copy file from \"%s\" to \"%s\""),source_file, szdest);
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
			InfoBox(_("Error"), _("Please Select An Image!"));
			return ;
	}

	char szName[256];
	sprintf(szName,"IDB_%s", vi->fileName);
	char* str = (char*)strrchr(szName,'.');
	if(str)
		*str = '\0';
	for(str = szName; *str; str++){
		if(isInvalidateChar(*str))
			*str = '_';
		else
			*str = toupper(*str);
	}

#if 0
	_input_dlg.dwAddData = (DWORD)szName;
	if(DialogBoxIndirectParam(&_input_dlg, dir_img_view->getHandler(), _importProc, 0))
#else
	if(DialogBoxIndirectParam(GetDlgTemplate(ID_IMPORTIMAGE), dir_img_view->getHandler(), _importProc, (DWORD)szName))
#endif
	{
		//import file
		int id = insertImageRes(vi->strFile.c_str(),vi->fileName, szName);
		if(id != -1)
		{
			img_res_list->AddRes(szName, id);
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
		int id = insertImageRes(vi->strFile.c_str(),vi->fileName,szName);
		if(id != -1)
		{
			img_res_list->AddRes(szName, id);
			badd = TRUE;
		}
	}
	if(badd)
		enableMenuItem(GBC_SAVE);
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

///////////////////////
//Create ResIdEditor
DECLARE_RESEDITOR(ImageEditor)
