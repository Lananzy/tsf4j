/*
pthread_cond.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tos.h"
#include "pal/tthread.h"
#include "pal/ttime.h"
#include "pal/terrno.h"

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    int res;

	if( !cond->cond )
		return EINVAL;

    res = PulseEvent(cond->cond);

    if( res )
		return 0;
	else
		return EINTR;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	if( cond->cond )
	{
		CloseHandle(cond->cond);
		cond->cond	=	NULL;
	}

	return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *condattr)
{
	memset(cond, 0, sizeof(*cond));

	cond->cond = CreateEvent(NULL, TRUE, FALSE, NULL);

	return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    DWORD res;

	if( !cond->cond )
		return EINVAL;

    res = SetEvent(cond->cond);

    if( res )
		return 0;
	else
		return EINTR;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec * abstime)
{
	DWORD res;
    DWORD timeout; 

	if( abstime )
		timeout = (abstime->tv_sec*1000 + abstime->tv_nsec/1000000);
	else
		timeout	=	INFINITE;

    cond->num_waiting++;

    pthread_mutex_unlock(mutex);

	res = WaitForSingleObject(cond->cond, timeout);

	pthread_mutex_lock(mutex);

	cond->num_waiting--;

    if (res != WAIT_OBJECT_0)
	{
        if (res == WAIT_TIMEOUT)
            return ETIMEDOUT;
		else
            return EINTR;
    }

    ResetEvent(cond->cond);

    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    return pthread_cond_timedwait(cond, mutex, NULL);

}

#endif /* WIN32 */
