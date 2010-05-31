#ifdef WIN32
#include <windows.h>
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "gbconfig.h"

#ifdef _MSTUDIO_OFFICIAL_RELEASE
#include "sense4.h"
#include "psense4.h"
#endif

#include "func-win.h"

#ifdef WIN32

/* Note:
 * MSDN says these functions are not thread-safe. We make no efforts to have
 * any kind of thread safety.
 */

typedef struct global_object {
    HMODULE hModule;
    struct global_object *previous;
    struct global_object *next;
} global_object;

static global_object first_object;

/* These functions implement a double linked list for the global objects. */
static global_object *global_search( HMODULE hModule )
{
    global_object *pobject;

    if( hModule == NULL )
        return NULL;

    for( pobject = &first_object; pobject ; pobject = pobject->next )
        if( pobject->hModule == hModule )
            return pobject;

    return NULL;
}

static void global_add( HMODULE hModule )
{
    global_object *pobject;
    global_object *nobject;

    if( hModule == NULL )
        return;

    pobject = global_search( hModule );

    /* Do not add object again if it's already on the list */
    if( pobject )
        return;

    for( pobject = &first_object; pobject->next ; pobject = pobject->next );

    nobject = (global_object *)malloc( sizeof(global_object) );

    /* Should this be enough to fail global_add, and therefore also fail
     * dlopen?
     */
    if( !nobject )
        return;

    pobject->next = nobject;
    nobject->next = NULL;
    nobject->previous = pobject;
    nobject->hModule = hModule;
}

static void global_rem( HMODULE hModule )
{
    global_object *pobject;

    if( hModule == NULL )
        return;

    pobject = global_search( hModule );

    if( !pobject )
        return;

    if( pobject->next )
        pobject->next->previous = pobject->previous;
    if( pobject->previous )
        pobject->previous->next = pobject->next;

    free( pobject );
}

/* POSIX says dlerror( ) doesn't have to be thread-safe, so we use one
 * static buffer.
 * MSDN says the buffer cannot be larger than 64K bytes, so we set it to
 * the limit.
 */
static char error_buffer[65535];
static char *current_error;

static int copy_string( char *dest, int dest_size, const char *src )
{
    int i = 0;

    /* gcc should optimize this out */
    if( !src && !dest )
        return 0;

    for( i = 0 ; i < dest_size-1 ; i++ )
    {
        if( !src[i] )
            break;
        else
            dest[i] = src[i];
    }
    dest[i] = '\0';

    return i;
}

static void save_err_str( const char *str )
{
    DWORD dwMessageId;
    DWORD pos;

    dwMessageId = GetLastError( );

    if( dwMessageId == 0 )
        return;

    /* Format error message to:
     * "<argument to function that failed>": <Windows localized error message>
     */
    pos  = copy_string( error_buffer,     sizeof(error_buffer),     "\"" );
    pos += copy_string( error_buffer+pos, sizeof(error_buffer)-pos, str );
    pos += copy_string( error_buffer+pos, sizeof(error_buffer)-pos, "\": " );
    pos += FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwMessageId,
                          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                          error_buffer+pos, sizeof(error_buffer)-pos, NULL );

    if( pos > 1 )
    {
        /* POSIX says the string must not have trailing <newline> */
        if( error_buffer[pos-2] == '\r' && error_buffer[pos-1] == '\n' )
            error_buffer[pos-2] = '\0';
    }

    current_error = error_buffer;
}

static void save_err_ptr_str( const void *ptr )
{
    char ptr_buf[19]; /* 0x<pointer> up to 64 bits. */

    sprintf( ptr_buf, "0x%p", ptr );

    save_err_str( ptr_buf );
}

void *dlopen( const char *file, int mode )
{
    HMODULE hModule;
    UINT uMode;

    current_error = NULL;

    /* Do not let Windows display the critical-error-handler message box */
    uMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    if( file == 0 )
    {
        /* POSIX says that if the value of file is 0, a handle on a global
         * symbol object must be provided. That object must be able to access
         * all symbols from the original program file, and any objects loaded
         * with the RTLD_GLOBAL flag.
         * The return value from GetModuleHandle( ) allows us to retrieve
         * symbols only from the original program file. For objects loaded with
         * the RTLD_GLOBAL flag, we create our own list later on.
         */
        hModule = GetModuleHandle( NULL );

        if( !hModule )
            save_err_ptr_str( file );
    }
    else
    {
        char lpFileName[MAX_PATH];
        int i;

        /* MSDN says backslashes *must* be used instead of forward slashes. */
        for( i = 0 ; i < sizeof(lpFileName)-1 ; i++ )
        {
            if( !file[i] )
                break;
            else if( file[i] == '/' )
                lpFileName[i] = '\\';
            else
                lpFileName[i] = file[i];
        }
        lpFileName[i] = '\0';

        /* POSIX says the search path is implementation-defined.
         * LOAD_WITH_ALTERED_SEARCH_PATH is used to make it behave more closely
         * to UNIX's search paths (start with system folders instead of current
         * folder).
         */
        hModule = LoadLibraryEx( (LPSTR) lpFileName, NULL,
                                 LOAD_WITH_ALTERED_SEARCH_PATH );

        /* If the object was loaded with RTLD_GLOBAL, add it to list of global
         * objects, so that its symbols may be retrieved even if the handle for
         * the original program file is passed. POSIX says that if the same
         * file is specified in multiple invocations, and any of them are
         * RTLD_GLOBAL, even if any further invocations use RTLD_LOCAL, the
         * symbols will remain global.
         */
        if( !hModule )
            save_err_str( lpFileName );
        else if( (mode & RTLD_GLOBAL) )
            global_add( hModule );
    }

    /* Return to previous state of the error-mode bit flags. */
    SetErrorMode( uMode );

    return (void *) hModule;
}

int dlclose( void *handle )
{
    HMODULE hModule = (HMODULE) handle;
    BOOL ret;

    current_error = NULL;

    ret = FreeLibrary( hModule );

    /* If the object was loaded with RTLD_GLOBAL, remove it from list of global
     * objects.
     */
    if( ret )
        global_rem( hModule );
    else
        save_err_ptr_str( handle );

    /* dlclose's return value in inverted in relation to FreeLibrary's. */
    ret = !ret;

    return (int) ret;
}

void *dlsym( void *handle, const char *name )
{
    FARPROC symbol;

    current_error = NULL;

    symbol = GetProcAddress( (HMODULE)handle, name );

    if( symbol == NULL )
    {
        HMODULE hModule;

        /* If the handle for the original program file is passed, also search
         * in all globally loaded objects.
         */

        hModule = GetModuleHandle( NULL );

        if( hModule == handle )
        {
            global_object *pobject;

            for( pobject = &first_object; pobject ; pobject = pobject->next )
            {
                if( pobject->hModule )
                {
                    symbol = GetProcAddress( pobject->hModule, name );
                    if( symbol != NULL )
                        break;
                }
            }
        }

        CloseHandle( hModule );
    }

    if( symbol == NULL )
        save_err_str( name );

    return (void*) symbol;
}

char *dlerror( void )
{
    char *error_pointer = current_error;

    /* POSIX says that invoking dlerror( ) a second time, immediately following
     * a prior invocation, shall result in NULL being returned.
     */
    current_error = NULL;

    return error_pointer;
}

void *win_mmap(const char *file)
{
	HANDLE obj;
	HANDLE hFile;
	int fileSize;
	void *data = NULL;

	hFile = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}

	fileSize = GetFileSize( hFile, NULL);

	obj = CreateFileMapping( hFile, NULL, PAGE_READWRITE,
           0, fileSize, file);

	if (obj){
	    data = MapViewOfFile( obj, FILE_MAP_WRITE, 0, 0, 0);
	}

	CloseHandle(obj);
	CloseHandle(hFile);
	return data;
}


void win_munmap(void *mem)
{
	UnmapViewOfFile(mem);
}

void win_signal(int sigNo, CB_SIGNAL _func)
{
	(void) signal(sigNo, _func);
}

int win_mkfifo (const char *name, int size)
{
	HANDLE hPipe = CreateNamedPipe(
          name,
          PIPE_ACCESS_DUPLEX,
          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
          PIPE_UNLIMITED_INSTANCES,
          size,
          size,
          0,
          NULL);

      if (hPipe == INVALID_HANDLE_VALUE)
      {
          printf("CreateNamedPipe failed, GLE=%d.\n");
          return -1;
      }
	  return (int)hPipe;
}

void win_closefifo (int hPipe)
{
	 CloseHandle((HANDLE)hPipe);
}

int win_connectfifo (int hPipe)
{
	ConnectNamedPipe((HANDLE)hPipe, NULL);
	return 0;
}

int win_getpid(void)
{
	return GetCurrentProcessId();
}

#if 0
const char* getcwd(char *buff, int max)
{
	GetCurrentDirectory(max, buff);
	return buff;
}
#endif

int win_get_drives(int len, char *buff)
{
	return GetLogicalDriveStrings(len ,buff);
}

int get_invalid_handle()
{
	return (int)INVALID_HANDLE_VALUE;
}

int get_file_data_size (void)
{
	return sizeof(WIN32_FIND_DATA);
}

int win_find_first_file (const char *dir, void *file_data)
{
	return (int)FindFirstFile(dir, (WIN32_FIND_DATA *)file_data);
}

int win_find_next_file (int hFind, void *file_data)
{
	return (int)FindNextFile((HANDLE)hFind, (WIN32_FIND_DATA *)file_data);
}

int win_is_dir(void *file_data)
{
	return(((WIN32_FIND_DATA *)file_data)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

char* win_get_file_name(void *file_data)
{
	return ((WIN32_FIND_DATA *)file_data)->cFileName;
}

void win_close_find(int hFind)
{
	FindClose((HANDLE)hFind);
}

int win_get_exe_path (char *szPath, int len)
{
	int dwReturn = 0;

    if (len <= 0 || szPath == NULL)
		return dwReturn;

	dwReturn = GetModuleFileName( NULL, szPath, len);

	if (dwReturn > 0)
    {
        char *slash = strrchr(szPath, '\\');
        if (slash)
            *++slash = '\0';
    }
	return dwReturn;
}

void win_setenv(char *name, char *value)
{
	//SetEnvironmentVariable(name, value);
	_putenv_s(name, value);
}

const char* win_getenv(char *name)
{
	//return GetEnvironmentVariable(TEXT(name), buff, len);
	return getenv(name);
}

void win_messagebox(char* caption, char* text)
{
	if(!caption  || !text)
		return;
	MessageBox((HWND)0, text,caption, 0);
}


extern int guibuilder_main(int argc, const char** argv);

static inline bool is_space(char ch)
{
	return (ch == ' ' || ch == '\t');
}
char* splite_arg(char** pbegin)
{
	char end = 0;
	int  idx = 0;
	char* begin = *pbegin;
	char* arg_start;
	while(begin[0] && is_space(begin[0]))
		begin ++;

	if(begin[0] == '\"')
	{
		end = '\"';
		begin ++;
	}

	arg_start = begin;

	while(begin[0])
	{
		begin ++;
		if(end == 0)
		{
			if(is_space(begin[0]))
				break;
		}
		else if(begin[0] == end)
		{
			break;
		}
	}

	if(end)
	{
		*begin = 0;
		begin ++;
	}
	if(*begin)
	{
		*begin = 0;
		begin ++;
	}

	*pbegin = begin;
	return arg_start;
}

#define MAX_ARGS    16
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

	char szPath[MAX_PATH+1];
	int  argc;
	char *argv[MAX_ARGS];
	char* strtmp = (char*)lpCmdLine;
	GetModuleFileName( hInstance, szPath, MAX_PATH);

	argc = 1;
	argv[0] = szPath;

	for(; argc < MAX_ARGS; argc ++)
	{
		argv[argc] = splite_arg(&strtmp);
		if(!strtmp[0])
			break;
	}
	argc ++;

	try{
	/*	char szLogfile[1024];
		sprintf(szLogfile, "%s.log",szPath);
		freopen(szLogfile,"wt",stdout);
		freopen(szLogfile,"wt",stderr);
*/
		return guibuilder_main(argc, (const char**)argv);
	}catch(...)
	{
		return -1;
	}

}

///////////////////////////////////////////////
static void * load_dll(const char* fileName)
{
	return (void*)LoadLibrary((LPCSTR)fileName);
}
static int strnocaseeq(const char* str1, const char* str2)
{
	int i = 0;
	if(str1 == NULL && str2 == NULL)
		return 1;
	else if(str1 == NULL || str2 == NULL)
		return 0;


	while(str1[i] && str2[i] && tolower(str1[i]) == tolower(str2[i]))
		i ++;
	if(!str1[i] && !str2[i])
		return 1;
	return 0;
}

void* win_look_load_lib(const char* libname)
{
	char libFileName[256];
	char libPath[1024];
	int  len;
	void *handle;
	if(libname == NULL)
		return NULL;

	//libname is a path of library
	if(strchr(libname,'\\') || strchr(libname,'/'))
		return load_dll(libname);

	strncpy(libFileName,libname, sizeof(libFileName)-1);
	libFileName[sizeof(libFileName)-1] = 0;

	len = strlen(libFileName);
	if(!strnocaseeq(libFileName+len-3, "dll"))
		strcpy(libFileName,".dll");

	memset(libPath, 0, sizeof(libPath));
	//try load from current
	win_get_exe_path(libPath,sizeof(libPath)-1);
	strcat(libPath,libFileName);
	handle = load_dll(libPath);
	if(!handle)
		return NULL;

	//try load from c:\window\system32
	libPath[0] = 0;
	len = GetSystemDirectory((LPSTR)libPath, sizeof(libPath)-1);
	if(len > 0)
	{
		sprintf(libPath+len, "32\\%s", libFileName);
		if((handle = load_dll(libPath)))
			return handle;

		sprintf(libPath+len, "\\%s", libFileName);
		if((handle = load_dll(libPath)))
			return handle;
	}

	//try c:\windows
	len = GetWindowsDirectory((LPSTR)libPath, sizeof(libPath)-1);
	if(len > 0)
	{
		sprintf(libPath+len,"\\%s",libFileName);
		handle = load_dll(libPath);
		if(handle)
			return handle;
	}
	return NULL;
}

void  win_close_lib(void* handle)
{
	FreeLibrary((HMODULE)handle);
}

void* win_dlsym(void* handle, const char*name)
{
	return GetProcAddress((HMODULE)handle, name);
}

//fifo
#define BUFSIZE 1024*4

int win_open_fifo(const char* fifo)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if(fifo == NULL)
		return -1;

	hPipe = CreateNamedPipe(
		fifo,             // pipe name
		PIPE_ACCESS_DUPLEX,       // read/write access
			PIPE_TYPE_MESSAGE |       // message type pipe
			PIPE_READMODE_MESSAGE |   // message-read mode
			PIPE_WAIT,                // blocking mode
		PIPE_UNLIMITED_INSTANCES, // max. instances
		BUFSIZE,                  // output buffer size
		BUFSIZE,                  // input buffer size
		0,                        // client time-out
		NULL);                    // default security attribute

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	return (int)hPipe;
}

void win_close_fifo(int fd)
{
	CloseHandle((HANDLE)fd);
}

int win_wait_fifo(int fd)
{
	return (ConnectNamedPipe((HANDLE)fd,NULL));
}

int win_read_fifo(int fd, char* szBuff, int size)
{
	DWORD cbBytesRead = 0;
	BOOL bsuceffess = ReadFile(
		(HANDLE)fd,// handle to pipe
		szBuff,// buffer to receive data
		size,// size of buffer
		&cbBytesRead, // number of bytes read
		NULL);        // not overlapped I/O

	if(bsuceffess)
		return (int)cbBytesRead;
	return -1;
}

void win_show_wvfb(bool bshow)
{
	HWND hwnd = FindWindow((LPCSTR)"wvfb",NULL);//(LPCSTR)"Windows_xVFB");
	ShowWindow(hwnd, SW_RESTORE);
}

int win_open_client_fifo(const char* fifo)
{
	HANDLE hPipe;
	if(!fifo)
		return -1;
	hPipe = CreateFile(
		fifo,   // pipe name
		GENERIC_WRITE,  // read and write access
		0,              // no sharing
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe
		0,              // default attributes
		NULL);          // no template file

	return (int)hPipe;
}

int win_write_fifo(int fifo, const char* szBuff, int size)
{
	DWORD cbWritten;
	if(WriteFile(
		(HANDLE)fifo,                  // pipe handle
		szBuff,             // message
		size,              // message length
		&cbWritten,             // bytes written
		NULL))
		return -1;
	return (int)cbWritten;
}

int win_process_running(int pid)
{
	DWORD exitCode;
	int   is_running = 0;
	HANDLE  hProcess = OpenProcess(READ_CONTROL|PROCESS_QUERY_INFORMATION,
				FALSE,
				(DWORD)pid);
	if(hProcess == NULL)
		return 0;

	if(GetExitCodeProcess(hProcess, &exitCode))
	{
		is_running = exitCode == STILL_ACTIVE?1:0;
	}

	CloseHandle(hProcess);

	return is_running;
}


const char* get_cur_user_name(char* puser_name, int max)
{
	puser_name[0] = 0;
	DWORD lpmax = max;

	if(GetUserName((LPTSTR)puser_name, &lpmax)){
		max = (int)lpmax;
		return puser_name;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////
//wait thread
void * win_get_current_thread()
{
	return (void*)GetCurrentThread();
}

void win_wait_thread(void* thread)
{
	WaitForSingleObject((HANDLE)thread, INFINITE);
}


#endif  //end of WIN32

#ifdef _MSTUDIO_OFFICIAL_RELEASE

struct FMSOFT_AUTH_INFO buff;

int read_auth_key(char *fid)
{
	SENSE4_CONTEXT ctx = {0};
	SENSE4_CONTEXT *pctx = NULL;
	unsigned long size = 0;
	unsigned long ret = 0;
	unsigned long FileSize=0;
	unsigned long WByte=0;

	S4Enum(pctx, &size);
	if (size == 0)
	{
		printf("EliteIV not found!\n");
		return MSTUDIO_ERR_NOAUTH;
	}

	pctx = (SENSE4_CONTEXT *)malloc(size);
	if (pctx == NULL)
	{
		printf("Not enough memory!\n");
		return MSTUDIO_ERR_NOAUTH;
	}

	ret = S4Enum(pctx, &size);
	if (ret != S4_SUCCESS)
	{
		printf("Enumerate EliteIV error!\n");
		free(pctx);
		return MSTUDIO_ERR_NOAUTH;
	}

	memcpy(&ctx, pctx, sizeof(SENSE4_CONTEXT));
	free(pctx);
	pctx = NULL;

	ret = S4Open(&ctx);
	if (ret != S4_SUCCESS)
	{
		printf("Open EliteIV failed!\n");
		return MSTUDIO_ERR_NOAUTH;
	}

	ret = S4ChangeDir(&ctx, "\\");
	if (ret != S4_SUCCESS)
	{
		printf("No root directory found!\n");
		S4Close(&ctx);
		return MSTUDIO_ERR_NOAUTH;
	}

	//FIXME: "12345678" is the UserPin for executing files
	ret = S4VerifyPin(&ctx, (BYTE*)"12345678", 8, S4_USER_PIN);
	if (ret != S4_SUCCESS)
	{
		printf("Verify user PIN failed!\n");
		S4Close(&ctx);
		return MSTUDIO_ERR_NOAUTH;
	}

	ret = S4Execute(&ctx, fid, 0, 0, &buff, sizeof(struct FMSOFT_AUTH_INFO), &size);
	if (ret != S4_SUCCESS)
	{
		printf("Execute EliteIV exe failed!\n");
		S4Close(&ctx);
		return MSTUDIO_ERR_INVALIDAUTH;
	}

	S4Close(&ctx);

	return MSTUDIO_ERR_VALIDAUTH;
}

int fmsoftCheckDogValidity(int *remainDay)
{
    //TODO: check softdog device
	int ret;
	struct tm *ptm;

	char file_id[] = "ef01";

	memset(&buff, 0, sizeof(struct FMSOFT_AUTH_INFO));

	//FIXME: ef01 is the fileid inside dongle
	ret = read_auth_key(file_id);

	if (ret != MSTUDIO_ERR_VALIDAUTH)
	{
		return ret;
	}

//	fprintf(stderr, "sign is %s, id is %d, validDate is %d, curDate is %d, expiredDate is %d\n",
//						buff.sign, buff.clientID, buff.validDate, buff.curDate, buff.expiredDate);

	if (buff.curDate >= buff.expiredDate /*|| buff.curDate <= buff.validDate*/)
	{
		return MSTUDIO_ERR_DUETOAUTH;
	}
	else
	{
		*remainDay = difftime(buff.expiredDate, buff.curDate) / 86400;
	}
//	fprintf(stderr, "remainDay is %d\n", *remainDay);
	return ret;
}
#endif


