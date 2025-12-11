
/*
pthread_rwlock.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tos.h"
#include "pal/tthread.h"
#include "pal/terrno.h"

int pthread_rwlock_destroy(pthread_rwlock_t *rwl)
{
	if( rwl->read_event )
	{
		CloseHandle(rwl->read_event);
		rwl->read_event	=	NULL;
	}

	if( rwl->write_mutex )
	{
		CloseHandle(rwl->write_mutex);
		rwl->write_mutex=	NULL;
	}

	return 0;
}

int pthread_rwlock_init(pthread_rwlock_t *rwl, const pthread_rwlockattr_t *rwlattr)
{
	rwl->readers = 0;

	if( rwlattr->pshared )
		return ENOSYS;

	if (!(rwl->read_event = CreateEvent(NULL, TRUE, FALSE, NULL)))
	{
        rwl->read_event = NULL;
        return EPERM;
    }

    if (! (rwl->write_mutex = CreateMutex(NULL, FALSE, NULL)))
	{
        CloseHandle(rwl->read_event);
        rwl->read_event = NULL;
        return EPERM;
    }

	return 0;
}

int pthread_rwlock_rdlock_i(pthread_rwlock_t * rwl, DWORD timeout)
{
	DWORD code;
	
	code = WaitForSingleObject(rwl->write_mutex, timeout);

    if (code == WAIT_FAILED || code == WAIT_TIMEOUT)
        return EBUSY;

    /* We've successfully acquired the writer mutex, we can't be locked
     * for write, so it's OK to add the reader lock.  The writer mutex
     * doubles as race condition protection for the readers counter.   
     */
    InterlockedIncrement(&rwl->readers);

	ResetEvent(rwl->read_event);
    	
    ReleaseMutex(rwl->write_mutex);
    
    return 0;

}

int pthread_rwlock_wrlock_i(pthread_rwlock_t * rwl, DWORD timeout)
{
	DWORD code;
	HANDLE ahDesc[2];
	DWORD dwBegin;
	DWORD dwTime;

	ahDesc[0]	=	rwl->write_mutex;
	ahDesc[1]	=	rwl->read_event;

	do
	{
		dwBegin		=	GetTickCount();
		
		code = WaitForMultipleObjects(2, ahDesc, TRUE, timeout);

		dwTime		=	GetTickCount() - dwBegin;

		if (code == WAIT_TIMEOUT)
		{
			return EBUSY;
		}
		if (code == WAIT_FAILED)
		{
			DWORD dw = GetLastError(); 
			return EINVAL;
		}
			

		if( 0==rwl->readers )
			break;

		ResetEvent( rwl->read_event );

		ReleaseMutex( rwl->write_mutex );

		if( INFINITE==timeout )
			continue;

		if( (dwTime) >= timeout )
			timeout	=	0;
		else
			timeout	-=	dwTime;

	}
	while( timeout>0 );
	
    return 0;

}

int pthread_rwlock_rdlock(pthread_rwlock_t * rwl)
{
    return pthread_rwlock_rdlock_i(rwl, INFINITE);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t * rwl)
{
	return pthread_rwlock_rdlock_i(rwl, 0);
}

int pthread_rwlock_wrlock(pthread_rwlock_t * rwl)
{
	return pthread_rwlock_wrlock_i(rwl, INFINITE);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t * rwl)
{
	return pthread_rwlock_wrlock_i(rwl, 0);
}

int pthread_rwlock_unlock(pthread_rwlock_t * rwl)
{
	/* it must be called by the write owner, so just return. */
    if( ReleaseMutex(rwl->write_mutex) )
		return 0;
    
	InterlockedDecrement(&rwl->readers);

	if( SetEvent(rwl->read_event) )
		return 0;
	else
		return EPERM;
}


#endif /* WIN32 */
