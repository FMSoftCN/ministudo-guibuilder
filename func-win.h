#ifndef _FUNC_WIN_H
#define _FUNC_WIN_H

#ifdef WIN32

/* POSIX says these are implementation-defined.
 * To simplify use with Windows API, we treat them the same way.
 */

#define RTLD_LAZY   0
#define RTLD_NOW    0

#define RTLD_GLOBAL (1 << 1)
#define RTLD_LOCAL  (1 << 2)

/* These two were added in The Open Group Base Specifications Issue 6.
 * Note: All other RTLD_* flags in any dlfcn.h are not standard compliant.
 */

#define RTLD_DEFAULT    0
#define RTLD_NEXT       0

typedef void (* CB_SIGNAL)(int);

void *dlopen ( const char *file, int mode );
int   dlclose( void *handle );
void *dlsym  ( void *handle, const char *name );
char *dlerror( void );

void * win_mmap(const char *file);

void win_munmap(void *mem);

void win_signal(int sigNo, CB_SIGNAL _func);

int win_connectfifo (int hPipe);

int win_mkfifo (const char *name, int size);

void win_closefifo (int hPipe);

int win_getpid(void);

//const char* getcwd(char *buff, int max);
/*
static inline void show_cur_dir(const char* file, int line)
{
	char strcwd[1024];
	printf("current dir:%s:%d:%s\n",file, line,getcwd(strcwd,1024));
}
#define SHOW_CUR_DIR  show_cur_dir(__FILE__, __LINE__)
*/

int win_get_drives(int len, char *buff);

int get_invalid_handle(void);

int get_file_data_size (void);

int win_find_first_file (const char *dir, void *file_data);

int win_find_next_file (int hFind, void *file_data);

int win_is_dir(void *file_data);

char* win_get_file_name(void *file_data);

void win_close_find(int hFind);

int win_get_exe_path (char *szPath, int len);

void win_setenv(char *name, char *value);

const char* win_getenv(char *name);

void win_messagebox(char* caption, char* text);


void* win_look_load_lib(const char* libname);
void  win_close_lib(void* handle);
void* win_dlsym(void* handle, const char*name);


//fifo
int win_open_fifo(const char* fifo);
void win_close_fifo(int fd);
int win_wait_fifo(int fd);
int win_read_fifo(int fd, char* szBuff, int size);
void win_show_wvfb(bool bshow);
int win_open_client_fifo(const char* fifo);
int win_write_fifo(int fifo, const char* szBuff, int size);

int win_process_running(int pid);


const char* get_cur_user_name(char* puser_name, int max);
#endif //#fidef WIN32

#ifdef _MSTUDIO_OFFICIAL_RELEASE
#ifdef WIN32
typedef __int32 _INT32;
typedef unsigned __int32 _UINT32;
struct FMSOFT_AUTH_INFO{
	char sign[8];
	_INT32  clientID;
	_UINT32 validDate;
	_UINT32 curDate;
	_UINT32 expiredDate;
};
#else
struct FMSOFT_AUTH_INFO{
	char sign[8];
	int  clientID;
	time_t validDate;
	time_t curDate;
	time_t expiredDate;
};
#endif

#define MSTUDIO_ERR_NOAUTH      0
//dueto softdog
#define MSTUDIO_ERR_DUETOAUTH   1
//invalid softdog
#define MSTUDIO_ERR_INVALIDAUTH 2
//valid softdog
#define MSTUDIO_ERR_VALIDAUTH   3

int fmsoftCheckDogValidity(int *remainDay);
#endif


#ifdef WIN32
void * win_get_current_thread();
void win_wait_thread(void* thread);
#endif //WIN32

#endif  //end of _FUNC_WIN_H
