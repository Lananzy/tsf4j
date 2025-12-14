/*
**  @file $RCSfile: fcntl.c,v $
**  general description of this module
**  $Id: fcntl.c,v 1.2 2008/11/12 04:07:08 jackyai Exp $
**  @author $Author: jackyai $
**  @date $Date: 2008/11/12 04:07:08 $
**  @version $Revision: 1.2 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifdef WIN32

#include "pal/tos.h"
#include "pal/tfcntl.h"
#include "pal/ttypes.h"
#include "pal/tstdio.h"
#include "pal/terrno.h"
#include <stdarg.h>

int fcntl(int fd, int cmd, ...)
{
	int arg;
	va_list ap;
	HANDLE hProc=NULL;
	HANDLE hOld=NULL;
	HANDLE hNew=NULL;
	int iRet=0;
	DWORD dwFlags=0;

	hOld	=	(HANDLE) _get_osfhandle(fd);

	if( INVALID_HANDLE_VALUE==hOld )
	{
		errno	=	EBADF;
		return -1;
	}

	switch (cmd)
	{
	case F_DUPFD:
		va_start(ap, cmd);
		arg = va_arg(ap, int);
		va_end(ap);

		hProc	=	GetCurrentProcess();

		if( !DuplicateHandle(hProc, hOld, hProc, (HANDLE*)&hNew, 
				0, TRUE, DUPLICATE_SAME_ACCESS) )
		{
			errno	=	EMFILE;
			return -1;
		}

		iRet	=	_open_osfhandle((intptr_t)hNew, 0);

		if( iRet<0 )
		{
			CloseHandle(hNew);
			hNew	=	NULL;

			return -1;
		}

    case F_GETFD:

		if( !GetHandleInformation(hOld, &dwFlags) )
		{
			errno	=	EPERM;
			return -1;
		}

		return ( dwFlags & HANDLE_FLAG_INHERIT ) ? 0 : FD_CLOEXEC;

    case F_SETFD:
		va_start(ap, cmd);
		arg = va_arg(ap, int);
		va_end(ap);

		if( arg & FD_CLOEXEC )
			dwFlags	=	0;
		else
			dwFlags	=	HANDLE_FLAG_INHERIT;

		if( !SetHandleInformation(hOld, HANDLE_FLAG_INHERIT, dwFlags) )
		{
			errno	=	EPERM;
			return -1;
		}

		return 0;

    case F_GETFL:
		return 0;

    case F_SETFL:
		return 0;

    case F_GETLK:
		errno=ENOSYS;
		return -1;

    case F_SETLK:
		errno=ENOSYS;
		return -1;

    case F_SETLKW:
		errno=ENOSYS;
		return -1;
	default:
		break;
    }

	errno=ENOSYS;

	return -1;
}

#endif /* WIN32 */
