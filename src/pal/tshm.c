/*
**  @file $RCSfile: tshm.c,v $
**  general description of this module
**  $Id: tshm.c,v 1.2 2008/11/12 04:07:08 jackyai Exp $
**  @author $Author: jackyai $
**  @date $Date: 2008/11/12 04:07:08 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tstdio.h"
#include "pal/tstring.h"
#include "pal/tipc.h"
#include "pal/tshm.h"

#ifdef WIN32

#pragma warning(disable:4996)


int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
	int iRet = 0;

	switch(cmd)
	{
	case IPC_STAT:
		{
			MEMORY_BASIC_INFORMATION   mbi; 
			void* pvBase = shmat(shmid, NULL, FILE_MAP_READ);
			memset(&mbi, 0, sizeof(mbi));
			memset(buf, 0, sizeof(*buf));
			if (NULL != pvBase)
			{				  
				VirtualQuery(pvBase,&mbi,sizeof(mbi));   
				buf->shm_segsz = mbi.RegionSize;
			}else
			{
				iRet = EINVAL;
			}
			break;
		}
		
	case IPC_SET:
		break;
	case IPC_RMID:
		CloseHandle((HANDLE)shmid);
		break;
	}

	return iRet;
}

int shmget(key_t key, int size, int shmflg)
{
	int iShm;
	SECURITY_ATTRIBUTES sa;
	char szKey[TIPC_MAX_NAME];

	tipc_key2name(szKey, (int) sizeof(szKey), key);

	if( shmflg & IPC_CREAT )	/* whether we need to create the shared memory. */
	{
#ifdef WINNT
		sa.lpSecurityDescriptor	=	tos_get_sd();
#else
		sa.lpSecurityDescriptor	=	NULL;
#endif
		iShm	=	(int) CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, (DWORD)size, szKey );

		if( (shmflg & IPC_EXCL) && iShm && ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle((HANDLE)iShm);
			iShm	=	0;
		}

#ifdef WINNT
		tos_free_sd(sa.lpSecurityDescriptor);
#endif
	}
	else
		iShm	=	(int) OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szKey );

	if(0==iShm)
	{
		printf("Could not open file mapping object (%d).\n", 
			GetLastError());
		return -1;
	}

	return iShm;
}

void* shmat(int shmid, const void* shmaddr, int shmflg)
{
	DWORD dwFlags=0;
	HANDLE hShm;

	hShm	=	(HANDLE) shmid;

	if( shmflg & SHM_RDONLY )
		return MapViewOfFile(hShm, FILE_MAP_READ, 0, 0, 0);
	else
		return MapViewOfFile(hShm, FILE_MAP_ALL_ACCESS, 0, 0, 0);

}

int shmdt(const void* shmaddr)
{
	if(UnmapViewOfFile(shmaddr))
		return 0;
	else
		return -1;
}

#endif /* WIN32 */

int tshmdelete(const char* pszPath, int iType)
{
#ifdef WIN32
	return 0;
#else
	int iShm;

	iShm	=	tshmopen(pszPath, 0, 0, iType);

	if( -1==iShm )
		return -1;

	return tshmclose(iShm, TSHMF_DELETE);
#endif
}

int tshmopen(const char* pszPath, int iSize, int iFlags, int iType)
{
#ifdef WIN32 /* for windows system.*/

	int iShm;
	SECURITY_ATTRIBUTES sa;
	char* pszName;
	char szPrefix[TSHM_MAX_PREFIX];
	char szKey[TSHM_MAX_KEY];
	char szBuff[TIPC_MAX_NAME];

	sa.nLength		=	sizeof(sa);
	sa.bInheritHandle	=	FALSE;
	sa.lpSecurityDescriptor	=	0;

	if( iFlags & TSHMF_PRIVATE )
	{
		iFlags	|=	TSHMF_CREATE;
		pszName	=	(char*)0;
	}
	else
	{
		szPrefix[sizeof(szPrefix)-1]	=	'\0';
		snprintf(szPrefix, sizeof(szPrefix)-1, TSHM_PATTERN, iType);
		
		if( (unsigned int)pszPath < 0xffff )
		{
			szKey[sizeof(szKey)-1]	=	'\0';
			snprintf(szKey, sizeof(szKey)-1, "%u", pszPath);

			pszPath	=	szKey;
		}

		if( tipc_path2name(szBuff, TIPC_MAX_NAME, pszPath, szPrefix)< 0 )
			return -1;
		
		pszName	=	szBuff;
	}
	
	if( iFlags & TSHMF_CREATE )	/* whether we need to create the shared memory. */
	{
#ifdef WINNT
		sa.lpSecurityDescriptor	=	tos_get_sd();
#else
		sa.lpSecurityDescriptor	=	NULL;
#endif
		iShm	=	(int) CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, (DWORD)iSize, pszName );

		if( (iFlags & TSHMF_EXCL) && iShm && ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle((HANDLE)iShm);
			iShm	=	0;
		}

#ifdef WINNT
		tos_free_sd(sa.lpSecurityDescriptor);
#endif
	}
	else
		iShm	=	(int) OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, pszName );

	if(0==iShm)
		return -1;
	else
		return iShm;
#else	/* for unix system. */
	int iKey;
	int iCreate=0;
	int iAccess=0;
	char* pszPos;
	int iLen;

	if( iFlags & TSHMF_PRIVATE )
		iKey	=	IPC_PRIVATE;
	else if( (unsigned int)pszPath < 0xffff )
	{
		iKey	=	(int) pszPath;
		iKey	+=	iType;

		if( -1==iKey )
			return -1;
	}
	else
	{
		iLen	=	strlen(pszPath);

		iKey	=	strtol(pszPath, &pszPos, 0);

		if( iLen==pszPos-pszPath )
			iKey	+=	iType;
		else
			iKey	=	ftok(pszPath, iType);
		
		if( -1==iKey ) 
			return -1;
	}

	iAccess	=	iFlags	& TSHM_MODE_MASK;
	
	if( 0==iAccess	)
		iAccess	=	TSHM_DFT_ACCESS;

	if( iFlags & TSHMF_CREATE )
		iCreate	|=	IPC_CREAT;

	if( iFlags & TSHMF_EXCL )
		iCreate |=	IPC_EXCL;

	return shmget(iKey, iSize, iAccess | iCreate);
#endif /* WIN32*/
}

int tshmclose(int iShm, int iFlags)
{
#ifdef WIN32
	if( CloseHandle((HANDLE)iShm ) )
		return 0;
	else
		return -1;
#else
	if( 0==(iFlags & TSHMF_DELETE ) )	/* if we need not delete the ipc, we just return success. */
		return 0;

	return shmctl(iShm, IPC_RMID, (struct shmid_ds *)0);
#endif
}

void* tshmat(int iShm, int iFlags)
{
#ifdef WIN32	
	void* pvBase;
#endif /* WIN32 */

#ifdef WIN32
	if( TSHMF_RDONLY==iFlags )
		pvBase	=	MapViewOfFile((HANDLE)iShm, FILE_MAP_READ, 0, 0, 0);
	else
		pvBase	=	MapViewOfFile((HANDLE)iShm, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if( NULL==pvBase )
		return (void*)-1;
	else
		return pvBase;
#else

	if( TSHMF_RDONLY==iFlags )
		iFlags	=	SHM_RDONLY;
	else
		iFlags	=	0;
	
	return shmat(iShm, (void*)0, iFlags);
#endif
}

int tshmdt(void* pvBase)
{
#ifdef WIN32
	if(UnmapViewOfFile(pvBase))
		return 0;
	else
		return -1;
#else
	return shmdt(pvBase);
#endif
}

