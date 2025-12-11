#include "pal/tos.h"
#include "pal/terrno.h"
#include "pal/tfile.h"
#include "pal/tstring.h"
#include "pal/tstdio.h"

#include <assert.h>

#ifdef WIN32
/**
   * Apply or remove an advisory lock on an open file
    */
int flock(int fd, int operation)
{
	DWORD dwFlags;
	HANDLE hFile;
	OVERLAPPED theOvInfo;
	BOOL bRet;

	errno = 0;

	hFile = (HANDLE) _get_osfhandle(fd);
	memset(&theOvInfo, 0, sizeof(OVERLAPPED));

	/* Don't deadlock ourselves */
	if (tos_is_winnt())
		bRet = UnlockFileEx(hFile, 0, 1, 0, &theOvInfo);
	else
		bRet = UnlockFile(hFile, 0, 0, 1, 0);

	if (operation & LOCK_UN)
	{
		if (!bRet && ((dwFlags = GetLastError()) != ERROR_NOT_LOCKED))
		{
			terrno_set_from_winerr(dwFlags);
			return -1;
		}
		else
			return 0;
	}

	if (operation & LOCK_EX)
	{
		dwFlags = LOCKFILE_EXCLUSIVE_LOCK;
	}
	else if (operation & LOCK_SH)
	{
		dwFlags = 0;
	}
	else
	{
		errno = EINVAL;
		return -1;
	}

	if (operation & LOCK_NB)
		dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;

	if (tos_is_winnt())
		bRet = LockFileEx(hFile, dwFlags, 0, 1, 0, &theOvInfo);
	else
		bRet = LockFile(hFile, 0, 0, 1, 0);

	if (! bRet)
	{
		terrno_set_from_winerr(GetLastError());
		return -1;
	}
	else
		return 0;
}


#else /* WIN32 */

#endif /* WIN32 */

int tfopen(const char* a_pszPath, int a_iMode)
{
#ifdef WIN32
	int iAccess;
	int iCreate;
	int iFile;
	SECURITY_ATTRIBUTES sa;

	iAccess	=	0;
	iCreate	=	0;

	sa.nLength		=	sizeof(sa);
	sa.bInheritHandle	=	FALSE;
	sa.lpSecurityDescriptor	=	0;

	if( a_iMode & TF_MODE_READ )
		iAccess	|=	GENERIC_READ;
	if( a_iMode & TF_MODE_WRITE )
		iAccess |=	GENERIC_WRITE;

	if( a_iMode & TF_MODE_CREATE )
	{
		if(a_iMode & TF_MODE_EXCL)
			iCreate	=	CREATE_NEW;
		else if( a_iMode & TF_MODE_TRUNCATE )
			iCreate	=	CREATE_ALWAYS;
		else
			iCreate	=	OPEN_ALWAYS;
	}
	else
	{
		if(a_iMode & TF_MODE_TRUNCATE)
			iCreate	=	TRUNCATE_EXISTING;
		else
			iCreate	=	OPEN_EXISTING;
	}

#ifdef WINNT
	sa.lpSecurityDescriptor	=	tos_get_sd();
#else
	sa.lpSecurityDescriptor	=	NULL;
#endif

	iFile	=	(int)CreateFile( a_pszPath, (DWORD)iAccess, FILE_SHARE_READ | FILE_SHARE_WRITE,
					&sa,(DWORD) iCreate, FILE_FLAG_WRITE_THROUGH,NULL );

#ifdef WINNT
	tos_free_sd(sa.lpSecurityDescriptor);
#endif

	if( INVALID_HANDLE_VALUE==(HANDLE)iFile )
		return -1;
	else
		return iFile;

#else
	int iAccess;
	int iCreate;

	iAccess	=	0;
	iCreate	=	0;

	if( ( a_iMode & TF_MODE_READ ) &&
		( a_iMode & TF_MODE_WRITE ) )
		iAccess =	O_RDWR;
	else if( a_iMode & TF_MODE_READ )
		iAccess	=	O_RDONLY;
	else if( a_iMode & TF_MODE_WRITE )
		iAccess	=	O_WRONLY;
	else
		return -1;

	if( a_iMode & TF_MODE_CREATE )
		iCreate	|=	O_CREAT;

	if( a_iMode & TF_MODE_TRUNCATE )
		iCreate |=	O_TRUNC;

	if( a_iMode & TF_MODE_EXCL )
		iCreate |=	O_EXCL;

	return open(a_pszPath, iAccess | iCreate, 0666);
#endif

}

int tfclose(int a_iFile)
{
#ifdef WIN32
	if( -1==a_iFile )
		return 0;

	if(CloseHandle((HANDLE)a_iFile))
		return 0;
	else
		return -1;
#else
	if( -1==a_iFile )
		return 0;

	return close(a_iFile);
#endif
}


int tfdelete(const char* a_pszPath)
{
#ifdef WIN32
	if(DeleteFile(a_pszPath))
		return 0;
	else
		return -1;
#else
	return unlink(a_pszPath);
#endif
}


int tfwrite(int a_iFile, const char* a_pszBuff, int a_iSize)
{
#ifdef WIN32
	DWORD dwWritten;

	if( a_iSize<=0 )
		return -1;

	if(WriteFile((HANDLE)a_iFile, a_pszBuff, (DWORD)a_iSize, &dwWritten, NULL))
		return (int)dwWritten;
	else
		return -1;
#else
	return write(a_iFile, a_pszBuff, (size_t)a_iSize);
#endif
}

int tfread(int a_iFile, char* a_pszBuff, int a_iSize)
{
#ifdef WIN32
	DWORD dwRead;

	if( a_iSize<=0 )
		return -1;
	
	if(ReadFile((HANDLE)a_iFile, a_pszBuff, (DWORD)a_iSize, &dwRead, NULL))
		return (int)dwRead;
	else
		return -1;
#else
	return read( a_iFile, a_pszBuff, (size_t) a_iSize);
#endif
}

int tfload(int a_iFile, char** a_ppszBuff, int* a_piSize)
{
	int iSize;
	int iRead;
	int iRet;
	char* pszData;
	
	assert( a_ppszBuff && a_piSize );

	iSize	=	tfsize(a_iFile);

	if( iSize<0 )
		return -1;

	if( 0==iSize )
	{
		*a_ppszBuff	=	NULL;
		*a_piSize		=	0;
		
		return 0;
	}

	*a_piSize	=	iSize;

	pszData	=	(char*) calloc(1, iSize+1);

	if( NULL==pszData )
		return -1;

	iRead	=	tfread(a_iFile, pszData, iSize);

	if( iRead==iSize )
	{
		*a_ppszBuff	=	pszData;
		pszData[iSize]	=	'\0';
		iRet		=	0;
	}
	else
	{
		free(pszData);
		iRet		=	-1;
	}

	return iRet;
}

int tflload(const char* a_pszPath, char** a_ppszBuff, int* a_piSize)
{
	int iFile;
	int iRet;

	iFile	=	tfopen(a_pszPath, TF_MODE_READ);

	if( -1==iFile )
		return -1;

	iRet	=	tfload(iFile, a_ppszBuff, a_piSize);

	tfclose(iFile);

	return iRet;
}

int tfsync(int a_iFile)
{
#ifdef WIN32
	if( FlushFileBuffers((HANDLE)a_iFile) )
		return 0;
	else
		return -1;
#else
	return fsync(a_iFile);
#endif
}

int tflock(int a_iFile, int a_iOff, int a_iLen, int a_iIsBlocking)
{
#ifdef WIN32
	int iRet;
	
	iRet	=	LockFile((HANDLE)a_iFile, (DWORD)a_iOff, 0, (DWORD)a_iLen, 0);

	if( iRet )
		return 0;
	else
		return -1;
#else
	int iRet;
	struct flock lock;

	lock.l_type	=	F_WRLCK;
	lock.l_whence	=	SEEK_SET;
	lock.l_start	=	a_iOff;
	lock.l_len	=	a_iLen;

	if( a_iIsBlocking )
	{
		iRet	=	fcntl(a_iFile, F_SETLKW, &lock);
		return iRet;
	}
	else
	{
		iRet	=	fcntl(a_iFile, F_SETLK, &lock);
		return iRet;
	}
#endif
}

int tfunlock(int a_iFile, int a_iOff, int a_iLen)
{
#ifdef WIN32
	int iRet;

	iRet	=	UnlockFile((HANDLE)a_iFile, (DWORD)a_iOff, 0, (DWORD)a_iLen, 0);

	if( iRet )
		return 0;
	else
		return -1;
#else
	struct flock lock;

	lock.l_type	=	F_UNLCK;
	lock.l_whence	=	SEEK_SET;
	lock.l_start	=	a_iOff;
	lock.l_len	=	a_iLen;

	return fcntl(a_iFile, F_SETLK, &lock);
#endif
}

int tfseek(int a_iFile, int a_iOff, int a_iOrigin)
{
#ifdef WIN32

	int iMethod;
	int iRet;

	switch(a_iOrigin)
	{
	case SEEK_SET:
		iMethod	=	FILE_BEGIN;
		break;
	case SEEK_CUR:
		iMethod	=	FILE_CURRENT;
		break;
	case SEEK_END:
		iMethod	=	FILE_END;
		break;
	default:
		return -1;
	}
	
	iRet	=	(int) SetFilePointer((HANDLE)a_iFile, a_iOff, 0, iMethod);

	if( -1==iRet )
		return -1;
	else
		return iRet;
#else
	return lseek(a_iFile, a_iOff, a_iOrigin);
#endif
}

int tfsize(int a_iFile)
{
#ifdef WIN32
	return (int) GetFileSize((HANDLE)a_iFile, (DWORD*)0);
#else
	struct stat stInfo;
	
	if( -1==fstat(a_iFile, &stInfo) )
		return -1;

	return (int)stInfo.st_size;
#endif
}

int tflsize(const char* a_pszPath)
{
#ifdef WIN32
	struct _stat stInfo;

	if( -1==_stat(a_pszPath, &stInfo) )
		return -1;

	return (int)stInfo.st_size;
#else
	struct stat stInfo;
	
	if( -1==stat(a_pszPath, &stInfo) )
		return -1;

	return (int)stInfo.st_size;
#endif
}

int tftruncate(int a_iFile, int a_iSize)
{
#ifdef WIN32
	if( -1==tfseek(a_iFile, a_iSize, SEEK_SET) )
		return -1;

	if( SetEndOfFile((HANDLE)a_iFile) )
		return 0;
	else
		return -1;
#else
	if( -1==tfseek(a_iFile, a_iSize, SEEK_SET) )
		return -1;

	return ftruncate(a_iFile, a_iSize);
#endif
}

int tfexist(const char* a_pszPath)
{
#ifdef WIN32
	if(-1==GetFileAttributes(a_pszPath))
		return -1;
	else
		return 0;
#else
	struct stat stInfo;

	if( -1==stat(a_pszPath, &stInfo) )
		return -1;
	else
		return 0;
#endif
}

int tfmkdir(const char* a_pszPath)
{
#ifdef WIN32
	if( CreateDirectory(a_pszPath, NULL) )
		return 0;
	else
		return -1;
#else
	return mkdir(a_pszPath, S_IRWXU | S_IRWXG);
#endif
}

int tfcopy(const char* a_pszDst, const char* a_pszSrc, int a_iIsReplaceExist)
{
#ifdef WIN32
	if( CopyFile(a_pszSrc, a_pszDst, a_iIsReplaceExist) )
		return 0;
	else
		return -1;
#else
	char szBuff[0x1000];
	FILE* fpSrc;
	FILE* fpDst;
	int iRead;
	int iWrite;
	int iRet=0;

	if( 0==a_iIsReplaceExist )	/* make sure the file is nonexist. */
	{
		fpDst	=	fopen(a_pszDst, "rb");

		if( fpDst )
		{
			fclose(fpDst);
			return -1;
		}
	}

	fpSrc	=	fopen(a_pszSrc, "rb");
	if( (FILE*)0==fpSrc )
		return -1;

	fpDst	=	fopen(a_pszDst, "wb");
	if( (FILE*)0==fpDst )
	{
		fclose(fpSrc);
		return -1;
	}

	do
	{
		iRead	=	fread(szBuff, 1, 0x1000, fpSrc);

		if( iRead>0 )
		{
			iWrite	=	fwrite(szBuff, 1, iRead, fpDst);

			if( iWrite!=iRead )
			{
				iRet	=	-1;
				break;
			}
		}
	}
	while( iRead>0 );

	fclose(fpSrc);
	fclose(fpDst);

	return iRet;
#endif
}

int tflstat(const char* a_pszPath, TFSTAT* a_pstStat)
{
#ifdef WIN32
	return _stat(a_pszPath, a_pstStat);
#else
	return stat(a_pszPath, a_pstStat);
#endif
}

int tfstat(int a_iFile, TFSTAT* a_pstStat)
{
#ifdef WIN32
	return _fstat(a_iFile, a_pstStat);
#else
	return fstat(a_iFile, a_pstStat);
#endif
}

