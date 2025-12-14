/*
semaphore.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tos.h"
#include "pal/tsemaphore.h"
#include "pal/ttime.h"
#include "pal/ttypes.h"
#include "pal/tfcntl.h"
#include "pal/tstdio.h"
#include "pal/terrno.h"
#include <sys/stat.h>
#include <stdarg.h>

typedef enum _SEMAPHORE_INFORMATION_CLASS
{
    SemaphoreBasicInformation
} SEMAPHORE_INFORMATION_CLASS, *PSEMAPHORE_INFORMATION_CLASS;

typedef struct _SEMAPHORE_BASIC_INFORMATION
{
	ULONG                   ulCurrentCount;
	ULONG                   ulMaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

typedef int (*PFNNTQUERYSEMAPHORE)( HANDLE, int, void*, ULONG, PULONG );


/*right now pshared is having no effect; will be implemented in future*/
int sem_init(sem_t *sem, int pshared, unsigned value)
{
	memset(sem, 0, sizeof(*sem));

	sem->sem = CreateSemaphore(NULL,
							value,
							SEM_VALUE_MAX,
							NULL);

	return 0;
}

int sem_destroy(sem_t *sem)
{
	if( sem->sem )
	{
		CloseHandle(sem->sem);
		sem->sem	=	NULL;
	}

	return 0;
}

int sem_getvalue(sem_t *sem, int *value)
{
	ULONG ulLen;
	static PFNNTQUERYSEMAPHORE s_pfnQuerySema=NULL;
	SEMAPHORE_BASIC_INFORMATION stInfo;
	int iRet;

	if( !value )
		return 0;

	if( !s_pfnQuerySema )
	{
		s_pfnQuerySema	=	(PFNNTQUERYSEMAPHORE) tos_get_ntdll_proc_i("NtQuerySemaphore");

		if( !s_pfnQuerySema )
		{
			errno	=	ENOSYS;
			return -1;
		}
	}

	ulLen	=	sizeof(stInfo);

	iRet	=	(*s_pfnQuerySema)(sem->sem, SemaphoreBasicInformation, &stInfo, sizeof(stInfo), &ulLen);

	if( iRet )
	{
		errno	=	ENOSYS;
		return -1;
	}

	*value	=	stInfo.ulCurrentCount;

	return 0;
}

/*ugly implemenation; needs to be replaced in future*/
sem_t *sem_open(const char *name, int oflag, ...)
{
	sem_t* sem;
	va_list ap;
	mode_t mode;
	int value;

	sem	=	(sem_t*) calloc(1, sizeof(sem));

	if( !sem )
	{
		errno	= ENOSPC;
		return NULL;
	}


	if( oflag & O_CREAT )
	{
		va_start(ap, oflag);

		mode	=	(mode_t) va_arg(ap, mode_t);

		value	=	(int) va_arg(ap, int);

		sem->sem = CreateSemaphore(NULL, value, SEM_VALUE_MAX, name);

		va_end(ap);

		if( sem->sem && (oflag & O_EXCL) && ERROR_ALREADY_EXISTS==GetLastError() )
		{
			CloseHandle(sem->sem);
			sem->sem	=	NULL;
		}
	}
	else
	{
		sem->sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
	}

	if( sem->sem )
		return sem;

	free(sem);

	errno	=	ENFILE;

	return NULL;
}

int sem_close(sem_t *sem)
{
	if( sem->sem )
	{
		CloseHandle(sem->sem);
		sem->sem	=	NULL;
	}

	free(sem);

	return 0;
}

int sem_post(sem_t *sem)
{
	DWORD ret;

	if(!sem)
	{
		errno	=	EINVAL;
		return -1;
	}

	ret = ReleaseSemaphore(sem->sem, 1, 0);

	if(ret != 0)
	{
		sem->lockedOrReferenced = 0;
		return 0;
	}
	else
	{
		errno	=	EPERM;
		return -1;
	}
}

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
	DWORD ret;
	DWORD timeout;

	timeout = (abs_timeout->tv_sec*1000 + abs_timeout->tv_nsec/1000000);
	if(!sem)
	{
		errno	=	EINVAL;
		return -1;
	}

	ret = WaitForSingleObject(sem->sem, timeout);

	if(ret != WAIT_FAILED)
	{
		sem->lockedOrReferenced = 1;
		return 0;
	}
	else
	{
		errno	=	EINVAL;
		return -1;
	}
}

int sem_trywait(sem_t *sem)
{
	DWORD ret;

	if(!sem)
	{
		errno	=	EINVAL;
		return -1;
	}

	ret = WaitForSingleObject(sem->sem, 0);
	
	if(ret != WAIT_FAILED)
	{
		sem->lockedOrReferenced = 1;
		return 0;
	}

	errno	=	EBUSY;

	return -1;
}

/*needs to be implemented in future*/
int sem_unlink(const char *name)
{
	return 0;
}

int sem_wait(sem_t *sem)
{
	DWORD ret;

	if(!sem)
	{
		errno	=	EINVAL;
		return EINVAL;
	}

	ret = WaitForSingleObject(sem->sem, INFINITE);

	if(ret != WAIT_FAILED)
	{
		sem->lockedOrReferenced = 1;
		return 0;
	}
	else
	{
		errno	=	EAGAIN;
		return -1;
	}
}

#endif /* WIN32 */
