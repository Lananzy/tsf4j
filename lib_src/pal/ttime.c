/*
**  @file $RCSfile: ttime.c,v $
**  general description of this module
**  $Id: ttime.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/ttime.h"
#include "pal/ttypes.h"
#include "pal/tsignal.h"
#include "pal/terrno.h"

#include <assert.h>


#ifdef WIN32

#pragma warning(disable:4996)


static int __timer_inited = 0;
static long __timer_res = 0;

static __itimer_id[ITIMER_MAX_TIMER] = { 0, 0, 0 };
static struct itimerval __itimer_ival[ITIMER_MAX_TIMER] = 
{ { {0,0},{0,0} },
  { {0,0},{0,0} },
  { {0,0},{0,0} } };

static int __init_timers()
{
	TIMECAPS tc;
	if( !__timer_inited )
	{
		if( timeGetDevCaps(&tc,sizeof(tc)) != TIMERR_NOERROR )
			return 0;

		__timer_res = tc.wPeriodMin;

		if( timeBeginPeriod(__timer_res) != TIMERR_NOERROR )
			return 0;

		__timer_inited = 1;
	}

	return 0;
}

#define TV2MSEC(sec,usec) (sec*1000+(usec+500)/1000)

static void CALLBACK itimer_real_handler(UINT uID,UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if( (int)uID==__itimer_id[ITIMER_REAL] )
		traise(SIGALRM);
	else if( (int)uID==__itimer_id[ITIMER_VIRTUAL] )
		traise(SIGVTALRM);
	else if( (int)uID==__itimer_id[ITIMER_PROF] )
		traise(SIGPROF);
}

int setitimer(int which,/*const*/ struct itimerval *value,struct itimerval *ovalue)
{
	long ival;
	int type = TIME_PERIODIC;
	struct itimerval* itimer_ival;
	int *itimer_id;

	if( which <0 || which>=ITIMER_MAX_TIMER )
	{
		errno = EINVAL;
		return -1;
	}

	itimer_ival	=	__itimer_ival + which;
	itimer_id	=	__itimer_id + which;

	__init_timers();

	if( *itimer_id )
	{
		timeKillEvent( *itimer_id );
		*itimer_id	=	0;
	}

	if( ovalue )
		*ovalue = *itimer_ival;

	*itimer_ival = *value;

	if( !value->it_value.tv_sec && !value->it_value.tv_usec )
		return 0;

	ival = TV2MSEC(value->it_interval.tv_sec,value->it_interval.tv_usec);

	if (!ival)
	{
		ival = TV2MSEC(value->it_value.tv_sec,value->it_value.tv_usec);
		type = TIME_ONESHOT;
	}

	*itimer_id = timeSetEvent(ival,
		0, //maximum accuracy
		&itimer_real_handler,
		0,
		type
		);

	return 0;  
}

int getitimer(int which, struct itimerval *value)
{
	if( which <0 || which>=ITIMER_MAX_TIMER )
	{
		errno = EINVAL;
		return -1;
	}

	*value = __itimer_ival[which];

	return 0;
}

#define MAX_SECONDS_WAIT		((0x7fffffff)/1000)

int nanosleep(const struct timespec *a_pstReq, struct timespec *a_pstRem)
{
	DWORD dwMilliSec;
	DWORD dwStart;
	DWORD dwEnd;
	DWORD dwUsed;

	DWORD dwRet;

	if( a_pstReq->tv_sec>=MAX_SECONDS_WAIT )
	{
		dwMilliSec	=	INFINITE;
	}
	else
	{
		dwMilliSec	=	a_pstReq->tv_sec*1000 + a_pstReq->tv_nsec/1000000;
	}

	dwStart	=	GetTickCount();

	dwRet	=	SleepEx(dwMilliSec, TRUE);

	if( dwRet )
	{
		dwEnd	=	GetTickCount();
		dwUsed	=	dwEnd - dwStart;

		a_pstRem->tv_sec	=	dwUsed / 1000;
		a_pstRem->tv_nsec	=	(dwUsed % 1000) * 1000000;

		errno	=	EINTR;
		return -1;
	}
	else
	{
		a_pstRem->tv_sec	=	0;
		a_pstRem->tv_nsec	=	0;
		return 0;
	}
}

#if _MSC_VER >= 1300
	#define _W32_FT_OFFSET (116444736000000000ULL)
#else /* _MSC_VER >= 1300 */
	#define _W32_FT_OFFSET (116444736000000000ui64)
#endif /* _MSC_VER >= 1300 */

int gettimeofday(struct timeval *a_pstTv, struct timezone *a_pstTz)
{
	FILETIME stFt;
	uint64_t *pullNs100;

	assert( a_pstTv );

	GetSystemTimeAsFileTime (&stFt);

	pullNs100	=	(uint64_t *) &stFt;

#if _MSC_VER >= 1300
	a_pstTv->tv_usec	=	(long)((*pullNs100 / 10ULL) % 1000000ULL );
	a_pstTv->tv_sec		=	(long)((*pullNs100 - _W32_FT_OFFSET) / 10000000ULL);
#else /* _MSC_VER >= 1300 */
	a_pstTv->tv_usec	=	(long)((*pullNs100 / 10ui64) % 1000000ui64 );
	a_pstTv->tv_sec		=	(long)((*pullNs100 - _W32_FT_OFFSET) / 10000000ui64);
#endif /* _MSC_VER >= 1300 */

	return 0;
}

int settimeofday(const struct timeval *a_pstTv , const struct timezone *a_pstTz)
{
	errno = ENOSYS;
	return 0;
}

int utimes(const char *a_pszFileName, const struct timeval a_astTv[2])
{
	struct utimbuf stTimBuf;

	stTimBuf.actime	=	a_astTv[0].tv_sec;
	stTimBuf.modtime=	a_astTv[1].tv_sec;

	return utime(a_pszFileName, &stTimBuf);
}

char* asctime_r(const struct tm *tm, char *buf)
{
	const char* pszResult;

	pszResult	=	asctime(tm);

	if( !pszResult )
		return NULL;

	strcpy(buf, pszResult);

	return buf;
}

char* ctime_r(const time_t *timep, char *buf)
{
	struct tm stTm;

	return asctime_r(localtime_r(timep, &stTm), buf);
}

struct tm* gmtime_r(const time_t *timep, struct tm *result)
{
	struct tm* pstRet;

	pstRet	=	gmtime(timep);

	if( !pstRet )
		return NULL;

	memcpy(result, pstRet, sizeof(*result));

	return result;
}

struct tm* localtime_r(const time_t *timep, struct tm *result)
{
	struct tm* pstRet;

	pstRet	=	localtime(timep);

	if( !pstRet )
		return NULL;

	memcpy(result, pstRet, sizeof(*result));

	return result;
}



#else /* WIN32 */

#endif /* WIN32 */
