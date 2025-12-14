/*
**  @file $RCSfile: texec.c,v $
**  general description of this module
**  $Id: texec.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/terrno.h"
#include "pal/texec.h"

#include <stdarg.h>

int texecl(const char *a_pszPath, const char *a_pszArg, ...)
{
	const char* apszArgs[TEXEC_ARG_MAX];
	const char* pszOneArg;
	va_list ap;
	int i=0;

	pszOneArg		=	a_pszArg;

	apszArgs[i++]	=	pszOneArg;

	va_start(ap, a_pszArg);

	while( pszOneArg && i<TEXEC_ARG_MAX )
	{
		pszOneArg		=	(const char*) va_arg(ap, const char*);
		apszArgs[i++]	=	pszOneArg;
	}

	va_end(ap);

	if( pszOneArg )
	{
		errno	=	E2BIG;
		return -1;
	}

	return texecv(a_pszPath, apszArgs);
}


int texeclp(const char *a_pszFile, const char *a_pszArg, ...)
{
	const char* apszArgs[TEXEC_ARG_MAX];
	const char* pszOneArg;
	va_list ap;
	int i=0;

	pszOneArg		=	a_pszArg;

	apszArgs[i++]	=	pszOneArg;

	va_start(ap, a_pszArg);

	while( pszOneArg && i<TEXEC_ARG_MAX )
	{
		pszOneArg		=	(const char*) va_arg(ap, const char*);
		apszArgs[i++]	=	pszOneArg;
	}

	va_end(ap);

	if( pszOneArg )
	{
		errno	=	E2BIG;
		return -1;
	}

	return texecvp(a_pszFile, apszArgs);
}


int texecle(const char *a_pszPath,  const char *a_pszArg, ...)
{
	const char* apszArgs[TEXEC_ARG_MAX];
	const char* *ppszEnv=NULL;
	const char* pszOneArg;
	va_list ap;
	int i=0;

	pszOneArg		=	a_pszArg;
	apszArgs[i++]	=	pszOneArg;

	va_start(ap, a_pszArg);

	while( pszOneArg && i<TEXEC_ARG_MAX )
	{
		pszOneArg		=	(const char*) va_arg(ap, const char*);
		apszArgs[i++]	=	pszOneArg;
	}

	if( NULL==pszOneArg )
		ppszEnv	=	(const char**) va_arg(ap, const char**);
	else
		ppszEnv	=	NULL;

	va_end(ap);

	if( pszOneArg )
	{
		errno	=	E2BIG;
		return -1;
	}

	return texecve(a_pszPath, apszArgs, ppszEnv);
}


int texecv(const char *a_pszPath, const char *a_apszArgv[])
{
#ifdef WIN32
	HANDLE hProc;
	pid_t iChildPid;

	hProc	=	(HANDLE) _spawnv(_P_NOWAIT, a_pszPath, a_apszArgv);

	if( INVALID_HANDLE_VALUE==hProc )
		return -1;

	iChildPid	=	(pid_t) tos_get_process_id_i(hProc);

	CloseHandle(hProc);

	return iChildPid;

#else /* WIN32 */
	int iRet;

	iRet	=	fork();

	if( iRet<0 )
		return -1;

	if( iRet>0 )
		return iRet;

	execv(a_pszPath, (char* const*)a_apszArgv);

	return 0;

#endif /* WIN32 */
}


int texecvp(const char *a_pszFile, const char *a_apszArgv[])
{
#ifdef WIN32
	HANDLE hProc;
	pid_t iChildPid;

	hProc	=	(HANDLE) _spawnvp(_P_NOWAIT, a_pszFile, a_apszArgv);

	if( INVALID_HANDLE_VALUE==hProc )
		return -1;

	iChildPid	=	(pid_t) tos_get_process_id_i(hProc);

	CloseHandle(hProc);

	return iChildPid;

#else /* WIN32 */
	int iRet;

	iRet	=	fork();

	if( iRet<0 )
		return -1;

	if( iRet>0 )
		return iRet;

	execvp(a_pszFile, (char* const*)a_apszArgv);

	return 0;

#endif /* WIN32 */
}

int texecve(const char *a_pszPath, const char *a_apszArgv[], const char *a_apszEnvp[])
{
#ifdef WIN32
	HANDLE hProc;
	pid_t iChildPid;

	hProc	=	(HANDLE) _spawnve(_P_NOWAIT, a_pszPath, a_apszArgv, a_apszEnvp);

	if( INVALID_HANDLE_VALUE==hProc )
		return -1;

	iChildPid	=	(pid_t) tos_get_process_id_i(hProc);

	CloseHandle(hProc);

	return iChildPid;

#else /* WIN32 */
	int iRet;

	iRet	=	fork();

	if( iRet<0 )
		return -1;

	if( iRet>0 )
		return iRet;

	execve(a_pszPath, (char* const*)a_apszArgv, (char* const*)a_apszEnvp);

	return 0;

#endif /* WIN32 */
}


