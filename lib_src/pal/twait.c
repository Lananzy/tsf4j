/*
**  @file $RCSfile: twait.c,v $
**  general description of this module
**  $Id: twait.c,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2008/05/28 07:35:00 $
**  @version $Revision: 1.1.1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#include "pal/tos.h"
#include "pal/twait.h"
#include "pal/terrno.h"

#ifdef WIN32

#pragma warning(disable:4996)

pid_t wait(int *a_piStatus)
{
	return waitpid(-1, a_piStatus, 0);
}


pid_t waitpid(pid_t a_iPid, int *a_piStatus, int a_iOptions)
{
	pid_t iPid;
	pid_t iPpid;
	pid_t iWaitPid;
	int iCount;
	int iHandle;
	DWORD dwWaitRet;
	DWORD dwTimeout;
	DWORD dwExitCode=0;
	pid_t aiPidList[TOS_MAX_CHILD];
	HANDLE ahProcList[TOS_MAX_CHILD];


	if( ( a_iOptions & WCONTINUED ) || ( a_iOptions & WSTOPPED ) )
	{
		errno	=	ENOSYS;
		return -1;
	}

	if( a_iOptions & WNOHANG )
		dwTimeout	=	0;
	else
		dwTimeout	=	INFINITE;

	if( -1==a_iPid )
	{
		iPpid	=	getpid();
		iPid	=	0;
	}
	else if( a_iPid<0 )
	{
		iPid	=	0;
		iPpid	=	-a_iPid;
	}
	else if( 0==a_iPid )
	{
		iPpid	=	getpgrp();
		iPid	=	0;
	}
	else
	{
		iPpid	=	0;
		iPid	=	a_iPid;
	}

	iCount	=	tos_get_pidlist_i(TOS_MAX_CHILD, aiPidList, iPid, iPpid);

	if( iCount<0 )
	{
		errno	=	E2BIG;
		return -1;
	}
	else if( 0==iCount )
	{
		errno	=	ESRCH;
		return -1;
	}

	iHandle	=	tos_open_proclist_i(iCount, aiPidList, ahProcList);

	if( iHandle<=0 )
		return -1;

	dwWaitRet	=	WaitForMultipleObjects(iHandle, ahProcList, FALSE, dwTimeout);

	if( WAIT_TIMEOUT==dwWaitRet )
	{
		iWaitPid	=	-1;
	}
	else if( dwWaitRet>=WAIT_OBJECT_0 && dwWaitRet<WAIT_OBJECT_0+iHandle )
	{
		iWaitPid	=	tos_get_process_id_i(ahProcList[dwWaitRet-WAIT_OBJECT_0]);
		GetExitCodeProcess(ahProcList[dwWaitRet-WAIT_OBJECT_0], &dwExitCode);
	}
	else if( dwWaitRet>=WAIT_ABANDONED_0&& dwWaitRet<WAIT_ABANDONED_0+iHandle )
	{
		iWaitPid	=	tos_get_process_id_i(ahProcList[dwWaitRet-WAIT_ABANDONED_0]);
		GetExitCodeProcess(ahProcList[dwWaitRet-WAIT_ABANDONED_0], &dwExitCode);
	}
	else
	{
		iWaitPid	=	-1;
	}

	tos_close_proclist_i(iHandle, ahProcList);

	if( a_piStatus )
		*a_piStatus	=	(int) W_EXITCODE(dwExitCode, 0);

	return iWaitPid;
}

#endif /* WIN32 */

