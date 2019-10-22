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
#ifndef MGAPP_H
#define MGAPP_H
#include "mgcomm.h"
#include "mgwnd.h"

/*
 * Function MGMainLoop
 *      The main loop of MainWnd
 * Param:
 *      HWND hMainWnd: the MainWindow recive message.if hMainWnd==NULL, use HWND_DESKTOP
 *      HACCEL hAccel: the Accelator, ignore
 * return
 *      int: the proccess exit code
 */

#define InitMiniGUIExt() (1)

int MGMainLoop(HWND hMainWnd,HACCEL hAccel=0);

#if defined(_LITE_VERSION) && !(_STAND_ALONE)
BOOL InitClientLayer(int argc,const char** argv);
#else
#define InitClientLayer(argc,argv)
#endif

#if defined(_LITE_VERSION) && !(_STAND_ALONE)
/*
 * The Proccess Version: the Main Function of client application which has a Dialog
 */
#define MGDlgMainInitFunc(TMGWnd,InitFunc) int MiniGUIMain(int argc,const char* argv[]){ \
	if(!(InitClientLayer(argc,argv) && (InitFunc))) return -1; \
	TMGWnd MainWnd; \
	int ret = MainWnd.DoMode(); \
	return ret; \
}
#else
#define MGDlgMainInitFunc(TMGWnd,InitFunc) int MiniGUIMain(int argc,const char* argv[]){ \
	if(!(InitFunc)) return -1; \
	TMGWnd MainWnd; \
	int ret = MainWnd.DoMode(); \
	return ret; \
}
#endif
#define MGDlgMain(TMGWnd) MGDlgMainInitFunc(TMGWnd,1)

//client main with main window
#if defined(_LITE_VERSION) && !(_STAND_ALONE)
/*
 * The Process Version: the Main Function of client appliction which use a Main Window
 */
#define MGWndMainInitFunc(TMGWnd,InitFunc) int MiniGUIMain(int argc,const char* argv[]){ \
	if(!(InitClientLayer(argc,argv) && InitMiniGUIExt() && (InitFunc))) return -1; \
	TMGWnd MainWnd; \
	if(!MainWnd.Create()) return -2; \
	int ret = MGMainLoop(MainWnd); \
	return ret; \
}
#else
#define MGWndMainInitFunc(TMGWnd,InitFunc) int MiniGUIMain(int argc,const char* argv[]){ \
	if(!(InitMiniGUIExt() && (InitFunc))) return -1; \
	TMGWnd MainWnd; \
	if(!MainWnd.Create()) return -2; \
	int ret = MGMainLoop(MainWnd); \
	return ret; \
}
#endif
#define MGWndMain(TMGWnd) MGWndMainInitFunc(TMGWnd,1)

///////////////////////////////////////////////////////////////////////////
/*
 * Process Version: The  mginit frame work
 */
#if defined(_LITE_VERSION) && !(_STAND_ALONE)
class MGClient:public MG_Client
{
public:
        MGClient();
        ~MGClient();
};

class MGLayer:public MG_Layer
{
public:
        MGLayer();
        ~MGLayer();
};

class MGServerApp
{
public:
    MGServerApp();
    virtual ~MGServerApp();

	char *m_argc;
	char **m_argv;

    virtual BOOL InitServer(int nr_globals=0,int def_nr_topmosts=0,int def_nr_normals=0);

	pid_t ExecApp(const char* file_name, const char* app_name);

	int ClientCount(){ return m_clientCount; }
	int LayerCount(){ return m_layerCount; }
protected:

        int m_clientCount;
        virtual BOOL OnNewClient(MGClient* pClient);
        virtual BOOL OnDeleteClient(MGClient* pClient);

        int m_layerCount;
        virtual BOOL OnNewLayer(MGLayer *pLayer,MGClient *pClient);
        virtual BOOL OnDeleteLayer(MGLayer *pLayer,MGClient *pClient);
        virtual BOOL OnJoinClient(MGLayer* pLayer,MGClient *pClient);
        virtual BOOL OnRemoveClient(MGLayer* pLayer,MGClient *pClient);
        virtual BOOL OnTopMostChanged(MGLayer* pLayer,MGClient *pClient);
        virtual BOOL OnActiveChanged(MGLayer* pLayer,MGClient *pClient);

	PMSG m_pmsg;
	virtual BOOL OnEventProc(int message,WPARAM wParam,LPARAM lParam,int *pret);
	virtual void OnWaitChild(int sig);
private:
        static void _on_new_del_client (int op, int cli);
        static void _on_change_layer (int op, MG_Layer* layer, MG_Client* client);
	static int _on_event_hook(PMSG pmsg);
	static void _on_wait_child(int sig);

	static MGServerApp *_server_app;
};

#define ServerMain(TMGServerApp) \
int MiniGUIMain(int argc,const char* argv[]) { \
	TMGServerApp ServerApp; \
	if(!ServerApp.InitServer()) \
		return -1; \
	return MGMainLoop(HWND_DESKTOP); \
}

#endif //defined(_LITE_VERSION) && !(_STAND_ALONE)

#endif
