/*
pthread_condattr.c

POSIX wrapper layer for windows threads

Copyright (C) 2005  Rajulu Ponnada

License: LGPL (see COPYING file in this distribution)

contact Rajulu Ponnada at open.ponnada@gmail.com
*/

#ifdef WIN32

#include "pal/tos.h"
#include "pal/tthread.h"

int pthread_condattr_destroy(pthread_condattr_t *condattr)
{
	return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *condattr, int *val)
{
	if( val )
		*val	=	condattr->pshared;

	return 0;
}

int pthread_condattr_init(pthread_condattr_t *condattr)
{
	return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *condattr, int val)
{
	condattr->pshared	=	val;

	return 0;
}

#endif /* WIN32 */
