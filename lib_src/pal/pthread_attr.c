
/*
pthread_attr.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/


#ifdef WIN32

#include "pal/tthread.h"
#include "pal/terrno.h"

#include <assert.h>

int pthread_attr_init(pthread_attr_t *attr)
{
	memset(attr, 0, sizeof(*attr));
	attr->psa = NULL;  /*can childthreads inherit attributes*/
	attr->detachstate = PTHREAD_CREATE_JOINABLE;
	attr->scope = PTHREAD_SCOPE_PROCESS;

	return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	/*nothing to implement*/

	return 0;
}


int   pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
	assert(attr && detachstate);

	*detachstate = attr->detachstate;

	return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
	assert( attr && guardsize );
	
	*guardsize	=	attr->guardsize;

	return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
	assert( attr && inheritsched );

	*inheritsched	=	attr->inheritsched;

	return 0;
}

int   pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
	assert( attr && param );

	*param	=	attr->schedparam;

	return 0;
}


int   pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
	assert( attr && policy );

	*policy	=	attr->schedpolicy;

	return 0;
}

int   pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
	assert(scope && attr);

	*scope = attr->scope;

	return 0;
}

int   pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
	assert(stackaddr && attr);

	*stackaddr = attr->stackaddr;

	return 0;
}

int   pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
	assert(stacksize && attr);

	*stacksize = attr->stacksize;

	return 0;
}

int   pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	assert( attr );

	/*check the validity of detach state*/
	if((detachstate != PTHREAD_CREATE_JOINABLE) && (detachstate != PTHREAD_CREATE_DETACHED))
		return EINVAL;
	 
	attr->detachstate = detachstate;

	return 0;
}

int   pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
	assert( attr );

	attr->guardsize	=	guardsize;

	return 0;
}

int   pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
	assert( attr );

	attr->inheritsched	=	inheritsched;

	return 0;
}

int   pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
	assert( attr && param );

	attr->schedparam	=	*param;

	return 0;
}

int   pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
	assert( attr );

	attr->schedpolicy	=	policy;

	return 0;
}

int   pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
	assert( attr );

	/*check the validity of connection scope*/
	if((scope != PTHREAD_SCOPE_PROCESS) && (scope != PTHREAD_SCOPE_SYSTEM))
		return ENOTSUP;
	 
	attr->scope = scope;

	return 0;
}

int   pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
	assert( attr );
	
	attr->stackaddr = stackaddr;

	return 0;
}

int   pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	assert( attr );

	attr->stacksize = stacksize;

	return 0;
}


#endif /* WIN32 */
