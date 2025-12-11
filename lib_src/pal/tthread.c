#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tstring.h"
#include "pal/tthread.h"

#ifdef WIN32
/*
int pthread_attr_init(pthread_attr_t * a_pstAttr)
{
	assert( a_pstAttr );

	memset(a_pstAttr, 0, sizeof(*a_pstAttr));

	a_pstAttr->__detachstate	=	PTHREAD_CREATE_JOINABLE;
	a_pstAttr->__schedpolicy	=	SCHED_OTHER;
	a_pstAttr->__inheritsched	=	PTHREAD_EXPLICIT_SCHED;
	a_pstAttr->__scope			=	PTHREAD_SCOPE_SYSTEM;

	return 0;
}

int pthread_attr_destroy(pthread_attr_t * a_pstAttr)
{
	assert( a_pstAttr );

	memset(a_pstAttr, 0, sizeof(*a_pstAttr));

	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *a_pstAttr, int a_iDetachState)
{
	assert( a_pstAttr );

	a_pstAttr->__detachstate	=	a_iDetachState;

	return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *a_pstAttr, int *a_piDetachState)
{
	assert( a_pstAttr && a_piDetachState );

	*a_piDetachState	=	a_pstAttr->__detachstate;

	return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *a_pstAttr, int a_iPolicy)
{
	assert( a_pstAttr );

	a_pstAttr->__schedpolicy	=	a_iPolicy;

	return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *a_pstAttr,  int *a_piPolicy)
{
	assert( a_pstAttr && a_piPolicy);

	*a_piPolicy	=	a_pstAttr->__schedpolicy;

	return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *a_pstAttr,  const struct sched_param *a_pstParam)
{
	assert( a_pstAttr && a_pstParam);

	memcpy( &a_pstAttr->__schedparam, a_pstParam, sizeof(*a_pstParam) );

	return 0;
}

int  pthread_attr_getschedparam(const  pthread_attr_t *a_pstAttr, struct sched_param *a_pstParam)
{
	assert( a_pstAttr && a_pstParam);

	memcpy( a_pstParam, &a_pstAttr->__schedparam, sizeof(*a_pstParam) );

	return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *a_pstAttr, int a_iInherit)
{
	assert( a_pstAttr );

	a_pstAttr->__inheritsched	=	a_iInherit;

	return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *a_pstAttr, int *a_piInherit)
{
	assert( a_pstAttr && a_piInherit);

	*a_piInherit	=	a_pstAttr->__inheritsched;
	
	return 0;
}

int pthread_attr_setscope(pthread_attr_t *a_pstAttr, int a_iScope)
{
	assert( a_pstAttr );

	a_pstAttr->__scope	=	a_iScope;

	return 0;
}

int pthread_attr_getscope(const pthread_attr_t *a_pstAttr, int *a_piScope)
{
	assert( a_pstAttr && a_piScope );

	*a_piScope	=	a_pstAttr->__scope;

	return 0;
}
*/

#else	/* WIN32 */


#endif	/* WIN32 */
