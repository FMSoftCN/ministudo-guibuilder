/*
 * select-project-type-dlg.cpp
 *
 *  Created on: 2009-4-26
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
#include "undo-redo.h"
#include "stream.h"
#include "resenv.h"

#include "select-project-type-dlg.h"

#ifdef WIN32
#include "func-win.h"
#endif

#include "dlgtmpls.h"

SelectProjectTypeDlg::SelectProjectTypeDlg(HWND hParent,const char* cfgPath) {
	// TODO Auto-generated constructor stub
	this->cfgPath = NULL;

	if(cfgPath)
		this->cfgPath = strdup(cfgPath);

	//create
	Create(hParent,GetDlgTemplate(ID_SELECTPROJECTTYPE));

	if(!getAndInitGUIBuilderCfg()){
		char szInfo[1024];
		sprintf(szInfo, "cannot read GUI Builder Configuration File, stop GUI Builder (\"%s\")", cfgPath);
		LOG_WARNING("%s", szInfo);
		throw(szInfo);
	}


	CenterWindow();
}

SelectProjectTypeDlg::~SelectProjectTypeDlg() {
    free(cfgPath);
}

BEGIN_MSG_MAP(SelectProjectTypeDlg)
	BEGIN_COMMAND_MAP
		MAP_COMMAND(IDOK, onOK)
		MAP_COMMAND(IDCANCEL, onCancel)
	END_COMMAND_MAP
	MAP_DESTROY(onDestroy)
END_MSG_MAP

void SelectProjectTypeDlg::onOK()
{
	//get current name of
	HWND hwnd = GetChild(100);
	int idx = ::SendMessage(hwnd, LB_GETCURSEL, 0, 0);
	if(idx < 0)
	{
		InfoBox(_("Error"), _("Please select the project type fristly!"));
		SetFocus(hwnd);
		return ;
	}

	const char* name = (const char*)::SendMessage(hwnd, LB_GETITEMADDDATA, idx, 0);
	if(name == NULL)
	{
		InfoBox(_("Error"), _("The project type is invalid, please select another one."));
		SetFocus(hwnd);
		return;
	}

	//read section from etc file
	GHANDLE hEtc = LoadEtcFile(cfgPath);
	if(hEtc == 0)
	{
		InfoBox(_("Fatel Error"), _("Cannot open the configuration file \"%s\", unable to continue GUI Builder, Please check your configuration fil"),cfgPath);
		throw("cannot open the configuration");
	}

	//create the project info
	char szPath[1024];
	sprintf(szPath,"%s/res/res.project",g_env->getProjectPath());
	FileStreamStorage fss(szPath);
	TextStream stream(&fss);
	stream.println("<res-project version=\"%s\">", g_env->getVersion());
	stream.indent();
	stream.printf("<configuration name=\"%s\"", name);
	//save caption
	if(GetValueFromEtc(hEtc,name,"caption",szPath, sizeof(szPath)-1) == ETC_OK)
		stream.printf(" caption=\"%s\"",szPath);
	stream.println(">");

		stream.indent();
		//save minigui-version
		if(GetValueFromEtc(hEtc, name, "minigui-version", szPath,sizeof(szPath)-1) == ETC_OK)
			stream.println("<minigui-version>%s</minigui-version>",szPath);
		//save controlset
		if(GetValueFromEtc(hEtc, name, "control-set",szPath,sizeof(szPath)-1) == ETC_OK)
			stream.println("<control-set>%s</control-set>",szPath);
		//save path
		if(GetValueFromEtc(hEtc, name, "path", szPath, sizeof(szPath)-1) == ETC_OK)
			stream.println("<path>%s</path>",szPath);
		//load library
		for(int i=1; ;i++)
		{
			char szKey[100];
			sprintf(szKey, "library%d",i);
			if(GetValueFromEtc(hEtc, name, szKey, szPath, sizeof(szPath)-1)!=ETC_OK)
				break;
			char *str;
			stream.println("<library>");
			stream.indent();
			//get path of library
			str = (char*)strchr(szPath, ':');
			if(str)
				*str = 0;
			stream.println("<libpath>%s</libpath>",szPath);
			if(str)
			{
				str ++;
				//get init function name
				char* str1 = strchr(str,':');
				if(str1)
					*str1 = '\0';
				stream.println("<init>%s</init>",str);
				if(str1)
					stream.println("<uninit>%s</uninit>",str1+1);
			}
			stream.unindent();
			stream.println("</library>");
		}
		stream.unindent();
		stream.println("</configuration>");

	stream.unindent();
	stream.println("</res-project>");

	UnloadEtcFile(hEtc);

	EndDialog(TRUE);
}

void SelectProjectTypeDlg::onCancel()
{
	exit(0);
	EndDialog(FALSE);
}

BOOL SelectProjectTypeDlg::getAndInitGUIBuilderCfg()
{
	if(cfgPath && initProjectInfo(cfgPath))
		return TRUE;

    return FALSE;
}

BOOL SelectProjectTypeDlg::initProjectInfo(const char* cfgPath)
{
	BOOL bret = FALSE;
	HWND hwnd = GetChild(100); //listbox

	GHANDLE hEtc = LoadEtcFile(cfgPath);

	if(hEtc == 0)
		return FALSE;

	int count = 0;
	if(GetIntValueFromEtc(hEtc, "guibuilder-config","count", &count) != ETC_OK)
		goto FAILED;

	if(count <= 0)
		goto FAILED;

	for(int i=0; i<count; i++)
	{
		char szKey[100];
		sprintf(szKey,"config%d",i+1);
		char szSection[256] = "\0";
		if(GetValueFromEtc(hEtc,"guibuilder-config",szKey,szSection, sizeof(szSection)-1)!=ETC_OK)
			continue;

		char szValue[256];
		//get Caption
		if(GetValueFromEtc(hEtc,szSection,"caption",szValue, sizeof(szValue))!=ETC_OK)
			continue;
		int idx = ::SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)szValue);
		if(idx < 0)
			continue;

		char* strName = strdup(szSection);
		::SendMessage(hwnd, LB_SETITEMADDDATA, idx, (LPARAM)strName);
	}

	bret = TRUE;

FAILED:
	UnloadEtcFile(hEtc);
	return bret;
}

void SelectProjectTypeDlg::onDestroy()
{
	HWND hwnd = GetChild(100);
	for(int i=0; i< ::SendMessage(hwnd, LB_GETCOUNT, 0, 0); i++)
	{
		char* str = (char*)::SendMessage(hwnd, LB_GETITEMADDDATA, i, 0);
        free(str);
	}
}
