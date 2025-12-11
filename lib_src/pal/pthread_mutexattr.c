
/*
pthread_mutexattr.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tthread.h"
#include "pal/tsched.h"
#include <errno.h>

int   pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	if(attr)
	{
		attr->protocol = 0;
		attr->pshared = PTHREAD_PROCESS_PRIVATE;
		attr->prioceiling = SCHED_FIFO;
		attr->type = 0;
		return 0;
	}

	return -1;
}

int   pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	/*nothing to implement*/
	return 0;
}

int   pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
	if(attr && prioceiling)
		*prioceiling = attr->prioceiling;
	else
		return -1;

	return 0;
}

int   pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
	if(attr && protocol)
		*protocol = attr->protocol;

	return 0;
}

int   pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
	if(attr && pshared)
		*pshared = attr->pshared;

	return 0;
}

int   pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	if(attr && type)
		*type = attr->type;

	return 0;
}

int   pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	if(attr)
		attr->prioceiling = prioceiling;

	return 0;
}

int   pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	if(attr)
		attr->protocol = protocol;

	return 0;
}

int   pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
	if(attr)
		attr->pshared = pshared;

	return 0;
}

int   pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	if(attr)
		attr->type = type;

	return 0;
}


#endif /* WIN32 */
