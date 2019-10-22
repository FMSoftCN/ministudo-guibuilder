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
#ifndef WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif
#include <sys/types.h>
#include <signal.h>
#include "mgapp.h"

/*
 * Function MGMainLoop
 *      The Main loop of MGFC Application
 */
int MGMainLoop(HWND hMainWnd,HACCEL hAccel/*=0*/)
{
	MSG msg;
	if(hMainWnd!=HWND_DESKTOP && !(hMainWnd!=(HWND)0 && IsMainWindow(hMainWnd)))
		return -1;
	while(GetMessage(&msg,hMainWnd))
	{
		 if(!hAccel || (hAccel && TranslateAccelerator(hAccel, &msg)))
                {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
	}

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}

#if defined(_LITE_VERSION) && !(_STAND_ALONE)
/*
 *  Function InitClientLayer
 *       initialize client appliction. joint itself into mginit
 *  Param:
 *       int argc, const char** argv. the param of main function
 *  return:
 *       BOOL
 */
BOOL InitClientLayer(int argc,const char** argv)
{
    int i;
    const char* layer = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp (argv[i], "-layer") == 0) {
            layer = argv[i + 1];
            break;
        }
    }

    GetLayerInfo (layer, NULL, NULL, NULL);

    if (JoinLayer (layer, argv[0], 0, 0) == INV_LAYER_HANDLE) {
        fprintf (stderr,"JoinLayer: invalid layer handle.\n");
        return FALSE;
    }
    return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////////////
#if defined(_LITE_VERSION) && !(_STAND_ALONE)
/*
 * Process Version: the mginit application framework
 */
MGServerApp *MGServerApp::_server_app=NULL;
MGServerApp::MGServerApp()
{
	if(_server_app!=NULL)
	{
		fprintf(stderr,"The MGServerApp only have one instance\n");
		exit(5);
	}
	_server_app = this;
        OnNewDelClient = MGServerApp::_on_new_del_client;
        OnChangeLayer  = MGServerApp::_on_change_layer;
        m_clientCount = 0;
        m_layerCount = 0;
}

MGServerApp::~MGServerApp()
{
	_server_app = NULL;
}

BOOL MGServerApp::InitServer(int nr_globals/*=0*/,int def_nr_topmosts/*=0*/,int def_nr_normals/*=0*/)
{
	struct sigaction siga;
        if(!ServerStartup(nr_globals,def_nr_topmosts,def_nr_normals))
		return FALSE;

	siga.sa_handler = _on_wait_child;
	siga.sa_flags  = 0;
	memset (&siga.sa_mask, 0, sizeof(sigset_t));
	sigaction (SIGCHLD, &siga, NULL);

	SetServerEventHook(_on_event_hook);
	return TRUE;
}

void MGServerApp::_on_new_del_client (int op, int cli)
{
//	printf("on new del client:op=%d,cli=%d\n",op,cli);
        if(_server_app)
        {
                MG_Client * client = mgClients + cli;

                if(op==LCO_NEW_CLIENT)
                {
                        _server_app->m_clientCount ++;
                        _server_app->OnNewClient(static_cast<MGClient*>(client));
                }
                else if(op==LCO_DEL_CLIENT)
                {
                        if(_server_app->m_clientCount<=0)
                        {
                                fprintf(stderr,"there is none of clients in MGFC Server\n");
                        }
                        else
                        {
                        	_server_app->m_clientCount --;
                                _server_app->OnDeleteClient(static_cast<MGClient*>(client));
                        }
                }
                else
                {
                        fprintf(stderr,"unknown operation(%d) in MGFC Server\n",op);
                }
        }
}

void MGServerApp::_on_change_layer (int op, MG_Layer* layer, MG_Client* client)
{
//	printf("on change layer:%d\n",op);
        if(_server_app)
        {
                switch(op)
                {
                case LCO_NEW_LAYER:
                        _server_app->m_layerCount ++;
                        _server_app->OnNewLayer(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                case LCO_DEL_LAYER:
                        _server_app->m_layerCount --;
                        _server_app->OnDeleteLayer(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                case LCO_JOIN_CLIENT:
                        _server_app->OnJoinClient(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                case LCO_REMOVE_CLIENT:
                        _server_app->OnRemoveClient(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                case LCO_TOPMOST_CHANGED:
                        _server_app->OnTopMostChanged(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                case LCO_ACTIVE_CHANGED:
                        _server_app->OnActiveChanged(static_cast<MGLayer*>(layer),static_cast<MGClient*>(client));
                        break;
                default:
                        fprintf(stderr,"unknown operation(%d) of layer in MGFC Server\n",op);
                        break;
                }
        }
}

int MGServerApp::_on_event_hook(PMSG pmsg)
{
	if(_server_app)
	{
		int ret=0;
		_server_app->m_pmsg = pmsg;
		if(_server_app->OnEventProc(pmsg->message,pmsg->wParam,pmsg->lParam,&ret))
		{
			return ret==0?HOOK_GOON:HOOK_STOP;
		}
		return HOOK_GOON;
	}
	return HOOK_GOON;
}

void MGServerApp::_on_wait_child(int sig)
{
	if(_server_app)
		_server_app->OnWaitChild(sig);
}


BOOL MGServerApp::OnNewClient(MGClient* pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnDeleteClient(MGClient* pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnNewLayer(MGLayer *pLayer,MGClient *pClient)
{

        return TRUE;
}

BOOL MGServerApp::OnDeleteLayer(MGLayer *pLayer,MGClient *pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnJoinClient(MGLayer* pLayer,MGClient *pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnRemoveClient(MGLayer* pLayer,MGClient *pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnTopMostChanged(MGLayer* pLayer,MGClient *pClient)
{

        return TRUE;
}

BOOL MGServerApp::OnActiveChanged(MGLayer* pLayer,MGClient *pClient)
{
        return TRUE;
}

BOOL MGServerApp::OnEventProc(UINT message,WPARAM wParam,LPARAM lParam,int *pret)
{
	return TRUE;
}

void MGServerApp::OnWaitChild(int sig)
{
    int pid;
    int status;

    while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED (status))
            printf ("--pid=%d--status=%x--rc=%d---\n", pid, status, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf ("--pid=%d--signal=%d--\n", pid, WTERMSIG (status));
    }
}

pid_t MGServerApp::ExecApp(const char* file_name, const char* app_name)
{
    pid_t pid = 0;

    if ((pid = vfork ()) > 0) {
        fprintf (stderr, "new child, pid: %d.\n", pid);
    }
    else if (pid == 0) {
        execl (file_name, app_name, NULL);
        perror ("execl");
        _exit (1);
    }
    else {
        perror ("vfork");
    }

    return pid;

}
#endif //defined(_LITE_VERSION) && !(_STAND_ALONE)

#ifndef _LITE_VERSION
#include <minigui/dti.c>
#endif

