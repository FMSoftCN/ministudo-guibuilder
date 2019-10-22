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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>

#include "mgheads.h"
#include "mgfcheads.h"
#include <mgplus/mgplus.h>

//#include "gbconfig.h"
#include "include/guibuilder.h"
#include "log.h"
#include "qvfb.h"

#ifdef WIN32
#include "func-win.h"
#else
#include <unistd.h>
#endif

#ifdef WIN32
static void * main_thread;
#else
static pthread_t main_thread;
#endif

HWND hMainWnd = HWND_NULL;

static char szPrjFile[MAX_PATH] = "\0";
extern HWND ShowMainWindow(int argc, const char* argv[]);
extern void SendCmdLine(const char* cmdLine);

static BOOL processArgs(int argc, const char* argv[]);
static int MiniGUIAppMain (int args, const char* argv[]);
static int registerSigHandlers(void);

HACCEL ghAccel = 0;

extern "C" {
    void ExitGUIBuilder(void)
    {

		PostMessage(hMainWnd, MSG_CLOSE, 0, 0);
    }

}

extern void get_exec_path(char *path, int len);
#ifdef WIN32
int main(int argc, const char** argv)
{
#else
int main(int argc, const char* argv[])
{
#endif

    int iRet = 0;

#ifdef WIN32
    char gbPath[MAX_PATH + 1];
    char mgPath[MAX_PATH + 1];

    main_thread = win_get_current_thread();
    win_get_exe_path(gbPath, MAX_PATH );
    // this gbPath like "C;\......\mstudio\guibuilder\"

    strcpy(mgPath, gbPath);
    strcat (mgPath, "config\\");

    win_setenv("GUIBUILDER_PATH", gbPath);
    win_setenv("MG_CFG_PATH", mgPath);
    win_setenv("NCS_CFG_PATH", mgPath);

	//DPRINT("%s:%d\n", __FILE__, __LINE__);
#else
    {
        char* gb_path;
        char mg_cfg_path[MAX_PATH] = {0};

        setenv("MG_IAL_ENGINE","pc_xvfb",1);

        gb_path = getenv("GUIBUILDER_PATH");
        if (gb_path) {
            if (*gb_path != '\0') {
                sprintf (mg_cfg_path, "%s", gb_path);
            }
        } else {
            get_exec_path(mg_cfg_path, MAX_PATH);
        }
        strcat (mg_cfg_path, "/etc");
        LOG_WARNING("GUIBUILDER =>set NCS_CFG_PATH:(%s)\n", mg_cfg_path);
        setenv("NCS_CFG_PATH", mg_cfg_path, 1);

        strcat (mg_cfg_path, "/guibuilder");
        LOG_WARNING("GUIBUILDER =>set MG_CFG_PATH:(%s)\n", mg_cfg_path);
        setenv("MG_CFG_PATH", mg_cfg_path, 1);
    }

    main_thread = pthread_self();
#endif

	//DPRINT("%s:%d\n", __FILE__, __LINE__);
    if(!processArgs(argc, argv))
        return 0;

	//DPRINT("%s:%d\n", __FILE__, __LINE__);
	
    if (InitGUI (argc, argv) != 0)
	{
		//log_dead("InitGUI failed!\n");
        return 1;
	}

	//DPRINT("%s:%d\n", __FILE__, __LINE__);
    if ( registerSigHandlers() == 0 )
        DPRINT("=======register signal ok\n");

	//DPRINT("%s:%d\n", __FILE__, __LINE__);
    iRet = MiniGUIAppMain (argc, argv);
    TerminateGUI (iRet);
	//DPRINT("%s:%d\n", __FILE__, __LINE__);
    return iRet;
}

////////////////////////////////////////////////
#ifdef WIN32
#define FIFO_FILE_INFO "\\\\.\\pipe\\gui-builder-%d"
#else
#define FIFO_FILE_INFO "/var/tmp/gui-builder-%d"
#endif
static pthread_t pip_id;
static char szFifo[100];
static int exit_system = 0;
static int fifo_fd = -1;

static void onTerminate(void)
{
    exit_system = 1;
    //char szExit[] = "@exit";
    //write(fifo_fd, szExit, strlen(szExit));

#ifndef WIN32  //fixme replace with windows pipe
    close(fifo_fd);
    remove(szFifo);
#endif

    //pthread_join(pip_id, NULL);
}

static BOOL processIsRunning(pid_t pid)
{
    char szProc[40];
    sprintf(szProc,"/proc/%d/stat",pid);
    FILE *fp = fopen(szProc, "rt");
    if(fp == NULL)
        return FALSE;

    //return state info
    int i = 0;
    while(1) {
        char ch = fgetc(fp);
        if(ch == ' ')
            i ++;
        if(i == 2)
            break;
    }
    char ch = fgetc(fp);

    fclose(fp);

    //get status "RSDZTW"
    if(ch == 'Z') //Zombie
        return FALSE;

    return TRUE;
}


static BOOL getProjectPath(int argc, const char* argv[], char* buff)
{
    int i;
    for(i = 1; i < argc - 1; i++) {
        if(argv[i][0] == '-') {
            if(strcmp(argv[i] ,"-project") == 0) {
                strcpy(buff, argv[i+1]);
                return TRUE;
            } else if(strcmp(argv[i], "-file") == 0
                      || strcmp(argv[i], "-project-file") == 0
                      || strcmp(argv[i], "-update-file") == 0) {
                const char* str = strrchr(argv[i+1],'/');
                if(str == NULL)
                    str = strrchr(argv[i+1], '\\');
                if(str == NULL)
                    continue;
                strncpy(buff, argv[i+1], str - argv[i+1]);
                buff[str - argv[i+1]] = '\0';
                return TRUE;
            }
        }
    }
    return FALSE;
}

//stat project info
static void start_project(const char* szPath);

static void usage()
{
#ifdef WIN32

#else
    printf("Use GUI Builder (%s)\n", PACKAGE_VERSION);
    printf("\t-project\t\tThe path of project\n");
    printf("\t-project-name\t\tThe name of project\n");
    printf("\t-file\t\tThe resource file want to open\n");
    printf("\t-project-file\t\tsame as -file\n");
    printf("\t-update-file\t\tThe File want to update\n");
    printf("\t-config-file \t\tThe config file GUI Builder used\n");
    printf("\t-def-renderer \t\tThe default renderer of Sample.\n");
    printf("\t-v|--version \t\t The version of GUI Builder.\n");
#endif
}

#include "build_version.h"
static void show_version()
{
    //printf("%s\n", PACKAGE_STRING);
    printf("GUIBuilder %s\n", PACKAGE_VERSION);
    printf("Build-Version: %s\n", BUILD_VERSION);
    printf("Copyright Beijing FMSoft Technologies CO., LTD.\n");
}

static BOOL processArgs(int argc, const char* argv[])
{
    char szPath[MAX_PATH];
    int path_len;
    //get project path
    if(argc <= 1 || argv == NULL) {
        usage();
        exit(-1);
    }

    if(argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version")==0)) {
        show_version();
        exit(0);
    }

    //1. get the project file
    if(!getProjectPath(argc, argv, szPath)) {
        LOG_WARNING("cannot open project path\n");
        exit(-2); //not find path
    }

    //2. test the dir exist
    struct stat sbuff;
    if(stat(szPath, &sbuff)!= 0 || !S_ISDIR(sbuff.st_mode)) {
        LOG_WARNING( "%s is not a directory or not exist\n", szPath);
        exit(-3);
    }

    path_len = strlen(szPath);
    //3. try to open the project info file
    strcpy(szPath+path_len, "/.guibuilder-prj");
    FILE *fp = fopen(szPath, "rb");
    if(fp == NULL) { //not exit, continue this process
        szPath[path_len] = '\0';
        start_project(szPath);
        return TRUE;
    }

    //4. read pid
    pid_t pid ;
    fread(&pid, 1, sizeof(pid), fp);

    fclose(fp);

    //get process state
#ifdef WIN32
    if(!win_process_running(pid)) {
#else
    if(!processIsRunning(pid)) {
#endif
        //remove the fifo file
        sprintf(szFifo, FIFO_FILE_INFO, pid);
        unlink(szFifo);
        szPath[path_len] = '\0';
        start_project(szPath);
        return TRUE; //clear and reset it
    }

    //open process FIFO
    sprintf(szPath, FIFO_FILE_INFO, pid);
#ifdef WIN32
    int fifo = win_open_client_fifo(szPath);
#else
    int fifo = open(szPath, O_WRONLY);
#endif
    if(fifo == -1) {
        LOG_WARNING( "cannot open fifo:%s\n", szPath);
        exit(-4);
    }

    //write info, show the project
    char szInfo[] = "@show\n";
#ifdef WIN32
    win_write_fifo(fifo, szInfo, strlen(szInfo));
#else
    write(fifo, szInfo, strlen(szInfo));
#endif
    //write argc and argv
    for(int i=1; i< argc; i++) {
#ifdef WIN32
        win_write_fifo(fifo, argv[i], strlen(argv[i]));
        win_write_fifo(fifo, " ", 1); //wirte space
#else
        write(fifo, argv[i], strlen(argv[i]));
        write(fifo, " ", 1); //wirte space
#endif
    }
#ifdef WIN32
    win_write_fifo(fifo,"\n", 1); //write end

    win_close_fifo(fifo);
#else
    write(fifo,"\n", 1); //write end

    close(fifo);
#endif
    return FALSE;

}

//////////////

/*
static void termination_handler (int signum)
{
	onTerminate();
}
*/

static void* wait_for_other_args(void * param)
{
    char szBuff[4096];
    int fd = *(int*)param;

    //struct timeval tv = {1, 500000};
    while(!exit_system) {
#ifdef WIN32
        int n = win_wait_fifo(fd);
#else
        fd_set fdread;
        FD_ZERO(&fdread);
        FD_SET(fd, &fdread);
        int n = select(fd+1, &fdread, NULL, NULL, NULL);
#endif
        //printf("fd=%d, n=%d\n", fd, n);
        if(n>0) {
#ifdef WIN32
            int len = win_read_fifo(fd, szBuff, sizeof(szBuff));
#else
            int len = read(fd, szBuff, sizeof(szBuff));
#endif
            if(len <= 0)
                goto REOPEN;
            szBuff[len] = '\0';
            if(strcmp(szBuff, "@exit") == 0)
                break;

            char* strCmd = (char*)strchr(szBuff, '\n');
            if(strCmd) {
                strCmd[0] = '\0';
                if(strcmp(szBuff, "@show") == 0) {
                    SendCmdLine(strCmd + 1);
                    VFBShowWindow(TRUE);
                    continue;
                }
            }
            //process args, and send to args
            SendCmdLine(szBuff);
            continue;
        }


REOPEN:
        //reopen
#ifdef WIN32
        win_close_fifo(fd);
        fifo_fd = win_open_fifo(szFifo);
#else
        close(fd);
        fifo_fd = open(szFifo, O_RDONLY|O_NDELAY);
#endif
        fd = fifo_fd;
    }

#ifdef WIN32
    win_close_fifo(fd);
#else
    close(fd);
    unlink(szFifo);

#endif
    return NULL;
}

//check and create resource directroies
#ifdef WIN32
//#include <direct.h>
#define MKDIR(szPath,len, dir_name) do{ \
	sprintf(szPath+len, "/%s", dir_name); \
	mkdir(szPath,0644); \
}while(0)
#else
#include <sys/types.h>
#include <sys/stat.h>
#define MKDIR(szPath,len, dir_name) do{ \
	sprintf(szPath+len, "/%s", dir_name); \
	mkdir(szPath,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IXUSR); \
}while(0)
#endif

static void check_create_res_dirs(const char* strProjectPath)
{
    if(strProjectPath == NULL)
        strProjectPath = ".";

    char szPath[1024];
    int len = strlen(strProjectPath);
    strcpy(szPath, strProjectPath);

    MKDIR(szPath, len, "include");
    MKDIR(szPath, len, "src");
    MKDIR(szPath, len, "res");

    len += strlen(szPath+len);

    MKDIR(szPath, len, "ui");
    MKDIR(szPath, len, "renderer");
    MKDIR(szPath, len, "text");
    MKDIR(szPath, len, "image");

}

//start project info
static void start_project(const char* strProjectPath)
{
    if(strProjectPath == NULL) {
        LOG_WARNING("no project\n");
        exit(-5);
    }
#ifdef WIN32
    int pid = win_getpid();
#else
    pid_t pid = getpid();
#endif
    //1. clear the fifo
    sprintf(szFifo, FIFO_FILE_INFO, pid);
    unlink(szFifo);

    //2. create guibuilder-prj file
    sprintf(szPrjFile,"%s/.guibuilder-prj", strProjectPath);
    FILE *fp = fopen(szPrjFile,"wb");
    if(fp == NULL) {
        LOG_WARNING("cannot open project info file:%s\n",szPrjFile);
        exit(-6);
    }

    //3. wirte pid
    fwrite(&pid, 1, sizeof(pid), fp);

    fclose(fp);

#ifdef WIN32  //fixme replace with windows pipe
    fifo_fd = win_open_fifo(szFifo);
#else
    //4. make fifo
    int ret = mkfifo(szFifo, S_IWUSR|S_IRUSR);
    if(ret == -1) {
        LOG_WARNING( "cannot make fifo:%s\n",szFifo);
        exit(-7);
    }

    int fd = open(szFifo, O_RDONLY|O_NDELAY);

    if(fd == -1) {
        LOG_WARNING( "open fifo %s failed\n", szFifo);
        exit(-8);
    }
    fifo_fd = fd;
#endif
    //check and create resource directroies
    check_create_res_dirs(strProjectPath);

    atexit(onTerminate);

    //start fifo thread
    pthread_create(&pip_id, NULL, wait_for_other_args, (void*)&fifo_fd);
}
extern void exitSystem(void);
static void segvsig_handler (int v)
{
    //exit system
    if (v == SIGTERM) {
        exitSystem();
    }
}


int registerSigHandlers(void)
{
#ifdef WIN32
    win_signal(SIGTERM, segvsig_handler);
    return 0;
#else
    struct sigaction siga;

    siga.sa_handler = segvsig_handler;
    siga.sa_flags = 0;
    memset (&siga.sa_mask, 0, sizeof (sigset_t));
    return sigaction (SIGTERM, &siga, NULL);
#endif
}

extern void DeleteMainFrame(void);
extern int _mainwnd_destroy ;
static int MiniGUIAppMain (int argc, const char* argv[])
{
    MSG msg;

    MGPlusRegisterFashionLFRDR();
    //init new control set
    ncsInitialize();

    SetDefaultWindowElementRenderer("flat");

    hMainWnd = ShowMainWindow(argc, argv);
    if(hMainWnd == HWND_INVALID)
        return -1;
    VFBAtExit(ExitGUIBuilder);

    while(GetMessage(&msg,hMainWnd)) {
        if(!ghAccel || (ghAccel && TranslateAccelerator(ghAccel, &msg))) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	if(!_mainwnd_destroy)
	    DestroyMainWindow(hMainWnd);
    MainWindowCleanup(hMainWnd);
    DeleteMainFrame();
    remove(szPrjFile);
    ncsUninitialize();
    PostQuitMessage(HWND_DESKTOP);
    return 0;
}
