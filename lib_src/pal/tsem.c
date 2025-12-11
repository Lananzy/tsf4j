/*
**  @file $RCSfile: tsem.c,v $
**  general description of this module
**  $Id: tsem.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/


#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tipc.h"
#include "pal/tsem.h"
#include "pal/tshm.h"
#include "pal/tstdio.h"
#include "pal/terrno.h"


#ifdef WIN32

struct tagTOneSem
{
	int iIsInited;			 /* this is a initialized semaphore.*/
	int iIsMutex;
	unsigned short  nZWait;  /* # waiting for zero */
    unsigned short  nWait;   /* # waiting for increase */
};

typedef	struct tagTOneSem				TONESEM;
typedef	struct tagTOneSem				*LPTONESEM;

struct tagTSemSharedInfo
{
	struct semid_ds stDs;
	TONESEM astSems[1];
};

typedef struct tagTSemSharedInfo		TSEMSHAREDINFO;
typedef struct tagTSemSharedInfo		*LPTSEMSHAREDINFO;

struct tagTSemDesc
{
	int iShm;
	HANDLE hMutex;
	TSEMSHAREDINFO* pstInfo;
	HANDLE ahSems[1];
};

typedef struct tagTSemDesc				TSEMDESC;
typedef struct tagTSemDesc				*LPTSEMDESC;

int semget(key_t key, int nsems, int semflg)
{
	LPTSEMDESC pstDesc=NULL;
	int iShm;
	int iShmSize;
	int iDescSize;

	if( nsems<=0 || nsems>SEMMSL )
	{
		errno	=	EINVAL;
		return -1;
	}

	iShmSize	=	offsetof(TSEMSHAREDINFO, astSems) + sizeof(TONESEM)*nsems;

	iDescSize	=	offsetof(TSEMDESC, ahSems) + sizeof(HANDLE);

	iShm	=	shmget(key, iShmSize, semflg);

	if( -1==iShm )
		return -1;

	pstDesc	=	(TSEMDESC*) calloc(1, iDescSize);
	if( !pstDesc )
	{
		shmctl(iShm, IPC_RMID, NULL);

		return -1;
	}

	pstDesc->iShm	=	iShm;
	pstDesc->pstInfo=	(TSEMSHAREDINFO*) shmat(iShm, NULL, 0);
	
	if( !pstDesc->pstInfo )
	{
		shmctl(iShm, IPC_RMID, NULL);
		return -1;
	}

	return (int)pstDesc;
}

int semctl(int semid, int semnum, int cmd, ...)
{
	LPTSEMDESC pstDesc;

	pstDesc	=	(LPTSEMDESC) semid;
	if( !pstDesc )
	{
		errno	=	EINVAL;
		return -1;
	}

	switch( cmd )
	{
	case IPC_STAT:
		break;
	case IPC_SET:
		break;
	case IPC_RMID:
		break;
	case GETALL:
		break;
	case GETNCNT:
		break;
	case GETPID:
		break;
	case GETVAL:
		break;
	case GETZCNT:
		break;
	case SETALL:
		break;
	case SETVAL:
		break;
	default:
		errno	=	EINVAL;
		return -1;
	}

	return 0;
}

int semop(int semid, struct sembuf *sops, unsigned nsops)
{
	return semtimedop(semid, sops, nsops, NULL);
}

int semtimedop(int semid, struct sembuf *sops, unsigned nsops, struct timespec *timeout)
{
	DWORD dwTimeout;
	DWORD dwWait;
	LPTSEMDESC pstDesc;
	int iSems=0;
	HANDLE ahSems[SEMOPM];

	pstDesc	=	(LPTSEMDESC) semid;
	if( !pstDesc )
	{
		errno	=	EINVAL;
		return -1;
	}

	if( nsops>SEMOPM )
	{
		errno	=	E2BIG;
		return -1;
	}

	if( timeout )
		dwTimeout	=	timeout->tv_sec * 1000 + timeout->tv_nsec/1000000;
	else
		dwTimeout	=	INFINITE;

	dwWait	=	WaitForMultipleObjects(iSems, ahSems, TRUE, dwTimeout);

	if( WAIT_TIMEOUT==dwWait )
	{
		errno	=	EAGAIN;
		return -1;
	}

	return 0;
}


#endif /* WIN32 */
