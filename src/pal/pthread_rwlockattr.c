
/*
pthread_rwlockattr.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tthread.h"
#include <assert.h>

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *rwlattr)
{
	return 0;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t * rwlattr, int * val)
{
	assert( rwlattr && val );

	*val	=	rwlattr->pshared;

	return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t * rwlattr)
{
	assert( rwlattr );

	memset( rwlattr, 0, sizeof(*rwlattr) );

	rwlattr->pshared	=	1;

	return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t * rwlattr, int val)
{
	assert( rwlattr );

	rwlattr->pshared	=	val;

	return 0;
}


#endif /* WIN32 */
