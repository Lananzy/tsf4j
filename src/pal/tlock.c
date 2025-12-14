/*
**  @file $RCSfile: tlock.c,v $
**  general description of this module
**  $Id: tlock.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tlock.h"
#include "pal/tstring.h"
#include "pal/tstdio.h"
#include "pal/tsem.h"

#ifdef WIN32

#pragma warning(disable:4996)

#endif /* WIN32 */

#ifndef WIN32
int tlockcheck(int a_iLock, int a_iMaxTries)
{
	struct semid_ds stInfo;
	union semun stArg;
	int iInitOK;
	int i;

	iInitOK		=	0;

	stArg.buf	=	&stInfo;

	for(i=0; i<a_iMaxTries; i++)
	{
		if( semctl(a_iLock, 0, IPC_STAT, stArg)==-1 )
			return -1;

		if(stArg.buf->sem_otime!=0)
		{
			iInitOK	=	1;
			break;
		}
		else
			tos_usleep(10000);	
	}

	if( iInitOK )
		return 0;
	else
		return -1;
}

int tlockinit(int a_iLock)
{
	union semun stArg;

	stArg.val	=	1;

	return semctl(a_iLock, 0, SETVAL, stArg);
}

int tlockkey(const char* a_pszPath, int a_iType)
{
	int iKey;
	char* pszPos;
	int iLen;

	if( (unsigned int)a_pszPath < 0xffff )
	{
		iKey	=	(int)a_pszPath;
		iKey	+=	a_iType;

		return iKey;
	}

	iLen	=	strlen(a_pszPath);

	iKey	=	strtol(a_pszPath, &pszPos, 0);

	if( iLen==pszPos-a_pszPath )
		iKey	+=	a_iType;
	else
		iKey	=	ftok(a_pszPath, a_iType);
	
	if( -1==iKey )
		return -1;
	else
		return iKey;
}
#endif

int tlockopen(const char* a_pszPath, int a_iFlags, int a_iType)
{
#ifdef WIN32 /* for windows system.*/

	int iLock;
	SECURITY_ATTRIBUTES sa;
	char* pszName;
	char szPrefix[TLOCK_MAX_PREFIX];
	char szKey[TLOCK_MAX_KEY];
	char szBuff[TIPC_MAX_NAME];

	sa.nLength		=	sizeof(sa);
	sa.bInheritHandle	=	FALSE;
	sa.lpSecurityDescriptor	=	0;

	szPrefix[sizeof(szPrefix)-1]	=	'\0';
	snprintf(szPrefix, sizeof(szPrefix)-1, "tlock:%u:", a_iType);

	if( (unsigned int)a_pszPath < 0xffff )
	{
		szKey[sizeof(szKey)-1]	=	'\0';
		snprintf(szKey, sizeof(szKey)-1, "%u", a_pszPath);
		a_pszPath	=	szKey;
	}

	if( tipc_path2name(szBuff, TIPC_MAX_NAME, a_pszPath, szPrefix)< 0 )
			return -1;
		
	pszName	=	szBuff;
	
	if( a_iFlags & TLOCKF_CREATE )	/* whether we need to create the shared memory. */
	{
#ifdef WINNT
		sa.lpSecurityDescriptor	=	tos_get_sd();
#else
		sa.lpSecurityDescriptor	=	NULL;
#endif
		iLock	=	(int) CreateMutex(&sa, 0, pszName);

		if( (a_iFlags & TLOCKF_EXCL) && iLock && ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle((HANDLE)iLock);
			iLock	=	0;
		}

#ifdef WINNT
		tos_free_sd(sa.lpSecurityDescriptor);
#endif
	}
	else
		iLock	=	(int) OpenMutex(MUTEX_ALL_ACCESS, 0, pszName);

	if(0==iLock)
		return -1;
	else
		return iLock;
#else	/* for unix system. */
	int iKey;
	int iLock;
	int iCreate=0;
	int iAccess=0;

	iKey	=	tlockkey(a_pszPath, a_iType);
	
	if( -1==iKey )
		return -1;

	iAccess	=	a_iFlags	& TLOCK_MODE_MASK;
	
	if( 0==iAccess	)
		iAccess	=	TLOCK_DFT_ACCESS;

	if( a_iFlags & TLOCKF_CREATE )
		iCreate	|=	IPC_CREAT;

	if( a_iFlags & TLOCKF_EXCL )
		iCreate |=	IPC_EXCL;

	/* create */
	if( a_iFlags & TLOCKF_CREATE )
	{
		/* first do really create. */
		iLock	=	semget(iKey, 1, IPC_CREAT | IPC_EXCL | iAccess);

		if( iLock>=0 )
		{
			if( tlockinit(iLock)<0 )
			{
				tlockclose(iLock, TLOCKF_DELETE);
				return -1;
			}
			else
			{
				if( tlockop(iLock, 1)>=0 )
					tlockop(iLock, 0);

				return iLock;
			}
		}
		
		if( a_iFlags & TLOCKF_EXCL )
			return -1;

		iLock	=	semget(iKey, 1, IPC_CREAT | iAccess);
		if( iLock<0 )
		{
			perror("Lock:");
			return -1;
		}
		
		if( tlockcheck(iLock, TLOCK_MAX_TRIES)<0 )
		{
			return -1;
		}
		else
			return iLock;
	}
	else /* only open */
	{
		iLock	=	semget(iKey, 1, 0);//iAccess);

		if( iLock<0 )
		{
			perror("Lock:");
			return -1;
		}

		if( tlockcheck(iLock, TLOCK_MAX_TRIES)<0 )
			return -1;
		else
			return iLock;
	}

#endif /* WIN32*/
}

int tlockclose(int a_iLock, int a_iFlags)
{
#ifdef WIN32
        if( CloseHandle((HANDLE)a_iLock) )
                return 0;
        else
                return -1;
#else
        if( 0==(a_iFlags & TLOCKF_DELETE ) )       /* if we need not delete the ipc
, we just return success. */
                return 0;

        return semctl(a_iLock, 0, IPC_RMID);
#endif
}

int tlockdelete(const char* a_pszPath, int a_iType)
{
#ifdef WIN32
        return 0;
#else
	int iKey;
    int iLock;

	iKey	=	tlockkey(a_pszPath, a_iType);
	if( -1==iKey )
		return -1;

	iLock	=	semget(iKey, 1, 0);
        if( -1==iLock)
                return -1;

        return tlockclose(iLock, TLOCKF_DELETE);
#endif /* WIN32 */
}

int tlockop(int a_iLock, int a_iIsLocking)
{
#ifdef WIN32
	DWORD dwRet;

	if( a_iIsLocking )
	{
		dwRet	=	WaitForSingleObject((HANDLE)a_iLock, INFINITE);

		if( WAIT_OBJECT_0==dwRet )
			return 0;
		else
			return -1;
	}
	else
	{
		if( ReleaseMutex((HANDLE)a_iLock) )
			return 0;
		else
			return -1;
	}
#else
	struct sembuf stOp;

	stOp.sem_num	=	0;
	stOp.sem_op	=	a_iIsLocking ? -1 : 1;
	stOp.sem_flg	=	SEM_UNDO;

	return semop(a_iLock, &stOp, 1);
#endif /* WIN32 */
}
