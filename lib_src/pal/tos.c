/*
**  @file $RCSfile: tos.c,v $
**  general description of this module
**  $Id: tos.c,v 1.6 2009/01/23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009/01/23 09:35:03 $
**  @version $Revision: 1.6 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tstring.h"
#include "pal/ttime.h"
#include "pal/tstdio.h"
#include "pal/terrno.h"
#include "pal/tsocket.h"

#include <signal.h>

#define TSIGNAL_BASIC
#include "pal/tsignal.h"
#undef TSIGNAL_BASIC

#include <assert.h>

static int gs_iOsType	=	0;

#ifdef WIN32

static HANDLE gs_hNtDll	=	NULL;
static HANDLE gs_hKernel32	=	NULL;


typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[229];
    PVOID Reserved3[59];
    ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef   struct   _OBJECT_ATTRIBUTES   {   
	DWORD cbSize;   /*   =   0x18    */
	DWORD dwReserved01;   
	DWORD dwReserved02;   
	BOOL bInheritHandle;   
	DWORD dwReserved03;   
	DWORD dwReserved04;   
}OBJECT_ATTRIBUTES,   *POBJECT_ATTRIBUTES;   

typedef   struct   _CLIENT_ID     
{   
	HANDLE   UniqueProcess;   
	HANDLE   UniqueThread;   
}   CLIENT_ID,   *PCLIENT_ID;   

typedef enum _PROCESSINFOCLASS
{
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    ProcessImageFileName,
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,
    ProcessDebugFlags,
    ProcessHandleTracing,
    ProcessIoPriority,
    ProcessExecuteFlags,
    ProcessTlsInformation,
    ProcessCookie,
    ProcessImageInformation,
    ProcessCycleTime,
    ProcessPagePriority,
    ProcessInstrumentationCallback,
    MaxProcessInfoClass
} PROCESSINFOCLASS;

typedef enum _THREADINFOCLASS
{
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    ThreadSwitchLegacyState,
    ThreadIsTerminated,
    ThreadLastSystemCall,
    ThreadIoPriority,
    ThreadCycleTime,
    ThreadPagePriority,
    ThreadActualBasePriority,
    MaxThreadInfoClass
} THREADINFOCLASS;


typedef int (WINAPI *PFNNTQUERYINFORMATIONPROCESS) ( HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG );
typedef DWORD (WINAPI *PFNGETPROCESSID)( HANDLE );
typedef HANDLE (WINAPI *PFNOPENTHREAD)(DWORD, BOOL, DWORD);
typedef int (WINAPI *PFNNTOPENTHREAD)(HANDLE*, DWORD, void*, void*);

int traise(int a_iSig);

static PFNNTQUERYINFORMATIONPROCESS gs_pfnNtQueryInfoProc	=	NULL;
static PFNGETPROCESSID gs_pfnGetProcId	=	NULL;
static PFNOPENTHREAD gs_pfnOpenThread	=	NULL;
static PFNNTOPENTHREAD gs_pfnNtOpenThread	=	NULL;

#pragma warning(disable:4996)

const char* W32_VAR_USERNAME	=	"USERNAME";

static gid_t s_gid		=	42;
static gid_t s_egid		=	42;

static uid_t s_uid		=	42;
static uid_t s_euid		=	42;

static pid_t s_grpid	=	0;


FARPROC tos_get_ntdll_proc_i(const char* pszName)
{
	if( !gs_hNtDll )
		return NULL;

	return GetProcAddress(gs_hNtDll, pszName);
}

FARPROC tos_get_kernel_proc_i(const char* pszName)
{
	if( !gs_hKernel32 )
		return NULL;

	return GetProcAddress(gs_hKernel32, pszName);
}

int tos_load_all_dll_i(void)
{
	if( !gs_hNtDll )
	{
		gs_hNtDll	=	LoadLibrary("ntdll.dll");
	}

	if( !gs_hKernel32 )
	{
		gs_hKernel32	=	LoadLibrary("kernel32.dll");
	}

	if( !gs_hNtDll || !gs_hKernel32 )
		return -1;

	return 0;
}

int tos_load_ntdll_func_i(void)
{
	if( !gs_hNtDll )
		return -1;

	if( !gs_pfnNtQueryInfoProc )
		gs_pfnNtQueryInfoProc	=	(PFNNTQUERYINFORMATIONPROCESS)GetProcAddress(gs_hNtDll, "NtQueryInformationProcess");

	if( !gs_pfnNtOpenThread )
		gs_pfnNtOpenThread		=	(PFNNTOPENTHREAD)GetProcAddress(gs_hNtDll, "NtOpenThread");

	return 0;
}

int tos_load_kernel32_func_i(void)
{
	if( !gs_hKernel32 )
		return -1;

	if(!gs_pfnGetProcId )
		gs_pfnGetProcId	=	(PFNGETPROCESSID)GetProcAddress(gs_hKernel32, "GetProcessId");

	if( !gs_pfnOpenThread )
		gs_pfnOpenThread	=	(PFNOPENTHREAD)GetProcAddress(gs_hKernel32, "OpenThread");

	return 0;
}

int tos_get_process_id_i(HANDLE hProc)
{
	PROCESS_BASIC_INFORMATION stProcInfo;
	unsigned long ulReturn;

	if( gs_pfnGetProcId )
		return (int) (*gs_pfnGetProcId)(hProc);

	if( !gs_pfnNtQueryInfoProc )
		return -1;

	memset(&stProcInfo, 0, sizeof(stProcInfo));

	if( 0!=(*gs_pfnNtQueryInfoProc)(hProc, 0, &stProcInfo, sizeof(stProcInfo), &ulReturn) )
		return -1;
	
	return (int) stProcInfo.UniqueProcessId;
	
}

HANDLE tos_open_thread_i(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId)
{
	HANDLE hThread;
	OBJECT_ATTRIBUTES stAttr;
	CLIENT_ID stClientId;


	if( gs_pfnOpenThread )
		return (*gs_pfnOpenThread)(dwDesiredAccess, bInheritHandle, dwThreadId);

	if( !gs_pfnNtOpenThread )
		return NULL;

	memset(&stAttr, 0, sizeof(stAttr));

	stAttr.cbSize	=	sizeof(stAttr);

	stAttr.bInheritHandle	=	bInheritHandle;

	stClientId.UniqueProcess	=	NULL;
	stClientId.UniqueThread		=	(HANDLE)dwThreadId;

	if( 0!=(*gs_pfnNtOpenThread)(&hThread, dwDesiredAccess, &stAttr, &stClientId) )
		return NULL;

	return hThread;
}

#ifdef WINNT

void* tos_get_sd(void)
{
	return NULL;
}

void tos_free_sd(void* pvSd)
{
	if( !pvSd )
		return;

	LocalFree(pvSd);

	return;
}

#endif

void usleep(unsigned long usec)
{
	SleepEx(usec/1000, TRUE); 
}

unsigned int sleep(unsigned int seconds)
{
	return SleepEx(seconds*1000, TRUE);
}

void *alloca(size_t size)
{
	return _alloca(size);
}

off64_t lseek64(int fd, off64_t offset, int whence)
{
	return _lseeki64(fd, offset, whence);
}

int pipe(int filedes[2])
{
	return _pipe(filedes, 1024, O_BINARY);
}

FILE *popen(const char *command, const char *type)
{
	return _popen(command, type);
}

int pclose(FILE *stream)
{
	return _pclose(stream);
}


uid_t getuid(void)
{
	return s_uid;
}

uid_t geteuid(void)
{
	return s_euid;
}

int setuid(uid_t uid)
{
	if( -1==uid )
	{
		errno	=	EINVAL;
		return -1;
	}

	s_uid	=	uid;

	return 0;
}

int seteuid(uid_t euid)
{
	if( -1==euid )
	{
		errno	=	EINVAL;
		return -1;
	}

	s_euid	=	euid;

	return 0;
}

gid_t getgid(void)
{
	return s_gid;
}

gid_t getegid(void)
{
	return s_egid;
}

int setgid(gid_t gid)
{
	if( -1==gid )
	{
		errno	=	EINVAL;
		return -1;
	}

	s_gid	=	gid;

	return 0;
}

int setegid(gid_t egid)
{
	if( -1==egid )
	{
		errno	=	EINVAL;
		return -1;
	}

	s_egid	=	egid;

	return 0;
}

int setreuid(uid_t ruid, uid_t euid)
{
	if( -1!=ruid )
		s_uid	=	ruid;

	if( -1!=euid )
		s_euid	=	euid;

	return 0;
}

int setregid(gid_t rgid, gid_t egid)
{
	if( -1!=rgid )
		s_gid	=	rgid;

	if( -1!=egid )
		s_egid	=	egid;
	
	return 0;
}


pid_t getpgrp(void)
{
	if( s_grpid )
		return s_grpid;

	s_grpid	=	getpid();

	return s_grpid;
}


int setpgrp(void)
{
	s_grpid	=	getpid();

	return 0;
}


pid_t getpgid(pid_t pid)
{
	if( pid==getpid() )
	{
		getpgrp();
		return 0;
	}
	
	errno	=	ENOTSUP;

	return -1;
}


int setpgid(pid_t pid, pid_t pgid)
{
	if( pid==getpid() )
	{
		s_grpid	=	pgid;

		return 0;
	}
	
	errno	=	ENOTSUP;

	return -1;
}


int getgroups(int size, gid_t list[])
{
	return 0;
}

int setgroups(size_t size, const gid_t *list)
{
	errno	=	ENOTSUP;

	return -1;
}

char *getlogin(void)
{
	return getenv(W32_VAR_USERNAME);
}

int getlogin_r(char *buf, size_t bufsize)
{
	const char* pszName;
	int iLen;

	pszName	=	getlogin();

	if( !pszName )
	{
		errno	=	ENOSYS;
		return -1;
	}

	iLen	=	strlen(pszName);

	if( (int)bufsize<iLen+1 )
	{
		errno	=	ENOMEM;
		return -1;
	}

	memcpy(buf, pszName, iLen+1);

	return 0;
}

char *cuserid(char *string)
{
	if( getlogin_r(string, L_cuserid)<0 )
		return NULL;
	else
		return string;
}


pid_t tcgetpgrp(int fd)
{
	return getpid();
}


int tcsetpgrp(int fd, pid_t pgrp)
{
	s_grpid	=	pgrp;

	return 0;
}


pid_t setsid(void)
{
	return 0;
}

int ftruncate(int fd, off_t length)
{
	HANDLE h;
	DWORD dwRet;

	h	=	(HANDLE) _get_osfhandle(fd);

	if( INVALID_HANDLE_VALUE == h )
		return -1;

	dwRet	=	SetFilePointer( h, length, NULL, FILE_BEGIN);

	if( 0xFFFFFFFF==dwRet && NO_ERROR!=GetLastError() )
		return -1;

	if( SetEndOfFile( h ) )
		return 0;

	errno	=	EPERM;

	return -1;
}

int truncate(const char *path, off_t length)
{
	int fd;
	int iRet;

	fd	=	open(path, O_RDWR);

	if( -1==fd )
		return -1;

	iRet	=	ftruncate( fd, length );

	close(fd);

	return iRet;
}

#include  <tlhelp32.h>

pid_t getppid(void)
{
	HANDLE hSnapshot;
	PROCESSENTRY32 stEntry;
	DWORD dwPid;
	DWORD dwPpid=-1;

	dwPid	=	GetCurrentProcessId();

	hSnapshot	=	CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0);

	if( !hSnapshot )
	{
		errno	=	EPERM;
		return -1;
	}

	stEntry.dwSize	=	sizeof(stEntry);

	if( !Process32First(hSnapshot, &stEntry) )
	{
		CloseHandle(hSnapshot);
		errno	=	ENOMEM;
		return -1;
	}

	if( stEntry.th32ProcessID==dwPid )
	{
		dwPpid	=	stEntry.th32ParentProcessID;
	}

	while( -1==dwPpid && Process32Next(hSnapshot, &stEntry) )
	{
		if( dwPid==stEntry.th32ProcessID )
		{
			dwPpid	=	stEntry.th32ParentProcessID;
			break;
		}
	}

	CloseHandle( hSnapshot );

	return (pid_t)dwPpid;
}

long sysconf(int name)
{
	long val;

	switch (name)
	{
	case _SC_ARG_MAX:	val=65535; break;
	case _SC_CHILD_MAX:	val=CHILD_MAX; break;
	case _SC_CLK_TCK:	val=CLOCKS_PER_SEC; break;
	case _SC_NGROUPS_MAX:	val=NGROUPS_MAX; break;
	case _SC_OPEN_MAX:	val=255; break;
	case _SC_JOB_CONTROL:	val=-1; break;
	case _SC_SAVED_IDS:	val=-1; break;
	case _SC_STREAM_MAX:	val=_POSIX_STREAM_MAX; break;
	case _SC_TZNAME_MAX:	val=TZNAME_MAX; break;
	case _SC_VERSION:	val=_POSIX_VERSION; break;

	default:
		errno = EINVAL;
		return -1;
	}

	return val;
}

long fpathconf(int filedes, int name)
{
	switch (name)
	{
	case _PC_LINK_MAX:		return LINK_MAX;
	case _PC_MAX_CANON:		return MAX_CANON;
	case _PC_MAX_INPUT:		return MAX_INPUT;
	case _PC_NAME_MAX:		return NAME_MAX;
	case _PC_PATH_MAX:		return PATH_MAX;
	case _PC_PIPE_BUF:		return PIPE_BUF;
	case _PC_CHOWN_RESTRICTED:	return _POSIX_CHOWN_RESTRICTED;
	case _PC_NO_TRUNC:		return _POSIX_NO_TRUNC;
	case _PC_VDISABLE:		return _POSIX_VDISABLE;

	default:
		errno = EINVAL;
		return -1;
	}
}


long pathconf(const char *path, int name)
{
	switch (name)
	{
	case _PC_LINK_MAX:		return LINK_MAX;
	case _PC_MAX_CANON:		return MAX_CANON;
	case _PC_MAX_INPUT:		return MAX_INPUT;
	case _PC_NAME_MAX:		return NAME_MAX;
	case _PC_PATH_MAX:		return PATH_MAX;
	case _PC_PIPE_BUF:		return PIPE_BUF;
	case _PC_CHOWN_RESTRICTED:	return _POSIX_CHOWN_RESTRICTED;
	case _PC_NO_TRUNC:		return _POSIX_NO_TRUNC;
	case _PC_VDISABLE:		return _POSIX_VDISABLE;

	default:
		errno = EINVAL;
		return -1;
	}
}

char* ttyname(int fd)
{
	switch (fd)
	{
	case STDIN_FILENO:
		return "CONIN$";

	case STDOUT_FILENO:
	case STDERR_FILENO:
		return "CONOUT$";

	default:
		errno	=	ENOTTY;
		return NULL;
	}
}

int ttyname_r(int fd, char* buf, size_t buflen)
{
	const char* name;

	switch (fd)
	{
	case STDIN_FILENO:
		name	=	"CONIN$";
		break;

	case STDOUT_FILENO:
	case STDERR_FILENO:
		name	=	"CONOUT$";
		break;

	default:
		errno	=	ENOTTY;
		return -1;
	}

	if( buflen<strlen(name)+1 )
	{
		errno	=	ERANGE;
		return -1;
	}

	strcpy(buf, name);

	return 0;
}

char *basename (const char *fname)
{
	const char *base = fname;

	if (fname && *fname)
	{
		if (fname[1] == ':')
		{
			fname += 2;
			base = fname;
		}

		while (*fname)
		{
			if (*fname == '\\' || *fname == '/')
				base = fname + 1;
			fname++;
		}
	}

	return (char*)base;
}


char *dirname (const char *fname)
{
	const char *p  = fname;
	const char *slash = NULL;

	if (fname)
	{
		size_t dirlen;
		char * dirpart;

		if (*fname && fname[1] == ':')
		{
			slash = fname + 1;
			p += 2;
		}

		/* Find the rightmost slash.  */
		while (*p)
		{
			if (*p == '/' || *p == '\\')
				slash = p;
			p++;
		}

		if (slash == NULL)
		{
			fname = ".";
			dirlen = 1;
		}
		else
		{
			/* Remove any trailing slashes.  */
			while (slash > fname && (slash[-1] == '/' || slash[-1] == '\\'))
				slash--;

			/* How long is the directory we will return?  */
			dirlen = slash - fname + (slash == fname || slash[-1] == ':');
			if (*slash == ':' && dirlen == 1)
				dirlen += 2;
		}

		dirpart = (char *)malloc (dirlen + 1);
		if (dirpart != NULL)
		{
			strncpy (dirpart, fname, dirlen);
			if (slash && *slash == ':' && dirlen == 3)
				dirpart[2] = '.';	/* for "x:foo" return "x:." */
			dirpart[dirlen] = '\0';
		}

		return dirpart;
	}

	return NULL;
}


int getpagesize(void)
{
	SYSTEM_INFO sys;

	GetSystemInfo(&sys);

	return (int) sys.dwPageSize;
}

int readlink(const char *path, char *buf, size_t bufsiz)
{
	char szDeref[_MAX_PATH + 1];
	int iLen;

	if(strlen(path) > _MAX_PATH)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(szDeref, path);

	if (win_deref(szDeref) == -1)
		return -1;

	if ((iLen = (int)strlen(szDeref)) > (int)bufsiz )
	{
		errno = ENAMETOOLONG;
		return -1;
	}
  
	strcpy(buf, szDeref);

	errno = 0;

	return iLen;
}

int link(const char *oldpath, const char *newpath)
{
	BOOL bRet;

	bRet	=	shortcut_create(oldpath, newpath);

	if( !bRet )
		terrno_set_from_winerr(GetLastError());

	return bRet ? 0 : -1;
}

int symlink(const char *oldpath, const char *newpath)
{
	BOOL bRet;

	/* CreateShortcut sets errno */
	bRet = shortcut_create(oldpath, newpath);

	if( !bRet )
		terrno_set_from_winerr(GetLastError());

	return bRet ? 0 : -1;
}

char *realpath(const char *path, char *resolved_path)
{
	char *pszRet;

	pszRet = _fullpath(resolved_path, path, MAX_PATH);

	if( NULL==pszRet )
		terrno_set_from_winerr(GetLastError());

	return pszRet;
}

unsigned int alarm(unsigned int seconds)
{
	struct itimerval val;
	struct itimerval old;

	memset(&val, 0, sizeof(val));
	
	val.it_value.tv_sec	=	seconds;

	if( setitimer(ITIMER_REAL, &val, &old)<0 )
		return 0;

	return old.it_value.tv_sec;
}

#define TOS_MATCH_PID(pat, o)	( (0==(pid_t)pat) || ((pid_t)pat==(pid_t)o) )

int tos_get_pidlist_i(int a_iMax, pid_t* a_piPidList, int a_iPid, int a_iPpid)
{
	HANDLE hSnapshot=NULL;
	PROCESSENTRY32 stEntry;
	int iProc=0;
	pid_t iCurPid=0;

	iCurPid		=	getpid();

	hSnapshot	=	CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0);

	if( !hSnapshot )
	{
		errno	=	EPERM;
		return -1;
	}

	stEntry.dwSize	=	sizeof(stEntry);

	if( !Process32First(hSnapshot, &stEntry) )
	{
		CloseHandle(hSnapshot);
		errno	=	ENOMEM;
		return -1;
	}

	if( iProc<a_iMax && iCurPid!=(pid_t)stEntry.th32ProcessID &&
		TOS_MATCH_PID(a_iPid, stEntry.th32ProcessID) &&
		TOS_MATCH_PID(a_iPid, stEntry.th32ParentProcessID) )
	{
		a_piPidList[iProc++]	=	(pid_t) stEntry.th32ProcessID;
	}

	while( iProc<a_iMax && Process32Next(hSnapshot, &stEntry) )
	{
		if( iCurPid!=(pid_t)stEntry.th32ProcessID &&
			TOS_MATCH_PID(a_iPid, stEntry.th32ProcessID) &&
			TOS_MATCH_PID(a_iPpid, stEntry.th32ParentProcessID) )
		{
			a_piPidList[iProc++]	=	(pid_t) stEntry.th32ProcessID;
		}
	}

	CloseHandle( hSnapshot );

	return iProc;
}

int tos_open_proclist_i(int a_iCount, pid_t* piPidList, HANDLE* phProcList)
{
	int i;
	int iProc=0;
	HANDLE hProc=NULL;

	for(i=0; i<a_iCount; i++)
	{
		hProc	=	OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, (DWORD)piPidList[i] );

		if( hProc )
		{
			phProcList[iProc]	=	hProc;
			iProc++;
		}
	}

	return iProc;
}

void tos_close_proclist_i(int a_iCount, HANDLE* phProcList)
{
	int i;

	for(i=0; i<a_iCount; i++)
	{
		if( phProcList[i] )
		{
			CloseHandle(phProcList[i]);
			phProcList[i]	=	NULL;
		}
	}
}

static void tos_on_signal_i(int a_iSig)
{
	signal(a_iSig, tos_on_signal_i);

	switch(a_iSig)
	{
	case SIGABRT:
		a_iSig	=	TSIGABRT;
		break;
	case SIGFPE:
		a_iSig	=	TSIGFPE;
		break;
	case SIGILL:
		a_iSig	=	TSIGILL;
		break;
	case SIGINT:
		a_iSig	=	TSIGINT;
		break;
	case SIGSEGV:
		a_iSig	=	TSIGSEGV;
		break;
	case SIGTERM:
		a_iSig	=	TSIGTERM;
		break;
	case SIGBREAK:
		a_iSig	=	TSIGQUIT;
		break;
	default:
		return;
	}

	traise(a_iSig);
}

static  BOOL WINAPI tos_ctrl_handler_i(DWORD dwType)
{
	int iSig=0;

	switch( dwType )
	{
	case CTRL_C_EVENT:
		iSig	=	TSIGINT;
		break;
	case CTRL_BREAK_EVENT:
		iSig	=	TSIGQUIT;
		break;
	case CTRL_CLOSE_EVENT:
		iSig	=	TSIGQUIT;
		break;
	case CTRL_LOGOFF_EVENT:
		iSig	=	TSIGPWR;
		break;
	case CTRL_SHUTDOWN_EVENT:
		iSig	=	TSIGPWR;
		break;
	default:
		break;
	}

	if( iSig )
		traise(iSig);

	return TRUE;
}

#else	/* WIN32 */

#endif /* WIN32 */

int tos_get_ostype(void)
{
#ifdef WIN32
	OSVERSIONINFO stVer;

	/* Get Windows version */

	if( !gs_iOsType )
		return gs_iOsType;

	stVer.dwOSVersionInfoSize = sizeof(stVer);
	GetVersionEx(&stVer);

	if( stVer.dwPlatformId == VER_PLATFORM_WIN32_NT )
		gs_iOsType	=	TOS_WINNT;
	else
		gs_iOsType	=	TOS_WIN9X;

	return gs_iOsType;

#else /* WIN32 */

	if( !gs_iOsType )
		return gs_iOsType;

	gs_iOsType	=	TOS_LINUX;

	return gs_iOsType;
#endif /* WIN32 */
}

int tos_is_linux(void)
{
#ifdef WIN32
	return 0;
#else /* WIN32 */
	return 1;
#endif /* WIN32 */
}

int tos_is_winnt(void)
{
#ifdef WIN32
	return 0;
#else /* WIN32 */

	if( tos_get_ostype()==TOS_WINNT )
		return 1;
	else
		return 0;

#endif /* WIN32 */
}

int tos_usleep(int a_iUs)
{
#ifdef WIN32
	if( INFINITE==(DWORD)a_iUs )
		return SleepEx(INFINITE, 0);
	else
		return SleepEx((DWORD)(a_iUs/1000), 0);
#else
	struct timespec stReq;
	
	stReq.tv_sec	=	a_iUs/1000000;
	stReq.tv_nsec	=	(a_iUs % 1000000)*1000;

	return nanosleep(&stReq, NULL);
#endif /* WIN32 */
}

void tos_make_daemon(const char* a_pszRoot)
{
#ifdef WIN32
	return;
#else
	pid_t pid;

	pid = fork() ;

	if (pid != 0)
		exit(0);

	setsid();

	signal(SIGINT,  SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);

	pid = fork() ;

	if (pid != 0)
		exit(0);

	if( a_pszRoot )
	{
		if( chdir(a_pszRoot) )
			exit(2);
	}

	umask(0);
#endif
}

void tos_ignore_pipe(void)
{
#ifdef WIN32
	return;
#else
        struct sigaction sa;

        sa.sa_handler = SIG_IGN;
        sa.sa_flags = 0;

        sigemptyset(&sa.sa_mask);
        sigaction(SIGPIPE,&sa,NULL);
#endif
}

void tos_file_to_macro(char* a_pszMacro, int a_iMacro, const char* a_pszFile)
{
	const char *pszTitle;
	int i;

	assert( a_pszMacro && a_pszFile && a_iMacro>0 );
	
	pszTitle	=	strrchr(a_pszFile, TOS_DIRSEP);

	if( pszTitle )
		pszTitle++;
	else
		pszTitle=	a_pszFile;

	i	=	0;

	while(i<a_iMacro && i<(int)strlen(pszTitle) )
	{
		if( TOS_DIRDOT==pszTitle[i] )
			a_pszMacro[i]	=	TOS_DIRBAR;
		else
			a_pszMacro[i]	=	(char)toupper(pszTitle[i]);

		i++;
	}

	a_pszMacro[i]	=	'\0';
}

int tos_get_pagesize(void)
{
#ifdef WIN32
	SYSTEM_INFO sys;

	GetSystemInfo(&sys);

	return (int) sys.dwPageSize;
#else
	return getpagesize();
#endif /* WIN32 */
}

int tos_time(char* a_pszBuff, int a_iBuff)
{
#ifdef WIN32
	SYSTEMTIME stSys;

	if( !a_pszBuff || a_iBuff<=0 )
		return -1;
	
	GetLocalTime(&stSys);

    snprintf(a_pszBuff, a_iBuff, "%04d-%02d-%02d %02d:%02d:%02d.%.6d",
            stSys.wYear, stSys.wMonth, stSys.wDay, stSys.wHour, 
			stSys.wMinute, stSys.wSecond, stSys.wMilliseconds*1000);

	return 0;
#else
	struct timeval stTv;
	struct tm stTm;

	if( !a_pszBuff || a_iBuff<=0 )
		return -1;

	gettimeofday(&stTv, NULL);
	localtime_r(&stTv.tv_sec, &stTm);

 	snprintf(a_pszBuff, a_iBuff, "%04d-%02d-%02d %02d:%02d:%02d.%.6d",
			stTm.tm_year+1900, stTm.tm_mon+1, stTm.tm_mday,
			stTm.tm_hour, stTm.tm_min, stTm.tm_sec, (int)stTv.tv_usec);

	return 0;

#endif /* WIN32 */
}


int tos_init(void* a_pvArg)
{
#ifdef WIN32
	signal(SIGABRT, tos_on_signal_i);
	signal(SIGFPE, tos_on_signal_i);
	signal(SIGILL, tos_on_signal_i);
	signal(SIGINT, tos_on_signal_i);
	signal(SIGSEGV, tos_on_signal_i);
	signal(SIGTERM, tos_on_signal_i);
	signal(SIGBREAK, tos_on_signal_i);

	tos_load_all_dll_i();
	tos_load_ntdll_func_i();
	tos_load_kernel32_func_i();

	return tsocket_init(2,0);

#else /* WIN32 */
	return 0;
#endif /* WIN32 */
}

int tos_fini(void)
{
#ifdef WIN32
	tsocket_fini();
	return 0;
#else /* WIN32 */
	return 0;
#endif /* WIN32 */
}


int tos_write_pid(const char* a_pszPidFile)
{
	FILE* fp;

	fp	=	fopen(a_pszPidFile, "wt");

	if( !fp )
		return -1;

	fprintf(fp, "%d\n", (int)getpid());

	fflush(fp);

	fclose(fp);

	return 0;
}


pid_t tos_read_pid(const char* a_pszPidFile)
{
	FILE* fp;
	int iPid;

	iPid	=	-1;

	fp	=	fopen(a_pszPidFile, "rt");

	if( !fp )
		return -1;

	fscanf(fp, "%d", &iPid);

	fclose(fp);

	return (pid_t)iPid;
}

int tos_get_exe(pid_t a_pid, char* a_pszPath, int a_iPath)
{
#ifdef WIN32

	MODULEENTRY32 stEntry;
	HANDLE hSnapshot;
	int iRet;

	hSnapshot	=	CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, (DWORD)a_pid);

	if( INVALID_HANDLE_VALUE==hSnapshot )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( Module32First(hSnapshot, &stEntry) )
		iRet	=	0;
	else
		iRet	=	-1;

	CloseHandle(hSnapshot);

	if( -1==iRet )
		return -1;

	if( a_iPath<(int)strlen(stEntry.szExePath)+1 )
	{
		errno	=	ENOMEM;
		return -1;
	}

	strcpy(a_pszPath, stEntry.szExePath);

	return 0;

#else /* WIN32 */
	int iRet;
	char szLink[PATH_MAX];

	szLink[sizeof(szLink)-1]	=	'\0';

	snprintf(szLink, sizeof(szLink)-1, "/proc/%d/exe", (int)a_pid);

	iRet	=	readlink(szLink, a_pszPath, (size_t)a_iPath);

	return iRet;
#endif /* WIN32 */
}

int tos_check_pid(pid_t a_pid, const char* a_pszExe)
{
	char szPath[PATH_MAX];

	if( tos_get_exe(a_pid, szPath, (int)sizeof(szPath))<0 )
		return -1;

	if( strstr(szPath, a_pszExe) )
		return 0;

	errno	=	0;

	return -1;
}

int tos_timed_wait(pid_t a_pid, int a_iTimeout)
{
	int iRet=0;
	time_t tLast;
	time_t tNow;

	time(&tLast);
	
	while(1)
	{
		usleep(0);
		kill(a_pid, 0);

		if( ESRCH==errno )
			break;

		time(&tNow);

		if( tNow - tLast > a_iTimeout )
		{
			errno	=	ETIMEDOUT;
			iRet	=	-1;
			break;
		}
	}

	return iRet;

}

int tos_send_signal(const char* a_pszPidFile, const char* a_pszExeFile, int a_iSig, pid_t* a_ppid)
{
	pid_t pid;

	pid	=	tos_read_pid(a_pszPidFile);
	if( a_ppid )
		*a_ppid	=	pid;

	if( (pid_t)-1==pid )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( tos_check_pid(pid, a_pszExeFile)<0 )
	{
		errno	=	EINVAL;
		return -1;
	}

	return kill(pid, a_iSig);
}



int tos_kill_prev(const char* a_pszPidFile, const char* a_pszExeFile, int a_iTimeout)
{
	pid_t pid;

	pid	=	tos_read_pid(a_pszPidFile);

	if( (pid_t)-1==pid )
		return 0;

	if( tos_check_pid(pid, a_pszExeFile)<0 )
		return 0;

#ifdef WIN32
	if( kill(pid, TSIGQUIT)<0 )
#else /* WIN32 */
	if( kill(pid, SIGQUIT)<0 )
#endif /* WIN32 */
	{
		if( ESRCH==errno )
			return 0;
		else
			return -1;
	}

	return tos_timed_wait(pid, a_iTimeout);
}


int tos_mkdir_fast(char* a_pszDir)
{
	int iRet;
	char* pszPos;

	iRet	=	mkdir(a_pszDir, 0755);

	if( 0==iRet || EEXIST==errno )
		return 0;

	pszPos	=	strrchr(a_pszDir, TOS_DIRSEP);

	if( !pszPos )
		return -1;

	pszPos[0]	=	'\0';
	iRet	=	tos_mkdir_fast(a_pszDir);
	pszPos[0]	=	TOS_DIRSEP;

	if( iRet<0 )
		return -1;

	iRet	=	mkdir(a_pszDir, 0755);

	return iRet;
}


int tos_mkdir(const char* a_pszDir)
{
	char* pszNew;
	int iRet;

	pszNew	=	strdup(a_pszDir);
	if( !pszNew )
		return -1;

	iRet	=	tos_mkdir_fast(pszNew);

	free( pszNew );

	return iRet;
}

int tos_mkdir_by_path_fast(char* a_pszPath)
{
	char* pszPos;
	int iRet;


	iRet	=	0;
	pszPos	=	strrchr(a_pszPath, TOS_DIRSEP);

	if( pszPos )
	{
		pszPos[0]	=	'\0';
		iRet	=	tos_mkdir_fast(a_pszPath);
	}

	return iRet;
}

int tos_mkdir_by_path(const char* a_pszPath)
{
	char* pszDir;
	int iRet;

	pszDir	=	strdup(a_pszPath);
	if( !pszDir )
		return -1;

	iRet	=	tos_mkdir_by_path_fast(pszDir);

	free( pszDir );

	return iRet;
}



