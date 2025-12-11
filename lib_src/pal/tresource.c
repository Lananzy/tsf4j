/*
**  @file $RCSfile: tresource.c,v $
**  general description of this module
**  $Id: tresource.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/ttypes.h"
#include "pal/tresource.h"
#include "pal/terrno.h"

#ifdef WIN32

int setrlimit (int rltype, const struct rlimit *rlimitp)
{
	/* check argument range */
	if (rlimitp->rlim_cur > rlimitp->rlim_max || rlimitp == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	switch (rltype)
	{
	case RLIMIT_CPU:
	case RLIMIT_FSIZE:
	case RLIMIT_DATA:
	case RLIMIT_STACK:
	case RLIMIT_CORE:
	case RLIMIT_RSS:
	case RLIMIT_MEMLOCK:
	case RLIMIT_NPROC:
	case RLIMIT_NOFILE:
		/* not supported */
		errno = EPERM;
		return -1;
	default:
		errno = EINVAL;
		return -1;
	}

	return 0;
}


int getrusage(int _who, struct rusage *_rusage)
{
	clock_t q;
	if (_rusage == 0)
	{
		errno = EFAULT;
		return -1;
	}
	if (_who != RUSAGE_SELF && _who != RUSAGE_CHILDREN)
	{
		errno = EINVAL;
		return -1;
	}
	memset(_rusage, 0, sizeof(struct rusage));

	q = clock();

	_rusage->ru_utime.tv_sec = q / CLOCKS_PER_SEC;
	_rusage->ru_utime.tv_usec = q % CLOCKS_PER_SEC * 1000000 / CLOCKS_PER_SEC;

	return 0;
}

int getrlimit (int rltype, struct rlimit *rlimitp)
{
	/* check argument range */
	if (rltype < 0 || rltype >= RLIM_NLIMITS || rlimitp == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	switch( rltype )
	{
	case RLIMIT_CPU:
	case RLIMIT_FSIZE:
	case RLIMIT_DATA:
	case RLIMIT_CORE:
	case RLIMIT_RSS:
	case RLIMIT_MEMLOCK:
		rlimitp->rlim_max	=	RLIM_INFINITY;
		rlimitp->rlim_cur	=	rlimitp->rlim_max;
		break;

	case RLIMIT_STACK:
		rlimitp->rlim_max	=	RLIM_INFINITY;
		rlimitp->rlim_cur	=	rlimitp->rlim_max;
		break;

	case RLIMIT_NPROC:
		rlimitp->rlim_max	=	RLIM_INFINITY;
		rlimitp->rlim_cur	=	rlimitp->rlim_max;
		break;
	case RLIMIT_NOFILE:
		rlimitp->rlim_max	=	sysconf (_SC_OPEN_MAX);
		rlimitp->rlim_cur	=	rlimitp->rlim_max;
		break;

	default:
		errno	=	EINVAL;
		return -1;
	}

	return 0;
}

#endif /* WIN32 */
